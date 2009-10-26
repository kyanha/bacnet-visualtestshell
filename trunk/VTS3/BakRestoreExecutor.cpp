// BakRestoreExecutor.cpp: implementation of the BakRestoreExecutor class.
// Jingbo Gao, Sep 20 2004
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vts.h"
#include "VTSDoc.h"
#include "BACnet.hpp"
#include "VTSBackupRestoreDlg.h"
#include "VTSBackupRestoreProgressDlg.h"
#include "ScriptExecutor.h"
#include "BakRestoreExecutor.h"
#include "PI.h"

namespace NetworkSniffer {
	extern char *BACnetFileAccessMethod[];
}

namespace PICS {
#include "db.h"
#include "service.h"
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

// global defines
BakRestoreExecutor		gBakRestoreExecutor;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BakRestoreExecutor::AnyValue::AnyValue()
{
}

BakRestoreExecutor::AnyValue::~AnyValue()
{
	pbacnetTypedValue = NULL;
}

BakRestoreExecutor::PropertyValue::PropertyValue()
{
}

BakRestoreExecutor::PropertyValue::PropertyValue(const BACnetEnumerated& propID, BACnetEncodeable& propValue)
{
	m_propID.enumValue = propID.enumValue;
	m_propValue.SetObject(&propValue);
}

BakRestoreExecutor::PropertyValue::PropertyValue(const PropertyValue& value)
{
	m_propID.enumValue = value.m_propID.enumValue;
	PropertyValue* p = const_cast<PropertyValue*>(&value);
	m_propValue.SetObject(p->m_propValue.GetObject());
}

BakRestoreExecutor::PropertyValue::~PropertyValue()
{
}

BakRestoreExecutor::BakRestoreExecutor()
: m_pPort(NULL), m_pName(NULL), m_nDeviceObjInst(0), m_strBackupFileName(""),
  m_strPassword(""), m_funToExe(ALL_BACKUP_RESTORE), m_execState(execIdle),
  m_pAPDU(NULL), m_bAbort(FALSE), m_bExpectPacket(FALSE),m_packetData(NULL),
  m_bExpectAPDU(TRUE), m_bUserCancelled(FALSE), m_pOutputDlg(NULL)
{
}

BakRestoreExecutor::~BakRestoreExecutor()
{
	if (m_pAPDU)
	{
		delete m_pAPDU;
	}

	if (m_packetData)
	{
		delete []m_packetData;
	}

}

void BakRestoreExecutor::AnyValue::SetObject(BACnetEncodeable * pbacnetEncodeable)
{
	pbacnetTypedValue = NULL;
	BACnetAnyValue::SetObject(pbacnetEncodeable);
}

void BakRestoreExecutor::PropertyValue::Encode(BACnetAPDUEncoder& enc)
{
	m_propID.Encode(enc, 0);
	BACnetOpeningTag().Encode(enc, 2);
	m_propValue.Encode(enc);
	BACnetClosingTag().Encode(enc, 2);
}

void BakRestoreExecutor::PropertyValue::Decode(BACnetAPDUDecoder &dec)
{
	// can not decode.
	ASSERT(FALSE);
}

void BakRestoreExecutor::ExecuteTest()
{
	CSingleLock lock(&m_cs);
	lock.Lock();

	if (m_execState != execIdle) {
		TRACE0( "Error: invalid executor state\n" );
		return;
	}

	VTSDocPtr	pVTSDoc = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();
	VTSPorts* pPorts = pVTSDoc->GetPorts();
	VTSNames* pNames = pVTSDoc->GetNames();

  VTSBackupRestoreDlg dlg(*pNames, *pPorts);
	if ( dlg.DoModal() == IDOK )
	{
		for ( int i = 0; i < pPorts->GetSize(); i++ )
		{
			if ( ((VTSPort *) pPorts->GetAt(i))->m_strName.CompareNoCase(dlg.m_strPort) == 0 )
			{
				m_pPort = (VTSPort *) pPorts->GetAt(i);
				break;
			}
		}
		m_pName = pNames->GetAt( pNames->FindIndex(dlg.m_strDevice) );
		m_funToExe = (FunctionToExecute)dlg.m_nFunction;
		if (dlg.m_strDevObjInst.IsEmpty())
		{
			m_nDeviceObjInst = DEVICE_OBJ_INST_NUM_UNSET;
		}
		else
		{
			m_nDeviceObjInst = atoi(dlg.m_strDevObjInst);
		}
		m_strBackupFileName = dlg.m_strBackupFileName;
		m_strPassword = dlg.m_strPassword;
		m_Delay = dlg.Delay;
		m_Backup_Timeout = dlg.timeout;
	}
	else
	{
		return;
	}

	if (m_pOutputDlg != NULL)
	{
		m_pOutputDlg->SetFocus();
	}
	else
	{
		m_pOutputDlg = new VTSBackupRestoreProgressDlg(NULL);
		m_pOutputDlg->Create(IDD_BACKUP_RESTORE_PROGRESS);
		m_pOutputDlg->ShowWindow(SW_SHOW);
	}

	// install the task
	taskType = oneShotTask;
	taskInterval = 0;
	InstallTask();

	lock.Unlock();

}

void BakRestoreExecutor::Kill()
{
	// verify the executor state
	if (m_execState != execRunning)
	{
		TRACE0( "Error: invalid executor state\n" );
		return;
	}

	m_bUserCancelled = TRUE;
	m_event.SetEvent();
}


void BakRestoreExecutor::ProcessTask()
{
	CSingleLock lock(&m_cs);
	lock.Lock();

	BOOL bSuccess = TRUE;
	try	{
		if ( !m_pPort || !m_pName || m_strBackupFileName.IsEmpty() )
		{
			throw("VTSPort, IUT or backfile doesn't specified\n");
		}

		m_IUTAddr = m_pName->m_bacnetaddr;
		if (m_IUTAddr.addrType != localStationAddr &&
			m_IUTAddr.addrType != remoteStationAddr)
		{
			throw("The IUT's address can only be local station or remote station\n");
		}

		m_execState = execRunning;

		m_pOutputDlg->BeginTestProcess();

		if (m_IUTAddr.addrType == remoteStationAddr)
		{
			FindRouterAddress();
		}

		switch(m_funToExe) {
		case ALL_BACKUP_RESTORE:
			DoBackupTest();
			DoRestoreTest();
			DoAuxiliaryTest();
			Msg("All Backup and Restore tests have been sucessfully completed!\n");
			break;
		case FULL_BACKUP_RESTORE:
			DoBackupTest();
			DoRestoreTest();
			Msg("Full Backup and Restore tests have been sucessfully completed!\n");
			break;
		case BACKUP_ONLY:
			DoBackupTest();
			Msg("Backup tests have been sucessfully completed!\n");
			break;
		case RESTORE_ONLY:
			DoRestoreTest();
			Msg("Restore tests have been sucessfully completed!\n");
			break;
		case AUXILIARY_BACKUP_RESTORE:
			DoAuxiliaryTest();
			Msg("Auxiliary backup and restore tests have been sucessfully completed!\n");
			break;
		default:
			;
		}
	}
	catch (const char *errMsg) {
		bSuccess = FALSE;
		Msg(errMsg);
	}
	catch (...) {
		bSuccess = FALSE;
		Msg("Decoding error! Maybe received uncorrect packet!\n");
	}

	m_execState = execIdle;
	if (bSuccess)
	{
		m_pOutputDlg->OutMessage("The whole test process has been successful completed");
	}
	else if (m_bUserCancelled)
	{
		m_bUserCancelled = FALSE;	// reset
		m_pOutputDlg->OutMessage("", TRUE);		//begin a new line
		m_pOutputDlg->OutMessage("Canceled by user");
	}
	else
	{
		m_pOutputDlg->OutMessage("Failed",TRUE);	// begin a new line
		m_pOutputDlg->OutMessage("Error occurs during the test");
	}

	m_pOutputDlg->EndTestProcess();
	lock.Unlock();
}


void BakRestoreExecutor::DoBackupTest()
{
	m_pOutputDlg->OutMessage("Begin Backup process...");
	m_pOutputDlg->OutMessage("Create backup index file...", FALSE);
	CString strIndexFileName(m_strBackupFileName + ".backupindex");
	WIN32_FIND_DATA fd;
	HANDLE	hFind = ::FindFirstFile(strIndexFileName, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char msg[200];
			sprintf(msg, "The file %s already existes, do you want to overwrite it?\n", fd.cFileName);
			if (IDNO == AfxMessageBox(msg, MB_YESNO|MB_ICONSTOP))
			{
				throw("backup file has already existed\n");
			}
		}
	}
	CFile backupIndexFile(strIndexFileName, CFile::modeWrite|CFile::modeCreate);
	m_pOutputDlg->OutMessage("OK");

	BACnetObjectIdentifier devObjID;
	BACnetUnsigned maxAPDULenAccepted;

	BACnetEnumerated propID;
	AnyValue propValue;
  	char buffer[300];

