
#include "stdafx.h"

#include "BACnet.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	BACnetClientTSM::BACnetClientTSM
//

BACnetClientTSM::BACnetClientTSM( BACnetDevicePtr dp )
	: BACnetTSM( dp )
{
}

//
//	BACnetClientTSM::~BACnetClientTSM
//

BACnetClientTSM::~BACnetClientTSM( void )
{
}

//
//	BACnetClientTSM::Request
//
//	This function is called when the CTSM would like to send a message.  It simply passes
//	the information along to the device to deal with.
//

void BACnetClientTSM::Request( const BACnetAPDU &apdu )
{
	tsmDevice->Indication( apdu );
}

//
//	BACnetClientTSM::Indication
//
//	This function is called when the client has a request it would like to send.  If this 
//	is an unconfirmed request, simply forward it to the device for delivery.  If this is 
//	confirmed, chop it up as necessary and wait for the response.
//

void BACnetClientTSM::Indication( const BACnetAPDU &apdu )
{
	BACnetDeviceInfoPtr	dip
	;
	
	// unless aborting, client must be idle
	if ((apdu.apduType != abortPDU) && (tsmState != tsmIdle))
		throw -1;
	
	switch (apdu.apduType) {
		case confirmedRequestPDU:
			// make sure it's to someplace specific
			if ((apdu.apduAddr.addrType != localStationAddr) && (apdu.apduAddr.addrType != remoteStationAddr))
				throw -1;
			tsmAddr = apdu.apduAddr;
			
			// make a copy of this to modify (and keep in case we need to rexmt)
			tsmSeg = new BACnetAPDUSegment( apdu, this );
			
			// if we support segmented responses, set the bit
			tsmSeg->segAPDU.apduSA = (	(tsmDevice->deviceSegmentation == segmentedReceive)
				 || (tsmDevice->deviceSegmentation == segmentedBoth)
				);
			
			// encode the maximum response
			tsmSeg->segAPDU.apduMaxResp = tsmDevice->deviceMaxAPDUSize;
			
			// get an invoke ID
			tsmInvokeID = tsmDevice->GetInvokeID();
			tsmSeg->segAPDU.apduInvokeID = tsmInvokeID;
			
			// see if we have info about the device
			dip = tsmDevice->GetInfo( apdu.apduAddr );
			
			// the segment size is the min of what I want to transmit and what the 
			// device can receive.
			if (!dip || (dip->deviceMaxAPDUSize > tsmDevice->deviceMaxAPDUSize))
				tsmSegmentSize = tsmDevice->deviceMaxAPDUSize;
			else
				tsmSegmentSize = dip->deviceMaxAPDUSize;
			
			// compute the segment count, minus header
			tsmSegmentCount = (apdu.pktLength + tsmSegmentSize - 4) / tsmSegmentSize;
			
			// do we support segmentation?
			if ( (tsmSegmentCount > 1)
				 && (tsmDevice->deviceSegmentation != segmentedTransmit)
				 && (tsmDevice->deviceSegmentation != segmentedBoth)
				) {
				Response( BACnetAbortAPDU( apdu.apduAddr, 0, tsmInvokeID, segmentationNotSupportedAbort ) );
				break;
			}
			
			// allow segmentation if the workstation allows it
			tsmSegmentation = tsmDevice->deviceSegmentation;
			
			// initialize the state vars
			tsmRetryCount = 0;
			tsmSegmentsSent = 0;
			tsmSegmentRetryCount = 0;
			tsmInitialSequenceNumber = 0;
			tsmActualWindowSize = tsmProposedWindowSize = tsmDevice->deviceWindowSize;
			
			// send out the first chunk
			if (tsmSegmentCount == 1) {
				StartTimer( tsmDevice->deviceAPDUTimeout );
				SetState( tsmAwaitConfirmation );
			} else {
				StartTimer( tsmDevice->deviceAPDUSegmentTimeout );
				SetState( tsmSegmentedRequest );
			}
			Request( (*tsmSeg)[0] );
			break;
		
		case unconfirmedRequestPDU:
			// make sure it's to someplace specific
			if (apdu.apduAddr.addrType == nullAddr)
				throw -1;
			
			// verify the device can send a message this big.  It would be nice if 
			// there was a way to send a message back to the client, but an unconfirmed 
			// request is just that, unconfirmed!
			if (apdu.pktLength > tsmDevice->deviceMaxAPDUSize)
				break;
			
			// forward to device for sending
			Request( apdu );
			break;
		
		case simpleAckPDU:
		case complexAckPDU:
		case segmentAckPDU:
		case errorPDU:
		case rejectPDU:
			Response( BACnetAbortAPDU( apdu.apduAddr, 0, tsmInvokeID, invalidAPDUInThisStateAbort ) );
			break;
		
		case abortPDU:
			// if we are idle, there is no context to abort
			if (tsmState == tsmIdle)
				throw -1;
			
			// cancel timer
			StopTimer();
			
			// send abort reason to the device.  If the abort wasn't passed const, I could 
			// just stuff the invokeID in the correct slot and send it rather than making 
			// a copy.
			Request( BACnetAbortAPDU( apdu.apduAddr, 0, tsmInvokeID, (BACnetAbortReason)apdu.pktBuffer[2] ) );
			
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			
			// return to idle
			SetState( tsmIdle );
			break;
	}
}

