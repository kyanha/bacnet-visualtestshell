// InconsistentParsExecutor.cpp: implementation of the InconsistentParsExecutor class.
// Lori Tribble - 10/17/2009
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vts.h"
#include "VTSDoc.h"
#include "BACnet.hpp"
#include "VTSInconsistentParsDlg.h"
#include "VTSInconsistentProgressDlg.h"
#include "ScriptExecutor.h"
#include "InconsistentParsExecutor.h"
#include "PI.h"

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
InconsistentParsExecutor gInconsistentParsExecutor;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


InconsistentParsExecutor::InconsistentParsExecutor()
: m_pPort(NULL), m_pName(NULL), 
  m_funToExe(INVALID_TAG_TEST), m_execState(execIdle),
  m_pAPDU(NULL), m_bAbort(FALSE), m_bExpectPacket(FALSE),m_packetData(NULL),
  m_bExpectAPDU(TRUE), m_bUserCancelled(FALSE), m_pOutputDlg(NULL)
{
}

InconsistentParsExecutor::~InconsistentParsExecutor()
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


void InconsistentParsExecutor::ExecuteTest()
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

    VTSInconsistentParsDlg dlg(*pNames, *pPorts);
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

		m_funToExe = (FunctionToExecute)dlg.m_nFunction;
		m_ObjID = dlg.m_ObjectID;
		m_propID = dlg.m_propCombo;
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
		m_pOutputDlg = new VTSInconsistentParsProgressDlg(NULL);
		m_pOutputDlg->Create(IDD_INCONSISTENT_PARS_PROGRESS);
		m_pOutputDlg->ShowWindow(SW_SHOW);
	}

	// install the task
	taskType = oneShotTask;
	taskInterval = 0;
	InstallTask();

	lock.Unlock();

}

void InconsistentParsExecutor::Kill()
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