	if (m_nDeviceObjInst != DEVICE_OBJ_INST_NUM_UNSET)
	{
		m_pOutputDlg->OutMessage("Use ReadProperty to set M2 = Max_APDU_Length...", FALSE);
		// Use ReadProperty to set M2 = Max_APDU_Length
		devObjID.SetValue(device, m_nDeviceObjInst);
		propID.enumValue = PICS::MAX_APDU_LENGTH_ACCEPTED;
		propValue.SetObject(&maxAPDULenAccepted);
		if (!SendExpectReadProperty(devObjID, propID, propValue))
		{
			throw("Can not read MAX_APDU_LENGTH_ACCEPTED value in IUT device object\n");
		}
		m_pOutputDlg->OutMessage("OK");
	}
	else
	{
		// Transmit a unicast Who-Is message to the IUT's address,
		// and get deviceID, Max APDU Length Accepted
		m_pOutputDlg->OutMessage("Transmit a unicast Who-Is message to the IUT's address...", FALSE);
		if (!SendExpectWhoIs(devObjID, maxAPDULenAccepted))
		{
			m_pOutputDlg->OutMessage("Failed");
			m_pOutputDlg->OutMessage("Transmit a ReadProperty to read the real Device objID...", FALSE);
			// Transmit a ReadProperty request to the IUT, using the wildcard Device ID to read
			// the real Device Object ID. Store the returned Device Object ID into the variable
			// DEVICE_ID, then use ReadProperty to set M2 = Max_APDU_Length_Accepted
			BACnetObjectIdentifier objID(device, 4194303);
			BACnetEnumerated propID(PICS::OBJECT_IDENTIFIER);
			propValue.SetObject(&devObjID);
			if (!SendExpectReadProperty(objID, propID, propValue))
			{
				// if failed received packet, the test failed
				throw("Dynamic discovery of the Device ID has failed!\n"
					"the user must specify a Device Instance number in the Backup/Restore dialog\n");
			}
			m_pOutputDlg->OutMessage("OK");
			m_pOutputDlg->OutMessage("Use ReadProperty to set M2 = Max_APDU_Length...", FALSE);
			propID.enumValue = PICS::MAX_APDU_LENGTH_ACCEPTED;
			propValue.SetObject(&maxAPDULenAccepted);
			if (!SendExpectReadProperty(devObjID, propID, propValue))
			{
				throw("Can not read MAX_APDU_LENGTH_ACCEPTED\n");
			}
			m_pOutputDlg->OutMessage("OK");
		}
		else
		{
			m_pOutputDlg->OutMessage("OK");
		}
	}

	m_pOutputDlg->OutMessage("Use ReadProperty requests to read Device/Database_Revision"
							"and Device/Last_Restore_Time...",	FALSE);
	// Use ReadProperty request to read the Device/Database_Revision and the
	// Device/Last_Restore_Time and record these in the first line of the
	propID.enumValue = PICS::DATABASE_REVISION;
	AnyValue any;
	BACnetUnsigned databaseRevision;
	propValue.SetObject(&databaseRevision);
	if (!SendExpectReadProperty(devObjID, propID, propValue))
	{
		throw("cannot read DATABASE_REVISION from IUT\n");
	}
	propID.enumValue = PICS::LAST_RESTORE_TIME;
	BACnetTimeStamp	lastRestoreTime;
	propValue.SetObject(&lastRestoreTime);
	if (!SendExpectReadProperty(devObjID, propID, propValue))
	{
		throw("cannot read LAST_RESTORE_TIME from IUT\n");
	}
	m_pOutputDlg->OutMessage("OK");
	// write to the .backupindex file
  char chEnc[256];

  devObjID.Encode(chEnc);
	int nStart = sprintf(buffer, "(%s), ", chEnc);
	maxAPDULenAccepted.Encode(chEnc);
	nStart +=  sprintf(buffer+nStart, "%s, ", chEnc);
	lastRestoreTime.pbacnetTypedValue->Encode(chEnc);
	nStart += sprintf(buffer+nStart, "%s\n", chEnc);
	backupIndexFile.Write(buffer, strlen(buffer));

	m_pOutputDlg->OutMessage("Write to the Device/Backup_Failure_Timeout...", FALSE);
	// use WriteProperty to write to the Device/Backup_Failure_Timeout property
	propID.enumValue = PICS::BACKUP_FAILURE_TIMEOUT;
	BACnetUnsigned backupFailureTimeout(m_Backup_Timeout);
	propValue.SetObject(&backupFailureTimeout);
	if (!SendExpectWriteProperty(devObjID, propID, propValue))
	{
		throw("Write to the Backup_Failure_Timeout is failed!\n");
	}
	m_pOutputDlg->OutMessage("OK");

	// Transmit a ReinitialzeDevice service
	m_pOutputDlg->OutMessage("Transmit a ReinitializeDevice service to start backup...", FALSE);
	if (!SendExpectReinitialize(STARTBACKUP))
	{
		throw("Result(+) is not received in response to the ReinitializeDevice\n");
	}
	m_pOutputDlg->OutMessage("OK");


	if(m_Delay)
	{
		CString str;
		str.Format("Waiting %d seconds", m_Delay);
		m_pOutputDlg->OutMessage((LPCTSTR)str);

		Sleep(m_Delay * 1000);
	}

	// Read array index zero of the Configuration_File property of the Device Object,
	// and store it in a variable named NUM_FILES.
	m_pOutputDlg->OutMessage("Read array index of the ConfigureFile Property...", FALSE);
	propID.enumValue = PICS::CONFIGURATION_FILES;
	BACnetUnsigned numFiles;
	propValue.SetObject(&numFiles);
	if (!SendExpectReadProperty(devObjID, propID, propValue, 0))
	{
		throw("fail to read CONFIGURATION_FILES\n");
	}
	if (numFiles.uintValue <= 0)
	{
		throw("NUM_FILES is not greater than zero\n");
	}
	m_pOutputDlg->OutMessage("OK");

	unsigned long i = 0;
	for(; i < numFiles.uintValue; i++)
	{
		// Read array index I of the Configuration_Files array.
		m_pOutputDlg->OutMessage("Read array index I of the Configure_Files array...", FALSE);
		propID.enumValue = PICS::CONFIGURATION_FILES;
		BACnetObjectIdentifier	fileID;
		propValue.SetObject(&fileID);
		if (!SendExpectReadProperty(devObjID, propID, propValue, i+1))
		{
			throw("fail to read CONFIGURATION_FILES\n");
		}
		// to check if it is a file object
		if (fileID.GetObjectType() != file)
		{
			throw("File object is expected here!\n");
		}
		UINT nFileInstance = fileID.objID & 0x003fffff;
		m_pOutputDlg->OutMessage("OK");

		m_pOutputDlg->OutMessage("Use ReadProperty to get the File_Access_Method...", FALSE);
		propID.enumValue = PICS::FILE_ACCESS_METHOD;
		BACnetEnumerated fileAccessMethod;
		propValue.SetObject(&fileAccessMethod);
		if (!SendExpectReadProperty(fileID, propID, propValue))
		{
			throw("fail to read File/File_Access_Method\n");
		}
		m_pOutputDlg->OutMessage("OK");

		m_pOutputDlg->OutMessage("Use ReadProperty to get the Object_Name...", FALSE);
		propID.enumValue = PICS::OBJECT_NAME;
		BACnetCharacterString	objName;
		propValue.SetObject(&objName);
		if (!SendExpectReadProperty(fileID, propID, propValue))
		{
			throw("fail to read File/Object_Name\n");
		}
		m_pOutputDlg->OutMessage("OK");

		BACnetUnsigned recordCount;
		if (fileAccessMethod.enumValue == PICS::RECORD_ACCESS)
		{
			m_pOutputDlg->OutMessage("Use ReadProperty to get the Record_Count...", FALSE);
			propID.enumValue = PICS::RECORD_COUNT;
			propValue.SetObject(&recordCount);
			if (!SendExpectReadProperty(fileID, propID, propValue))
			{
				throw("fail to read File/Record_Count\n");
			}
			m_pOutputDlg->OutMessage("OK");
		}

		m_pOutputDlg->OutMessage("Use ReadProperty to get the File_Size...", FALSE);
		propID.enumValue = PICS::FILE_SIZE;
		BACnetUnsigned fileSize;
		propValue.SetObject(&fileSize);
		if (!SendExpectReadProperty(fileID, propID, propValue))
		{
			throw("fail to read File/File_Size\n");
		}
		m_pOutputDlg->OutMessage("OK");

		// creata a new empty disk file
		m_pOutputDlg->OutMessage("Create an empty backup data file...", FALSE);
		CString strDataFileName;
		strDataFileName.Format("%s.file%d.backupdata", m_strBackupFileName, nFileInstance);
		m_pOutputDlg->OutMessage("OK");

		// write a new line to the .backupindex file
		// write to the .backupindex file

		m_pOutputDlg->OutMessage("Write to backup index file...", FALSE);
		BACnetUnsigned(nFileInstance).Encode(chEnc);

		nStart = sprintf(buffer, "%s, ", chEnc);
		nStart +=  sprintf(buffer+nStart, "%s, ", NetworkSniffer::BACnetFileAccessMethod[fileAccessMethod.enumValue]);
		fileSize.Encode(chEnc);
		nStart += sprintf(buffer+nStart, "%s, ", chEnc);
		// add record count to index file if and only if RECORD_ACCESS type.
		if (fileAccessMethod.enumValue == PICS::RECORD_ACCESS)
		{
			recordCount.Encode(chEnc);
			nStart += sprintf(buffer+nStart, "%s, ", chEnc);
		}
		objName.Encode(chEnc);
		nStart += sprintf(buffer+nStart, "%s, ", chEnc);
		nStart += sprintf(buffer+nStart, "%s\n", (LPCSTR)strDataFileName);
		backupIndexFile.Write(buffer, strlen(buffer));

		m_pOutputDlg->OutMessage("OK");

		if (fileAccessMethod.enumValue == PICS::RECORD_ACCESS)
		{
			BACnetBoolean	endofFile(fileSize.uintValue == 0 ? TRUE : FALSE);
			UINT nX = 0;
			BACnetOctetString fileRecordData;

			while (!endofFile.boolValue)
			{
				// use AtomicReadFile (Record Access) to read one record from the file
				// in the IUT
				m_pOutputDlg->OutMessage("Use AtomicReadFile to read octets from the file in the IUT...", FALSE);
				BACnetInteger	fileStartRecord(nX);
				if (!SendExpectAtomicReadFile_Record(fileID, fileStartRecord, fileRecordData, endofFile))
				{
					throw("read IUT's configuration file failed\n");
				}
				m_pOutputDlg->OutMessage("OK");
				m_pOutputDlg->OutMessage("Write File record data to the backup data file...", FALSE);

				// Create new file for each record read
				CString strDataFileName_record;
				strDataFileName_record.Format("%s.record%d", strDataFileName, nX);
				CFile backupDataFile_record(strDataFileName_record, CFile::modeWrite|CFile::modeCreate);

				backupDataFile_record.Write(fileRecordData.strBuff, fileRecordData.strLen);
				backupDataFile_record.Close();

				m_pOutputDlg->OutMessage("OK");
				nX++;
			}
		}
		else
		{
			// now open the new file for stream access
			CFile backupDataFile(strDataFileName, CFile::modeWrite|CFile::modeCreate);

			// calculate the Maximun Requested Octet Count
			UINT nMROC;
			if (m_pPort->m_pdevice)
			{
				UINT	nM1 = m_pPort->m_pdevice->m_nMaxAPDUSize;
				UINT	nM2 = maxAPDULenAccepted.uintValue;
				nMROC = min(nM1, nM2) - 21;
			}
			else
			{	// if this port does not bind to a device
				nMROC = maxAPDULenAccepted.uintValue - 21;
			}

			UINT nX = 0;
			while (nX < fileSize.uintValue)
			{
				UINT nROC = min(nMROC, (fileSize.uintValue - nX));
				// Use AtomicReadFile to read octets from the file in the IUT
				m_pOutputDlg->OutMessage("Use AtomicReadFile to read octets from the file in the IUT...", FALSE);
				BACnetOctetString fileData;
				BACnetInteger fileStartPosition(nX);
				BACnetUnsigned ROC(nROC);
				if (!SendExpectAtomicReadFile_Stream(fileID, fileStartPosition, ROC, fileData))
				{
					throw("fail to read one record from the file in the IUT\n");
				}
				m_pOutputDlg->OutMessage("OK");

				// append the octets to the .backupdata disk file
				m_pOutputDlg->OutMessage("Append the octets to the backup index file...", FALSE);
				backupDataFile.Write(fileData.strBuff, fileData.strLen);
				m_pOutputDlg->OutMessage("OK");
				nX += nROC;
			}
			backupDataFile.Close();
		}
	}

