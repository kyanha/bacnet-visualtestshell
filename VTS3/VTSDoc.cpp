// VTSDoc.cpp : implementation of the VTSDoc class
//

#include "stdafx.h"
#include <afxmt.h>

#include "VTS.h"

#include "VTSDoc.h"
#include "VTSValue.h"
#include "VTSPortDlg.h"
#include "VTSPortIPDialog.h"
#include "VTSPortEthernetDialog.h"
#include "VTSNamesDlg.h"
#include "VTSDevicesDlg.h"
#include "FrameContext.h"

#include "Send.h"
#include "SendPage.h"

#include "ScriptExecutor.h"

#include "VTSStatisticsCollector.h"
#include "VTSStatisticsDlg.h"

//Xiao Shiyuan 2002-9-23
#include "VTSWinPTPPort.h"
#include "VTSPortPTPDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define NT_DebugMemState	0

/////////////////////////////////////////////////////////////////////////////
// VTSDoc

IMPLEMENT_DYNCREATE(VTSDoc, CDocument)

BEGIN_MESSAGE_MAP(VTSDoc, CDocument)
	//{{AFX_MSG_MAP(VTSDoc)
	ON_COMMAND(ID_VIEW_STATISTICS, OnViewStatistics)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSDoc construction/destruction

#if NT_DebugMemState
_CrtMemState		s1, s2, s3;
#endif

//create the statistics dialog object
//VTSStatisticsDlg	dlg(NULL);

VTSDoc::VTSDoc()
	: m_PacketCount(0), m_pDB(0), m_FrameContexts(0)
	, m_pPortDlg(0), m_postMessages(false),m_pStatitiscsDlg(0)
{
#if NT_DebugMemState
	_CrtMemCheckpoint( &s1 );
#endif
	m_bStatisticsDlgInUse=false;
	m_pStatisticsCollector=NULL;
//	m_pStatitiscsDlg=&dlg;
//	m_pStatitiscsDlg->m_pDoc=this;
}

//
//	VTSDoc::~VTSDoc
//
//	The only responsibility of the destructor is cleaning up after itself.
//	When a new document is canceled, the document object is destroyed before 
//	the clients, so they really don't know their context has gone away.
//

VTSDoc::~VTSDoc()
{
	while (m_FrameContexts)
		UnbindFrameContext( m_FrameContexts );

#if NT_DebugMemState
	_CrtMemCheckpoint( &s2 );
	_CrtMemDumpAllObjectsSince( &s1 );
	_CrtMemDifference( &s3, &s1, &s2 );
	_CrtMemDumpStatistics( &s3 );
#endif
}

//
//	VTSDoc::OnNewDocument
//

BOOL VTSDoc::OnNewDocument()
{
	// be nice to the base class, but it is not used
	if (!CDocument::OnNewDocument())
		return FALSE;

	CFileDialog	fd( FALSE, "vdb", "Untitled", OFN_OVERWRITEPROMPT, "VTS Session Database (*.vdb)|*.vdb||" );
	if (fd.DoModal() != IDOK)
		return FALSE;

	// create a new database object and initialize it
	m_pDB = new VTSDB();
	m_pDB->NewFile( fd.GetPathName().GetBuffer(0) );
	
	// it should be empty!
	SetPacketCount( m_pDB->GetPacketCount() );
	
	// bind to the (empty) device list
	m_Devices.Load( this );

	// bind to the name list
	m_Names.Load( this );

	// bind to the (empty) port list
	m_Ports.Load( this );

	// associate with the global list of documents
	gDocList.Append( this );

	// ready for messages
	m_postMessages = true;

	// create a new statistics collector
	m_pStatisticsCollector=new VTSStatisticsCollector();
	//gStatisticsCollector=new VTSStatisticsCollector();
	
	return TRUE;
}

//
//	VTSDoc::OnOpenDocument
//

BOOL VTSDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// be nice to the base class, but it is not used
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// create a new statistics collector
	m_pStatisticsCollector=new VTSStatisticsCollector();
//	gStatisticsCollector=new VTSStatisticsCollector();

	m_pDB = new VTSDB();
	try {
		m_pDB->OpenFile( (char *)lpszPathName );
	}
	catch (char *errMsg) {
		AfxMessageBox( errMsg );
		delete m_pDB;
		m_pDB = 0;
		return FALSE;
	}


	// bind to the port list and open the ports
	m_Devices.Load( this );

	// bind to the name list
	m_Names.Load( this );

	// bind to the port list and open the ports
	m_Ports.Load( this );

	// associate with the global list of documents
	gDocList.Append( this );

	// ready for messages
	m_postMessages = true;

	// get the packet count
	SetPacketCount( m_pDB->GetPacketCount() );

	//get the statistics from the loading db file
	for(int i=0;i<m_pDB->GetPacketCount();i++)
	{
		VTSPacket	pkt;
		m_pDB->ReadPacket(i,pkt);
		BACnetPIInfo	summary( true, false );
		summary.Interpret( (BACnetPIInfo::ProtocolType)pkt.packetHdr.packetProtocolID
			, (char *)pkt.packetData
			, pkt.packetLen);
		m_pStatisticsCollector->Update(summary.summaryLine,pkt.packetLen,pkt.packetHdr.packetType,pkt.packetHdr.packetProtocolID);
	//	gStatisticsCollector->Update(summary.summaryLine,pkt.packetLen,pkt.packetHdr.packetType,pkt.packetHdr.packetProtocolID);
	}
	

	return TRUE;
}

//
//	VTSDoc::OnCloseDocument
//

void VTSDoc::OnCloseDocument() 
{
	// delete the statistics collector
	if (m_pStatisticsCollector)
		delete m_pStatisticsCollector;

	// no more messages please
	m_postMessages = false;

	// if the executor is associated with this document, kill it
	if (gExecutor.IsBound(this))
		gExecutor.Kill();

	// unload the port list
	m_Ports.Unload();

	// unload the device list
	m_Devices.Unload();

	// disassociate with the global list of documents
	gDocList.Remove( this );

	// close the database
	if (m_pDB) {
		m_pDB->CloseFile();
		delete m_pDB;
	}

	// pass along to the framework
	CDocument::OnCloseDocument();


//	delete gStatisticsCollector;
}

//
//	VTSDoc::DoPortsDialog
//

void VTSDoc::DoPortsDialog( void )
{
	VTSPortDlg		dlg( &m_Ports, &m_Devices )
	;

	m_pPortDlg = &dlg;
	dlg.DoModal();
	m_pPortDlg = 0;
}

//
//	VTSDoc::DoNamesDialog
//

void VTSDoc::DoNamesDialog( void )
{
	VTSNamesDlg		dlg( &m_Names, &m_Ports )
	;

	dlg.DoModal();
}

//
//	VTSDoc::DoDevicesDialog
//

void VTSDoc::DoDevicesDialog( void )
{
	VTSDevicesDlg		dlg( &m_Devices )
	;

	dlg.DoModal();
}

//
//	VTSDoc::DoPreferencesDialog
//

void VTSDoc::DoPreferencesDialog( void )
{
}

//
//	VTSDoc::PortStatusChange
//

void VTSDoc::PortStatusChange( void )
{
	if (m_pPortDlg)
		m_pPortDlg->PortStatusChange();
}

//
//	VTSDoc::DoSendWindow
//

extern int gSelectedGroup, gSelectedItem;

void VTSDoc::DoSendWindow( int iGroup, int iItem )
{
	CSend		*sendp = new CSend( "Send" )
	;
	
	// pass the group and item through globals
	gSelectedGroup = iGroup;
	gSelectedItem = iItem;

	// create the window and show it
	//sendp->Create( AfxGetApp()->m_pMainWnd );
	sendp->Create(); //Make send window a client window, Xiao Shiyuan 2002-10-24
	sendp->ShowWindow( SW_SHOW );
	
	// reset the selection
	gSelectedGroup = -1;
	gSelectedItem = -1;
}

/////////////////////////////////////////////////////////////////////////////
// VTSDoc serialization

void VTSDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// VTSDoc diagnostics

