
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
//	BACnetServerTSM::BACnetServerTSM
//

BACnetServerTSM::BACnetServerTSM( BACnetDevicePtr dp )
	: BACnetTSM( dp )
{
}

//
//	BACnetServerTSM::~BACnetServerTSM
//

BACnetServerTSM::~BACnetServerTSM( void )
{
}


//
//	BACnetServerTSM::Response
//
//	When the server has a message to send to the device, it calls this function.  It 
//	simply passes the information along to the device.  According to the client/server 
//	pattern, this function would normally call Confirmation(), but in this case the 
//	device is acting as a server to the STSM.
//

void BACnetServerTSM::Response( const BACnetAPDU &pdu )
{
	tsmDevice->Indication( pdu );
}
		
//
//	BACnetServerTSM::Indication
//
//	When the device has an incoming request that needs to be processed, it calls this 
//	function.  If this is a confirmed request, send it along to the server by calling 
//	Request() and wait for the response.  If this is unconfirmed, do the same thing but 
//	don't bother making the transition out of idle.
//
//	This may be an abort, which should also be forwarded, and go back to idle.
//

void BACnetServerTSM::Indication( const BACnetAPDU &apdu )
{
	switch (tsmState) {
		case tsmIdle:
			Idle( apdu );
			break;
		case tsmSegmentedRequest:
			SegmentedRequest( apdu );
			break;
		case tsmAwaitResponse:
			AwaitResponse( apdu );
			break;
		case tsmSegmentedResponse:
			SegmentedResponse( apdu );
			break;
		case tsmAwaitConfirmation:
		case tsmSegmentedConfirmation:
		case tsmBusy:
			break;
	}
}

//
//	BACnetServerTSM::Confirmation
//
//	This function is called when the server has an unconfirmed request to send, or a 
//	response to send back to the client.
//

void BACnetServerTSM::Confirmation( const BACnetAPDU &apdu )
{
	switch (apdu.apduType) {
		case unconfirmedRequestPDU:
			// allow server objects to send unconfirmed requests (usually these are 
			// sent from a BACnetClient)
			Response( apdu );
			break;
			
		case confirmedRequestPDU:
		case segmentAckPDU:
			throw_(1); // ###
		
		case complexAckPDU:
			// compute the segment count
			tsmSegmentCount = (apdu.pktLength + tsmSegmentSize - 4) / tsmSegmentSize;
			
			// might not need segmentation
			if (tsmSegmentCount == 1)
				goto fini;
			
			// verify request said the device supports a segmented reply
			if (	(tsmSegmentation == segmentedTransmit)
				 || (tsmSegmentation == noSegmentation)
					) {
				StopTimer();
				SetState( tsmIdle );
				// tell device to abort
				Response( BACnetAbortAPDU( tsmAddr, 1, tsmInvokeID, segmentationNotSupportedAbort ) );
				break;
			}
			
			// prepare for segmented response
			tsmSeg = new BACnetAPDUSegment( apdu, this );
			
			tsmSegmentsSent = 0;
			tsmSegmentRetryCount = 0;
			tsmInitialSequenceNumber = 0;
			tsmProposedWindowSize = tsmDevice->deviceWindowSize;
			tsmActualWindowSize = 1;
			StartTimer( tsmDevice->deviceAPDUSegmentTimeout );
			SetState( tsmSegmentedResponse );
			Response( (*tsmSeg)[0] );
			break;
			
		case simpleAckPDU:
		case errorPDU:
		case rejectPDU:
		case abortPDU:
fini:		StopTimer();
			SetState( tsmIdle );
			Response( apdu );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
	}
}

//
//	BACnetServerTSM::ProcessTask
//
//	Called when a timer expires.  Which timer depends on what state the server is in.
//

void BACnetServerTSM::ProcessTask( void )
{
	switch (tsmState) {
		case tsmSegmentedRequest:
			SegmentedRequestTimeout();
			break;
		case tsmSegmentedResponse:
			SegmentedResponseTimeout();
			break;
		default:
			throw_(2); // ###
	}
}

//
//	BACnetServerTSM::Idle
//
//	This function is called when the server is in an idle state and ready to process 
//	requests.
//