	backupIndexFile.Close();
	// close the .backupindex file
	// Transmit a ReinitializeDevice service
	m_pOutputDlg->OutMessage("Transmit a ReinitalizeDevice service...", FALSE);
	if (!SendExpectReinitialize(ENDBACKUP))
	{
		throw("can not end backup process\n");
	}
	m_pOutputDlg->OutMessage("OK");

}



void BakRestoreExecutor::DoRestoreTest()
{
	m_pOutputDlg->OutMessage("Begin restore process...");
	m_pOutputDlg->OutMessage("Open backup index file...", FALSE);
	CString strIndexFileName = m_strBackupFileName;
  strIndexFileName += ".backupindex";
	CStdioFile backupIndexFile;
	if (!backupIndexFile.Open(strIndexFileName, CFile::modeRead))
	{
    CString err;
    err.Format("Can not open %s, maybe this file doesn't exist", (LPCTSTR)strIndexFileName);
		throw((LPCTSTR)err);
	}
	m_pOutputDlg->OutMessage("OK");

	BACnetObjectIdentifier devObjID;
	BACnetUnsigned maxAPDULenAccepted;

	BACnetEnumerated propID;
	AnyValue propValue;
	CString strText="";

	if (m_nDeviceObjInst != DEVICE_OBJ_INST_NUM_UNSET)
	{
		// Use ReadProperty to set M2 = Max_APDU_Length
		m_pOutputDlg->OutMessage("Use ReadProperty to set M2 = Max_APDU_Length...", FALSE);
		devObjID.SetValue(device, m_nDeviceObjInst);
		propID.enumValue = PICS::MAX_APDU_LENGTH_ACCEPTED;
		propValue.SetObject(&maxAPDULenAccepted);
		if (!SendExpectReadProperty(devObjID, propID, propValue))
		{
			throw("Can not read MAX_APDU_LENGTH_ACCEPTED value in IUT device object\n");
		}
		m_pOutputDlg->OutMessage("OK");
	}
	else
	{
		m_pOutputDlg->OutMessage("Transmit a a unicast Who-Is message to the IUT's address...", FALSE);
		if (!SendExpectWhoIs(devObjID, maxAPDULenAccepted))
		{
			m_pOutputDlg->OutMessage("Failed");
			m_pOutputDlg->OutMessage("Transmit a ReadProperty to read the real Device objID...", FALSE);
			BACnetObjectIdentifier objID(device, 4194303);
			BACnetEnumerated propID(PICS::OBJECT_IDENTIFIER);
			propValue.SetObject(&devObjID);
			if (!SendExpectReadProperty(objID, propID, propValue))
			{
				// if failed received packet, the test failed
				throw("Dynamic discovery of the Device ID has failed!\n"
					"the user must specify a Device Instance number in the Backup/Restore dialog\n");
			}
			propID.enumValue = PICS::MAX_APDU_LENGTH_ACCEPTED;
			propValue.SetObject(&maxAPDULenAccepted);
			if (!SendExpectReadProperty(devObjID, propID, propValue))
			{
				throw("Can not read MAX_APDU_LENGTH_ACCEPTED\n");
			}
			m_pOutputDlg->OutMessage("OK");
		}
		else
		{
			m_pOutputDlg->OutMessage("OK");
		}
	}

	// Read first line of the .backupindex file, do not used during the test
	m_pOutputDlg->OutMessage("Read the first line of the .backupindex file", FALSE);
	backupIndexFile.ReadString(strText);
	m_pOutputDlg->OutMessage("OK");

	// Transmit a ReinitialzeDevice service
	m_pOutputDlg->OutMessage("Transmit a ReinitializeDevice service to start restore...", FALSE);
	if (!SendExpectReinitialize(STARTRESTORE))
	{
		throw("Result(+) is not received in response to the ReinitializeDevice\n");
	}
	m_pOutputDlg->OutMessage("OK");


    if(m_Delay)
	{
      CString str;
      str.Format("Waiting %d seconds", m_Delay);
      m_pOutputDlg->OutMessage((LPCTSTR)str);

      Sleep(m_Delay * 1000);
	}

	m_pOutputDlg->OutMessage("Use ReadProperty to read the Device/Object_List...", FALSE);
	BACnetArrayOf<BACnetObjectIdentifier>	objList;
//	int nType = objList.DataType();
	propID.enumValue = PICS::OBJECT_LIST;
	// suppose both devices do not support segmentation
	// First to determine how many elements are in the list
	BACnetUnsigned numOfObjects;
	propValue.SetObject(&numOfObjects);
	if (!SendExpectReadProperty(devObjID, propID, propValue, 0))
	{
		throw("can not determine how many objects in Device/Object_List\n");
	}
	for(UINT i = 0; i < numOfObjects.uintValue; i++)
	{
		BACnetObjectIdentifier objID;
		propValue.SetObject(&objID);
		if (!SendExpectReadProperty(devObjID, propID, propValue, i+1))
		{
			throw("can not read in Device/Object_List\n");
		}
		objList.AddElement(&objID);
	}
	m_pOutputDlg->OutMessage("OK");

	while (backupIndexFile.ReadString(strText))
	{
		// parse the string
		CString	strToken = "";
		strText.Remove(' ');	// remove whitespace
		int nPos1 = strText.Find(',');  //After instance number
		strToken = strText.Left(nPos1++);
		UINT nFileInstance = atoi(strToken);
		int nPos2 = strText.Find(',', nPos1);   //After Access type
		strToken = strText.Mid(nPos1, nPos2 - nPos1);
		BACnetEnumerated fileAccessMethod;
		if (strToken == "RECORD-ACCESS")
		{
			fileAccessMethod.enumValue = PICS::RECORD_ACCESS;
		}
		else if (strToken == "STREAM-ACCESS")
		{
			fileAccessMethod.enumValue = PICS::STREAM_ACCESS;
		}
		else
		{
			throw("Incorrect file access method in backup data file.\n"
				  "Only record_access and stream_access\n");
		}
		nPos1 = nPos2 + 1;

		nPos2 = strText.Find(',', nPos1);       //After file size
		strToken = strText.Mid(nPos1, nPos2 - nPos1);
		BACnetUnsigned fileSize(atoi(strToken));
        nPos1 = nPos2 + 1;

		BACnetUnsigned recordCount;
		if (fileAccessMethod.enumValue == PICS::RECORD_ACCESS)
		{
			nPos2 = strText.Find(',', nPos1);       //After record_count
			strToken = strText.Mid(nPos1, nPos2 - nPos1);
			recordCount.uintValue = atoi(strToken);
			nPos1 = nPos2 + 1;
		}

        nPos2 = strText.Find(',', nPos1);       //After object name
        strToken = strText.Mid(nPos1, nPos2 - nPos1);

        BACnetCharacterString objName;
        objName.Decode( strToken );

		strToken = strText.Right(strText.GetLength() - nPos2 - 1);  //Data file is remaining
		CString strDataFileName(strToken);

		int nCount = objList.GetItemCount();
		BOOL bFind = FALSE;
		for (int i = 0; i < nCount; i++)
		{
			UINT nInstanceNum = (objList.GetElement(i)->objID) & 0x003fffff ;
			if (nFileInstance == nInstanceNum)
			{
				bFind = TRUE;
				break;
			}
		}

		BACnetObjectIdentifier	fileID(file, nFileInstance);
		if (bFind)
		{
			if (fileAccessMethod.enumValue == PICS::RECORD_ACCESS)
			{
				m_pOutputDlg->OutMessage("Use ReadProperty to read the Record_Count property...", FALSE);
				propID.enumValue = PICS::RECORD_COUNT;
				BACnetUnsigned	recordCountIUT;
				propValue.SetObject(&recordCountIUT);
				if (!SendExpectReadProperty(fileID, propID, propValue))
				{
					throw("unable to read File/RECORD_COUNT\n");
				}
				m_pOutputDlg->OutMessage("OK");
 				if (recordCount.uintValue != recordCountIUT.uintValue)
				{
					// use write_property to set the Record_Count to zero
					m_pOutputDlg->OutMessage("Use WriteProperty to set the Record_Count to zero...", FALSE);
					propID.enumValue = PICS::RECORD_COUNT;
					BACnetUnsigned temp(0);
					propValue.SetObject(&temp);
					if (!SendExpectWriteProperty(fileID, propID, propValue))
					{
						throw("can not write File/Record_Count to zero\n");
					}
					m_pOutputDlg->OutMessage("OK");
				}
			}
			else
			{
				m_pOutputDlg->OutMessage("Use ReadProperty to read the File_Size property...", FALSE);
				propID.enumValue = PICS::FILE_SIZE;
				BACnetUnsigned	fileSizeIUT;
				propValue.SetObject(&fileSizeIUT);
				if (!SendExpectReadProperty(fileID, propID, propValue))
				{
					throw("unable to read File/FILE_SIZE\n");
				}
				m_pOutputDlg->OutMessage("OK");
 				if ((fileSizeIUT.uintValue != 0) && (fileSize.uintValue != fileSizeIUT.uintValue))
				{
					// use write_property to set the File_Size to zero
					m_pOutputDlg->OutMessage("Use WriteProperty to set the File_Size to zero...", FALSE);
					propID.enumValue = PICS::FILE_SIZE;
					BACnetUnsigned temp(0);
					propValue.SetObject(&temp);
					if (!SendExpectWriteProperty(fileID, propID, propValue))
					{
						throw("can not write File/File_Size to zero\n");
					}
					m_pOutputDlg->OutMessage("OK");

				}
			}
		}
		else
		{	// use the CreateObject service to create (File, FILE_INSTANCE) in the IUT
			m_pOutputDlg->OutMessage("Use the CreateObject service to create (File, File_Instance)"
				"in the IUT...", FALSE);
			BACnetSequenceOf<PropertyValue>	listOfInitialValues;
			listOfInitialValues.AddElement(
				&PropertyValue(BACnetEnumerated(PICS::OBJECT_NAME), objName)
				);
			// Note: The JCI version of fixes removed the following two elements from
			//       the create object list.  Does this make it more interoperable to do
			//       this?  I chose to leave these values in for now.  11/21/2006 LJT
			//listOfInitialValues.AddElement(
			//	&PropertyValue(BACnetEnumerated(PICS::FILE_ACCESS_METHOD), fileAccessMethod)
			//	);
			//listOfInitialValues.AddElement(
			//	&PropertyValue(BACnetEnumerated(PICS::FILE_SIZE), fileSize)
			//	);

			if (!SendExpectCreatObject(fileID, listOfInitialValues)) 
			{
				throw("Unable to create File object in the IUT\n");
			}
			m_pOutputDlg->OutMessage("OK");
		}

		m_pOutputDlg->OutMessage("Open the backup data file...", FALSE);

		if (fileAccessMethod.enumValue == PICS::RECORD_ACCESS)
		{
			CStdioFile backupDataFile_record;

			UINT nX = 0;

			CString strDataFileName_record;
			for ( int rcount = 0; rcount < recordCount.uintValue; rcount++)
			{
				strDataFileName_record.Format("%s.record%d", strDataFileName, nX);
				if (!backupDataFile_record.Open(strDataFileName_record, CFile::modeRead | CFile::typeBinary))
				{
					throw("Can not open \".backupData\" file, maybe this file doesn't exist\n");
				}
				m_pOutputDlg->OutMessage("OK");
	
				// create a buffer max size TD's APDU size
				// LJT: Note we assume here that the Record will fit in the MaxAPDU size and
				//      therefore we create our buffer to the MaxAPDU size.
				UINT nMWOC;
				if (m_pPort->m_pdevice) 
				{
					UINT	nM1 = m_pPort->m_pdevice->m_nMaxAPDUSize;
					UINT	nM2 = maxAPDULenAccepted.uintValue;
					nMWOC = min(nM1, nM2); // - 21 + 3 //(octetstring header);				
				}
				else	
				{	// if this port does not bind to a device
					nMWOC = maxAPDULenAccepted.uintValue; // - 21 + 3 // (octetstring header);
				}

				BYTE* pBuffer = new BYTE[nMWOC];
				int len_read = backupDataFile_record.Read(pBuffer, nMWOC);

				m_pOutputDlg->OutMessage("Use AtomicWriteFile to write one record to (File, File_Instance)"
						"in the IUT...", FALSE);
				BACnetInteger	fileStartRecord(nX);
				BACnetOctetString fileRecordData(pBuffer, len_read);
				if (!SendExpectAtomicWriteFile_Record(fileID, fileStartRecord, fileRecordData))
				{
					throw("Unable to write one record to FILE in the IUT\n");
				}
				m_pOutputDlg->OutMessage("OK");
//				delete pBuff;
				delete[] pBuffer;
				nX++;

				backupDataFile_record.Close();
			}
		}
		else
		{
			CFile backupDataFile;
			if (!backupDataFile.Open(strDataFileName, CFile::modeRead))
			{
			  throw("Can not open \".backupData\" file, maybe this file doesn't exist\n");
			}
			m_pOutputDlg->OutMessage("OK");
			// calculate the Maximun Requested Octet Count
			UINT nMWOC;
			if (m_pPort->m_pdevice)
			{
				UINT	nM1 = m_pPort->m_pdevice->m_nMaxAPDUSize;
				UINT	nM2 = maxAPDULenAccepted.uintValue;
				nMWOC = min(nM1, nM2) - 21;
			}
			else
			{	// if this port does not bind to a device
				nMWOC = maxAPDULenAccepted.uintValue - 21;
			}

			UINT nX = 0;
			BYTE* pBuffer = new BYTE[nMWOC];
			ULONGLONG dwBytesRemaining = backupDataFile.GetLength();
			while (dwBytesRemaining)
			{
				m_pOutputDlg->OutMessage("Use AtomicWriteFile to write octets to (File, File_Instance)"
					"in the IUT...", FALSE);
				backupDataFile.Seek((LONGLONG)nX, CFile::begin);
				UINT nWOC = backupDataFile.Read(pBuffer, nMWOC);
				BACnetInteger fileStartPosition(nX);
				BACnetOctetString fileData(pBuffer, nWOC);
				if (!SendExpectAtomicWriteFile_Stream(fileID, fileStartPosition, fileData))
				{
					throw("Unable to write File Data to IUT\n");
				}
				m_pOutputDlg->OutMessage("OK");
				nX += nWOC;
				dwBytesRemaining -= (ULONGLONG)nWOC;
			}
			delete[] pBuffer;
		}
	}

	// close the .backupindex file
	m_pOutputDlg->OutMessage("Transmit a ReinitializeDevice service to stop restore...", FALSE);
	if (!SendExpectReinitialize(ENDRESTORE))
	{
		throw("can not end restore process\n");
	}
	m_pOutputDlg->OutMessage("OK");

}



