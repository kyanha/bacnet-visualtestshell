
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "BACnet.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	BACnetDevice::BACnetDevice
//

BACnetDevice::BACnetDevice( void )
	: deviceClients(0), deviceServers(0)
{
	// defaults
	deviceSegmentation = segmentedBoth;		// supports segments requests
	deviceWindowSize = 5;					// how many to send
	deviceNextInvokeID = 1;					// next invoke ID for client
	deviceNext = 0;							// empty list of known devices
	
	// these parameters should come from the endpoint somehow
	deviceSegmentSize = 1024;				// how big to divide up chunks
	deviceMaxAPDUSize = 1024;				// maximum APDU size
	
	deviceAPDUTimeout = 3000;				// how long to wait for ack
	deviceAPDUSegmentTimeout = 1000;		// how long to wait between segments
	deviceAPDURetries = 5;					// how many retries are acceptable
}

//
//	BACnetDevice::Bind
//

void BACnetDevice::Bind( BACnetClientPtr cp )
{
	cp->clientNext = deviceClients;
	deviceClients = cp;
}

//
//	BACnetDevice::Unbind
//

void BACnetDevice::Unbind( BACnetClientPtr cp )
{
	BACnetClientPtr	*cur = &deviceClients
	;
	
	// find it
	while (*cur && (*cur != cp))
		cur = &(*cur)->clientNext;
	
	// found it?
	if (*cur)
		*cur = cp->clientNext;
}

//
//	BACnetDevice::Bind
//

void BACnetDevice::Bind( BACnetServerPtr sp )
{
	sp->serverNext = deviceServers;
	deviceServers = sp;
}

//
//	BACnetDevice::Unbind
//

void BACnetDevice::Unbind( BACnetServerPtr sp )
{
	BACnetServerPtr	*cur = &deviceServers
	;
	
	// find it
	while (*cur && (*cur != sp))
		cur = &(*cur)->serverNext;
	
	// found it?
	if (*cur)
		*cur = sp->serverNext;
}

//
//	BACnetDevice::GetInfo
//

BACnetDeviceInfoPtr BACnetDevice::GetInfo( unsigned int inst )
{
	BACnetDeviceInfoPtr		cur = this
	;
	
	// find it
	while (cur)
		if (cur->deviceInst == inst)
			return cur;
		else
			cur = cur->deviceNext;
	
	// not found
	return 0;
}

//
//	BACnetDevice::GetInfo
//

BACnetDeviceInfoPtr BACnetDevice::GetInfo( const BACnetAddress &addr )
{
	BACnetDeviceInfoPtr		cur = this
	;
	
	// find it
	while (cur)
		if (cur->deviceAddr == addr)
			return cur;
		else
			cur = cur->deviceNext;
	
	// not found
	return 0;
}

//
//	BACnetDevice::Indication
//
//	This function is called by a server or client TSM that has a packet to 
//	send.  This simply repackages the request and sends it along.  Just like the 
//	network layer was ripped off by BACnetRouter on upstream packets, it will be 
//	built correctly on downstream ones.
//

void BACnetDevice::Indication( const BACnetAPDU &apdu )
{
	BACnetAPDUEncoder	enc( apdu.pktLength + 6 )
	;
	
	// encode the apdu
	apdu.Encode( enc );

	// build an NPDU out of the results and send it down
	Request(
		BACnetNPDU( apdu.apduAddr, enc.pktBuffer, enc.pktLength
			, apdu.apduExpectingReply, apdu.apduNetworkPriority
			)
		);
}

//
//	BACnetDevice::Confirmation
//
//	Some message from the network layer has been received and it should be 
//	interpreted.  If this is a new request (or unconfirmed request) pass it 
//	along to an idle server.  If this is a ack, error, or reject, pass it 
//	along to the client that is expecting it.  If this is an abort it may be 
//	related to a request or response.
//

