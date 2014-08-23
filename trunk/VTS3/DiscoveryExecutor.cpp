// DiscoveryExecutor.h  - Device and object discovery and EPICS generation
// John Hartman 19 August 2014
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vts.h"
#include "VTSDoc.h"
#include "BACnet.hpp"
#include "VTSDiscoveryDlg.h"

#include "DiscoveryExecutor.h"
#include "PI.h"

namespace PICS {
#include "db.h"
#include "service.h"
#include "stdobjpr.h"
#include "vtsapi.h"
#include "props.h"
#include "bacprim.h"
#include "dudapi.h"
#include "dudtool.h"
#include "propid.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// TODO: yes, we SHOULD have a full set of error enumeration values in some .h file.
// Go right ahead.  I'll wait here
#define ERR_CLASS_PROPERTY          2
#define ERR_CODE_READ_ACCESS_DENIED 27
#define ERR_CODE_UNKNOWN_PROPERTY   32

// global defines
DiscoveryExecutor gDiscoveryExecutor;

//=============================================================================
// Construction/Destruction
//=============================================================================
DiscoveryExecutor::DiscoveryExecutor()
: SendReceiveExecutor("Discovery and EPICS Generation")
, m_funToExe(FN_IDLE)
, m_fileName()
, m_pFile(NULL)
, m_protocolRevision(0)
, m_discoveredDevices()
{
}

//=============================================================================
DiscoveryExecutor::~DiscoveryExecutor()
{
   if (m_pFile != NULL)
   {
      fclose(m_pFile);
   }

   ClearDiscoveryList();
}

//=============================================================================
void DiscoveryExecutor::ExecuteTest( FunctionToExecute theFunction )
{
   CSingleLock lock(&m_cs);
   lock.Lock();

   if (m_execState != execIdle)
   {
      TRACE0( "Error: invalid executor state\n" );
      lock.Unlock();
      return;
   }

   // Set the function to be performed
   m_funToExe = theFunction;

   VTSDocPtr pVTSDoc = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();
   VTSPorts* pPorts = pVTSDoc->GetPorts();
   VTSNames* pNames = pVTSDoc->GetNames();

   VTSDiscoveryDlg dlg( (m_funToExe == FN_CREATE_EPICS), *pNames, *pPorts);
   if (dlg.DoModal() == IDOK)
   {
      for (int i = 0; i < pPorts->GetSize(); i++)
      {
         if ( ((VTSPort *) pPorts->GetAt(i))->m_strName.CompareNoCase(dlg.m_strPort) == 0 )
         {
            m_pPort = (VTSPort *) pPorts->GetAt(i);
            break;
         }
      }

      m_pName = pNames->GetAt( pNames->FindIndex(dlg.m_strDevice) );
      m_IUTAddr = m_pName->m_bacnetaddr;

      if (dlg.m_strDevObjInst.IsEmpty())
      {
         // Not specified
         m_nDeviceObjInst = (UINT)~0;
      }
      else
      {
         m_nDeviceObjInst = (UINT)atoi( dlg.m_strDevObjInst );
      }

      m_fileName = dlg.m_strFileName;

      m_includeAllValues = (dlg.m_includeAllValues != 0);
   }
   else
   {
      return;
   }

   StartTest();
   lock.Unlock();
}

//=============================================================================
void DiscoveryExecutor::ProcessTask()
{
   CSingleLock lock(&m_cs);
   lock.Lock();

   BOOL bSuccess = TRUE;
   try
   {
      if (!m_pPort)
      {
         throw("VTSPort or IUT not specified");
      }

      m_pOutputDlg->BeginTestProcess();
      m_execState = execRunning;

      FindRouterAddress();

      if (m_funToExe == FN_DISCOVER_DEVICES)
      {
         // Do Discovery
         DoDiscovery();
      }
      else
      {
         if ((m_IUTAddr.addrType != localStationAddr) && (m_IUTAddr.addrType != remoteStationAddr))
         {
            throw("The IUT's address must be a local or remote station");
         }

         // Create an EPICS
         if (!m_fileName.IsEmpty())
         {
            OpenEpicsFile();
         }

         if (m_nDeviceObjInst == (UINT)~0)
         {
            FindDeviceInstance();
         }

         CString msg;
         msg.Format( "Reading objects from (Device, %u)", m_nDeviceObjInst );
         m_pOutputDlg->OutMessage( msg );

         if (m_pFile)
         {
            msg.Format( "Generating EPICS file %s", (LPCTSTR)m_fileName );
            m_pOutputDlg->OutMessage( msg );
         }

         ReadObjects();
      }
   }
   catch (const char *errMsg)
   {
      bSuccess = FALSE;
      Msg(errMsg);
   }
   catch (...)
   {
      bSuccess = FALSE;
      Msg("Decoding error! Maybe received uncorrect packet!");
   }

   m_execState = execIdle;
   if (bSuccess)
   {
      m_pOutputDlg->OutMessage("Operation completed successfully");
      Msg("Operation completed successfully");
   }
   else if (m_bUserCancelled)
   {
      m_bUserCancelled = false;  // reset
      m_pOutputDlg->OutMessage("", TRUE);    //begin a new line
      m_pOutputDlg->OutMessage("Canceled by user");
   }
   else
   {
      m_pOutputDlg->OutMessage("Failed",TRUE);  // begin a new line
      m_pOutputDlg->OutMessage("Error occured during the operation");
   }

   m_pOutputDlg->EndTestProcess();

   if (m_pFile != NULL)
   {
      fclose(m_pFile);
      m_pFile = NULL;
   }

   lock.Unlock();
}

//=============================================================================
// Find the device instance at the specified address
void DiscoveryExecutor::FindDeviceInstance()
{
   if (m_nDeviceObjInst == (UINT)~0)
   {
      try
      {
         // Try to find a device object using a Read of the wildcard device instance.
         // Unicast requires no BBMD on IP, but device must be new enough to handle
         // wildcard instance
         BACnetObjectIdentifier wildID( OBJ_DEVICE, 0x003FFFFF );
         BACnetEnumerated propID;
         propID.m_enumValue = PICS::OBJECT_IDENTIFIER;
         AnyValue propValue;
         BACnetObjectIdentifier deviceID;
         propValue.SetObject( &deviceID );
         if (SendExpectReadProperty( wildID, propID, propValue ))
         {
            m_nDeviceObjInst = deviceID.objID & 0x003FFFFF;
         }
      }
      catch (...)
      {
         m_pOutputDlg->OutMessage("Failed to find Device Instance by reading (Device, 0x003FFFFF)");
      }
   }

   if (m_nDeviceObjInst == (UINT)~0)
   {
      try
      {
         // Read failed, perhaps because the device is an old one that doesn't handle
         // wildcard instance.  Try unicast Who-Is.
         BACnetObjectIdentifier deviceID;
         BACnetUnsigned         maxAPDULen;
         if (SendExpectWhoIs( deviceID, maxAPDULen ))
         {
            m_nDeviceObjInst = deviceID.objID & 0x003FFFFF;
         }
      }
      catch (...)
      {
         m_pOutputDlg->OutMessage("Failed to find Device Instance by sending Who-Is");
         throw;
      }
   }
}

//=============================================================================
// Read and dump the properties of all objects in the current device
bool DiscoveryExecutor::ReadObjects()
{
   // Read Device's protocol revision
   ReadProtocolRevision();

   // Get the Device's list of objects
   // Throws failure
   BACnetArrayOf<BACnetObjectIdentifier> objList;
   GetObjectList( objList );

   // Dump the device object first
   BACnetObjectIdentifier obj( OBJ_DEVICE, m_nDeviceObjInst );
   ReadObjectProperties( obj );

   int nCount = objList.GetItemCount();
   for (int ix = 0; !m_bUserCancelled && (ix < nCount); ix++)
   {
      UINT nObjectType = objList.GetElement(ix)->GetObjectType();
      UINT nInstanceNum = objList.GetElement(ix)->GetObjectInstance();
      if ((nObjectType != OBJ_DEVICE) && (nInstanceNum != m_nDeviceObjInst))
      {
         BACnetObjectIdentifier obj( nObjectType, nInstanceNum );
         ReadObjectProperties( obj );
      }
   }

   if (m_pFile)
   {
      fprintf( m_pFile,
               "}\n"
               "End of BACnet Protocol Implementation Conformance Statement\n"
             );
   }
   return true;
}

//=============================================================================
// Read protocol-revision from the current device
bool DiscoveryExecutor::ReadProtocolRevision()
{
   BACnetObjectIdentifier devID( OBJ_DEVICE, m_nDeviceObjInst );
   BACnetEnumerated propID;
   AnyValue propValue;

   propID.m_enumValue = PICS::PROTOCOL_REVISION;
   BACnetUnsigned rev;
   propValue.SetObject( &rev );
   if (SendReadPropertyOptional( devID, propID, propValue ))
   {
      m_protocolRevision = rev.uintValue;
   }

   return true;
}

//=============================================================================
// Read and dump the properties of the specified object in the current device
bool DiscoveryExecutor::ReadObjectProperties( BACnetObjectIdentifier &objID )
{
   CString str, msg;
   objID.Encode(str, BACnetEncodeable::FMT_EPICS );

   msg.Format( "Reading %s", (LPCTSTR)str );
   m_pOutputDlg->OutMessage( msg );

   if (m_pFile)
   {
      fprintf( m_pFile, "  {\n" );
   }

   const PICS::propdescriptor *pd = PICS::GetPropDescriptorTable( objID.objID >> 22 );
   if (pd)
   {
      BACnetEnumerated propID;
      while (!m_bUserCancelled)
      {
         const char *propName = NetworkSniffer::BAC_STRTAB_BACnetPropertyIdentifier.EnumString( pd->PropID );

         bool required = (pd->PropFlags & R) && (m_protocolRevision >= pd->firstRevision);

         propID.m_enumValue = pd->PropID;
         DiscoAnyValue propValue( pd->ParseType, pd->PropID );

         try
         {
            if (SendExpectReadProperty( objID, propID, propValue ))
            {
               if (!m_includeAllValues && (pd->PropFlags & QVAL))
               {
                  // Show a "?" for this oft-changing value
                  str = "?";
               }
               else
               {
                  // Show the value
                  propValue.Encode( str, BACnetEncodeable::FMT_EPICS );
               }

               if (m_pFile)
               {
                  fprintf( m_pFile, "    %s: %s\n", propName, (LPCTSTR)str );
               }
               else
               {
                  msg.Format( "  %s: %s", propName, (LPCTSTR)str );
                  m_pOutputDlg->OutMessage( msg );
               }
            }
            else
            {
               ShowPropertyError( "Communications error while trying to read", propName );
            }
         }
         catch (...)
         {
            // LOG_BUFFER can't be read by ReadProperty,
            // so turn (property, read-access-denied) into a value of "?"
            if ((m_errorClass == ERR_CLASS_PROPERTY) && (m_errorCode == ERR_CODE_READ_ACCESS_DENIED))
            {
               if (m_pFile)
               {
                  fprintf( m_pFile, "    %s: ?\n", propName );
               }
            }
            else if (required)
            {
               ShowPropertyError( "Failed(2) to read required property", propName );
            }
            else if ((m_errorClass != ERR_CLASS_PROPERTY) || (m_errorCode != ERR_CODE_UNKNOWN_PROPERTY))
            {
               // Complain about any error other than the expected
               // (property, unknown-property) for unsupported optional properties
               ShowPropertyError( "Failed(2) to read", propName );
            }
         }

         if (pd->PropGroup & LAST)
         {
            break;
         }
         pd++;
      }
   }

   if (m_pFile)
   {
      fprintf( m_pFile, "  }\n" );
   }

   return true;
}

//=============================================================================
// Display an error about the specified property
void DiscoveryExecutor::ShowPropertyError( const char *pTitle, const char *pPropName )
{
   CString errType;
   ErrorString(errType);

   CString msg;
   msg.Format( "  %s %s  %s", pTitle, pPropName, (LPCTSTR)errType );
   m_pOutputDlg->OutMessage( msg );

   if (m_pFile)
   {
      fprintf( m_pFile, "--  %s: TODO: %s  %s\n", pPropName, pTitle, (LPCTSTR)errType );
   }
}

//=============================================================================
// Open or create an EPICS file
void DiscoveryExecutor::OpenEpicsFile()
{
   // Attempt to rename any existing file as a backup
   CString backupName = m_fileName + ".bak";
   remove( backupName );
   int result = rename( m_fileName, backupName );

   // Create the new file
   m_pFile = fopen( m_fileName, "w" );
   if (m_pFile)
   {
      if (result == 0)
      {
         // Rename succeeded.  Copy old content
         m_pOutputDlg->OutMessage( "Updating existing EPICS file" );

         CStdioFile oldFile( backupName, CFile::modeRead );
         CString line;
         while (oldFile.ReadString(line))
         {
            // Copy the line to the output file
            fprintf( m_pFile, "%s\n", (LPCTSTR)line );

            if (line.Find("List of Objects in Test Device:") >= 0)
            {
               // Stop copying
               break;
            }
         }

         fprintf( m_pFile, "{\n" );
      }
      else
      {
         // Start a new file
         m_pOutputDlg->OutMessage( "Creating new EPICS file" );

         fprintf( m_pFile,
            "PICS 0\n"
            "BACnet Protocol Implementation Conformance Statement\n"
            "\n"
            "Vendor Name: \"TODO\"\n"
            "Product Name: \"TODO\"\n"
            "Product Model Number: \"TODO\"\n"
            "Product Description: \"TODO\"\n"
            "\n"
            "-- TODO: you need to fill in a lot of stuff here.\n"
            "-- Take a look at 135.1 Annex A, or allTestEpics.txt in the VTS Docs directory.\n"
            "-- Next time you specify this file during EPICS creation,\n"
            "-- everything before the list of objects will be preserved.\n"
            "\n"
            "List of Objects in Test Device:\n"
            "{\n"
             );
      }
   }
}

//=============================================================================
// BACnetAnyValue that sets it own type
DiscoveryExecutor::DiscoAnyValue::DiscoAnyValue( UINT a_parseType, UINT a_propertyID )
: m_parseType(a_parseType)
, m_propertyID(a_propertyID)
{
}

//=============================================================================
// virtual
DiscoveryExecutor::DiscoAnyValue::~DiscoAnyValue()
{
}

//=============================================================================
// virtual
void DiscoveryExecutor::DiscoAnyValue::Decode( BACnetAPDUDecoder& dec )
{
   // Make an object to receive the value and decode into it
   BACnetEncodeable *pEnc = BACnetEncodeable::Factory( m_parseType, dec, m_propertyID );
   if (pEnc == NULL)
   {
      throw("Can't make an encodeable object of this type");
   }
   // This is our value
   SetObject( pEnc );
}

//=============================================================================
// Perform discovery using Who-Is
void DiscoveryExecutor::DoDiscovery()
{
   CString str;
   int nSeen = 0;

   ClearDiscoveryList();

   // Sending just a full-range Who-Is may miss some responses if there are a 
   // large number of devices, due to buffer overflows and hardware limitations.
   // Thus, we send a an initial full-range Who-Is, follwed by additional
   // Who-Is for the the empty ranges between known Devices.
   int low  = 0 - 1;
   int high = 0x3FFFFF + 1;
   do
   {
      int savedHigh;
      do
      {
         if (low + 1 < high)
         {
            str.Format( "Sending Who Is for the range %d to %d", low + 1, high - 1 );
            m_pOutputDlg->OutMessage(str);
            SendWhoIs( low + 1, high - 1 );
            UINT lastDeviceTime = GetTickCount();

            // Wait for any replies
            UINT now;
            do
            {
               Sleep( 250 );
               now = GetTickCount();
               int nDevices = m_discoveredDevices.GetCount();
               if (nDevices > nSeen)
               {
                  // One or more new devices.  Reset time
                  lastDeviceTime = now;

                  str.Format( "Discovered %d devices", nDevices - nSeen );
                  m_pOutputDlg->OutMessage(str);
                  nSeen = nDevices;
               }
            }
            while (!m_bUserCancelled && (now - lastDeviceTime < 3000));
         }

         // Set high limit to any intermediate device discovered after
         // the current low limit.
         savedHigh = high;
         high = InstanceAfter( low );
      }
      while (!m_bUserCancelled && (high < savedHigh)); // until we stop finding new ones

      // Advance to next range in device table
      low = high;
      high = InstanceAfter( low );
   }
   while (!m_bUserCancelled && (low < high));

   // Dump the device list.
   for (int ix = 0; ix < m_discoveredDevices.GetCount(); ix++)
   {
      const DiscoveryInfo *pElem  = m_discoveredDevices.GetAt( ix );
      str.Format( "Device %u at %s.  Vendor: %s.  maxAPDU:%u.  %s",
                  pElem->m_instance,
                  (LPCTSTR)pElem->m_address.PrettyAddress( m_pPort ),
                  NetworkSniffer::BAC_STRTAB_BACnetVendorID.EnumString( pElem->m_vendorID ),
                  pElem->m_mxaAPDU,
                  NetworkSniffer::BAC_STRTAB_BACnetSegmentation.EnumString( pElem->m_segmentationSupport )
                );
      m_pOutputDlg->OutMessage( str );
      Msg( str );

      // Read and dump various interesting properties about this device
      m_nDeviceObjInst = pElem->m_instance;
      m_IUTAddr = pElem->m_address;
      FindRouterAddress();
      DumpDeviceProperties();
   }
}

//=============================================================================
// Clear the list of discovered devices
void DiscoveryExecutor::ClearDiscoveryList()
{
   for (int ix = 0; ix < m_discoveredDevices.GetCount(); ix++)
   {
      delete m_discoveredDevices.GetAt( ix );
   }

   m_discoveredDevices.RemoveAll();
}

//=============================================================================
// Send a Who-Is, with or without a Device Instance range
void DiscoveryExecutor::SendWhoIs( int theLowInstance, int theHighInstance )
{
   m_bExpectAPDU = true;
   m_nExpectAPDUType = unconfirmedRequestPDU;
   m_nExpectAPDUServiceChoice = iAm;

   // encode the packet
   CByteArray contents;
   contents.Add((BYTE)0x10);
   contents.Add((BYTE)whoIs);

   if (theLowInstance >= 0)
   {
      BACnetAPDUEncoder enc;
      BACnetUnsigned val( (UINT)theLowInstance );
      val.Encode(enc, 0);
      val.uintValue = (UINT)theHighInstance;
      val.Encode(enc, 1);

      // copy the encoding into the byte array
      for (int i = 0; i < enc.pktLength; i++)
      {
         contents.Add( enc.pktBuffer[i] );
      }
   }

   SendUnconfirmed(contents, true);
}

//=============================================================================
// Return the next device instance in the table after theInstance,
// or 0x3FFFFF+1 if there are no more entries.
int DiscoveryExecutor::InstanceAfter( int theInstance )
{
   int nextInstance = 0x3FFFFF + 1;
   for (int ix = 0; ix < m_discoveredDevices.GetCount(); ix++)
   {
      int instance = m_discoveredDevices.GetAt( ix )->m_instance;
      if ((instance > theInstance) && (instance < nextInstance))
      {
         nextInstance = instance;
         // The table is sorted, so this is the next instance
         break;
      }
   }

   return nextInstance;
}

//=============================================================================
// virtual
// Process an APDU that matches our requested APDU type
void DiscoveryExecutor::ReceiveAPDU( const BACnetAPDU &theAPDU )
{
   if ((m_funToExe == FN_DISCOVER_DEVICES) && (theAPDU.apduService == iAm))
   {
      // Got an I-Am while discovering devices
      BACnetAPDUDecoder dec(theAPDU.pktBuffer, theAPDU.pktLength);

      // Decode the I-Am
      BACnetObjectIdentifier id(dec);
      BACnetUnsigned maxAPDU(dec);
      BACnetEnumerated segSupport(dec);
      BACnetUnsigned vendor(dec);

      DiscoveryInfo *pInfo = new DiscoveryInfo;
      pInfo->m_instance = id.objID & 0x3FFFFF;
      pInfo->m_address = theAPDU.apduAddr;
      pInfo->m_mxaAPDU = maxAPDU.uintValue;
      pInfo->m_segmentationSupport = segSupport.m_enumValue;
      pInfo->m_vendorID = vendor.uintValue;

      // Add the data to the discovery list, sorting by device instance
      for (int ix = 0; ix < m_discoveredDevices.GetCount(); ix++)
      {
         const DiscoveryInfo *pElem  = m_discoveredDevices.GetAt( ix );
         if (pInfo->m_instance < pElem->m_instance)
         {
            // New element comes BEFORE this one
            m_discoveredDevices.InsertAt(ix, pInfo);
            pInfo = NULL;
            break;
         }
         else if (pInfo->m_instance == pElem->m_instance)
         {
            // New element has the SAME instance as this one.
            if (pInfo->m_address == pElem->m_address)
            {
               // Exact duplicate: ignore it
               delete pInfo;
               pInfo = NULL;
               break;
            }

            // New element indicates a non-unique Device Instance.
            // Continue loop, to store it after the first-found
         }
      }

      if (pInfo != NULL)
      {
         // List is empty, or new element comes after all existing elements.
         m_discoveredDevices.Add(pInfo);
      }
   }
   else
   {
      // Default processing
      SendReceiveExecutor::ReceiveAPDU( theAPDU );
   }
}

//=============================================================================
// Read and show various properties of the device specified by m_nDeviceObjInst and m_IUTAddr
void DiscoveryExecutor::DumpDeviceProperties()
{
   CString msg, str;

   BACnetObjectIdentifier devID( OBJ_DEVICE, m_nDeviceObjInst );
   BACnetEnumerated propID;

   AnyValue propValue;
   BACnetCharacterString bnString;
   propValue.SetObject( &bnString );

   propID.m_enumValue = PICS::OBJECT_NAME;
   if (SendReadPropertyOptional( devID, propID, propValue ))
   {
      bnString.Encode(str, BACnetEncodeable::FMT_EPICS);
      msg.Format( "  object-name: %s", (LPCTSTR)str );
      m_pOutputDlg->OutMessage(msg);
   }

   propID.m_enumValue = PICS::MODEL_NAME;
   if (SendReadPropertyOptional( devID, propID, propValue ))
   {
      bnString.Encode(str, BACnetEncodeable::FMT_EPICS);
      msg.Format( "  model-name: %s", (LPCTSTR)str );
      m_pOutputDlg->OutMessage(msg);
   }

   propID.m_enumValue = PICS::APPLICATION_SOFTWARE_VERSION;
   if (SendReadPropertyOptional( devID, propID, propValue ))
   {
      bnString.Encode(str, BACnetEncodeable::FMT_EPICS);
      msg.Format( "  application-software-version: %s", (LPCTSTR)str );
      m_pOutputDlg->OutMessage(msg);
   }

   propID.m_enumValue = PICS::PROTOCOL_REVISION;
   BACnetUnsigned rev;
   propValue.SetObject( &rev );
   if (SendReadPropertyOptional( devID, propID, propValue ))
   {
      msg.Format( "  protocol-revision: %u", rev.uintValue );
      m_pOutputDlg->OutMessage(msg);
   }
}