void BakRestoreExecutor::DoAuxiliaryTest()
{
	BACnetObjectIdentifier devObjID;
	BACnetUnsigned maxAPDULenAccepted;

	BACnetEnumerated propID;
	AnyValue propValue;

	m_pOutputDlg->OutMessage("Begin auxiliary backup and restore processes");
	if (m_nDeviceObjInst != DEVICE_OBJ_INST_NUM_UNSET)
	{
		devObjID.SetValue(device, m_nDeviceObjInst);
	}
	else
	{
		m_pOutputDlg->OutMessage("Transmit a a unicast Who-Is message to the IUT's address...", FALSE);
		if (!SendExpectWhoIs(devObjID, maxAPDULenAccepted))
		{
			m_pOutputDlg->OutMessage("Failed");
			m_pOutputDlg->OutMessage("Transmit a ReadProperty to read the real Device objID...", FALSE);
			BACnetObjectIdentifier objID(device, 4194303);
			BACnetEnumerated propID(PICS::OBJECT_IDENTIFIER);
			propValue.SetObject(&devObjID);
			if (!SendExpectReadProperty(objID, propID, propValue))
			{
				// if failed received packet, the test failed
				throw("Dynamic discovery of the Device ID has failed!\n"
					"the user must specify a Device Instance number in the Backup/Restore dialog\n");
			}
			propID.enumValue = PICS::MAX_APDU_LENGTH_ACCEPTED;
			propValue.SetObject(&maxAPDULenAccepted);
			if (!SendExpectReadProperty(devObjID, propID, propValue))
			{
				throw("Can not read MAX_APDU_LENGTH_ACCEPTED\n");
			}
			m_pOutputDlg->OutMessage("OK");
		}
		else
		{
			m_pOutputDlg->OutMessage("OK");
		}
	}

//	DoAuxiliaryTest_1();
	DoAuxiliaryTest_2();
//	DoAuxiliaryTest_3();
	DoAuxiliaryTest_4();
	DoAuxiliaryTest_5(devObjID);
	DoAuxiliaryTest_6(devObjID);
	DoAuxiliaryTest_7();
	DoAuxiliaryTest_8();
	DoAuxiliaryTest_9(devObjID);
	DoAuxiliaryTest_10(devObjID);
	DoAuxiliaryTest_11(devObjID);
	DoAuxiliaryTest_12(devObjID);

}

/*
//Initiaing a backup procedure while already performing a backup procedure
void BakRestoreExecutor::DoAuxiliaryTest_1()
{
	if (!SendExpectReinitialize(STARTBACKUP))
	{
		throw("Can not start Backup procedure in the IUT");
	}

	// try to find out another active port, which bind to different network number and
	// different MAC address. Use this port to simulate a different client.
	VTSDocPtr pVTSdoc = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();
	VTSPorts * pPorts = pVTSdoc == NULL ? NULL : pVTSdoc->GetPorts();
	VTSPort * pPort;
	BOOL bFind = FALSE;

	for ( int i = 0; i < pPorts->GetSize(); i++ )
	{
		pPort = ((VTSPort *) pPorts->GetAt(i));
		if (pPort != m_pPort
			&& pPort->IsEnabled()
			&& (pPort->m_nNet >= 0)
			&& (pPort->m_nPortType == m_pPort->m_nPortType))
		{
			bFind = TRUE;
			break;
		}
	}
	if (!bFind)
	{	// can not find another active port, this test needs a different devcie
		Msg("Warning: Cannot find a different active port. This test has been skipped!");
		return;
	}

	BACnetAddress IUTAddrTemp(m_IUTAddr);
	BACnetAddress routerAddrTemp(m_routerAddr);
	VTSPortPtr	pPortTemp = m_pPort;

	m_pPort = pPort;
	SetIUTAddress(pPort, m_pName);
	if (m_IUTAddr.addrType == remoteStationAddr)
	{
		FindRouterAddress();
	}

	BACnetEnumerated	errorClass;
	BACnetEnumerated	errorCode;
	if (!SendExpectReinitializeNeg(STARTBACKUP, errorClass, errorCode))
	{
		throw("Expect BACnetErrorAPDU\n");
	}

	if (errorClass.enumValue != ErrorClass::DEVICE)
	{
		throw("Wrong Error Class\n");
	}

	if (errorCode.enumValue != ErrorCode::CONFIGURATION_IN_PROGRESS)
	{
		throw("Wrong Error Code\n");
	}

	// rollback
	m_pPort = pPortTemp;
	m_IUTAddr = IUTAddrTemp;
	m_routerAddr = routerAddrTemp;

}*/