//
//	BACnetClientTSM::Confirmation
//

void BACnetClientTSM::Confirmation( const BACnetAPDU &apdu )
{
	switch (tsmState) {
		case tsmSegmentedRequest:
			SegmentedRequest( apdu );
			break;
		case tsmAwaitConfirmation:
			AwaitConfirmation( apdu );
			break;
		case tsmSegmentedConfirmation:
			SegmentedConfirmation( apdu );
			break;
		case tsmIdle:
		case tsmAwaitResponse:
		case tsmSegmentedResponse:
		case tsmBusy:
			break;
	}
}

//
//	BACnetClientTSM::ProcessTask
//

void BACnetClientTSM::ProcessTask( void )
{
	switch (tsmState) {
		case tsmSegmentedRequest:
			SegmentedRequestTimeout();
			break;
		case tsmAwaitConfirmation:
			AwaitConfirmationTimeout();
			break;
		case tsmSegmentedConfirmation:
			SegmentedConfirmationTimeout();
			break;
	}
}

//
//	BACnetClientTSM::SegmentedRequest
//

void BACnetClientTSM::SegmentedRequest( const BACnetAPDU &apdu )
{
	BACnetAbortAPDU abrt( apdu.apduAddr, 0, tsmInvokeID, invalidAPDUInThisStateAbort )
	;
	
#if _TSMDebug
	cout << "[BACnetClientTSM::SegmentedRequest]" << endl;
#endif

	switch (apdu.apduType) {
		case confirmedRequestPDU:
		case unconfirmedRequestPDU:
			StopTimer();
			// abort in both directions
			Request( abrt );
			Response( abrt );
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
		
		// ### can we really start accepting a complex ack in this state?  We haven't 
		// finished sending our request yet!
		case complexAckPDU:
			// might be unsegmented
			if (!apdu.apduSeg)
				goto fini;
			
			// must be first segment
			if (apdu.apduSeq != 0) {
				StopTimer();
				// abort in both directions
				Request( abrt );
				Response( abrt );
				SetState( tsmIdle );
				if (tsmSeg) {
					delete tsmSeg;
					tsmSeg = 0;
				}
				break;
			}
			
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			tsmSeg = new BACnetAPDUSegment( 2560, this );
			tsmSeg->AddSegment( apdu );
			
			StopTimer();
			tsmActualWindowSize = apdu.apduWin;		// accept what device gave us
			Request( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 0, 0, 0, tsmActualWindowSize ) );
			StartTimer( tsmDevice->deviceAPDUSegmentTimeout );
			tsmLastSequenceNumber = 0;
			tsmInitialSequenceNumber = 0;
			SetState( tsmSegmentedConfirmation );
			break;
		
		case segmentAckPDU:
			// duplicate?
			if (!InWindow(apdu.apduSeq,tsmInitialSequenceNumber)) {
				RestartTimer( tsmDevice->deviceAPDUSegmentTimeout );
				break;
			}
			
			// add how many packets were accepted
			tsmSegmentsSent += (apdu.apduSeq + 256 - tsmInitialSequenceNumber) % 256 + 1;
			if (tsmSegmentsSent == tsmSegmentCount) {
				StopTimer();
				StartTimer( tsmDevice->deviceAPDUTimeout );
				SetState( tsmAwaitConfirmation );
				break;
			}
			
			tsmInitialSequenceNumber = tsmSegmentsSent % 256;
			tsmActualWindowSize = apdu.apduWin;
			RestartTimer( tsmDevice->deviceAPDUSegmentTimeout );
			FillWindow( tsmSegmentsSent );
			break;
		
		case simpleAckPDU:
		case errorPDU:
		case rejectPDU:
		case abortPDU:
fini:		// cancel timer
			StopTimer();
			
			// send it to the client
			Response( apdu );
			
			// return to idle
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
	}
}

