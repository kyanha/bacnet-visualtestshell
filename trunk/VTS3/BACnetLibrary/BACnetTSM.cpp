
#include "stdafx.h"

#include "BACnet.hpp"
#include "BACnetIP.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	BACnetTSM::BACnetTSM
//

BACnetTSM::BACnetTSM( BACnetDevicePtr dp )
	: tsmDevice(dp), tsmState(tsmIdle), tsmSeg(0)
	, tsmInvokeID(0)
	, tsmRetryCount(0)
	, tsmSegmentRetryCount(0)
	, tsmLastSequenceNumber(0)
	, tsmInitialSequenceNumber(0)
	, tsmActualWindowSize(0)
	, tsmProposedWindowSize(0)
{
}

//
//	BACnetTSM::~BACnetTSM
//

BACnetTSM::~BACnetTSM( void )
{
}

//
//	BACnetTSM::StartTimer
//

void BACnetTSM::StartTimer( int msecs )
{
	// if this is active, pull it
	if (isActive)
		SuspendTask();
	
	// now install this
	taskType = oneShotTask;
	taskInterval = msecs;
	InstallTask();
}

//
//	BACnetTSM::StopTimer
//

void BACnetTSM::StopTimer( void )
{
	SuspendTask();
}

//
//	BACnetTSM::RestartTimer
//

void BACnetTSM::RestartTimer( int msecs )
{
	// if this is active, pull it
	if (isActive)
		SuspendTask();
	
	taskType = oneShotTask;
	taskInterval = msecs;
	InstallTask();
}

//
//	BACnetTSM::FillWindow
//

void BACnetTSM::FillWindow( int seqNum )
{
	for (int i = 0; i < tsmActualWindowSize; i++ ) {
		const BACnetAPDU &apdu = (*tsmSeg)[ seqNum + i ]
		;
		
		// send the message to the device
		tsmDevice->Indication( apdu );
		
		// check for no more follows
		if ((apdu.pktBuffer[0] & 0x04) == 0)
			return;
	}
}

//
//	BACnetTSM::InWindow
//

int BACnetTSM::InWindow( int seqNum, int initSeqNum )
{
	return ((((seqNum + 256 - initSeqNum) % 256) < tsmActualWindowSize) ? 1 : 0);
}