// Initiating a backup procedure while already performing a restore procedure
void BakRestoreExecutor::DoAuxiliaryTest_2()
{
	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start restore...", FALSE);
	if (!SendExpectReinitialize(STARTRESTORE))
	{
		throw("Unable to initialize a restore procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

  if(m_Delay)
  {
    CString str;
    str.Format("Waiting %d seconds", m_Delay);
    m_pOutputDlg->OutMessage((LPCTSTR)str);

    Sleep(m_Delay * 1000);
  }

	m_pOutputDlg->OutMessage("Transmit Reinitialize_Request to start backup...", FALSE);
	BACnetEnumerated	errorClass;
	BACnetEnumerated	errorCode;
	if (!SendExpectReinitializeNeg(STARTBACKUP, errorClass, errorCode))
	{
		throw("Expect BACnetErrorAPDU\n");
	}
	m_pOutputDlg->OutMessage("OK");

	if (errorClass.enumValue != ErrorClass::DEVICE)
	{
		throw("Wrong Error Class\n");
	}

	if (errorCode.enumValue != ErrorCode::CONFIGURATION_IN_PROGRESS)
	{
		throw("Wrong Error Code\n");
	}

  m_pOutputDlg->OutMessage("Transmit ABORTSTORE...", FALSE);
  if (!SendExpectReinitialize(ABORTSTORE))
	{
		throw("Unable to abot a restore procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

}

/*
// Initiating a restore procedure while already performing a backup procedure
void BakRestoreExecutor::DoAuxiliaryTest_3()
{
	if (!SendExpectReinitialize(STARTBACKUP))
	{
		throw("Can not start Backup procedure in the IUT");
	}

	// try to find out another active port, which bind to different network number and
	// different MAC address. Use this port to simulate a different client.
	VTSDocPtr pVTSdoc = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();
	VTSPorts * pPorts = pVTSdoc == NULL ? NULL : pVTSdoc->GetPorts();
	VTSPort * pPort;
	BOOL bFind = FALSE;

	for ( int i = 0; i < pPorts->GetSize(); i++ )
	{
		pPort = ((VTSPort *) pPorts->GetAt(i));
		if (pPort != m_pPort
			&& pPort->IsEnabled()
			&& (pPort->m_nNet >= 0)
			&& (pPort->m_nPortType == m_pPort->m_nPortType))
		{
			bFind = TRUE;
			break;
		}
	}
	if (!bFind)
	{	// can not find another active port, this test needs a different devcie
		Msg("Warning: Cannot find a different active port. This test has been skipped!");
		return;
	}

	BACnetAddress IUTAddrTemp(m_IUTAddr);
	BACnetAddress routerAddrTemp(m_routerAddr);
	VTSPortPtr	pPortTemp = m_pPort;

	m_pPort = pPort;
	SetIUTAddress(pPort, m_pName);
	if (m_IUTAddr.addrType == remoteStationAddr)
	{
		FindRouterAddress();
	}

	BACnetEnumerated	errorClass;
	BACnetEnumerated	errorCode;
	if (!SendExpectReinitializeNeg(STARTRESTORE, errorClass, errorCode))
	{
		throw("Expect BACnetErrorAPDU\n");
	}

	if (errorClass.enumValue != ErrorClass::DEVICE)
	{
		throw("Wrong Error Class\n");
	}

	if (errorCode.enumValue != ErrorCode::CONFIGURATION_IN_PROGRESS)
	{
		throw("Wrong Error Code\n");
	}

	// rollback
	m_pPort = pPortTemp;
	m_IUTAddr = IUTAddrTemp;
	m_routerAddr = routerAddrTemp;

}
*/

// Initiating a restore procedure while already performing a restore procedure
void BakRestoreExecutor::DoAuxiliaryTest_4()
{
	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start restore...", FALSE);
	if (!SendExpectReinitialize(STARTRESTORE))
	{
		throw("Unable to initialize a restore procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Transmit Reinitalize-Request to start restore...", FALSE);
	BACnetEnumerated	errorClass;
	BACnetEnumerated	errorCode;

  if(m_Delay)
  {
    CString str;
    str.Format("Waiting %d seconds", m_Delay);
    m_pOutputDlg->OutMessage((LPCTSTR)str);

    Sleep(m_Delay * 1000);
  }

	if (!SendExpectReinitializeNeg(STARTRESTORE, errorClass, errorCode))
	{
		throw("Expect BACnetErrorAPDU\n");
	}
	m_pOutputDlg->OutMessage("OK");

	if (errorClass.enumValue != ErrorClass::DEVICE)
	{
		throw("Wrong Error Class\n");
	}

	if (errorCode.enumValue != ErrorCode::CONFIGURATION_IN_PROGRESS)
	{
		throw("Wrong Error Code\n");
	}

  m_pOutputDlg->OutMessage("Transmit ABORTSTORE...", FALSE);
  if (!SendExpectReinitialize(ABORTSTORE))
	{
		throw("Unable to abort a restore procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

}

// Ending backup and restore procedure via timeout
void BakRestoreExecutor::DoAuxiliaryTest_5(BACnetObjectIdentifier& devObjID)
{
	m_pOutputDlg->OutMessage("Transmit WriteProperty-Request to wrtie 30 to Backup_Failure_Timeout...", FALSE);
	BACnetEnumerated propID(PICS::BACKUP_FAILURE_TIMEOUT);
	BACnetUnsigned	backupFailureTimeout(30);
	AnyValue propValue;
	propValue.SetObject(&backupFailureTimeout);
	if (!SendExpectWriteProperty(devObjID, propID, propValue))
	{
		throw("Can not write Backup_Failure_Timeout to the IUT");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Verify Backup_Failure_Timeout = 30...", FALSE);
	if (!SendExpectReadProperty(devObjID, propID, propValue))
	{
		throw("Can not read Backup_Failure_Timeout in the IUT");
	}
	if (backupFailureTimeout.uintValue != 30)
	{
		throw("Verify Backup_Failure_Timeout in the IUT failed");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start backup...", FALSE);
	if (!SendExpectReinitialize(STARTBACKUP))
	{
		throw("Unable to initialize a backup procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Wait 40 seconds");
	Sleep(40000);

	m_pOutputDlg->OutMessage("Verify System_Status != BACKUP_IN_PROGRESS...", FALSE);
	BACnetEnumerated	systemStatus;
	propID.enumValue = PICS::SYSTEM_STATUS;
	propValue.SetObject(&systemStatus);
	if (!SendExpectReadProperty(devObjID, propID, propValue))
	{
		throw("Can not read System_Status in the IUT\n");
	}
	if (systemStatus.enumValue == PICS::BACKUP_IN_PROGRESS)
	{
		throw("The System_Status in the IUT's Device Object is still BACKUP_IN_PROGRESS\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start restore...", FALSE);
	if (!SendExpectReinitialize(STARTRESTORE))
	{
		throw("Unable to initialize a backup procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Wait 40 seconds");
	Sleep(40000);

	m_pOutputDlg->OutMessage("Verify System_STatus != DOWNLOAD_IN_PROGRESS...", FALSE);
	if (!SendExpectReadProperty(devObjID, propID, propValue))
	{
		throw("Can not read System_Status in the IUT\n");
	}
	if (systemStatus.enumValue == PICS::DOWNLOAD_IN_PROGRESS)
	{
		throw("The System_Status in the IUT's Device Object is still DOWNLOAD_IN_PROGRESS\n");
	}
	m_pOutputDlg->OutMessage("OK");

}


// Ending backup and restore procedures via abort
void BakRestoreExecutor::DoAuxiliaryTest_6(BACnetObjectIdentifier& devObjID)
{
	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start backup...", FALSE);
	if (!SendExpectReinitialize(STARTBACKUP))
	{
		throw("Unable to initialize a backup procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to end backup...", FALSE);
	if (!SendExpectReinitialize(ENDBACKUP))
	{
		throw("Unable to end a backup procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Verify System_Status != BACKUP_IN_PROGRESS...", FALSE);
	BACnetEnumerated	systemStatus;
	BACnetEnumerated	propID(PICS::SYSTEM_STATUS);
	AnyValue	propValue;
	propValue.SetObject(&systemStatus);
	if (!SendExpectReadProperty(devObjID, propID, propValue))
	{
		throw("Can not read System_Status in the IUT\n");
	}
	if (systemStatus.enumValue == PICS::BACKUP_IN_PROGRESS)
	{
		throw("The System_Status in the IUT's Device Object is still BACKUP_IN_PROGRESS\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start restore...", FALSE);
	if (!SendExpectReinitialize(STARTRESTORE))
	{
		throw("Unable to initialize a backup procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

  if(m_Delay)
  {
    CString str;
    str.Format("Waiting %d seconds", m_Delay);
    m_pOutputDlg->OutMessage((LPCTSTR)str);

    Sleep(m_Delay * 1000);
  }

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to abort restore...", FALSE);
	if (!SendExpectReinitialize(ABORTSTORE))
	{
		throw("Unable to abort a backup procedure\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Verify System_Status != DOWNLOAD_IN_PROGRESS...", FALSE);
	if (!SendExpectReadProperty(devObjID, propID, propValue))
	{
		throw("Can not read System_Status in the IUT\n");
	}
	if (systemStatus.enumValue == PICS::DOWNLOAD_IN_PROGRESS)
	{
		throw("The System_Status in the IUT's Device Object is still DOWNLOAD_IN_PROGRESS\n");
	}
	m_pOutputDlg->OutMessage("OK");

}

// Initiating a backup procedure with an invalid password
void BakRestoreExecutor::DoAuxiliaryTest_7()
{
	if (m_strPassword.IsEmpty())
	{
		Msg("no password is required, this test has been omitted\n");
		return;		// do not need carry on this test
	}

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start restore with invalid password...", FALSE);
	CString strPasswordTemp(m_strPassword);

	m_strPassword += "123";		// to create a invalid password
	BACnetEnumerated	errorClass;
	BACnetEnumerated	errorCode;
	if (!SendExpectReinitializeNeg(STARTBACKUP, errorClass, errorCode))
	{
		m_strPassword = strPasswordTemp;
		throw("Expect BACnetErrorAPDU\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Verify received Error PDU...", FALSE);
	if ((errorClass.enumValue != ErrorClass::SECURITY) ||
		(errorCode.enumValue != ErrorCode::PASSWORD_FAILURE))
	{
		m_strPassword = strPasswordTemp;
		throw("Wrong Error Class\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_strPassword = strPasswordTemp;
}

// Initiating a restore procedure with an invalid password
void BakRestoreExecutor::DoAuxiliaryTest_8()
{
	if (m_strPassword.IsEmpty())
	{
		Msg("no password is required, this test has been omitted\n");
		return;		// do not need carry on this test
	}

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start restore with invalid password...", FALSE);
	CString strPasswordTemp(m_strPassword);

	m_strPassword += "123";		// to create a invalid password
	BACnetEnumerated	errorClass;
	BACnetEnumerated	errorCode;
	if (!SendExpectReinitializeNeg(STARTRESTORE, errorClass, errorCode))
	{
		m_strPassword = strPasswordTemp;
		throw("Expect BACnetErrorAPDU\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Verify received Error PDU...", FALSE);
	if ((errorClass.enumValue != ErrorClass::SECURITY) ||
		(errorCode.enumValue != ErrorCode::PASSWORD_FAILURE))
	{
		m_strPassword = strPasswordTemp;
		throw("Wrong Error Class\n");
	}
	m_pOutputDlg->OutMessage("OK");

	m_strPassword = strPasswordTemp;
}

// Initiating and ending a backup procedure when a password is not required
void BakRestoreExecutor::DoAuxiliaryTest_9(BACnetObjectIdentifier& devObjID)
{
	if (!m_strPassword.IsEmpty())
	{
		Msg("A specific password is required, this test has been omitted\n");
		return;		// do not need carry on this test
	}

	CString strPasswordTemp(m_strPassword);
	m_strPassword = "123";		// any non-zero length password

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start backup with any password...", FALSE);
	if (!SendExpectReinitialize(STARTBACKUP))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not start Backup procedure");
		m_strPassword = strPasswordTemp;
		return;
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to end backup with any password...", FALSE);
	if (!SendExpectReinitialize(ENDBACKUP))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not end Backup procedure");
		m_strPassword = strPasswordTemp;
		return;
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Verify System_Status != BACKUP_IN_PROGRESS...", FALSE);
	BACnetEnumerated systemStatus;
	AnyValue value;
	value.SetObject(&systemStatus);
	BACnetEnumerated propID(PICS::SYSTEM_STATUS);
	if (!SendExpectReadProperty(devObjID, propID, value))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not read Device/System_Status in the IUT");
		m_strPassword = strPasswordTemp;
		return;
	}

	if (systemStatus.enumValue == PICS::BACKUP_IN_PROGRESS)
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("After the backup procedure, the system_statue should not still be BACKUP_IN_PROGRESS");
		m_strPassword = strPasswordTemp;
		return;
	}

	m_pOutputDlg->OutMessage("OK");
	m_strPassword = strPasswordTemp;
}

// Initiating and ending a restore procedure when a password is not required
void BakRestoreExecutor::DoAuxiliaryTest_10(BACnetObjectIdentifier& devObjID)
{
	if (!m_strPassword.IsEmpty())
	{
		Msg("A specific password is required, this test has been omitted\n");
		return;		// do not need carry on this test
	}

	CString strPasswordTemp(m_strPassword);
	m_strPassword = "123";

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to start restore with any password...", FALSE);
	if (!SendExpectReinitialize(STARTRESTORE))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not start Backup procedure");
		m_strPassword = strPasswordTemp;
		return;
	}
	m_pOutputDlg->OutMessage("OK");

  if(m_Delay)
  {
    CString str;
    str.Format("Waiting %d seconds", m_Delay);
    m_pOutputDlg->OutMessage((LPCTSTR)str);

    Sleep(m_Delay * 1000);
  }

	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice-Request to end restore...", FALSE);
	if (!SendExpectReinitialize(ENDRESTORE))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not end Backup procedure");
		m_strPassword = strPasswordTemp;
		return;
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Verify system_status != DOWNLOAD_IN_PROGRESS");
	BACnetEnumerated systemStatus;
	AnyValue value;
	value.SetObject(&systemStatus);
	BACnetEnumerated propID(PICS::SYSTEM_STATUS);
	if (!SendExpectReadProperty(devObjID, propID, value))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not read Device/System_Status in the IUT");
		m_strPassword = strPasswordTemp;
		return;
	}

	if (systemStatus.enumValue == PICS::DOWNLOAD_IN_PROGRESS)
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("After the restore procedure, the system_statue should not still be DOWNLOAD_IN_PROGRESS");
		m_strPassword = strPasswordTemp;
		return;
	}
	m_pOutputDlg->OutMessage("OK");

	m_strPassword = strPasswordTemp;
}


// System_Status during a backup procedure
void BakRestoreExecutor::DoAuxiliaryTest_11(BACnetObjectIdentifier& devObjID)
{
	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice request to start backup...", FALSE);
	if (!SendExpectReinitialize(STARTBACKUP))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not start Backup procedure");
		return;
	}
	m_pOutputDlg->OutMessage("OK");

	m_pOutputDlg->OutMessage("Verify system_status = BACKUP_IN_PROGRESS");
	BACnetEnumerated systemStatus;
	AnyValue value;
	value.SetObject(&systemStatus);
	BACnetEnumerated propID(PICS::SYSTEM_STATUS);
	if (!SendExpectReadProperty(devObjID, propID, value))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not read Device/System_Status in the IUT");
		return;
	}

	if (systemStatus.enumValue != PICS::BACKUP_IN_PROGRESS)
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("The system_statue should be BACKUP_IN_PROGRESS during the backup procedure");
		return;
	}
	m_pOutputDlg->OutMessage("OK");

  m_pOutputDlg->OutMessage("Transmit a ReinitalizeDevice service to End Backup...", FALSE);
	if (!SendExpectReinitialize(ENDBACKUP))
	{
		throw("can not end backup process\n");
	}
	m_pOutputDlg->OutMessage("OK");

}


// System_Status during a restore procedure
void BakRestoreExecutor::DoAuxiliaryTest_12(BACnetObjectIdentifier& devObjID)
{
	m_pOutputDlg->OutMessage("Transmit ReinitializeDevice request to start backup...", FALSE);
	if (!SendExpectReinitialize(STARTRESTORE))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not start Restore procedure");
		return;
	}
	m_pOutputDlg->OutMessage("OK");

  if(m_Delay)
  {
    CString str;
    str.Format("Waiting %d seconds", m_Delay);
    m_pOutputDlg->OutMessage((LPCTSTR)str);

    Sleep(m_Delay * 1000);
  }

	m_pOutputDlg->OutMessage("Verify system_status = DOWNLOAD_IN_PROGRESS");
	BACnetEnumerated systemStatus;
	AnyValue value;
	value.SetObject(&systemStatus);
	BACnetEnumerated propID(PICS::SYSTEM_STATUS);
	if (!SendExpectReadProperty(devObjID, propID, value))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not read Device/System_Status in the IUT");
		return;
	}

	if (systemStatus.enumValue != PICS::DOWNLOAD_IN_PROGRESS)
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("The system_statue should be DOWNLOAD_IN_PROGRESS during the download procedure");
		return;
	}
	m_pOutputDlg->OutMessage("OK, Sending an Abort Restore ...", FALSE);

  if (!SendExpectReinitialize(ABORTSTORE))
	{
		m_pOutputDlg->OutMessage("Failed");
		Msg("Can not start Backup procedure");
		return;
	}

  m_pOutputDlg->OutMessage("OK");

}

void BakRestoreExecutor::ReceiveNPDU(const BACnetNPDU& npdu)
{
	// if were not running, just toss the message
	if (m_execState != execRunning || !m_bExpectPacket)
		return;

	// make a copy of the data
	BACnetOctet packetData[MAX_NPDU_LENGTH];
	memcpy( packetData, npdu.pduData, npdu.pduLen );
	// filter the packet

	// the rest of this code will need a decoder
	BACnetAPDUDecoder	dec( packetData, npdu.pduLen );
	// Decode the packet
	if (0x81 == *dec.pktBuffer)
	{
		(dec.pktLength--,*dec.pktBuffer++);	// fix by Trevor from Delta 1/30/2008
		// decode BVLCI
		int nBVLCfunc = (dec.pktLength--,*dec.pktBuffer++);
		if ((nBVLCfunc != 4) && (nBVLCfunc != 9)
			&& (nBVLCfunc != 10) && (nBVLCfunc != 11))
			return;		// do not contain NPDU, it is obvious that it isn't the packet which we want
		// verify the length
		int len;
		len = (dec.pktLength--,*dec.pktBuffer++);
		len = (len << 8) + (dec.pktLength--,*dec.pktBuffer++);
		if (len != npdu.pduLen)
			return;		// "BVLCI length incorrect";
		if (nBVLCfunc == 4)
		{
			dec.pktLength -= 6;
			dec.pktBuffer += 6;
		}
	}

	(dec.pktLength--, dec.pktBuffer++);		// version
	BACnetOctet ctrl =  (dec.pktLength--, *dec.pktBuffer++);
	if (m_bExpectAPDU && (ctrl & 0x80))
	{
		return;		// Not an application message and APDU is expect
	}
	if ((ctrl & 0x40) || (ctrl & 0x10))
		return;		// 6th and 4th bit should be 0


	if (ctrl & 0x20)	// has DNET
	{
		dec.pktLength -= 2;		// DNET
		dec.pktBuffer += 2;
		int len = (dec.pktLength--, *dec.pktBuffer++);	// DLEN
		if (len == 0)
			return;		// No DADR in packet
		dec.pktLength -= len;		// DADR
		dec.pktBuffer += len;
	}

	if (ctrl & 0x08)
	{
		dec.pktLength -= 2;		// SNET
		dec.pktBuffer += 2;
		int len = (dec.pktLength--, *dec.pktBuffer++);	// SNET
		if (len == 0)
			return;		// No SADR in packet
		dec.pktLength -= len;		// SADR
		dec.pktBuffer += len;
	}

	if (ctrl & 0x20)
	{
		(dec.pktLength--, dec.pktBuffer++);		// hop count
	}

	if (!m_bExpectAPDU && (ctrl & 0x80))		// Expect NPDU
	{
		// During the backup and restore test, only two kinds of network messages are possible
		// received: I-Am-Router-To-Network and I-Could-Be-Router-To-Network
		if (0x01 == *dec.pktBuffer || 0x02 == *dec.pktBuffer)
			// receive I-AM / I-Could-be-Router-To-Network
		{
			m_routerAddr = npdu.pduAddr;
			m_event.SetEvent();
		}
		return;		// It's OK
	}

	BACnetAPDU apdu;
	apdu.Decode(dec);

	if (apdu.apduType == errorPDU && m_nExpectAPDUType != errorPDU)
	{
		m_bAbort = TRUE;		// received a Result(-) response from the IUT, terminate the test
		m_event.SetEvent();
		return;
	}

	if (apdu.apduType != m_nExpectAPDUType || apdu.apduService != m_nExpectAPDUServiceChoice)
	{
		return;		// according to the pdu type and service choice, this is not the packet which we expect
	}

	if (apdu.apduType == simpleAckPDU)
	{
		m_event.SetEvent();
		return;			// it's OK.
	}

	if (m_pAPDU != NULL)
	{
		delete m_pAPDU;
	}
	m_pAPDU = new BACnetAPDU(apdu);
	if (apdu.pktLength > 0)
	{
		if (m_packetData != NULL)
		{
			delete []m_packetData;
		}
		m_packetData = new BACnetOctet[apdu.pktLength];
		memcpy(m_packetData, apdu.pktBuffer, apdu.pktLength);
		m_pAPDU->pktBuffer = m_packetData;
	}

	// trigger the event
	m_event.SetEvent();
}



BOOL BakRestoreExecutor::SendExpectReadProperty(BACnetObjectIdentifier& objID, BACnetEnumerated& propID,
												AnyValue& propValue, int propIndex /*= -1*/)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = complexAckPDU;
	m_nExpectAPDUServiceChoice = readProperty;
	// encode the packet
	CByteArray contents;
	BACnetAPDUEncoder	enc;
	objID.Encode(enc,0);
	propID.Encode(enc,1);
	if (propIndex != -1)
	{
		BACnetUnsigned propertyArrayIndex(propIndex);
		propertyArrayIndex.Encode(enc,2);
	}
	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x0C);		// Service Choice = 12(ReadProperty-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}

	BACnetAPDUDecoder dec(m_pAPDU->pktBuffer, m_pAPDU->pktLength);
	objID.Decode(dec);
	propID.Decode(dec);
	if (propIndex != -1)
	{
		BACnetUnsigned index;
		index.Decode(dec);
	}
	BACnetOpeningTag().Decode(dec);
	propValue.Decode(dec);
	BACnetClosingTag().Decode(dec);

	if(dec.pktLength != 0)
	{
		Msg("Decoding hasn't finished!");
	}

	return TRUE;
}

BOOL BakRestoreExecutor::SendExpectWriteProperty(BACnetObjectIdentifier& objID, BACnetEnumerated& propID,
												 AnyValue& propValue)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = simpleAckPDU;
	m_nExpectAPDUServiceChoice = writeProperty;
	CByteArray contents;
	BACnetAPDUEncoder	enc;
	objID.Encode(enc, 0);
	propID.Encode(enc, 1);
	BACnetOpeningTag().Encode(enc,3);
	propValue.Encode(enc);
	BACnetClosingTag().Encode(enc,3);

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x0F);		// Service Choice = 15(WriteProperty-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	return SendExpectPacket(contents);

}

BOOL BakRestoreExecutor::SendExpectWhoIs(BACnetObjectIdentifier& iAmDeviceID, BACnetUnsigned& maxAPDULenAccepted)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = unconfirmedRequestPDU;
	m_nExpectAPDUServiceChoice = iAm;
	// encode the packet
	CByteArray contents;
	contents.InsertAt(0, (BYTE)0x08);
	contents.InsertAt(0, (BYTE)0x10);

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}
	// set the contents of the following variables from the parameters contained
	// in the I-Am message
	BACnetAPDUDecoder dec(m_pAPDU->pktBuffer, m_pAPDU->pktLength);
	iAmDeviceID.Decode(dec);
	maxAPDULenAccepted.Decode(dec);
	BACnetEnumerated().Decode(dec);		// segSupported
	BACnetUnsigned().Decode(dec);		// vendorID

	if(dec.pktLength != 0)
	{
		Msg("Decoding hasn't finished!");
	}

	return TRUE;
}


BOOL BakRestoreExecutor::SendExpectReinitialize(ReinitializedStateOfDevice nRreinitState)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = simpleAckPDU;
	m_nExpectAPDUServiceChoice = reinitializeDevice;
	CByteArray contents;
	BACnetAPDUEncoder	enc;
	BACnetEnumerated	reinitState(nRreinitState);
	reinitState.Encode(enc, 0);
	if (!m_strPassword.IsEmpty())
	{
		BACnetCharacterString  password(m_strPassword);
		password.Encode(enc, 1);
	}
	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x14);		// Service Choice = 20(ReinitializeDevice-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	return SendExpectPacket(contents);
}

BOOL BakRestoreExecutor::SendExpectAtomicReadFile_Stream(BACnetObjectIdentifier& fileID, BACnetInteger& fileStartPosition,
														 BACnetUnsigned& ROC, BACnetOctetString& fileData)
{

	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = complexAckPDU;
	m_nExpectAPDUServiceChoice = atomicReadFile;
	CByteArray contents;
	BACnetAPDUEncoder	enc;
	fileID.Encode(enc);
	// access method:	Stream Access
	BACnetOpeningTag().Encode(enc,0);
	fileStartPosition.Encode(enc);
	ROC.Encode(enc);
	BACnetClosingTag().Encode(enc,0);
	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x06);		// Service Choice = 06(AtomicReadFile-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}

	BACnetAPDUDecoder dec(m_pAPDU->pktBuffer, m_pAPDU->pktLength);
	BACnetBoolean endofFile;
	endofFile.Decode(dec);
	BACnetOpeningTag().Decode(dec);
	fileStartPosition.Decode(dec);
	fileData.Decode(dec);
	BACnetClosingTag().Decode(dec);

	if(dec.pktLength != 0)
	{
		Msg("Decoding hasn't finished!");
	}

	return TRUE;
}


// only read one record back each time
// Requested Record Count = 1;
BOOL BakRestoreExecutor::SendExpectAtomicReadFile_Record(BACnetObjectIdentifier& fileID, BACnetInteger& fileStartRecord,
						 BACnetOctetString& fileRecordData, BACnetBoolean& endofFile)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = complexAckPDU;
	m_nExpectAPDUServiceChoice = atomicReadFile;

	CByteArray contents;
	BACnetAPDUEncoder	enc;

	fileID.Encode(enc);
	// access method:	Stream Access
	BACnetOpeningTag().Encode(enc,1);
	fileStartRecord.Encode(enc);
	BACnetUnsigned	requestedRecordCount(1);
	requestedRecordCount.Encode(enc);
	BACnetClosingTag().Encode(enc,1);
	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x06);		// Service Choice = 6(AtomicReadFile-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}

	BACnetAPDUDecoder dec(m_pAPDU->pktBuffer, m_pAPDU->pktLength);
	endofFile.Decode(dec);
	BACnetOpeningTag().Decode(dec);
	fileStartRecord.Decode(dec);
	requestedRecordCount.Decode(dec);
	if (requestedRecordCount.uintValue != 1)
	{
		return FALSE;		// only read one record back each time
	}
	while(dec.pktLength != 0)
	{
		//if the code is closing tag then break
		BACnetAPDUTag tag;
		dec.ExamineTag(tag);
		if(tag.tagClass == closingTagClass)
			break;
		fileRecordData.Decode(dec);
	}

	return TRUE;
}