//
//	BACnetClientTSM::SegmentedRequestTimeout
//

void BACnetClientTSM::SegmentedRequestTimeout( void )
{
#if _TSMDebug
	cout << "[BACnetClientTSM::SegmentedRequestTimeout]" << endl;
#endif

	if (tsmSegmentRetryCount < tsmDevice->deviceAPDURetries) {
		tsmSegmentRetryCount += 1;
		StartTimer( tsmDevice->deviceAPDUSegmentTimeout );
		FillWindow( tsmSegmentsSent );
	} else {
		// send an error message back to application
		Response( BACnetAbortAPDU( tsmAddr, 0, tsmInvokeID, apduTimeoutAbort ) );
		
		// return to idle
		SetState( tsmIdle );
		if (tsmSeg) {
			delete tsmSeg;
			tsmSeg = 0;
		}
	}
}

//
//	BACnetClientTSM::AwaitConfirmation
//

void BACnetClientTSM::AwaitConfirmation( const BACnetAPDU &apdu )
{
	BACnetAbortAPDU abrt( tsmAddr, 0, tsmInvokeID, invalidAPDUInThisStateAbort )
	;
	
#if _TSMDebug
	cout << "[BACnetClientTSM::AwaitConfirmation]" << endl;
#endif

	switch (apdu.apduType) {
		case confirmedRequestPDU:
		case unconfirmedRequestPDU:
			StopTimer();
			// abort in both directions
			Request( abrt );
			Response( abrt );
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
		
		case complexAckPDU:
			// might be unsegmented
			if (!apdu.apduSeg)
				goto fini;
			
			// must be first segment
			if (apdu.apduSeq != 0) {
				StopTimer();
				// abort in both directions
				Request( abrt );
				Response( abrt );
				SetState( tsmIdle );
				if (tsmSeg) {
					delete tsmSeg;
					tsmSeg = 0;
				}
				break;
			}
			
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			tsmSeg = new BACnetAPDUSegment( 2560, this );
			tsmSeg->AddSegment( apdu );
			
			StopTimer();
			tsmActualWindowSize = apdu.apduWin;		// accept what device gave us
			Request( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 0, 0, 0, tsmActualWindowSize ) );
			StartTimer( tsmDevice->deviceAPDUSegmentTimeout );
			tsmLastSequenceNumber = 0;
			tsmInitialSequenceNumber = 0;
			SetState( tsmSegmentedConfirmation );
			break;
		
		case segmentAckPDU:
			// nothing to do
			break;
		
		case simpleAckPDU:
		case errorPDU:
		case rejectPDU:
		case abortPDU:
fini:		// cancel timer
			StopTimer();
			
			// send apdu to the client
			Response( apdu );
			
			// return to idle
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
	}
}

//
//	BACnetClientTSM::AwaitConfirmationTimeout
//

void BACnetClientTSM::AwaitConfirmationTimeout( void )
{
#if _TSMDebug
	cout << "[BACnetClientTSM::AwaitConfirmationTimeout]" << endl;
#endif

	if (tsmRetryCount < tsmDevice->deviceAPDURetries) {
		// well, lets start again
		tsmRetryCount += 1;
		
		// see if we have info about the device
		BACnetDeviceInfoPtr dip = tsmDevice->GetInfo( tsmSeg->segAPDU.apduAddr );
		
		// the segment size is the min of what I want to transmit and what the 
		// device can receive.
		if (!dip || (dip->deviceMaxAPDUSize > tsmDevice->deviceMaxAPDUSize))
			tsmSegmentSize = tsmDevice->deviceMaxAPDUSize;
		else
			tsmSegmentSize = dip->deviceMaxAPDUSize;
		
		// compute the segment count
		tsmSegmentCount = (tsmSeg->segAPDU.pktLength + tsmSegmentSize - 4) / tsmSegmentSize;
		
		// reinitialize the state vars
		tsmSegmentsSent = 0;
		tsmSegmentRetryCount = 0;
		tsmInitialSequenceNumber = 0;
		tsmProposedWindowSize = tsmDevice->deviceWindowSize;
		tsmActualWindowSize = 1;
		
		// send out the first chunk
		if (tsmSegmentCount == 1) {
			StartTimer( tsmDevice->deviceAPDUTimeout );
			SetState( tsmAwaitConfirmation );
		} else {
			StartTimer( tsmDevice->deviceAPDUSegmentTimeout );
			SetState( tsmSegmentedRequest );
		}
		Request( (*tsmSeg)[0] );
	} else {
		// return to idle
		SetState( tsmIdle );
		if (tsmSeg) {
			delete tsmSeg;
			tsmSeg = 0;
		}
		
		// send an error message back to application
		Response( BACnetAbortAPDU( tsmAddr, 0, tsmInvokeID, apduTimeoutAbort ) );
	}
}