#ifdef _DEBUG
void VTSDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void VTSDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// VTSDoc commands

//
//	VTSDoc::BindFrameContext
//
//	Binding a frame context to a document is the mechanism that changes to a
//	document can be forwarded to all of the views.
//

void VTSDoc::BindFrameContext( CFrameContext *pfc )
{
	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();

	// let it know its bound to this document
	pfc->m_pDoc = this;

	// add it to the list of other frame contexts
	pfc->m_NextFrame = m_FrameContexts;
	m_FrameContexts = pfc;

	// initialize the frame count, such as it is.  Note that this bind function
	// might be called before the document is properly initialized because the 
	// clients are created before OnNewDocument() or OnOpenDocument() is called.
	pfc->m_PacketCount = m_PacketCount;

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}

//
//	VTSDoc::UnbindFrameContext
//
//	Remove a frame context from the documents list of contexts.  This is called 
//	when a view goes away.
//

void VTSDoc::UnbindFrameContext( CFrameContext *pfc )
{
	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();

	// let it know its properly unbound
	pfc->m_pDoc = 0;

	// add it to the list of other frame contexts
	for (CFrameContext **cur = &m_FrameContexts; *cur; cur = &(*cur)->m_NextFrame)
		if (*cur == pfc) {
			*cur = (*cur)->m_NextFrame;
			break;
		}

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}

//
//	VTSDoc::SetFrameCount
//
//	Tell all of the frame contexts that the frame count of the document 
//	has changed.  The frame context will pass the message along to 
//	all of its associated views.
//

void VTSDoc::SetPacketCount( int count )
{
	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();
	
	// make sure there is really a change
	if (m_PacketCount != count) {
		// change the local copy
		m_PacketCount = count;

		// tell the frame contexts that the frame count has changed
		for (CFrameContext* cur = m_FrameContexts; cur; cur = cur->m_NextFrame)
		{
			cur->SetPacketCount( count );
		}
	}

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}

//
//	VTSDoc::DeletePackets
//

void VTSDoc::DeletePackets( void )
{
	m_pDB->DeletePackets();

	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();
	
	// make sure there is really a change
	if (m_PacketCount != 0) {
		// change the local copy
		m_PacketCount = 0;

		// tell the frame contexts that the frame count has changed
		for (CFrameContext* cur = m_FrameContexts; cur; cur = cur->m_NextFrame) {
			cur->SetPacketCount( 0 );
			cur->SetCurrentPacket( -1 );
		}
	}

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}

//
//	VTSDoc::NewPacketCount
//
//	This function is called when an IO thread has told the main application that 
//	there are new packets in a database.  The VTSApp::PreTranslateMessage function 
//	calls this function, which gets the count out of the database and then tells 
//	all of the frame contexts.
//

void VTSDoc::NewPacketCount( void )
{

	int NewPacket=m_pDB->GetPacketCount()-m_PacketCount;

	// Edited By Hu Meng 
	// Notice: I made some changes here.
	// Sometimes when this function is called, the packet has not been written into the
	// vdb file. So the NewPacket will be 0,and I will ignore it. 
	// Otherwise the last packet  in the db file will be counted in the statistics collector 
	// for two times
	// The new coming packet will be counted in the statistics collector the next time this
	// function is called.

	if (NewPacket>0)
	{
		SetPacketCount( m_pDB->GetPacketCount() );
		
		for(int i=1;i<=NewPacket;i++)
		{
			VTSPacket	pkt;
			m_pDB->ReadPacket(m_PacketCount-i,pkt);

			BACnetPIInfo	summary( true, false );
			summary.Interpret( (BACnetPIInfo::ProtocolType)pkt.packetHdr.packetProtocolID
				, (char *)pkt.packetData
				, pkt.packetLen);

			m_pStatisticsCollector->Update(summary.summaryLine,pkt.packetLen,pkt.packetHdr.packetType,pkt.packetHdr.packetProtocolID);
			//gStatisticsCollector->Update(summary.summaryLine,pkt.packetLen,pkt.packetHdr.packetType,pkt.packetHdr.packetProtocolID);
			if(m_bStatisticsDlgInUse)
			{
				m_pStatitiscsDlg->Update(summary.summaryLine);
			}
		}
		
	}
}

//
//	VTSDoc::OnViewStatistics
//

void VTSDoc::OnViewStatistics() 
{
	m_bStatisticsDlgInUse=true;
	VTSStatisticsDlg dlg;
	m_pStatitiscsDlg=&dlg;
	m_pStatitiscsDlg->m_pDoc=this;
	dlg.DoModal();
	m_bStatisticsDlgInUse=false;
}

/////////////////////////////////////////////////////////////////////////////
// VTSDocList

VTSDocList	gDocList;

//
//	VTSDocList::VTSDocList
//

VTSDocList::VTSDocList( void )
{
}

//
//	VTSDocList::~VTSDocList
//

VTSDocList::~VTSDocList( void )
{
}

//
//	VTSDocList::Append
//

void VTSDocList::Append( VTSDocPtr vdp )
{
	// add to the end of the list
	AddTail( vdp );
}

//
//	VTSDocList::Remove
//
//	This simply finds the document pointer in the list and removes it.  Note that
//	if the document creation or open was canceled this member function is still 
//	called by the framework, so pos could be NULL and it is not an error.
//

void VTSDocList::Remove( VTSDocPtr vdp )
{
	POSITION	pos = Find( vdp )
	;

	if (pos != NULL)
		RemoveAt( pos );
}

//
//	VTSDocList::Length
//
//	Return the number of open documents.
//

int VTSDocList::Length( void )
{
	return CList<VTSDocPtr,VTSDocPtr>::GetCount();
}

//
//	VTSDocList::Child
//
//	Return a pointer to a specific document.  This is used to build a list of the 
//	open documents so the user can select one for test results.
//

VTSDocPtr VTSDocList::Child( int indx )
{
	POSITION	pos = FindIndex( indx )
	;

	if (pos != NULL )
		return (VTSDocPtr)GetAt( pos );
	else
		return 0;
}

/////////////////////////////////////////////////////////////////////////////
// VTSPort

VTSPortList gMasterPortList;

//
//	VTSPort::VTSPort
//

VTSPort::VTSPort( VTSDocPtr dp, objId id )
	: portDoc(dp), portSendGroup(0), portDescID(id)
	, portStatus(1), portStatusDesc(0)
	, portEndpoint(0), portFilter(0)
	, portBTR(0), portBBMD(0), portBIPSimple(0), portBIPForeign(0), portBindPoint(0)
	, portDevice(0)
{
	// add this to the master list
	AddToMasterList();

	// most ports are bound to a document and have a descriptor
	if (dp && id) {
		// read in the descriptor
		ReadDesc();

		// see if it can be turned on
		Refresh();
	}
}

//
//	VTSPort::~VTSPort
//

VTSPort::~VTSPort( void )
{
	// remove this from the master list
	RemoveFromMasterList();

	// if it is bound to a device, unbind it
	if (portDevice)
		portDevice->Unbind( this );

	// if there is an endpoint, delete it
	if (portEndpoint)
		delete portEndpoint;

	// if there is a filter, delete it
	if (portFilter)
		delete portFilter;

	// if there is a BTR, delete it
	if (portBTR)
		delete portBTR;

	// if there is a BBMD, delete it
	if (portBBMD)
		delete portBBMD;

	// if there is a simple BIP object, delete it
	if (portBIPSimple)
		delete portBIPSimple;

	// if there is a foreign BIP object, delete it
	if (portBIPForeign)
		delete portBIPForeign;
}

//
//	VTSPort::ReadDesc
//

void VTSPort::ReadDesc( void )
{
	int		stat
	;

	// ask the database for the descriptor
	stat = portDoc->m_pDB->pObjMgr->ReadObject( portDescID, &portDesc );

	// make sure it's the correct version
	if (portDesc.objSize != kVTSPortDescSize) {
		// initialize the new variables
		portDesc.portNet = -1;
		portDesc.portDeviceObjID = 0;

		// update the size
		portDesc.objSize = kVTSPortDescSize;

		// write it back
		stat = portDoc->m_pDB->pObjMgr->WriteObject( portDescID, &portDesc );
	}
}