void BACnetServerTSM::Idle( const BACnetAPDU &apdu )
{
	
	switch (apdu.apduType) {
		case confirmedRequestPDU:
#if _TSMDebug
			cout << "[BACnetServerTSM::confirmedRequestPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
#endif
			// set the segmented reply accepted based on the request
			tsmSegmentation = (apdu.apduSA ? segmentedReceive : noSegmentation);
			
			// set the segment size going back to what the device gave us in this request
			tsmSegmentSize = apdu.apduMaxResp;
			
			// clip it to what can be sent
			if (tsmSegmentSize > tsmDevice->deviceMaxAPDUSize)
				tsmSegmentSize = tsmDevice->deviceMaxAPDUSize;
			
			// is this an unsegmented request?
			if (!apdu.apduSeg) {
				SetState( tsmAwaitResponse );
				Request( apdu );
				break;
			}
			
			// make sure our device is supposed to support segmentation
			if ( (tsmDevice->deviceSegmentation == segmentedTransmit)
				|| (tsmDevice->deviceSegmentation == noSegmentation)) {
				Response( BACnetAbortAPDU( apdu.apduAddr, 1, tsmInvokeID, segmentationNotSupportedAbort ) );
				break;
			}
			
			// must be first segment
			if (apdu.apduSeq != 0) {
				Response( BACnetAbortAPDU( apdu.apduAddr, 1, tsmInvokeID, invalidAPDUInThisStateAbort ) );
				break;
			}
			
			tsmSeg = new BACnetAPDUSegment( 2560, this );
			tsmSeg->AddSegment( apdu );
			
			// extract the proposed window size
			tsmProposedWindowSize = apdu.apduWin;
			
			// actual window size is the min of what we accept and what the device wants
			if (tsmProposedWindowSize > tsmDevice->deviceWindowSize)
				tsmActualWindowSize = tsmDevice->deviceWindowSize;
			else
				tsmActualWindowSize = tsmProposedWindowSize;
			
			tsmLastSequenceNumber = 0;
			tsmInitialSequenceNumber = 0;
			SetState( tsmSegmentedRequest );
			Response( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 0, 1, 0, tsmActualWindowSize ) );
			StartTimer( tsmDevice->deviceAPDUSegmentTimeout * 4 );
			break;
			
		case unconfirmedRequestPDU:
#if _TSMDebug
			cout << "[BACnetServerTSM::unconfirmedRequestPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
#endif
			// pass it along to workstation
			Request( apdu );
			break;
			
		case simpleAckPDU:
		case complexAckPDU:
		case segmentAckPDU:
		case errorPDU:
		case rejectPDU:
#if _TSMDebug
			cout << "[BACnetServerTSM::other]" << endl;
#endif
			Response( BACnetAbortAPDU( apdu.apduAddr, 1, tsmInvokeID, invalidAPDUInThisStateAbort ) );
			break;
		
		case abortPDU:
#if _TSMDebug
			cout << "[BACnetServerTSM::abortPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
#endif
			break;
	}
}

//
//	BACnetServerTSM::SegmentedRequest
//

void BACnetServerTSM::SegmentedRequest( const BACnetAPDU &apdu )
{
	BACnetAPDUSegmentPtr	tempSeg
	;
	
#if _TSMDebug
	cout << "[BACnetServerTSM::SegmentedRequest]" << endl;
#endif

	switch (apdu.apduType) {
		case confirmedRequestPDU:
			// must be segmented
			if (!apdu.apduSeg) {
				StopTimer();
				SetState( tsmIdle );
				if (tsmSeg) {
					delete tsmSeg;
					tsmSeg = 0;
				}
				Response( BACnetAbortAPDU( tsmAddr, 1, tsmInvokeID, invalidAPDUInThisStateAbort ) );
				break;
			}
			
			// proper segment number?
			if (apdu.apduSeq != ((tsmLastSequenceNumber + 1) % 256)) {
				RestartTimer( tsmDevice->deviceAPDUSegmentTimeout );
				Response( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 1, 1, tsmLastSequenceNumber, tsmActualWindowSize ) );
				tsmInitialSequenceNumber = tsmLastSequenceNumber;
				break;
			}
			
			// will it fit?
			if (tsmSeg->AddSegment( apdu ) != 0) {
				StopTimer();
				SetState( tsmIdle );
				if (tsmSeg) {
					delete tsmSeg;
					tsmSeg = 0;
				}
				Response( BACnetAbortAPDU( tsmAddr, 1, tsmInvokeID, bufferOverflowAbort ) );
			}
			tsmLastSequenceNumber = (tsmLastSequenceNumber + 1) % 256;
			
			// last segment?
			if (!apdu.apduMor) {
				StopTimer();
				
				// send an ack to the server
				Response( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 0, 1, tsmLastSequenceNumber, tsmActualWindowSize ) );
				
				// make the state transition.  It is possible that the server will satisify 
				// the request right off, and the response is going to be stored in the 
				// tsmSeg ptr.
				SetState( tsmAwaitResponse );
				tempSeg = tsmSeg;
				tsmSeg = 0;
				
				// send complete apdu to the workstation
				Request( tempSeg->ResultAPDU() );
				
				// it's OK to delete the buffer now
				delete tempSeg;
				break;
			}
			
			// last in the group?
			if (apdu.apduSeq == ((tsmInitialSequenceNumber + tsmActualWindowSize) % 256)) {
				RestartTimer( tsmDevice->deviceAPDUSegmentTimeout * 4 );
				Response( BACnetSegmentAckAPDU( tsmAddr, tsmInvokeID, 0, 1, tsmLastSequenceNumber, tsmActualWindowSize ) );
				tsmInitialSequenceNumber = tsmLastSequenceNumber;
			}
			break;
			
		case unconfirmedRequestPDU:
		case simpleAckPDU:
		case complexAckPDU:
		case segmentAckPDU:
		case errorPDU:
		case rejectPDU:
			// ???
			break;
			
		case abortPDU:
			StopTimer();
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
	}
}