void BACnetDevice::Confirmation( const BACnetNPDU &npdu )
{
	BACnetAPDU				apdu( 0 )
	;
	BACnetClientPtr			cp = deviceClients
	;
	BACnetServerPtr			sp = deviceServers
	;
	
	// decode the contents
	apdu.apduAddr = npdu.pduAddr;
	apdu.Decode( BACnetAPDUDecoder( npdu ) );

	// now do something with it
	switch (apdu.apduType) {
		case confirmedRequestPDU:
#if _TSMDebug
			cout << "[BACnetDevice::confirmedRequestPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
#endif
			
			// look for the bound server
			while (sp)
				if ( (sp->serverTSM.tsmState != tsmIdle)
					&& (sp->serverTSM.tsmInvokeID == apdu.apduInvokeID)
					&& (sp->serverTSM.tsmAddr == npdu.pduAddr)
					) {
					sp->serverTSM.Indication( apdu );
					break;
				} else
					sp = sp->serverNext;
			
			// if we didn't find a match, start a new one
			if (!sp) {
				// look for an idle server
				sp = deviceServers;
				while (sp)
					if (sp->serverTSM.tsmState == tsmIdle) {
						// bind it
						sp->serverTSM.tsmAddr = npdu.pduAddr;
						sp->serverTSM.tsmInvokeID = apdu.apduInvokeID;
						
						// forward the request along
						sp->serverTSM.Indication( apdu );
						break;
					} else
						sp = sp->serverNext;
#if _TSMDebug
				if (!sp)
					cout << "[BACnetDevice::confirmedRequestPDU: no idle servers available]" << endl;
#endif
			}
			break;
			
		case unconfirmedRequestPDU:
#if _TSMDebug
			cout << "[BACnetDevice::unconfirmedRequestPDU]" << endl;
#endif
			
			// look for an idle server
			while (sp)
				if (sp->serverTSM.tsmState == tsmIdle) {
					// bind it
					sp->serverTSM.tsmAddr = npdu.pduAddr;
					
					// forward the request along
					sp->serverTSM.Indication( apdu );
					break;
				} else
					sp = sp->serverNext;
#if _TSMDebug
			if (!sp)
				cout << "[BACnetDevice::unconfirmedRequestPDU: no idle servers available]" << endl;
#endif
			break;
			
		case simpleAckPDU:
#if _TSMDebug
			cout << "[BACnetDevice::simpleAckPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
#endif
			
			// find the client
			while (cp)
				if ((cp->clientTSM.tsmInvokeID == apdu.apduInvokeID) && (cp->clientTSM.tsmState != tsmIdle)) {
					cp->clientTSM.Confirmation( apdu );
					break;
				} else
					cp = cp->clientNext;
#if _TSMDebug
			if (!cp)
				cout << "[BACnetDevice::simpleAck: no client matching invokeID " << apdu.apduInvokeID << " found]" << endl;
#endif
			break;
			
		case complexAckPDU:
#if _TSMDebug
			cout << "[BACnetDevice::complexAckPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
#endif
			
			// find the client
			while (cp)
				if ((cp->clientTSM.tsmInvokeID == apdu.apduInvokeID) && (cp->clientTSM.tsmState != tsmIdle)) {
					cp->clientTSM.Confirmation( apdu );
					break;
				} else
					cp = cp->clientNext;
#if _TSMDebug
			if (!cp)
				cout << "[BACnetDevice::complexAckPDU: no client matching invokeID " << apdu.apduInvokeID << " found]" << endl;
#endif
			break;
			
		case errorPDU:
#if _TSMDebug
			cout << "[BACnetDevice::errorPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
#endif
			
			// find the client
			while (cp)
				if ((cp->clientTSM.tsmInvokeID == apdu.apduInvokeID) && (cp->clientTSM.tsmInvokeID != tsmIdle)) {
					cp->clientTSM.Confirmation( apdu );
					break;
				} else
					cp = cp->clientNext;
#if _TSMDebug
			if (!cp)
				cout << "[BACnetDevice::error/rejectPDU: no client matching invokeID " << apdu.apduInvokeID << " found]" << endl;
#endif
			break;
			
		case rejectPDU:
#if _TSMDebug
			cout << "[BACnetDevice::rejectPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
#endif
			
			// find the client
			while (cp)
				if ((cp->clientTSM.tsmInvokeID == apdu.apduInvokeID) && (cp->clientTSM.tsmInvokeID != tsmIdle)) {
					cp->clientTSM.Confirmation( apdu );
					break;
				} else
					cp = cp->clientNext;
#if _TSMDebug
			if (!cp)
				cout << "[BACnetDevice::error/rejectPDU: no client matching invokeID " << apdu.apduInvokeID << " found]" << endl;
#endif
			break;
			
		case segmentAckPDU:
		case abortPDU:
#if _TSMDebug
			if (apdu.apduType == segmentAckPDU)
				cout << "[BACnetDevice::segmentAckPDU: invokeID " << apdu.apduInvokeID << "]" << endl;
			else
				cout << "[BACnetDevice::abortPDU: invokeID " << apdu.apduInvokeID
					<< ", reason = " << apdu.apduAbortRejectReason
					<< "]" << endl;
#endif
			
			if (apdu.apduSrv) {
				// server sent the abort, find the client
				while (cp)
					if ((cp->clientTSM.tsmInvokeID == apdu.apduInvokeID) && (cp->clientTSM.tsmInvokeID != tsmIdle)) {
						cp->clientTSM.Confirmation( apdu );
						break;
					} else
						cp = cp->clientNext;
#if _TSMDebug
				if (!cp)
					cout << "[BACnetDevice::segmentAck/abortPDU: no client matching invokeID " << apdu.apduInvokeID << " found]" << endl;
#endif
			} else {
				// client sent the abort, look for the server
				while (sp)
					if ((sp->serverTSM.tsmInvokeID == apdu.apduInvokeID) && (sp->serverTSM.tsmAddr == npdu.pduAddr)) {
						sp->serverTSM.Indication( apdu );
						break;
					} else
						sp = sp->serverNext;
#if _TSMDebug
				if (!sp)
					cout << "[BACnetDevice::segmentAck/abortPDU: no server matching invokeID " << apdu.apduInvokeID << " found]" << endl;
#endif
			}
			break;
			
		default:
#if _TSMDebug
			cout << "[BACnetDevice::*** unknown PDU type ***]" << endl;
#endif
			break;
	}
}

