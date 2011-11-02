// BakRestoreExecutor.h: interface for the BakRestoreExecutor class.
// Jingbo Gao, Sep 20 2004
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BAKRESTOREEXECUTOR_H__B11EB238_C49B_4046_84A9_0CF30BFD844B__INCLUDED_)
#define AFX_BAKRESTOREEXECUTOR_H__B11EB238_C49B_4046_84A9_0CF30BFD844B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>
#include "bacnet.hpp"
#include "vtsdoc.h"
#include "afxmt.h"

class VTSDoc;
typedef VTSDoc * VTSDocPtr;
struct BACnetNPDU;
class VTSBackupRestoreProgressDlg;

enum ReinitializedStateOfDevice 
{
	COLDSTART		= 0,
	WARMSTART		= 1,
	STARTBACKUP		= 2,
	ENDBACKUP		= 3,
	STARTRESTORE	= 4,
	ENDRESTORE		= 5,
	ABORTRESTORE	= 6
};

class BakRestoreExecutor : public BACnetTask  
{
public:
	// the purpose to inheritance BACnetAnyValue is to prevent BACnetAnyValue automatically delete
	// the data which it is hold when it isn't hold that data any longer.
	class AnyValue : public BACnetAnyValue
	{
	public:
		AnyValue();
		virtual ~AnyValue() ;

		void SetObject(BACnetEncodeable * pbacnetEncodeable);
	};
	// Create a new data type to be used in CreateObject Service
	// do not support sequencof, arrayof property, etc.
	class PropertyValue
	{
	public:
		BACnetEnumerated m_propID;
		AnyValue m_propValue;
		
		PropertyValue(const BACnetEnumerated& propID, BACnetEncodeable& propValue);
		PropertyValue();
		PropertyValue(const PropertyValue& propValue);
		virtual ~PropertyValue() ;
		
		void Encode(BACnetAPDUEncoder &enc);
		void Decode(BACnetAPDUDecoder &dec);
	};
	enum BakRestoreExecutorState 
	{ 
		execIdle,
		execRunning, 
		execStopped
	};
	enum FunctionToExecute
	{
		ALL_BACKUP_RESTORE = 0,
		FULL_BACKUP_RESTORE,
		BACKUP_ONLY,
		RESTORE_ONLY,
		AUXILIARY_BACKUP_RESTORE
	};
	enum {	DEVICE_OBJ_INST_NUM_UNSET = 4194304,
			MAX_NPDU_LENGTH = 1497	};

	
	void ExecuteTest();
	void Kill();
	void ProcessTask();
	void ReceiveNPDU(const BACnetNPDU& npdu);
	BOOL IsRunning(void) { return m_execState == execRunning; }
	void DestoryOutputDlg(void) { m_pOutputDlg = NULL; }
	void ReadDatabaseRevAndRestoreTime(	BACnetObjectIdentifier &devObjID,
										BACnetUnsigned         &databaseRevision,
                                        BACnetTimeStamp	       &lastRestoreTime );

public:
	BakRestoreExecutor();
	virtual ~BakRestoreExecutor();

private:
	VTSPort* m_pPort;
	VTSName* m_pName;
	BACnetAddress m_routerAddr;		// Router's MAC address
	BACnetAddress m_IUTAddr;		// the IUT's address
//	BOOL m_bIsOnSameNetwork;		// whether the BACnetAddress of the IUT is on a remote network
	UINT m_nDeviceObjInst;
	CString m_strBackupFileName;	// the base name of the files to create
	CString m_strPassword;
	FunctionToExecute m_funToExe;

	BakRestoreExecutorState m_execState;	
	BACnetAPDU* m_pAPDU;

	CCriticalSection m_cs;
	BOOL m_bAbort;		// indicate the reception of "Result(-)"
	BOOL m_bExpectPacket;	// tell the main thread to receive packet and write it to the buffer
	CEvent m_event;		// is uesed by the main thread that the packet has been correctly received
	BACnetOctet* m_packetData;	// buffer used to hold the received packet
	BOOL m_bUserCancelled;
	CByteArray m_segmentData; //Segment data

	UINT m_maxAPDULen;


	BOOL m_bExpectAPDU;		// if expect a APDU or not
	BACnetAPDUType m_nExpectAPDUType;	// expected APDU type if expect a APDU
	int m_nExpectAPDUServiceChoice;		// expected Service Choice if expect a APDU
	UINT m_Delay;
	UINT m_Backup_Timeout;

	VTSBackupRestoreProgressDlg* m_pOutputDlg;
	void DoBackupTest();
	void DoRestoreTest();
	void DoAuxiliaryTest();

//	void DoAuxiliaryTest_1();
	void DoAuxiliaryTest_2();
//	void DoAuxiliaryTest_3();
	void DoAuxiliaryTest_4();
	void DoAuxiliaryTest_5(BACnetObjectIdentifier& devObjID);
	void DoAuxiliaryTest_6(BACnetObjectIdentifier& devObjID);
	void DoAuxiliaryTest_7();
	void DoAuxiliaryTest_8();
	void DoAuxiliaryTest_9(BACnetObjectIdentifier& devObjID);
	void DoAuxiliaryTest_10(BACnetObjectIdentifier& devObjID);
	void DoAuxiliaryTest_11(BACnetObjectIdentifier& devObjID);
	void DoAuxiliaryTest_12(BACnetObjectIdentifier& devObjID);
	
	BOOL SendExpectReadProperty(BACnetObjectIdentifier& objID, BACnetEnumerated& propID, 
								AnyValue& propValue, int propIndex = -1);
	BOOL SendExpectWriteProperty(BACnetObjectIdentifier& objID, BACnetEnumerated& propID,
								 AnyValue& propValue);
	BOOL SendExpectWhoIs(BACnetObjectIdentifier& iAmDeviceID, BACnetUnsigned& maxAPDULenAccepted);
	BOOL SendExpectReinitialize(ReinitializedStateOfDevice nReinitState);
	BOOL SendExpectAtomicReadFile_Stream(BACnetObjectIdentifier& fileID, BACnetInteger& fileStartPosition, 
										 BACnetUnsigned& ROC, BACnetOctetString& fileData);
	BOOL SendExpectAtomicReadFile_Record(BACnetObjectIdentifier& fileID, BACnetInteger& fileStartRecord, 
										 BACnetOctetString& fileRecordData, 
										 BACnetBoolean& endofFile);
	BOOL SendExpectCreatObject(BACnetObjectIdentifier& objID, 
							   BACnetSequenceOf<PropertyValue>& listOfInitialValues);
	BOOL SendExpectAtomicWriteFile_Stream(BACnetObjectIdentifier& fileID, BACnetInteger& fileStartPosition, 
										   BACnetOctetString& fileData);
	BOOL SendExpectAtomicWriteFile_Record(BACnetObjectIdentifier& fileID, BACnetInteger& fileStartRecord, 
										  BACnetOctetString& fileRecordData);
	
	BOOL SendExpectReinitializeNeg(ReinitializedStateOfDevice nReinitState,
								   BACnetEnumerated& errorClass, BACnetEnumerated& errorCode);
	
	BOOL SendExpectPacket(CByteArray& contents, BOOL resp = TRUE);
	void InsertMaxAPDULenAccepted(CByteArray& contents);
//	void SetIUTAddress(VTSPort* pPort, VTSName* pName);
	void FindRouterAddress(void);

	void Msg(const char* errMsg);

	void Delay( UINT delaySec );
};

#endif // !defined(AFX_BAKRESTOREEXECUTOR_H__B11EB238_C49B_4046_84A9_0CF30BFD844B__INCLUDED_)