//
//	BACnetServerTSM::SegmentedRequestTimeout
//

void BACnetServerTSM::SegmentedRequestTimeout( void )
{
#if _TSMDebug
	cout << "[BACnetServerTSM::SegmentedRequestTimeout]" << endl;
#endif

	// return to idle
	SetState( tsmIdle );
	if (tsmSeg) {
		delete tsmSeg;
		tsmSeg = 0;
	}
	
	// send an error message back to the device
	Response( BACnetAbortAPDU( tsmAddr, 1, tsmInvokeID, apduTimeoutAbort ) );
}

//
//	BACnetServerTSM::AwaitResponse
//
//	In this state, the TSM is waiting for a message from the server 
//	that will satisfy the request from the client device.  This function is 
//	called when there is some message from the client, only the abort 
//	makes any real sense.  Chances are the Confirmation() function will 
//	be called to send the response and this state will exit.
//

void BACnetServerTSM::AwaitResponse( const BACnetAPDU &apdu )
{
#if _TSMDebug
	cout << "[BACnetServerTSM::AwaitResponse]" << endl;
#endif

	switch (apdu.apduType) {
		case confirmedRequestPDU:
		case unconfirmedRequestPDU:
		case simpleAckPDU:
		case complexAckPDU:
		case segmentAckPDU:
		case errorPDU:
		case rejectPDU:
			break;
			
		case abortPDU:
			StopTimer();
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			// forward a copy to the server
			Request( apdu );
			break;
	}
}

//
//	BACnetServerTSM::SegmentedResponse
//

void BACnetServerTSM::SegmentedResponse( const BACnetAPDU &apdu )
{
#if _TSMDebug
	cout << "[BACnetServerTSM::SegmentedResponse]" << endl;
#endif

	switch (apdu.apduType) {
		case confirmedRequestPDU:
		case unconfirmedRequestPDU:
		case simpleAckPDU:
		case complexAckPDU:
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
				SetState( tsmIdle );
				if (tsmSeg) {
					delete tsmSeg;
					tsmSeg = 0;
				}
				break;
			}
			
			tsmSegmentRetryCount = 0;
			tsmInitialSequenceNumber = tsmSegmentsSent % 256;
			tsmActualWindowSize = apdu.apduWin;
			RestartTimer( tsmDevice->deviceAPDUSegmentTimeout );
			FillWindow( tsmSegmentsSent );
			break;
			
		case errorPDU:
		case rejectPDU:
		case abortPDU:
			StopTimer();
			SetState( tsmIdle );
			if (tsmSeg) {
				delete tsmSeg;
				tsmSeg = 0;
			}
			break;
	}
}

//
//	BACnetServerTSM::SegmentedResponseTimeout
//

void BACnetServerTSM::SegmentedResponseTimeout( void )
{
#if _TSMDebug
	cout << "[BACnetServerTSM::SegmentedResponseTimeout]" << endl;
#endif

	if (tsmSegmentRetryCount < tsmDevice->deviceAPDURetries) {
		tsmSegmentRetryCount += 1;
		StartTimer( tsmDevice->deviceAPDUSegmentTimeout );
		FillWindow( tsmSegmentsSent );
	} else {
		// return to idle
		SetState( tsmIdle );
		if (tsmSeg) {
			delete tsmSeg;
			tsmSeg = 0;
		}
	}
}