// only use BACnetObjectIdentifier as object specifier
// do not consider PropertyArrayIndex and Priority
BOOL BakRestoreExecutor::SendExpectCreatObject(BACnetObjectIdentifier& objID,
											   BACnetSequenceOf<PropertyValue>& listOfInitialValues)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = complexAckPDU;
	m_nExpectAPDUServiceChoice = createObject;

	CByteArray contents;
	BACnetAPDUEncoder	enc;

	BACnetOpeningTag().Encode(enc, 0);
	objID.Encode(enc, 1);
	BACnetClosingTag().Encode(enc, 0);

	BACnetOpeningTag().Encode(enc, 1);
	for(int index = 0; index < listOfInitialValues.GetItemCount(); index++)
	{
		PropertyValue* pValue = listOfInitialValues.GetElement(index);
		pValue->Encode(enc);
	}
	BACnetClosingTag().Encode(enc, 1);

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x0A);		// Service Choice = 13(creatObject-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}

	BACnetAPDUDecoder dec(m_pAPDU->pktBuffer, m_pAPDU->pktLength);
	objID.Decode(dec);

	if(dec.pktLength != 0)
	{
		Msg("Decoding hasn't finished!");
	}

	return TRUE;
}


BOOL BakRestoreExecutor::SendExpectAtomicWriteFile_Stream(BACnetObjectIdentifier& fileID, BACnetInteger& fileStartPosition,
														   BACnetOctetString& fileData)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = complexAckPDU;
	m_nExpectAPDUServiceChoice = atomicWriteFile;

	CByteArray contents;
	BACnetAPDUEncoder	enc;

	fileID.Encode(enc);
	BACnetOpeningTag().Encode(enc, 0);
	fileStartPosition.Encode(enc);
	fileData.Encode(enc);
	BACnetClosingTag().Encode(enc, 0);
	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x07);		// Service Choice = 7(atomicWriteFile-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}

	BACnetAPDUDecoder dec(m_pAPDU->pktBuffer, m_pAPDU->pktLength);
	fileStartPosition.Decode(dec);

	if(dec.pktLength != 0)
	{
		Msg("Decoding hasn't finished!");
	}

	return TRUE;
}