//
//	BACnetClientTSM::SegmentedConfirmation
//

void BACnetClientTSM::SegmentedConfirmation( const BACnetAPDU &apdu )
{
	BACnetAbortAPDU			abrt( tsmAddr, 0, tsmInvokeID, invalidAPDUInThisStateAbort )
	;
	BACnetAPDUSegmentPtr	tempSeg
	;
	
#if _TSMDebug
	cout << "[BACnetClientTSM::SegmentedConfirmation]" << endl;
#endif

	switch (apdu.apduType) {
		case confirmedRequestPDU:
		case unconfirmedRequestPDU:
		case simpleAckPDU:
		case segmentAckPDU:
abort:		StopTimer();
			// abort in both directions
			Request( abrt );
			Response( abrt );
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
		
		case complexAckPDU:
			// must be segmented
			if (!apdu.apduSeg) {
				goto abort;
			}
			
			// proper segment number?
			if (apdu.apduSeq != ((tsmLastSequenceNumber + 1) % 256)) {
				RestartTimer( tsmDevice->deviceAPDUSegmentTimeout );
				Request( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 1, 0, tsmLastSequenceNumber, tsmActualWindowSize ) );
				tsmInitialSequenceNumber = tsmLastSequenceNumber;
				break;
			}
			
			// will it fit?
			if (tsmSeg->AddSegment( apdu ) != 0) {
				abrt.apduAbortRejectReason = (int)bufferOverflowAbort;
				goto abort;
			}
			tsmLastSequenceNumber = (tsmLastSequenceNumber + 1) % 256;
			
			// last segment?
			if (!apdu.apduMor) {
				StopTimer();
				
				// send an ack to the server
				Request( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 0, 0, tsmLastSequenceNumber, tsmActualWindowSize ) );
				
				// back to idle.  If the client decides that it has another request to send 
				// before returning from the call to its Confirmation(), this would send this
				// CTSM out of idle and may require the tsmSeg ptr to be clear.
				SetState( tsmIdle );
				tempSeg = tsmSeg;
				tsmSeg = 0;
				
				// send complete apdu to the client
				Response( tempSeg->ResultAPDU() );
				
				// now it is OK to delete it
				delete tempSeg;
				break;
			}
			
			// last in the group?
			if (apdu.apduSeq == ((tsmInitialSequenceNumber + tsmActualWindowSize) % 256)) {
				RestartTimer( tsmDevice->deviceAPDUSegmentTimeout );
				Request( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 0, 0, tsmLastSequenceNumber, tsmActualWindowSize ) );
				tsmInitialSequenceNumber = tsmLastSequenceNumber;
			}
			break;
		
		case errorPDU:
		case rejectPDU:
		case abortPDU:
			// cancel timer
			StopTimer();
			
			// send abort to the client
			Response( apdu );
			
			// return to idle
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
	}
}

//
//	BACnetClientTSM::SegmentedConfirmationTimeout
//

void BACnetClientTSM::SegmentedConfirmationTimeout( void )
{
#if _TSMDebug
	cout << "[BACnetClientTSM::SegmentedConfirmationTimeout]" << endl;
#endif

	// return to idle
	SetState( tsmIdle );
	if (tsmSeg) {
		delete tsmSeg;
		tsmSeg = 0;
	}
	
	// send an error message back to application
	Response( BACnetAbortAPDU( tsmAddr, 0, tsmInvokeID, apduTimeoutAbort ) );
}