//
//	VTSPort::WriteDesc
//

void VTSPort::WriteDesc( void )
{
	int		stat
	;

	// save the descriptor in the database
	stat = portDoc->m_pDB->pObjMgr->WriteObject( portDescID, &portDesc );

	// if there's a filter, pass along the name
	if (portFilter)
		strcpy( portFilter->filterName, portDesc.portName );
}

//
//	VTSPort::AddToMasterList
//

void VTSPort::AddToMasterList( void )
{
	// add this to the end, regardless of its status
	gMasterPortList.AddTail( this );
}

//
//	VTSPort::RemoveFromMasterList
//

void VTSPort::RemoveFromMasterList( void )
{
	POSITION pos = gMasterPortList.Find( this )
	;

	ASSERT( pos != NULL );
	gMasterPortList.RemoveAt( pos );
}

//
//	VTSPort::Refresh
//

void VTSPort::Refresh( void )
{
	// unbind from the device, if bound
	if (portDevice) {
		portDevice->Unbind( this );
		portDevice = 0;
	}

	// shutdown the existing endpoint, if any
	if (portEndpoint) {
		// unbind from the filter
		Unbind( portFilter, portEndpoint );

		// delete the port object
		delete portEndpoint;

		portStatus = 3;
		portStatusDesc = "Port shut down";
		portEndpoint = 0;
	}

	// if there is a filter, delete it
	if (portFilter) {
		delete portFilter;
		portFilter = 0;
	}

	// if there is a BTR, delete it
	if (portBTR) {
		delete portBTR;
		portBTR = 0;
	}

	// if there is a BBMD, delete it
	if (portBBMD) {
		delete portBBMD;
		portBBMD = 0;
	}

	// if there is a simple BIP object, delete it
	if (portBIPSimple) {
		delete portBIPSimple;
		portBIPSimple = 0;
	}

	// if there is a foreign BIP object, delete it
	if (portBIPForeign) {
		delete portBIPForeign;
		portBIPForeign = 0;
	}

	// see if the port should be enabled
	if (!portDesc.portEnabled) {
		portStatus = 0;
		portStatusDesc = 0;
		return;
	}

	// null ports cannot be enabled
	if (portDesc.portType == nullPort) {
		portStatus = 3;
		portStatusDesc = "Null ports cannot be enabled";
		return;
	}

	// see if a port is already active that matches this configuration
	for (POSITION pos = gMasterPortList.GetHeadPosition(); pos; ) {
		VTSPortPtr	cur = gMasterPortList.GetNext( pos )
		;

		// skip this port if it's not loaded
		if ((cur->portStatus != 1) || (!cur->portEndpoint))
			continue;

		// skip if port types don't match
		if (cur->portDesc.portType != portDesc.portType)
			continue;
		
		// if config strings match, flag this one
		if (strcmp(portDesc.portConfig,cur->portDesc.portConfig) == 0) {
			portStatus = 2;
			portStatusDesc = "Existing port with this configuration";
			return;
		}
	}

	// port should be one of those listed below
	portStatus = 3;
	portStatusDesc = "Unknown port type";

	// enable the port
	switch (portDesc.portType) {
		case ipPort: {
				VTSWinIPPortPtr		pp
				;

				portStatus = 1;
				portStatusDesc = 0;
				portEndpoint = pp = new VTSWinIPPort( this );
				
				// define the TD name
				portDoc->m_Names.DefineTD( portDescID, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
				break;
			}

		case ethernetPort: {
				VTSWinPacket32PortPtr	pp
				;
				
				portStatus = 1;
				portStatusDesc = 0;
				portEndpoint = pp = new VTSWinPacket32Port( this );
				
				// define the TD name
				portDoc->m_Names.DefineTD( portDescID, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
				break;
			}

		case arcnetPort: {
				VTSWinPacket32PortPtr	pp
				;
				
				portStatus = 2;		// good luck
				portStatusDesc = "ARCNET untested";
				portEndpoint = pp = new VTSWinPacket32Port( this );
				
				// define the TD name
				portDoc->m_Names.DefineTD( portDescID, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
				break;
			}		

		case ptpPort: {
			VTSWinPTPPortPtr pp;
			portStatus = 1;		// supported by Xiao Shiyuan 2002-4-22
			portStatusDesc = 0;
			portEndpoint = pp = new VTSWinPTPPort( this );
			break;
		}
	}

	// see if a filter should be set up
	if (portEndpoint) {
		// create a script filter and in the darkness bind them
		portFilter = new ScriptNetFilter( portDesc.portName );
		Bind( portFilter, portEndpoint );

		// default bind point is the filter
		portBindPoint = portFilter;

		// if this is an IP port, we have more work to do
		if (portDesc.portType == ipPort) {
			int		argc = 0
			;
			char	config[kVTSPortConfigLength], *src
			,		*argv[16]
			;

			// copy the configuration, split it up
			strcpy( config, portDesc.portConfig );
			for (src = config; *src; ) {
				argv[argc++] = src;
				while (*src && (*src != ','))
					src++;
				if (*src == ',')
					*src++ = 0;
			}
			for (int i = argc; i < 16; i++)
				argv[i] = src;

			if (argc == 1) {
			} else {
				int option = atoi( argv[1] );
				switch (option) {
					case 0:
						break;
					case 1:
						portBTR = new BACnetBTR();
						Bind( portBTR, portFilter );
						portBindPoint = portBTR;
						break;
					case 2:
						portBBMD = new BACnetBBMD( portEndpoint->portLocalAddr );
						Bind( portBBMD, portFilter );
						portBindPoint = portBBMD;
						break;
					case 3:
						portBIPSimple = new BACnetBIPSimple();
						Bind( portBIPSimple, portFilter );
						portBindPoint = portBIPSimple;
						break;
					case 4: {
							unsigned long	host
							;
							unsigned short	port
							;
							int				ttl
							;

							portBIPForeign = new BACnetBIPForeign();
							Bind( portBIPForeign, portFilter );
							portBindPoint = portBIPForeign;

							BACnetIPAddr::StringToHostPort( argv[2], &host, 0, &port );
							ttl = atoi( argv[3] );
							portBIPForeign->Register( host, port, ttl );
							break;
						}
				}
			}
		}
	}

	// see if we can bind to a device
	if (portDesc.portDeviceObjID != 0) {
		portDevice = portDoc->m_Devices.FindDevice( portDesc.portDeviceObjID );
		if (portDevice)
			portDevice->Bind( this, portDesc.portNet );
	}
}

//
//	VTSPort::Configure
//

void VTSPort::Configure( CString *cp )
{
	switch (portDesc.portType) {
		case ipPort: {
				TRACE0( "IP config request\n" );

				VTSPortIPDialog	dlg( cp );
				dlg.DoModal();
				break;
			}

		case ethernetPort: {
				TRACE0( "Ethernet config request\n" );

				VTSPortEthernetDialog	dlg( cp );
				dlg.DoModal();
				break;
			}

		case arcnetPort:
			TRACE0( "ARCNET config request\n" );
			break;

		case mstpPort:
			TRACE0( "MS/TP config request\n" );
			break;

		case ptpPort:
			//supported by Xiao Shiyuan 2002-4-22
            VTSPortPTPDialog	dlg( cp );
			dlg.DoModal(); 
			break;
	}
}

//
//	VTSPort::SendData
//
//	This is a common passthru function that allows the application to point to 
//	a VTSPort and gives it access to the SendData function provided by the 
//	endpoint (if one has been successfully established).
//

void VTSPort::SendData( BACnetOctet *data, int len )
{
	if (portEndpoint)
		portEndpoint->SendData( data, len );
}

/////////////////////////////////////////////////////////////////////////////
// VTSPortList

//
//	VTSPortList::VTSPortList
//
//	This empty constructor is used to build the master port list.  At the time 
//	the master list is created there is no database to read configurations from 
//	and no INI file.
//

VTSPortList::VTSPortList( void )
	: m_pDoc(0)
{
}

//
//	VTSPortList::~VTSPortList
//

VTSPortList::~VTSPortList( void )
{
}

//
//	VTSPortList::Load
//

void VTSPortList::Load( VTSDocPtr docp )
{
	int			stat
	;
	objId		curID
	;
	JDBListPtr	plist
	;
	VTSPortPtr	cur
	;

	// keep track of the database
	m_pDoc = docp;

	// get a pointer to the list
	plist = &m_pDoc->m_pDB->dbPortList;

	// intialize the port list
	for (int i = 0; i < plist->Length(); i++) {
		// get the port descriptor ID's
		stat = plist->ReadElem( i, &curID );
		ASSERT( stat == 0 );

		// create a new port object to match
		cur = new VTSPort( m_pDoc, curID );

		// add it to our list of ports
		AddTail( cur );
	}
}

//
//	VTSPortList::Unload
//

void VTSPortList::Unload( void )
{
	for (POSITION pos = GetHeadPosition(); pos; )
		delete GetNext( pos );
}

//
//	VTSPortList::Add
//
//	This function is called when the user has requested a new port in the VTSPortDlg
//	dialog.  It defaults to an untitled port.  The port descriptor must be created 
//	in the database before the VTSPort object can be created (its ID is in the ctor).
//

void VTSPortList::Add( void )
{
	int				stat, elemLoc
	;
	objId			newDescID
	;
	VTSPortPtr		cur
	;
	VTSPortDesc		newDesc
	;

	// create a new descriptor
	stat = m_pDoc->m_pDB->pObjMgr->NewObject( &newDescID, &newDesc, kVTSPortDescSize );

	// initalize it
	newDesc.portType = nullPort;
	newDesc.portEnabled = 0;
	newDesc.portConfig[0] = 0;
	strcpy( newDesc.portName, "Untitled" );
	newDesc.portNet = -1;			// no network, or local
	newDesc.portDeviceObjID = 0;	// no bound device/router object

	// save it
	stat = m_pDoc->m_pDB->pObjMgr->WriteObject( newDescID, &newDesc );
	ASSERT( stat == 0 );

	// add the object ID on the end of the port list
	elemLoc = 0x7FFFFFFF;
	stat = m_pDoc->m_pDB->dbPortList.NewElem( &elemLoc, &newDescID );
	ASSERT( stat == 0 );

	// create a new port object to match
	cur = new VTSPort( m_pDoc, newDescID );

	// add it to our list of ports
	AddTail( cur );
}

//
//	VTSPortList::Remove
//
//	This function is currently not called.  If a port is deleted there must be some 
//	way of telling all of the packets and names that the port they used to refer to 
//	is gone.  This is not overly complicated, but I'm putting it off for now.  Note 
//	that there is no delete button in the port dialog box!
//

void VTSPortList::Remove( int i )
{
	ASSERT( 0 );
/*
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	delete GetAt( pos );
	RemoveAt( pos );
*/
}

//
//	VTSPortList::FindPort
//
//	This function is called by the scripting engine to find a port with a given 
//	name as described in the packet description in the script.  Used for sending 
//	low level messages directly to a port.
//

VTSPortPtr VTSPortList::FindPort( const char *name )
{
	for (POSITION pos = GetHeadPosition(); pos; ) {
		VTSPortPtr cur = (VTSPortPtr)GetNext( pos );

		if (strcmp(name,cur->portDesc.portName) == 0)
			return cur;
	}

	// failed to find it
	return 0;
}

//
//	VTSPortList::Length
//

int VTSPortList::Length( void )
{
	return CList<VTSPortPtr,VTSPortPtr>::GetCount();
}

//
//	VTSPortList::operator []
//
//	When the user selects a port from the list of defined ports in the VTSPortDlg, this 
//	function is called to return a pointer to the VTSPort object.  
//

VTSPortPtr VTSPortList::operator []( int i )
{
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	return GetAt( pos );
}

/////////////////////////////////////////////////////////////////////////////
// VTSNameList

//
//	VTSNameList::VTSNameList
//
//	The name list object acts as a simple interface to the JDB object functions 
//	that handle the names.  Unlike ports, where the port list contains objId's 
//	of port descriptions, the names list is actually a list of the name 
//	descriptions themselves.  This is so that name lookups are faster (the 
//	complete list is loaded into one contiguous block) and simpler.
//

VTSNameList::VTSNameList( void )
	: m_pDoc(0)
{
}

//
//	VTSNameList::~VTSNameList
//

VTSNameList::~VTSNameList( void )
{
}

//
//	VTSNameList::Load
//
//	Loading names is easy, the document has already loaded the list from the 
//	database.
//

void VTSNameList::Load( VTSDocPtr docp )
{
	m_pDoc = docp;
}

//
//	VTSNameList::Add
//

void VTSNameList::Add( void )
{
	int				stat, elemLoc
	;
	VTSNameDesc		newDesc
	;

	// initalize it
	strcpy( newDesc.nameName, "Untitled" );
	newDesc.nameAddr.addrType = nullAddr;
	newDesc.nameAddr.addrNet = 0;
	newDesc.nameAddr.addrLen = 0;
	memset( newDesc.nameAddr.addrAddr, 0, kMaxAddressLen );
	newDesc.namePort = 0;

	// add it on the end of the name list
	elemLoc = 0x7FFFFFFF;
	stat = m_pDoc->m_pDB->dbNameList.NewElem( &elemLoc, &newDesc );
}

//
//	VTSNameList::Remove
//
//	This simply deletes the name from the database list.  It could turn around and 
//	save the list, but it doesn't.  I could see about a 'Cancel' button in the Names
//	dialog, but it doesn't seem worth it right now.
//

void VTSNameList::Remove( int i )
{
	int			stat
	;

	stat = m_pDoc->m_pDB->dbNameList.DeleteElem( i );
}

//
//	VTSNameList::Length
//

int VTSNameList::Length( void )
{
	return m_pDoc->m_pDB->dbNameList.Length();
}

//
//	VTSNameList::AddrToName
//
//	Given an address and a port return the first match that is found.  If there's 
//	no match, return null.  Note that this returns a pointer to an internal static
//	buffer, so be sure to make a copy of the results before calling the function 
//	again.
//

const char* VTSNameList::AddrToName( const BACnetAddress &addr, objId portID )
{
	int					len = Length()
	;
	static VTSNameDesc	addrRec
	;

	for (int i = 0; i < len; i++) {
		// load the record
		ReadName( i, &addrRec );

		// if the name record requires a specific port and it's not this one, continue
		if ((addrRec.namePort != 0) && (addrRec.namePort != portID))
			continue;

		// overloaded comparison operator
		if (!(addrRec.nameAddr == addr))
			continue;

		// must be the one we're looking for
		return addrRec.nameName;
	}

	// nothing found
	return 0;
}

//
//	VTSNameList::ReadName
//

void VTSNameList::ReadName( int i, VTSNameDescPtr ndp )
{
	int		stat
	;

	stat = m_pDoc->m_pDB->dbNameList.ReadElem( i, ndp );
}

//
//	VTSNameList::WriteName
//

void VTSNameList::WriteName( int i, VTSNameDescPtr ndp )
{
	int		stat
	;

	stat = m_pDoc->m_pDB->dbNameList.WriteElem( i, ndp );
}

//
//	VTSNameList::DefineTD
//

void VTSNameList::DefineTD( objId port, const BACnetOctet *addr, int len )
{
	int				stat, elemLoc
	;

	// define the name
	strcpy( searchName.nameName, "TD" );
	searchName.namePort = port;
	searchName.nameAddr.addrType = localStationAddr;
	searchName.nameAddr.addrNet = 0;
	memcpy( searchName.nameAddr.addrAddr, addr, len );
	searchName.nameAddr.addrLen = len;

	// look for an existing desc
	elemLoc = m_pDoc->m_pDB->dbNameList.FindElem( 0, (JDBListCompFnPtr)TDSearch );

	// if no match was found, add a new name
	if (elemLoc == -1) {
		// add it on the end of the name list
		elemLoc = 0x7FFFFFFF;
		stat = m_pDoc->m_pDB->dbNameList.NewElem( &elemLoc, &searchName );
	} else
		// save new version
		stat = m_pDoc->m_pDB->dbNameList.WriteElem( elemLoc, &searchName );
}

//
//	VTSNameList::FindTD
//

const BACnetAddress *VTSNameList::FindTD( objId port )
{
	int		stat, elemLoc
	;

	// set up for the search
	searchName.namePort = port;

	// look for a desc
	elemLoc = m_pDoc->m_pDB->dbNameList.FindElem( 0, (JDBListCompFnPtr)TDSearch );

	// if not found, bail out
	if (elemLoc == -1)
		return 0;

	// read the description
	stat = m_pDoc->m_pDB->dbNameList.ReadElem( elemLoc, &searchName );

	// return a pointer to just the address portion
	return &searchName.nameAddr;
}

//
//	VTSNameList::TDSearch
//
//	You would expect the matching function to return non-zero for a successful 
//	match and zero to mean different.  In this case I wanted the return value to 
//	be the same as what memcmp() would return.
//

VTSNameDesc VTSNameList::searchName;

int VTSNameList::TDSearch( const VTSNameDescPtr, const VTSNameDescPtr ndp )
{
	if ((ndp->namePort == searchName.namePort) && (strcmp(ndp->nameName,"TD") == 0))
		return 0;
	else
		return 1;
}

//
//	SetLookupContext
//

namespace NetworkSniffer {

JDBListPtr	gNameListSearchList;
VTSNameDesc	gNameListSearchName;

void SetLookupContext( objId port, JDBListPtr list )
{
	gNameListSearchName.namePort = port;
	gNameListSearchList = list;
}

//
//	LookupName
//
//	This function is called by the interpreters to lookup an address and see if it
//	can be translated into a name.  It is assumed that the global list and port 
//	will be set correctly before the interpreter is called.
//

const char* LookupName( int net, const BACnetOctet *addr, int len )
{
	int					stat, elemLoc
	;
	static VTSNameDesc	searchRslts
	;

	// set up the search block
	gNameListSearchName.nameAddr.addrNet = net;
	memcpy( gNameListSearchName.nameAddr.addrAddr, addr, len );
	gNameListSearchName.nameAddr.addrLen = len;

	// look for a desc
	elemLoc = gNameListSearchList->FindElem( 0, (JDBListCompFnPtr)NameSearch );

	// if not found, bail out
	if (elemLoc == -1)
		return 0;

	// read the description
	stat = gNameListSearchList->ReadElem( elemLoc, &searchRslts );

	// return a pointer to just the name portion
	return searchRslts.nameName;
}

//
//	NameSearch
//

int NameSearch( const VTSNameDescPtr, const VTSNameDescPtr ndp )
{
	// see if the port is a match
	if ((ndp->namePort != 0) && (gNameListSearchName.namePort != ndp->namePort))
		return 1;

	// if a network was specified and its 65535, match for a global broadcast
	if (gNameListSearchName.nameAddr.addrNet == 65535)
		if (ndp->nameAddr.addrType == globalBroadcastAddr)
			return 0;
		else
			return 1;

	// network numbers must match (0 implies local station)
	if (gNameListSearchName.nameAddr.addrNet != ndp->nameAddr.addrNet)
		return 1;

	// address length must match (zero for broadcast)
	if (gNameListSearchName.nameAddr.addrLen != ndp->nameAddr.addrLen)
		return 1;

	// address must match
	return memcmp( gNameListSearchName.nameAddr.addrAddr, ndp->nameAddr.addrAddr, ndp->nameAddr.addrLen );
}

}

/////////////////////////////////////////////////////////////////////////////
// VTSClient

//
//	VTSClient::VTSClient
//

VTSClient::VTSClient( VTSDevicePtr dp )
: clientDev(dp), BACnetClient( &dp->devDevice )
{
}

//
//	VTSClient::Confirmation
//

void VTSClient::Confirmation( const BACnetAPDU &apdu )
{
	// pass it along to the executor for a match
	gExecutor.ReceiveAPDU( apdu );
}

//
//	VTSDevice::IAm
//

void VTSClient::IAm( void )
{
	BACnetUnconfirmedServiceAPDU	hereIAm( iAm )
	;

	BACnetObjectIdentifier( 8, clientDev->devDesc.deviceInstance ).Encode( hereIAm );
	BACnetUnsigned( clientDev->devDesc.deviceMaxAPDUSize ).Encode( hereIAm );
	BACnetEnumerated( clientDev->devDesc.deviceSegmentation ).Encode( hereIAm );
	BACnetUnsigned( clientDev->devDesc.deviceVendorID ).Encode( hereIAm );

	hereIAm.apduAddr.GlobalBroadcast();
	Request( hereIAm );
}

/////////////////////////////////////////////////////////////////////////////
// VTSServer

//
//	VTSServer::VTSServer
//

VTSServer::VTSServer( VTSDevicePtr dp )
: serverDev(dp), BACnetServer( &dp->devDevice )
{
}

//
//	VTSServer::Indication
//

void VTSServer::Indication( const BACnetAPDU &apdu )
{
	// ### log the message

	// pass it along to the executor for a match
	gExecutor.ReceiveAPDU( apdu );

	// process it
	if (apdu.apduType == unconfirmedRequestPDU) {
		switch ((BACnetUnconfirmedServiceChoice)apdu.apduService) {
			case whoIs:
				WhoIs( apdu );
				break;
			case iAm:
				IAm( apdu );
				break;
		}
	} else
	if (apdu.apduType == confirmedRequestPDU) {
		switch ((BACnetConfirmedServiceChoice)apdu.apduService) {
			case readProperty:
				ReadProperty( apdu );
				break;
			case writeProperty:
				WriteProperty( apdu );
				break;
			default:
				Response( BACnetRejectAPDU( apdu.apduInvokeID, unrecognizedService ) );
		}
	} else
		;
}

//
//	VTSServer::Response
//
//	DEVICE_LOOPBACK is used to allow the developer to send requests directly to the
//	internal device object.  There is currently no way to log messages that the device 
//	object sends and receives, so responses are tossed.
//

void VTSServer::Response( const BACnetAPDU &pdu )
{
	// ### log the message

	// filter out null addresses
	if (pdu.apduAddr.addrType == nullAddr)
		return;

	BACnetAppServer::Response( pdu );
}

//
//	VTSServer::WhoIs
//

void VTSServer::WhoIs( const BACnetAPDU &apdu )
{
	BACnetAPDUDecoder	dec( apdu )
	;
	BACnetUnsigned		loLimit, hiLimit
	;
	unsigned int		myInst = serverDev->devDesc.deviceInstance
	;

	try {
		if (dec.pktLength != 0) {
			loLimit.Decode( dec );
			hiLimit.Decode( dec );

			TRACE2( "WhoIs %d..%d\n", loLimit.uintValue, hiLimit.uintValue );

			if ((myInst < loLimit.uintValue) || (myInst > hiLimit.uintValue))
				return;
		}

		BACnetUnconfirmedServiceAPDU hello( iAm );

		// send it to everyone
		hello.apduAddr.GlobalBroadcast();

		// encode the parameters
		BACnetObjectIdentifier( 8, serverDev->devDesc.deviceInstance ).Encode( hello );
		BACnetUnsigned( serverDev->devDesc.deviceMaxAPDUSize ).Encode( hello );
		BACnetEnumerated( serverDev->devDesc.deviceSegmentation ).Encode( hello );
		BACnetUnsigned( serverDev->devDesc.deviceVendorID ).Encode( hello );

		Response( hello );
	}
	catch (...) {
		TRACE0( "WhoIs decoding error\n" );
	}
}

//
//	VTSServer::IAm
//

void VTSServer::IAm( const BACnetAPDU &apdu )
{
	// reserved (add to device address binding list?)
}

//
//	VTSServer::ReadProperty
//

void VTSServer::ReadProperty( const BACnetAPDU &apdu )
{
	BACnetAPDUDecoder		dec( apdu )
	;
	BACnetObjectIdentifier	objId
	;
	BACnetEnumerated		propId
	;
	BACnetUnsigned			arryIndx
	;
	bool					gotIndex = false
	;

	try {
		objId.Decode( dec );
		propId.Decode( dec );

		if (dec.pktLength != 0) {
			gotIndex = true;
			arryIndx.Decode( dec );
		}

		TRACE3( "ReadProperty %d, %d, %d\n", objId.objID, propId.enumValue, arryIndx.uintValue );

		// build an ack
		BACnetComplexAckAPDU	ack( readProperty, apdu.apduInvokeID );

		// send the response back to where the request came from
		ack.apduAddr = apdu.apduAddr;

		// encode the properties from the request
		objId.Encode( ack, 0 );
		propId.Encode( ack, 1 );
		if (gotIndex)
			arryIndx.Encode( ack, 2 );

		// encode the result
	    BACnetOpeningTag().Encode( ack, 3 );
		serverDev->devObjPropValueList->Encode( objId.objID, propId.enumValue
			, (gotIndex ? arryIndx.uintValue : -1)
			, ack
			);
	    BACnetClosingTag().Encode( ack, 3 );

		// send it
		Response( ack );
	}
	catch (int errCode) {
		TRACE1( "ReadProperty execution error - %d\n", errCode );

		BACnetErrorAPDU error( readProperty, apdu.apduInvokeID );
		error.apduAddr = apdu.apduAddr;

		// encode the Error Class
		if ((errCode == 2) || (errCode == 25)) // configuration-in-progress, operational-problem
			BACnetEnumerated( 0 ).Encode( error );		// DEVICE
		else
		if (errCode == 42) // invalid-array-index
			BACnetEnumerated( 2 ).Encode( error );		// PROPERTY
		else
			BACnetEnumerated( 1 ).Encode( error );		// OBJECT

		// encode the Error Code
		BACnetEnumerated( errCode ).Encode( error );

		Response( error );
	}
	catch (...) {
		TRACE0( "ReadProperty execution error\n" );

		BACnetRejectAPDU goAway( apdu.apduInvokeID, otherReject );
		goAway.apduAddr = apdu.apduAddr;

		Response( goAway );
	}
}

//
//	VTSServer::WriteProperty
//

void VTSServer::WriteProperty( const BACnetAPDU &apdu )
{
	BACnetAPDUTag			t
	;
	BACnetAPDUDecoder		dec( apdu )
	;
	BACnetObjectIdentifier	objId
	;
	BACnetEnumerated		propId
	;
	BACnetUnsigned			arryIndx
	;
	bool					gotIndex = false
	;

	try {
		objId.Decode( dec );
		propId.Decode( dec );

		// look at the next tag
		dec.ExamineTag( t );

		if ((t.tagClass == contextTagClass) && (t.tagNumber == 2)) {
			gotIndex = true;
			arryIndx.Decode( dec );

			// look at the next tag
			dec.ExamineTag( t );
		}

		TRACE3( "WriteProperty %d, %d, %d\n", objId.objID, propId.enumValue, arryIndx.uintValue );

		// build an ack
		BACnetSimpleAckAPDU	ack( readProperty, apdu.apduInvokeID );

		// send the response back to where the request came from
		ack.apduAddr = apdu.apduAddr;

		// make sure we have an opening tag
		if ((t.tagClass == openingTagClass) && (t.tagNumber == 3)) {
			// remove it from the decoder
			BACnetOpeningTag().Decode( dec );
		} else
			throw (invalidTagReject);

		// decode the contents
		serverDev->devObjPropValueList->Decode( objId.objID, propId.enumValue
			, (gotIndex ? arryIndx.uintValue : -1)
			, dec
			);

		// look at the next tag
		dec.ExamineTag( t );

		// make sure it's a closing tag
		if ((t.tagClass == closingTagClass) && (t.tagNumber == 3)) {
			// remove it from the decoder
			BACnetClosingTag().Decode( dec );
		} else
			throw (invalidTagReject);

		// send it
		Response( ack );
	}
	catch (int errCode) {
		TRACE1( "WriteProperty execution error - %d\n", errCode );

		BACnetErrorAPDU error( readProperty, apdu.apduInvokeID );
		error.apduAddr = apdu.apduAddr;

		// encode the Error Class
		if ((errCode == 2) || (errCode == 25)) // configuration-in-progress, operational-problem
			BACnetEnumerated( 0 ).Encode( error );		// DEVICE
		else
		if (errCode == 42) // invalid-array-index
			BACnetEnumerated( 2 ).Encode( error );		// PROPERTY
		else
			BACnetEnumerated( 1 ).Encode( error );		// OBJECT

		// encode the Error Code
		BACnetEnumerated( errCode ).Encode( error );

		Response( error );
	}
	catch (BACnetRejectReason rr) {
		TRACE0( "WriteProperty execution error\n" );

		BACnetRejectAPDU goAway( apdu.apduInvokeID, rr );
		goAway.apduAddr = apdu.apduAddr;

		Response( goAway );
	}
	catch (...) {
		TRACE0( "WriteProperty execution error\n" );

		BACnetRejectAPDU goAway( apdu.apduInvokeID, otherReject );
		goAway.apduAddr = apdu.apduAddr;

		Response( goAway );
	}
}

/////////////////////////////////////////////////////////////////////////////
// VTSDevice

VTSDeviceList gMasterDeviceList;

//
//	VTSDevice::VTSDevice
//

#pragma warning( disable : 4355 )
VTSDevice::VTSDevice( VTSDocPtr dp, objId id )
	: devDoc(dp), devDescID(id)
	, devClient(this), devServer(this)
{
	// read in the descriptor
	ReadDesc();

	// bind the device to the router (global Bind(), not our local one)
	::Bind( &devDevice, &devRouter );

	// add this to the master list
	AddToMasterList();

	// create a dummy port
	devPort = new VTSPort( 0, 0 );

	// the port name matches the device name
	strcpy( devPort->portDesc.portName, devDesc.deviceName );

	// create a funny endpoint that redirects requests back to this device
	devPortEndpoint = new VTSDevicePort( devPort, this );

	// read in the defined objects, properties and values
	devObjPropValueList = new VTSObjPropValueList( dp, devDesc.deviceObjPropValueListID );
}
#pragma warning( default : 4355 )

//
//	VTSDevice::~VTSDevice
//

VTSDevice::~VTSDevice( void )
{
	// remove this from the master list
	RemoveFromMasterList();

	// delete the dummy port which will also delete the endpoint
	delete devPort;

	// toss the object, properties and values manager
	delete devObjPropValueList;
}

//
//	VTSDevice::AddToMasterList
//

void VTSDevice::AddToMasterList( void )
{
	// add this to the end, regardless of its status
	gMasterDeviceList.AddTail( this );
}

//
//	VTSDevice::RemoveFromMasterList
//

void VTSDevice::RemoveFromMasterList( void )
{
	POSITION pos = gMasterDeviceList.Find( this )
	;

	ASSERT( pos != NULL );
	gMasterDeviceList.RemoveAt( pos );
}

//
//	VTSDevice::ReadDesc
//

void VTSDevice::ReadDesc( void )
{
	int		stat
	;

	// ask the database for the descriptor
	stat = devDoc->m_pDB->pObjMgr->ReadObject( devDescID, &devDesc );

	// make sure the device matches the descriptor contents
	devDevice.deviceInst = devDesc.deviceInstance;
	devDevice.deviceSegmentSize = devDesc.deviceSegmentSize;
	devDevice.deviceWindowSize = devDesc.deviceWindowSize;
	devDevice.deviceNextInvokeID = devDesc.deviceNextInvokeID;
	devDevice.deviceMaxAPDUSize = devDesc.deviceMaxAPDUSize;
	devDevice.deviceAPDUTimeout = devDesc.deviceAPDUTimeout;
	devDevice.deviceAPDUSegmentTimeout = devDesc.deviceAPDUSegmentTimeout;
	devDevice.deviceAPDURetries = devDesc.deviceAPDURetries;
}

//
//	VTSDevice::WriteDesc
//

void VTSDevice::WriteDesc( void )
{
	int		stat
	;

	// save the descriptor in the database
	stat = devDoc->m_pDB->pObjMgr->WriteObject( devDescID, &devDesc );

	// make sure the device matches the descriptor contents
	devDevice.deviceInst = devDesc.deviceInstance;
	devDevice.deviceSegmentSize = devDesc.deviceSegmentSize;
	devDevice.deviceWindowSize = devDesc.deviceWindowSize;
	devDevice.deviceNextInvokeID = devDesc.deviceNextInvokeID;
	devDevice.deviceMaxAPDUSize = devDesc.deviceMaxAPDUSize;
	devDevice.deviceAPDUTimeout = devDesc.deviceAPDUTimeout;
	devDevice.deviceAPDUSegmentTimeout = devDesc.deviceAPDUSegmentTimeout;
	devDevice.deviceAPDURetries = devDesc.deviceAPDURetries;

	// make sure the dummy port has the correct name
	strcpy( devPort->portDesc.portName, devDesc.deviceName );
}

//
//	VTSDevice::Bind
//
//	A binding operation is pretty simple, just pass the request along 
//	to the built-in router.
//

void VTSDevice::Bind( VTSPortPtr pp, int net )
{
	devRouter.BindToEndpoint( pp->portBindPoint, net );
//	devRouter.SetLocalAddress( net, pp->portEndpoint->portLocalAddr );
}

//
//	VTSDevice::Unbind
//

void VTSDevice::Unbind( VTSPortPtr pp )
{
	devRouter.UnbindFromEndpoint( pp->portBindPoint );
}

//
//	VTSDevice::IAm
//

void VTSDevice::IAm( void )
{
	devClient.IAm();
}

//
//	VTSDevice::SendAPDU
//

void VTSDevice::SendAPDU( const BACnetAPDU &apdu )
{
	bool	asClient
	;

	switch (apdu.apduType) {
		case confirmedRequestPDU:
		case unconfirmedRequestPDU:
			asClient = true;
			break;

		case simpleAckPDU:
		case complexAckPDU:
			asClient = false;
			break;

		case segmentAckPDU:
			asClient = (apdu.apduSrv == 0);
			break;

		case errorPDU:
		case rejectPDU:
			asClient = false;
			break;

		case abortPDU:
			asClient = (apdu.apduSrv == 0);
			break;

	}

	try {
#if DEVICE_LOOPBACK
		// null address is like a loopback
		if (apdu.apduAddr.addrType == nullAddr) {
			if (asClient)
				devServer.Indication( apdu );
			else
				devClient.Confirmation( apdu );
		} else
#endif
		if (asClient)
			devClient.Request( apdu );
		else
			devServer.Response( apdu );
	}
	catch (...) {
		TRACE0( "---------- Something went wrong ----------\n" );
	}
}

/////////////////////////////////////////////////////////////////////////////
// VTSDeviceList

//
//	VTSDeviceList::VTSDeviceList
//

VTSDeviceList::VTSDeviceList( void )
	: m_pDoc(0)
{
}

//
//	VTSDeviceList::~VTSDeviceList
//

VTSDeviceList::~VTSDeviceList( void )
{
}

//
//	VTSDeviceList::Load
//

void VTSDeviceList::Load( VTSDocPtr docp )
{
	int				stat
	;
	objId			curID
	;
	JDBListPtr		plist
	;
	VTSDevicePtr	cur
	;

	// keep track of the database
	m_pDoc = docp;

	// get a pointer to the list
	plist = &m_pDoc->m_pDB->dbDeviceList;

	// intialize the port list
	for (int i = 0; i < plist->Length(); i++) {
		// get the port descriptor ID's
		stat = plist->ReadElem( i, &curID );
		ASSERT( stat == 0 );

		// create a new device object to match
		cur = new VTSDevice( m_pDoc, curID );

		// add it to our list of ports
		AddTail( cur );
	}
}

//
//	VTSDeviceList::Unload
//

void VTSDeviceList::Unload( void )
{
	for (POSITION pos = GetHeadPosition(); pos; )
		delete GetNext( pos );
}

//
//	VTSDeviceList::Add
//

void VTSDeviceList::Add( void )
{
	int					stat, elemLoc
	;
	objId				newDescID
	;
	VTSDevicePtr		cur
	;
	VTSDeviceDesc		newDesc
	;
	JDBListPtr			lp
	;

	// create a new descriptor
	stat = m_pDoc->m_pDB->pObjMgr->NewObject( &newDescID, &newDesc, kVTSDeviceDescSize );

	// initalize it
	strcpy( newDesc.deviceName, "Untitled" );
	newDesc.deviceInstance = 0;
	newDesc.deviceRouter = 0;
	newDesc.deviceSegmentation = noSegmentation;
	newDesc.deviceSegmentSize = 1024;
	newDesc.deviceWindowSize = 1;
	newDesc.deviceMaxAPDUSize = 1024;
	newDesc.deviceNextInvokeID = 0;
	newDesc.deviceAPDUTimeout = 5000;
	newDesc.deviceAPDUSegmentTimeout = 1000;
	newDesc.deviceAPDURetries = 3;
	newDesc.deviceVendorID = 15;				// default to Cornell

	// create a new object and property list
	lp = new JDBList();
	stat = m_pDoc->m_pDB->pListMgr->NewList( *lp, kVTSObjPropValueSize );
	newDesc.deviceObjPropValueListID = lp->objID;
	delete lp;

	// save it
	stat = m_pDoc->m_pDB->pObjMgr->WriteObject( newDescID, &newDesc );
	ASSERT( stat == 0 );

	// add it on the end of the device list
	elemLoc = 0x7FFFFFFF;
	stat = m_pDoc->m_pDB->dbDeviceList.NewElem( &elemLoc, &newDescID );

	// create a new device object to match
	cur = new VTSDevice( m_pDoc, newDescID );

	// add it to our list of ports
	AddTail( cur );
}

//
//	VTSDeviceList::Remove
//
//	This function is currently not called.  Note there is no delete button in 
//	the device dialog box.
//

void VTSDeviceList::Remove( int i )
{
	ASSERT( 0 );
/*
	int			stat
	;
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	delete GetAt( pos );
	RemoveAt( pos );

	stat = m_pDoc->m_pDB->dbDeviceList.DeleteElem( i );
*/
}

//
//	VTSPortList::FindPort
//
//	This function is called by the scripting engine to find a port with a given 
//	name as described in the packet description in the script.  Used for sending 
//	low level messages directly to a port.
//

VTSDevicePtr VTSDeviceList::FindDevice( const char *name )
{
	for (POSITION pos = GetHeadPosition(); pos; ) {
		VTSDevicePtr cur = (VTSDevicePtr)GetNext( pos );

		if (strcmp(name,cur->devDesc.deviceName) == 0)
			return cur;
	}

	// failed to find it
	return 0;
}

//
//	VTSDeviceList::FindDevice
//
//	This function is used by a port to bind itself properly to a device object.
//

VTSDevicePtr VTSDeviceList::FindDevice( objId id )
{
	for (POSITION pos = GetHeadPosition(); pos; ) {
		VTSDevicePtr cur = (VTSDevicePtr)GetNext( pos );

		if (cur->devDescID == id)
			return cur;
	}

	// failed to find it
	return 0;
}

//
//	VTSDeviceList::Length
//

int VTSDeviceList::Length( void )
{
	return CList<VTSDevicePtr,VTSDevicePtr>::GetCount();
}

//
//	VTSDeviceList::operator []
//
//	When the user selects a port from the list of defined ports in the VTSPortDlg, this 
//	function is called to return a pointer to the VTSPort object.  
//

VTSDevicePtr VTSDeviceList::operator []( int i )
{
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	return GetAt( pos );
}

/////////////////////////////////////////////////////////////////////////////
// VTSWinPacket32Port

//
//	VTSWinPacket32Port::VTSWinPacket32Port
//

extern CSendGroupPtr gEthernetGroupList[];
extern CSendGroupPtr gARCNETGroupList[];

VTSWinPacket32Port::VTSWinPacket32Port( VTSPortPtr pp )
	: WinPacket32( pp->portDesc.portConfig )
	, m_pPort( pp )
{
	// let the port know which send group to use
	if (portStatus == 0) {
		if (this->m_AdapterType == ETH_802_3_ADAPTER)
			m_pPort->portSendGroup = gEthernetGroupList;
		else
		if (this->m_AdapterType == ARCNET_ADAPTER)
			m_pPort->portSendGroup = gARCNETGroupList;
		else
			;
	}
}

//
//	VTSWinPacket32Port::~VTSWinPacket32Port
//

VTSWinPacket32Port::~VTSWinPacket32Port( void )
{
	// reset the send group
	m_pPort->portSendGroup = 0;
}

//
//	VTSWinPacket32Port::FilterData
//

void VTSWinPacket32Port::FilterData( BACnetOctet *data, int len, BACnetPortDirection dir )
{
	VTSPacket	pkt
	;
	
	// fill in the packet header
	pkt.packetHdr.packetPortID = m_pPort->portDescID;
	pkt.packetHdr.packetFlags = 0;
	pkt.packetHdr.packetType = (dir == portSending) ? txData : rxData;

	// parse the source and destination addresses
	if (m_AdapterType == ETH_802_3_ADAPTER) {
		pkt.packetHdr.packetProtocolID = (int)BACnetPIInfo::ProtocolType::ethernetProtocol;

		// check for broadcast destination
		if (memcmp(data,"\377\377\377\377\377\377",6) == 0)
			pkt.packetHdr.packetDestination.LocalBroadcast();
		else
			pkt.packetHdr.packetDestination.LocalStation( data, 6 );

		// source is always a station
		pkt.packetHdr.packetSource.LocalStation( data + 6, 6 );
	} else
	if (m_AdapterType == ARCNET_ADAPTER) {
		pkt.packetHdr.packetProtocolID = (int)BACnetPIInfo::ProtocolType::arcnetProtocol;

		// check for broadcast destination
		if (data[1] == 0)
			pkt.packetHdr.packetDestination.LocalBroadcast();
		else
			pkt.packetHdr.packetDestination.LocalStation( data + 1, 1 );

		// source is always a station
		pkt.packetHdr.packetSource.LocalStation( data, 1 );
	} else
		return;

	// skip processing packets from myself
	if ((dir == portReceiving) && (pkt.packetHdr.packetSource == portLocalAddr))
		return;

	// let the packet refer to the pdu contents, cast away const
	pkt.NewDataRef( data, len );

	// save it in the database;
	m_pPort->portDoc->m_pDB->WritePacket( -1, pkt );

	// tell the application
	if (m_pPort->portDoc->m_postMessages)
		::PostThreadMessage( AfxGetApp()->m_nThreadID
			, WM_VTS_RCOUNT, (WPARAM)0, (LPARAM)m_pPort->portDoc
			);
}

//
//	VTSWinPacket32Port::PortStatusChange
//

void VTSWinPacket32Port::PortStatusChange( void )
{
	// set the VTSPort info to reflect this status
	if (portStatus == 0) {
		m_pPort->portStatus = 0;
		m_pPort->portStatusDesc = 0;
	} else {
		m_pPort->portStatus = 2;
		m_pPort->portStatusDesc = "Something is wrong";
	}

	// tell the application that something changed.
	::PostThreadMessage( AfxGetApp()->m_nThreadID
		, WM_VTS_PORTSTATUS, (WPARAM)0, (LPARAM)m_pPort->portDoc
		);
}

/////////////////////////////////////////////////////////////////////////////
// VTSWinIPPort

//
//	VTSWinIPPort::VTSWinIPPort
//

extern CSendGroupPtr gIPGroupList[];

VTSWinIPPort::VTSWinIPPort( VTSPortPtr pp )
	: WinIP( pp->portDesc.portConfig )
	, m_pPort( pp )
{
	// let the port know which send group to use
	m_pPort->portSendGroup = gIPGroupList;
}

//
//	VTSWinIPPort::~VTSWinIPPort
//

VTSWinIPPort::~VTSWinIPPort( void )
{
	// reset the send group
	m_pPort->portSendGroup = 0;
}

//
//	VTSWinIPPort::FilterData
//

void VTSWinIPPort::FilterData( BACnetOctet *data, int len, BACnetPortDirection dir )
{
	VTSPacket	pkt
	;

	// fill in the packet header
	pkt.packetHdr.packetPortID = m_pPort->portDescID;
	pkt.packetHdr.packetProtocolID = (int)BACnetPIInfo::ProtocolType::ipProtocol;
	pkt.packetHdr.packetFlags = 0 /* (pdu.pduExpectingReply << 8) + pdu.pduNetworkPriority */;
	pkt.packetHdr.packetType = (dir == portSending) ? txData : rxData;
	
	// parse the header and suck out the source or destination
	if (dir == portSending)
		pkt.packetHdr.packetDestination.LocalStation( data, 6 );
	else
	if (dir == portReceiving) {
		pkt.packetHdr.packetSource.LocalStation( data, 6 );
	} else
		;

	// let the packet refer to the pdu contents, cast away const
	pkt.NewDataRef( data, len );
	
	// save it in the database;
	m_pPort->portDoc->m_pDB->WritePacket( -1, pkt );
	
	// tell the application there is a new packet count
	if (m_pPort->portDoc->m_postMessages)
		::PostThreadMessage( AfxGetApp()->m_nThreadID
			, WM_VTS_RCOUNT, (WPARAM)0, (LPARAM)m_pPort->portDoc
			);
}

//
//	VTSWinIPPort::PortStatusChange
//

void VTSWinIPPort::PortStatusChange( void )
{
	// set the VTSPort info to reflect this status
	if (portStatus == 0) {
		m_pPort->portStatus = 0;
		m_pPort->portStatusDesc = 0;
	} else {
		m_pPort->portStatus = 2;
		m_pPort->portStatusDesc = "Something is wrong";
	}

	// tell the application that something changed.
	::PostThreadMessage( AfxGetApp()->m_nThreadID
		, WM_VTS_PORTSTATUS, (WPARAM)0, (LPARAM)m_pPort->portDoc
		);
}

/////////////////////////////////////////////////////////////////////////////
// VTSDevicePort

//
//	VTSDevicePort::VTSDevicePort
//

VTSDevicePort::VTSDevicePort( VTSPortPtr pp, VTSDevicePtr dp )
	: m_pPort(pp), m_pDevice(dp)
{
	// the port should think this is its endpoint
	pp->portEndpoint = this;

	// let the port know which send group to use
	pp->portSendGroup = gDeviceGroupList;
}

//
//	VTSDevicePort::~VTSDevicePort
//

VTSDevicePort::~VTSDevicePort( void )
{
	// reset the send group
	m_pPort->portSendGroup = 0;
}

//
//	VTSDevicePort::Indication
//
//	This function is required because a VTSDevicePort derives from BACnetNetServer.
//	It should never be called, these objects aren't bound to anything.
//

void VTSDevicePort::Indication( const BACnetNPDU &pdu )
{
	ASSERT( 0 );
}

//
//	VTSDevicePort::SendData
//

void VTSDevicePort::SendData( BACnetOctet *data, int len )
{
	int			net, addrLen
	;
	BACnetAPDU	apdu
	;

	// rip apart the address that was so carefully encoded by CSendDevice::EncodePage
	apdu.apduAddr.addrType = (BACnetAddressType)(len--,*data++);

	net = (len--,*data++);
	net = (net << 8) + (len--,*data++);
	apdu.apduAddr.addrNet = net;

	addrLen = (len--,*data++);
	apdu.apduAddr.addrLen = addrLen;
	for (int i = 0; i < addrLen; i++)
		apdu.apduAddr.addrAddr[i] = (len--,*data++);

	// build a temporary decoder out of the rest
	BACnetAPDUDecoder dec( data, len )
	;

	// turn it into an APDU
	apdu.Decode( dec );

	// pass it along to the device to process
	m_pDevice->SendAPDU( apdu );
}

void VTSDoc::OnFileSave() 
{
	// TODO: Add your command handler code here
	
}
