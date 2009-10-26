// InconsistentParsExecutor.h  - executes the 13.4.3, 13.4.4, and 13.4.5 tests
// Lori Tribble - 10/17/2009
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INCONSISTENTPARSEXECUTOR_H__B11EB238_C49B_4046_84A9_0CF30BFD844B__INCLUDED_)
#define AFX_INCONSISTENTPARSEXECUTOR_H__B11EB238_C49B_4046_84A9_0CF30BFD844B__INCLUDED_

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
class VTSInconsistentParsProgressDlg;

class InconsistentParsExecutor : public BACnetTask  
{
public:
	// the purpose to inheritance BACnetAnyValue is to prevent BACnetAnyValue automatically delete
	// the data which it is hold when it isn't hold that data any longer.
	enum InconsistentParsExecutorState 
	{ 
		execIdle,
		execRunning, 
		execStopped
	};
	enum RejectReason
	{
		OTHER = 0,
		BUFFER_OVERFLOW,
		INCONSISTENT_PARAMETERS,
		INVALID_PARAMETER_DATA_TYPE,
		INVALID_TAG,
		MISSING_REQUIRED_PARAMETER,
		PARAMETER_OUT_OF_RANGE,
		TOO_MANY_ARGUMENTS,
		UNDEFINED_ARGUMENTS,
		UNRECOGNIZED_SERVICE
	};
	enum FunctionToExecute
	{
		INVALID_TAG_TEST = 0,
		MISSING_REQUIRED_TEST,
		TOO_MANY_ARGS_TEST
	};
	enum {	DEVICE_OBJ_INST_NUM_UNSET = 4194304,
			MAX_NPDU_LENGTH = 1497	};

	
	void ExecuteTest();
	void Kill();
	void ProcessTask();
	void ReceiveNPDU(const BACnetNPDU& npdu);
	BOOL IsRunning(void) { return m_execState == execRunning; }
	void DestoryOutputDlg(void) { m_pOutputDlg = NULL; }

public:
	InconsistentParsExecutor();
	virtual ~InconsistentParsExecutor();

private:
	VTSPort* m_pPort;
	VTSName* m_pName;
	BACnetAddress m_routerAddr;		// Router's MAC address
	BACnetAddress m_IUTAddr;		// the IUT's address
	//UINT m_nDeviceObjInst;
	BACnetAPDU* m_pAPDU;

	BACnetObjectIdentifier m_ObjID;
	BACnetEnumerated m_propID;

	CCriticalSection m_cs;
	BOOL m_bAbort;		// indicate the reception of "Result(-)"
	BOOL m_bExpectPacket;	// tell the main thread to receive packet and write it to the buffer
	CEvent m_event;		// is uesed by the main thread that the packet has been correctly received
	BACnetOctet* m_packetData;	// buffer used to hold the received packet
	BOOL m_bUserCancelled;
	FunctionToExecute m_funToExe;
	InconsistentParsExecutorState m_execState;	
	
	BOOL m_bExpectAPDU;		// if expect a APDU or not
	BACnetAPDUType m_nExpectAPDUType;	// expected APDU type if expect a APDU
	int m_nExpectAPDUServiceChoice;		// expected Service Choice if expect a APDU

	VTSInconsistentParsProgressDlg* m_pOutputDlg;
	void DoInvalidTagTest();
	void DoMissingRequiredTest();
	void DoTooManyArgsTest();


	BOOL SendReadPropertyExpectReject(BACnetAPDUEncoder *enc, RejectReason expectedErrors[], int len);
	
	BOOL SendExpectPacket(CByteArray& contents);
	void InsertMaxAPDULenAccepted(CByteArray& contents);
	void FindRouterAddress(void);

	void Msg(const char* errMsg);
	
};

#endif // !defined(AFX_INCONSISTENTPARSEEXECUTOR_H__B11EB238_C49B_4046_84A9_0CF30BFD844B__INCLUDED_)