BOOL BakRestoreExecutor::SendExpectAtomicWriteFile_Record(BACnetObjectIdentifier& fileID, BACnetInteger& fileStartRecord,
														  BACnetOctetString& fileRecordData)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = complexAckPDU;
	m_nExpectAPDUServiceChoice = atomicWriteFile;

	CByteArray contents;
	BACnetAPDUEncoder	enc;

	fileID.Encode(enc);
	BACnetOpeningTag().Encode(enc, 1);
	fileStartRecord.Encode(enc);
	BACnetUnsigned returnedRecordCount(1);
	returnedRecordCount.Encode(enc);
	fileRecordData.Encode(enc);
	BACnetClosingTag().Encode(enc, 1);
	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x07);		// Service Choice = 7(atomicWriteFile-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}

	BACnetAPDUDecoder dec(m_pAPDU->pktBuffer, m_pAPDU->pktLength);
	fileStartRecord.Decode(dec);

	if(dec.pktLength != 0)
	{
		Msg("Decoding hasn't finished!");
	}

	return TRUE;
}


BOOL BakRestoreExecutor::SendExpectReinitializeNeg(ReinitializedStateOfDevice nReinitState,
												   BACnetEnumerated& errorClass, BACnetEnumerated& errorCode)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = errorPDU;
	m_nExpectAPDUServiceChoice = reinitializeDevice;
	CByteArray contents;
	BACnetAPDUEncoder	enc;
	BACnetEnumerated	reinitState(nReinitState);
	reinitState.Encode(enc, 0);
	if (!m_strPassword.IsEmpty())
	{
		BACnetCharacterString  password(m_strPassword);
		password.Encode(enc, 1);
	}
	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
	{
		contents.Add( enc.pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x14);		// Service Choice = 20(ReinitializeDevice-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}
	BACnetAPDUDecoder dec(m_pAPDU->pktBuffer, m_pAPDU->pktLength);
//	(dec.pktLength--, dec.pktBuffer++);
	errorClass.Decode(dec);
	errorCode.Decode(dec);

	return TRUE;
}