void InconsistentParsExecutor::ProcessTask()
{
	CSingleLock lock(&m_cs);
	lock.Lock();

	BOOL bSuccess = TRUE;
	try	{
		if ( !m_pPort || !m_pName )
		{
			throw("VTSPort or IUT not specified\n");
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

		switch(m_funToExe) 
		{
		case INVALID_TAG_TEST:
			DoInvalidTagTest();
			Msg("13.4.3: Invalid Tag test has been sucessfully completed!\n");
			break;
		case MISSING_REQUIRED_TEST:
			DoMissingRequiredTest();
			Msg("13.4.4: Missing Required test has been sucessfully completed!\n");
			break;
		case TOO_MANY_ARGS_TEST:
			DoTooManyArgsTest();
			Msg("13.4.5: Too Many Args test has been sucessfully completed!\n");
			break;
		default:
			DoInvalidTagTest();
			DoMissingRequiredTest();
			DoTooManyArgsTest();
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
		m_pOutputDlg->OutMessage("Selected tests completed successfully");
		Msg("Selected tests completed successfully");
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


void InconsistentParsExecutor::DoInvalidTagTest()
{
	m_pOutputDlg->OutMessage("Begin Invalid Tag test...");

	BACnetObjectIdentifier objObjID;
	BACnetEnumerated propID;
	RejectReason expectedErrors[] = {INVALID_TAG, INCONSISTENT_PARAMETERS, INVALID_PARAMETER_DATA_TYPE,
		MISSING_REQUIRED_PARAMETER, TOO_MANY_ARGUMENTS};

	m_pOutputDlg->OutMessage("Send ReadProperty...",	FALSE);
	// TODO: get prop and obj from DLG and then send special packet
	//propID.enumValue = PICS::OBJECT_IDENTIFIER;
	//objObjID.SetValue( (BACnetObjectType)8, m_nDeviceObjInst);
	objObjID = m_ObjID;
	propID = m_propID;

	// encoder with objObjID, PropID with invalid tag
	// encode the packet
	BACnetAPDUEncoder	enc;
	objObjID.Encode(enc,0);
	propID.Encode(enc,5);  // note incorrect tag specified here on purpose

	if (!SendReadPropertyExpectReject(&enc, expectedErrors, 5))
	{
		throw("Incorrect Error Code returned by IUT\n");
	}
	m_pOutputDlg->OutMessage("OK");

}



void InconsistentParsExecutor::DoMissingRequiredTest()
{
	m_pOutputDlg->OutMessage("Begin Missing Required Test...");

	BACnetObjectIdentifier objObjID;
	RejectReason expectedErrors[] = {INVALID_TAG, MISSING_REQUIRED_PARAMETER};

	m_pOutputDlg->OutMessage("Send ReadProperty...",	FALSE);
	// TODO: get obj from DLG and then send special packet
	//objObjID.SetValue( (BACnetObjectType)8, m_nDeviceObjInst);
	objObjID = m_ObjID;

	BACnetAPDUEncoder	enc;
	objObjID.Encode(enc,0);

	if (!SendReadPropertyExpectReject(&enc, expectedErrors, 2))
	{
		throw("Incorrect Error Code returned by IUT\n");
	}
	m_pOutputDlg->OutMessage("OK");

}


void InconsistentParsExecutor::DoTooManyArgsTest()
{
	m_pOutputDlg->OutMessage("Begin Too Many Args Test...");

	BACnetObjectIdentifier objObjID;
	BACnetEnumerated propID;

	RejectReason expectedErrors[] = {INVALID_TAG, TOO_MANY_ARGUMENTS};

	m_pOutputDlg->OutMessage("Send ReadProperty...",	FALSE);
	// TODO: get prop and obj from DLG and then send special packet
	//propID.enumValue = PICS::OBJECT_IDENTIFIER;
	//objObjID.SetValue( (BACnetObjectType)8, m_nDeviceObjInst);
	objObjID = m_ObjID;
	propID = m_propID;

	BACnetAPDUEncoder	enc;
	objObjID.Encode(enc,0);
	propID.Encode(enc, 1);
	propID.Encode(enc, 1);  // add second property ID to encoding

	if (!SendReadPropertyExpectReject(&enc, expectedErrors, 2))
	{
		throw("Incorrect Error Code returned by IUT\n");
	}
	m_pOutputDlg->OutMessage("OK");

}

// called by the script executor when a packet is received
//    Note: must modify the ScriptNetFilter::Confirmation in ScriptExecutor.cpp to know about gInconsistentParsExecutor
void InconsistentParsExecutor::ReceiveNPDU(const BACnetNPDU& npdu)
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

	// locate type of packet received
	if (apdu.apduType == errorPDU && m_nExpectAPDUType != errorPDU)
	{
		m_bAbort = TRUE;		// received a Result(-) response from the IUT, terminate the test
		m_event.SetEvent();
		return;
	}

	if (apdu.apduType != m_nExpectAPDUType)
	{
		return;		// according to the pdu type and service choice, this is not the packet which we expect
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



BOOL InconsistentParsExecutor::SendReadPropertyExpectReject(BACnetAPDUEncoder *enc, RejectReason Allowed_Errors[], int len)
{
	m_bExpectAPDU = TRUE;
	m_nExpectAPDUType = rejectPDU;
	//m_nExpectAPDUServiceChoice = readProperty;
	// encode the packet
	CByteArray contents;
	int i = 0;

	// copy the encoding into the byte array
	for (i = 0; i < enc->pktLength; i++)
	{
		contents.Add( enc->pktBuffer[i] );
	}
	contents.InsertAt(0, (BYTE)0x0C);		// Service Choice = 12(ReadProperty-Request)
	contents.InsertAt(0, (BYTE)0x01);		// Invoke ID = 1;
	InsertMaxAPDULenAccepted(contents);		// Maximum APDU Size Accepted
	contents.InsertAt(0, (BYTE)0x00);		// PDU Type=0 (BACnet-Confirmed-Request-PDU, SEG=0, MOR=0, SA=0)

	if (!SendExpectPacket(contents))
	{
		return FALSE;
	}

	BACnetEnumerated rejectReason;

	rejectReason.enumValue = m_pAPDU->apduAbortRejectReason;

	// see if we received one of the expected error codes
	for (i = 0; i < len; i++ )
	{
		if ( rejectReason.enumValue == Allowed_Errors[i] )
			return TRUE;
	}

	return FALSE;

}


BOOL InconsistentParsExecutor::SendExpectPacket(CByteArray& contents)
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


void InconsistentParsExecutor::InsertMaxAPDULenAccepted(CByteArray& contents)
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


void InconsistentParsExecutor::FindRouterAddress()
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

void InconsistentParsExecutor::Msg(const char* errMsg)
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