//
//	BACnetDevice::GetInvokeID
//
//	Called by a CTSM when it needs a new invoke ID, and to make sure that it 
//	isn't in use by some other client.  If a client is idle, it's not actually 
//	using the invoke ID that is sitting in BACnetTSM::tsmInvokeID.
//

int BACnetDevice::GetInvokeID( void )
{
	int				newID
	;
	BACnetClientPtr	cur
	;
	
again:
	newID = deviceNextInvokeID;
	deviceNextInvokeID = ((deviceNextInvokeID + 1) % 256);
	
	for (cur = deviceClients; cur; cur = cur->clientNext)
		if ((cur->clientTSM.tsmState != tsmIdle) && (cur->clientTSM.tsmInvokeID == newID))
			goto again;
	
	return newID;
}

//
//	MaxAPDUEncode
//

BACnetOctet MaxAPDUEncode( int siz )
{
	switch (siz) {
		case   50:	return 0;
		case  128:	return 1;
		case  206:	return 2;
		case  480:	return 3;
		case 1024:	return 4;
		case 1476:	return 5;
		default:
			return 0;
	}
}

//
//	MaxAPDUDecode
//

int MaxAPDUDecode( BACnetOctet siz )
{
	switch (siz) {
		case 0:	return 50;
		case 1:	return 128;
		case 2:	return 206;
		case 3:	return 480;
		case 4:	return 1024;
		case 5:	return 1476;
		default:
			return 0;
	}
}