BOOL BakRestoreExecutor::SendExpectPacket(CByteArray& contents)
{
	// network layer
	CByteArray buf;
	buf.Add((BYTE)0x01);      // version
	if (m_IUTAddr.addrType == remoteStationAddr)
	{	// if the IUT is on the remote network
		BYTE control = 0;
		control |= 0x24;							// Set control to include DNET, DLEN, DADR, Hop and DER 
		buf.Add(control);
		buf.Add((m_IUTAddr.addrNet & 0xFF00) >> 8);	// DNET
		buf.Add(m_IUTAddr.addrNet & 0x00FF);
		buf.Add((BYTE)m_IUTAddr.addrLen);			// DLEN

		for(int index = 0; index < m_IUTAddr.addrLen; index++)
		{
			buf.Add(m_IUTAddr.addrAddr[index]);		// DADR
		}

		buf.Add((BYTE)0xFF);		// hop count
	}
	else
	{
		buf.Add((BYTE)0x04);		// control
	}
	contents.InsertAt(0, &buf);
	if (m_pPort->m_nPortType == ipPort)
	{
		int len = contents.GetSize() + 4;		 // the size of npdu plus BVLC
		contents.InsertAt(0, (BYTE)(len & 0xFF));
		contents.InsertAt(0, (BYTE)(len >> 8));
		contents.InsertAt(0, (BYTE)0x0A);
		contents.InsertAt(0, (BYTE)0x81);
	}

	VTSDevicePtr pdevice = m_pPort->m_pdevice;
	int nNumOfAPDURetries;
	int nAPDUTimeOut;
	if (pdevice != NULL)
	{
		nNumOfAPDURetries = pdevice->m_nAPDURetries;
		nAPDUTimeOut = pdevice->m_nAPDUTimeout;
	}
	else
	{
		nNumOfAPDURetries = 3;
		nAPDUTimeOut = 10000;	// millisecond
	}

	BACnetAddress	addr;
	if (m_IUTAddr.addrType == remoteStationAddr)
	{
		addr = m_routerAddr;
	}
	else
	{
		addr = m_IUTAddr;
	}
	// Before sending a packet, modify the signal bit to notify the main thread to
	// prepare accept packet
	m_bExpectAPDU = TRUE;
	m_bExpectPacket = TRUE;
	BOOL bReceived = FALSE;
	for (int i = 0; i < nNumOfAPDURetries; i++)
	{
		// send packet
		m_pPort->portFilter->Indication(
			BACnetNPDU(addr, contents.GetData(), contents.GetSize())
			);
		if (m_bUserCancelled)
		{
			m_bUserCancelled = FALSE;	// reset
			throw("the backup restore process has been cancelled by user\n");
		}
		if (m_event.Lock(nAPDUTimeOut))
		{
			if (m_bUserCancelled)
			{
				throw("the backup restore process has been cancelled by user\n");
			}
			bReceived = TRUE;
			if (m_bAbort == TRUE)
			{
				m_bAbort = FALSE;		// reset
				throw("receive a Result(-) response from IUT\n");
			}
			break;
		}
	}

	m_bExpectPacket = FALSE;	// OK, we have gotten the packet
	return bReceived;
}


void BakRestoreExecutor::InsertMaxAPDULenAccepted(CByteArray& contents)
{
	VTSDevicePtr	pdevice = m_pPort->m_pdevice;
	int nMaxAPDUSize;
	if (pdevice)
	{
		nMaxAPDUSize = pdevice->m_nMaxAPDUSize;
	}
	else
	{
		nMaxAPDUSize = 1024;
	}

	if (nMaxAPDUSize <= 50)
	{
		contents.InsertAt(0, (BYTE)0x00);
	}else if (nMaxAPDUSize <= 128)
	{
		contents.InsertAt(0, (BYTE)0x01);
	}else if (nMaxAPDUSize <= 206)
	{
		contents.InsertAt(0, (BYTE)0x02);
	}else if (nMaxAPDUSize <= 480)
	{
		contents.InsertAt(0, (BYTE)0x03);
	}else if (nMaxAPDUSize <= 1024)
	{
		contents.InsertAt(0, (BYTE)0x04);
	}else
	{
		contents.InsertAt(0, (BYTE)0x05);
	}

}

// To determine if the IUT on a remote network
//void BakRestoreExecutor::SetIUTAddress(VTSPort* pPort, VTSName* pName)
//{
//	// get IUT's network number and address
//	unsigned short		nIUTNet;
//	if (pName->m_bacnetaddr.addrType == localStationAddr)
//	{	//
//		nIUTNet = m_pName->m_pportLink->m_nNet;
//	}
//	else
//	{
//		nIUTNet = m_pName->m_bacnetaddr.addrNet;
//	}
//
//	if (m_pPort->m_nNet == nIUTNet)
//	{
//		m_IUTAddr.LocalStation(pName->m_bacnetaddr.addrAddr, pName->m_bacnetaddr.addrLen);
//	}
//	m_IUTAddr.RemoteStation(nIUTNet, pName->m_bacnetaddr.addrAddr,
//							pName->m_bacnetaddr.addrLen);
//}



void BakRestoreExecutor::FindRouterAddress()
{
	// Send Who-Is-Router-To-Network message
	unsigned short	addrNet = m_IUTAddr.addrNet;
	CByteArray contents;
	contents.InsertAt( 0, (BYTE)(0x00FF & addrNet) );      // DNET, 2 octets
	contents.InsertAt( 0, (BYTE)(addrNet >> 8) );
	contents.InsertAt( 0, (BYTE)0x00 );      // Message Type = X'00' Who-Is-Router-To-Network
	contents.InsertAt( 0, (BYTE)0x80 );      // control
	contents.InsertAt( 0, (BYTE)0x01 );		 // version
	if (m_pPort->m_nPortType == ipPort)
	{
		int len = contents.GetSize() + 4;		 // the size of npdu plus BVLC
		contents.InsertAt( 0, (BYTE)(len & 0x00FF) );
		contents.InsertAt( 0, (BYTE)(len >> 8) );
//		contents.InsertAt( 0, (BYTE)0x0A );  // This is Original Unicast message specifier
		contents.InsertAt( 0, (BYTE)0x0B );  // This is Original Broadcast message specifier
		contents.InsertAt( 0, (BYTE)0x81 );
	}

	BACnetAddress	addr(localBroadcastAddr);
	m_bExpectAPDU = FALSE;		// expect NPDU
	m_bExpectPacket = TRUE;
	m_pPort->portFilter->Indication(
		BACnetNPDU(addr, contents.GetData(), contents.GetSize())
		);
	if (!m_event.Lock(30000)) 	// timeout setting: 30 seconds
//	if (!m_event.Lock())
	{
		throw("Can not find router to the IUT");
	}
	m_bExpectPacket = FALSE;
	m_bExpectAPDU = TRUE;		// reset
}

void BakRestoreExecutor::Msg(const char* errMsg)
{
	VTSPacket pkt;
	pkt.packetHdr.packetProtocolID = (int)BACnetPIInfo::ProtocolType::bakRestoreMsgProtocol;
	pkt.packetHdr.packetFlags = 0;
	pkt.packetHdr.packetType = msgData;
	BACnetOctet* buff = new BACnetOctet[strlen(errMsg)+1];
	memcpy(buff, errMsg, strlen(errMsg)+1);
	pkt.NewDataRef(buff, strlen(errMsg)+1);

	VTSDocPtr	vdp = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();
	vdp->WritePacket( pkt );

	delete []buff;
}
