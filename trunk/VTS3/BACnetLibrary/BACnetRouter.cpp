
#include "stdafx.h"

#include <stdlib.h>
#include <string.h>

#include "BACnet.hpp"
#include "BACnetRouter.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	BACnetRouterAdapter::Confirmation
//

void BACnetRouterAdapter::Confirmation( const BACnetNPDU &pdu )
{
	adapterRouter->ProcessNPDU( this, pdu );
}

//
//	BACnetRouter::BindToEndpoint
//
//	Binding a router object to an endpoint needs an adapter to associate a network 
//	number with that endpoint.
//

void BACnetRouter::BindToEndpoint( BACnetNetServerPtr endp, int net )
{
	if (adapterListLen == kBACnetRouterMaxAdapters)
		throw (-1); // out of adapter space
	
	for (int i = 0; i < adapterListLen; i++)
		if (adapterList[i]->adapterNet == net)
			throw (-1); // already an adapter for this network
	
	// create a new adapter that refers back to this router
	BACnetRouterAdapterPtr	adapter = new BACnetRouterAdapter( net, this );
	
	// add it to the list
	adapterList[adapterListLen++] = adapter;
	
	// bind the adapter (client) to the endpoint (server)
	Bind( adapter, endp );
}

//
//	BACnetRouter::ProcessNPDU
//
//	This function is called by BACnetRouterAdapter::Confirmation() when it receives
//	a PDU coming up from the endpoint.  All it does is pass along the pdu with a 
//	pointer to itself so the router knows which endpoint it came from.
//
//	There are two types of messages: those specific to the network layer, and those 
//	that are repackaged as APDU's and sent up to the application layer.  The current 
//	version of the software only supports one application layer object to be attached 
//	to the router.  If it is necessary to have different BACnetAppClients on different 
//	BACnetNetServers, the two cannot share a BACnetRouter.
//

void BACnetRouter::ProcessNPDU( BACnetRouterAdapterPtr adapter, const BACnetNPDU &npdu )
{
	int					netLayerMessage, dnetPresent, snetPresent
	,					dataExpectingReply, networkPriority, hopCount
	,					i, apduLen, msgType, vendorID
	;
	BACnetAddress		srcAddr, destAddr, *addrPtr
	;
	const BACnetOctet	*npci = npdu.pduData, *apduData
	;
	BACnetOctet			*msg, *mptr
	;
	
	// check the length
	if (npdu.pduLen < 2)
		return;	// invalid length
	
	// only version 1 messages supported
	if (*npci++ != 0x01)
		return; // version 1 only
	
	// extract the flags
	netLayerMessage = (*npci & 0x80);
	dnetPresent = (*npci & 0x20);
	snetPresent = (*npci & 0x08);
	dataExpectingReply = (*npci & 0x04);
	networkPriority = (*npci & 0x03);
	npci += 1;
	
	// extract the destination address
	if (dnetPresent) {
		int		dnet, dlen
		;
		
		dnet = *npci++;
		dnet = (dnet << 8) + *npci++;
		dlen = *npci++;
		
		if (dnet == 0xFFFF)
			destAddr.addrType = globalBroadcastAddr;
		else {
			destAddr.addrNet = dnet;
			if (dlen == 0)
				destAddr.addrType = remoteBroadcastAddr;
			else {
				destAddr.addrType = remoteStationAddr;
				destAddr.addrLen = dlen;
				memcpy( destAddr.addrAddr, npci, dlen );
				npci += dlen;
			}
		}
	}
	
	// extract the source address, or copy the one from the endpoint
	if (snetPresent) {
		int		snet, slen
		;
		
		snet = *npci++;
		snet = (snet << 8) + *npci++;
		slen = *npci++;
		
		if (slen == 0xFFFF)
			throw (-1); // source can't be a global broadcast
		if (slen == 0)
			throw (-1); // source can't be a remote broadcast
		
		srcAddr.addrType = remoteStationAddr;
		srcAddr.addrNet = snet;
		srcAddr.addrLen = slen;
		memcpy( srcAddr.addrAddr, npci, slen );
		npci += slen;
	} else {
		srcAddr = npdu.pduAddr;						// source was a local station
		srcAddr.addrNet = adapter->adapterNet;		// handy if it needs forwarding
	}
	
	// extract the hop count
	if (dnetPresent)
		hopCount = *npci++;
	
	// extract the network layer message type (if present)
	if (netLayerMessage) {
		msgType = *npci++;
		if ((msgType >= 0x80) && (msgType <= 0xFF)) {
			// extract the vendor ID
			vendorID = *npci++;
			vendorID = (vendorID << 8) + *npci++;
		}
	}
	
	// check to see if this is our network layer message
	if (netLayerMessage && (!dnetPresent)) {
		// check for vendor specific message
		if ((msgType >= 0x80) && (msgType <= 0xFF)) {
			if (vendorID == kVendorID)
				ProcessVendorNetMessage( adapter, msgType, srcAddr, destAddr, npci, npdu.pduLen - (npci - npdu.pduData) );
		} else
			ProcessNetMessage( adapter, msgType, srcAddr, destAddr, npci, npdu.pduLen - (npci - npdu.pduData) );
		return;
	}
	
	// set up these for easier processing
	apduData = npci;
	apduLen = npdu.pduLen - (npci - npdu.pduData);
	
	//
	//	This is an application layer message that we should send up to the application layer 
	//	object iff: it is not a network layer message, it has no DNET/DADR (which means it 
	//	doesn't need to be forwarded on) or it is a global broadcast (which means everybody 
	//	should get a chance to process it).
	//
	if ((!netLayerMessage) && ((!dnetPresent) || (destAddr.addrType == globalBroadcastAddr))) {
		// only forward to the application layer if we are bound to one
		if (serverPeer)
			Response( BACnetNPDU( srcAddr, apduData, apduLen, dataExpectingReply, networkPriority ) );
		
		// don't continue processing if this was just for our benefit
		if (!dnetPresent)
			return;
	}
	
#if 0
	//
	//	Check to see if we can do any routing.  If the adapter the message came 
	//	from is our local network, we don't know what network we are connected
	//	to and the adapterNet is -1.  This ProcessNPDU() function we are in is
	//	only called from a connected adapter, so our adapterListLen is at least
	//	one.
	//
	if (adapter->adapterNet == -1)
		return;
#endif
	// don't bother to check for others if there is only one!
	if (adapterListLen == 1)
		return;
		
	// make sure it hasn't been looped around
	if (hopCount == 0)
		return;
	hopCount -= 1;
	
	//
	//	A global broadcast message gets forwarded to all of the adapters except the 
	//	one the original message came from.
	//
	if (destAddr.addrType == globalBroadcastAddr) {
		mptr = msg = new BACnetOctet[ 10 + srcAddr.addrLen + apduLen ];
		
		*mptr++ = 0x01;										// version
		// build a message to be broadcast on all endpoints, include source address
		*mptr++ = (netLayerMessage) + 0x28 + (dataExpectingReply << 2) + (networkPriority);
		*mptr++ = 0xFF;										// DNET
		*mptr++ = 0xFF;
		*mptr++ = 0;										// DLEN
		*mptr++ = (srcAddr.addrNet >> 8) & 0xFF;			// SNET
		*mptr++ = (srcAddr.addrNet & 0xFF);
		*mptr++ = srcAddr.addrLen;							// SLEN
		memcpy( mptr, srcAddr.addrAddr, srcAddr.addrLen );	// SADR
		mptr += srcAddr.addrLen;
		*mptr++ = hopCount;									// hop count
		if (netLayerMessage) {
			*mptr++ = msgType;
			if ((msgType >= 0x80) && (msgType <= 0xFF)) {
				*mptr++ = (vendorID >> 8) & 0xFF;
				*mptr++ = (vendorID & 0xFF);
			}
		}
		memcpy( mptr, apduData, apduLen );
		mptr += apduLen;
		
		// build a local broadcast NPDU
		BACnetNPDU	npdu( BACnetAddress( localBroadcastAddr ), msg, (mptr - msg), dataExpectingReply, networkPriority );
		
		// send it to all adapters, except the one it came from
		for (i = 0; i < adapterListLen; i++)
			if (adapter != adapterList[i])
				adapterList[i]->Request( npdu );
		
		// toss the buffer
		delete[] msg;
		return;
	}
	
	//
	//	See if this message should go to one of our adapters.  If it does, include the 
	//	SNET/SADR from the source of this packet.
	//
	for (i = 0; i < adapterListLen; i++)
		if (adapterList[i]->adapterNet == destAddr.addrNet)
			break;
	if (i < adapterListLen) {
		// don't route to self adapter
		if (adapterList[i] == adapter)
			return; // requesting routing back to itself
		
		mptr = msg = new BACnetOctet[ 3 + apduLen ];
		
		*mptr++ = 0x01;										// version
		// build the header, append the APDU
		*mptr++ = (netLayerMessage) + 0x08 + (dataExpectingReply << 2) + (networkPriority);
		*mptr++ = (srcAddr.addrNet >> 8) & 0xFF;			// SNET
		*mptr++ = (srcAddr.addrNet & 0xFF);
		*mptr++ = srcAddr.addrLen;							// SLEN
		memcpy( mptr, srcAddr.addrAddr, srcAddr.addrLen );	// SADR
		mptr += srcAddr.addrLen;
		if (netLayerMessage) {
			*mptr++ = msgType;
			if ((msgType >= 0x80) && (msgType <= 0xFF)) {
				*mptr++ = (vendorID >> 8) & 0xFF;
				*mptr++ = (vendorID & 0xFF);
			}
		}
		memcpy( mptr, apduData, apduLen );
		mptr += apduLen;
		
		// if this is a remote broadcast, it turns into a local broadcast here
		if (destAddr.addrType == remoteBroadcastAddr)
			addrPtr = new BACnetAddress( localBroadcastAddr );
		else
			addrPtr = new BACnetAddress( destAddr.addrAddr, destAddr.addrLen );
		
		// send it along
		adapterList[i]->Request( BACnetNPDU( *addrPtr, msg, (mptr - msg), dataExpectingReply, networkPriority ) );
		delete addrPtr;
		
		// toss the buffer
		delete[] msg;
		return;
	}
	
	//
	//	The packet can't be sent directly to a local station on our of our adapters,
	//	so there is probably a router connected to us somehow that should know how 
	//	to deliver it.
	//
	for (i = 0; i < routerListLen; i++)
		if (routerList[i].routerNet == destAddr.addrNet)
			break;
	if (i < routerListLen) {
		mptr = msg = new BACnetOctet[ 10 + destAddr.addrLen + srcAddr.addrLen + apduLen ];
		
		// build a message to be routed
		*mptr++ = 0x01;										// version
		*mptr++ = (netLayerMessage) + 0x28 + (dataExpectingReply << 2) + (networkPriority);
		*mptr++ = (destAddr.addrNet >> 8);							// DNET
		*mptr++ = (destAddr.addrNet & 0x00FF);
		*mptr++ = destAddr.addrLen;									// DLEN
		if (destAddr.addrLen != 0) {
			memcpy( mptr, destAddr.addrAddr, destAddr.addrLen );	// DADR
			mptr += destAddr.addrLen;
		}
		*mptr++ = (srcAddr.addrNet >> 8) & 0xFF;					// SNET
		*mptr++ = (srcAddr.addrNet & 0xFF);
		*mptr++ = srcAddr.addrLen;									// SLEN
		memcpy( mptr, srcAddr.addrAddr, srcAddr.addrLen );			// SADR
		mptr += srcAddr.addrLen;
		*mptr++ = hopCount;											// hop count
		if (netLayerMessage) {
			*mptr++ = msgType;
			if ((msgType >= 0x80) && (msgType <= 0xFF)) {
				*mptr++ = (vendorID >> 8) & 0xFF;
				*mptr++ = (vendorID & 0xFF);
			}
		}
		
		memcpy( mptr, apduData, apduLen );
		mptr += apduLen;
		
		// build a PDU, using the correct localStationAddr
		BACnetNPDU	npdu( routerList[i].routerAddr, msg, (mptr - msg), dataExpectingReply, networkPriority );
		
		// send it along
		routerList[i].routerAdapter->Request( npdu );
		
		// toss the buffer
		delete[] msg;
		return;
	}
	
	// This is bad news for this packet.  Our last chance was to see if there was somebody 
	// that could deal with it, but there isn't.  Send back a Reject-Message-To-Network.
	// We could start up a state machine and see if one of our other adapters has a path 
	// to get there, then process the packet again.  Somebody thinks we are a router to 
	// that network, give them the benefit of the doubt.
	mptr = msg = new BACnetOctet[ 10 + destAddr.addrLen + srcAddr.addrLen + apduLen ];
	
	// build a message to be routed
	*mptr++ = 0x01;											// version
	// routed to get here?
	if (srcAddr.addrNet == adapter->adapterNet)
		*mptr++ = 0x80 + (networkPriority);
	else {
		*mptr++ = 0xA0 + (networkPriority);
		*mptr++ = (srcAddr.addrNet >> 8);							// DNET
		*mptr++ = (srcAddr.addrNet & 0x00FF);
		*mptr++ = srcAddr.addrLen;									// DLEN
		if (srcAddr.addrLen != 0) {
			memcpy( mptr, srcAddr.addrAddr, srcAddr.addrLen );		// DADR
			mptr += srcAddr.addrLen;
		}
		*mptr++ = hopCount;											// hop count
	}
	*mptr++ = (unsigned char)RejectMessageToNetwork;				// Reject-Message-To-Network
	*mptr++ = 0x01;													// not connected
	*mptr++ = (destAddr.addrNet >> 8);								// network number
	*mptr++ = (destAddr.addrNet & 0x00FF);
	
	// build a PDU, send it back the way it came
	adapter->Request(
		BACnetNPDU( npdu.pduAddr, msg, (mptr - msg)
			, dataExpectingReply, networkPriority
			)
		);
	
	// toss the buffer
	delete[] msg;
	return;
}

//
//	BACnetRouter::ProcessVendorNetMessage
//

void BACnetRouter::ProcessVendorNetMessage(
	BACnetRouterAdapterPtr adapter,
	const int msgType,
	const BACnetAddress &srcAddr,
	const BACnetAddress &destAddr,
	const BACnetOctet *msgPtr,
	const int msgLen
	)
{
#pragma unused (adapter,msgType,srcAddr,destAddr,msgPtr,msgLen)
}

//
//	BACnetRouter::ProcessNetMessage
//

void BACnetRouter::ProcessNetMessage(
	BACnetRouterAdapterPtr adapter,
	const int msgType,
	const BACnetAddress &srcAddr,
	const BACnetAddress &destAddr,
	const BACnetOctet *msgPtr,
	const int mLen
	)
{
#pragma unused (destAddr)
	int				i, j, net, found, msgLen = mLen
	,				netListLen, *netList
	;
	BACnetOctet		*msg, *mptr
	;
	
	// interpret type
	switch ((BACnetNetworkMessageType)msgType) {
	
		case WhoIsRouterToNetwork:
			// extract the network number
			if (msgLen == 0)
				found = 1;
			else {
				found = 0;
				net = (msgLen--,*msgPtr++);
				net = (net << 8) + (msgLen--,*msgPtr++);
			}
			
			// check for direct connected endpoints
			if (!found)
				for (i = 0; i < adapterListLen; i++)
					if ((adapterList[i] != adapter) && (adapterList[i]->adapterNet == net)) {
						found = 1;
						break;
					}
			
			// check for other routers I can get to
			if (!found)
				for (i = 0; i < routerListLen; i++)
					if ((routerList[i].routerAdapter != adapter) && (routerList[i].routerNet == net)) {
						found = 1;
						break;
					}
			
			if (found)
				BroadcastRoutingTable( adapter );
			else {
				// attempt to discover the router
				mptr = msg = new BACnetOctet[ 8 + srcAddr.addrLen ];
				*mptr++ = 0x01;										// version
				*mptr++ = 0x88;										// network + SNET/SADR present
				*mptr++ = (srcAddr.addrNet >> 8) & 0xFF;			// SNET
				*mptr++ = (srcAddr.addrNet & 0xFF);
				*mptr++ = srcAddr.addrLen;							// SLEN
				memcpy( mptr, srcAddr.addrAddr, srcAddr.addrLen );	// SADR
				mptr += srcAddr.addrLen;
				
				*mptr++ = (unsigned char)WhoIsRouterToNetwork;		// Who-Is-Router-To-Network
				*mptr++ = (net >> 8) & 0xFF;
				*mptr++ = (net & 0xFF);
				
				BACnetNPDU	npdu( BACnetAddress( localBroadcastAddr ), msg, (mptr - msg) );
				
				for (i = 0; i < adapterListLen; i++)
					if (adapterList[i] != adapter)
						adapterList[i]->Request( npdu );
				delete[] msg;
			}
			break;
			
		case IAmRouterToNetwork:
			// build an array of the new route list
			netListLen = msgLen / 2;
			netList = new int[netListLen];

			for (i = 0; i < netListLen; i++) {
				netList[i] = *msgPtr++;
				netList[i] = (netList[i] << 8) + *msgPtr++;
			}
			
			// delete existing entries
			BACnetRouterList	*src, *dst;
			src = dst = routerList;
			for (i = 0; i < routerListLen; i++) {
				for (j = 0; j < netListLen; j++)
					if (src->routerNet == netList[j])
						break;
				if (j >= netListLen)
					*dst++ = *src++;
			}
			
			// add these new entries
			for (i = 0; i < netListLen; i++) {
				dst->routerNet = netList[i];
				dst->routerAddr = srcAddr;
				dst->routerAdapter = adapter;
				dst += 1;
			}
			
			// update the router list length
			routerListLen = (dst - routerList);
			
			// tell everybody else our new table
			for (i = 0; i < adapterListLen; i++)
				if (adapterList[i] != adapter)
					BroadcastRoutingTable( adapterList[i] );
			break;
			
		case ICouldBeRouterToNetwork:
			break;
			
		case RejectMessageToNetwork:
			// ### pass up to application somehow
			break;
			
		case RouterBusyToNetwork:
			// ### pass up to application somehow
			break;
			
		case RouterAvailableToNetwork:
			// ### pass up to application somehow
			break;
			
		case InitializeRoutingTable:
			break;
			
		case InitializeRoutingTableAck:
			break;
			
		case EstablishConnectionToNetwork:
			break;
			
		case DisconnectConnectionToNetwork:
			break;
			
		default:
			throw (-1); // unknown message type
	}
}

//
//	BACnetRouter::BroadcastRoutingTable
//
//	This function builds an I-Am-Router-To-Network message containing all of the 
//	reachable networks for a particular adapter.  It is called by BroadcastRoutingTables
//	when during startup, and also in response to a Who-Is-Router-To-Network made 
//	by a device on one of the attached networks.
//

void BACnetRouter::BroadcastRoutingTable( BACnetRouterAdapterPtr adapter )
{
	int				i
	;
	BACnetOctet		*msg, *mptr
	;
	
	// build a message for this adapter
	mptr = msg = new BACnetOctet[ 3 + (2 * adapterListLen) + (2 * routerListLen) ];
	*mptr++ = 0x01;
	*mptr++ = 0x80;
	*mptr++ = (unsigned char)IAmRouterToNetwork;
	// append those directly connected
	for (i = 0; i < adapterListLen; i++)
		if ((adapterList[i] != adapter) && (adapterList[i]->adapterNet != -1)) {
			*mptr++ = (adapterList[i]->adapterNet >> 8) & 0xFF;
			*mptr++ = (adapterList[i]->adapterNet & 0xFF);
		}
	// append those reachable
	for (i = 0; i < routerListLen; i++)
		if (routerList[i].routerAdapter != adapter) {
			*mptr++ = (routerList[i].routerNet >> 8) & 0xFF;
			*mptr++ = (routerList[i].routerNet & 0xFF);
		}

	// send it out the adapter
	adapter->Request( BACnetNPDU( BACnetAddress( localBroadcastAddr ), msg, (mptr - msg) ) );

	// all done with the buffer
	delete[] msg;
}

//
//	BACnetRouter::BroadcastRoutingTables
//
//	This function broadcasts its routing tables to all adapters.  It is used during 
//	startup, and may also be called periodicly.
//

void BACnetRouter::BroadcastRoutingTables( void )
{
	// loop through the adapters
	for (int i = 0; i < adapterListLen; i++)
		BroadcastRoutingTable( adapterList[i] );
}

//
//	BACnetRouter::Indication
//
//	This PDU is coming down from a device and needs the properly formed NPDU header
//	attached to the front.  In terms of the protocol, this is really an APDU, but it is 
//	not a BACnetAPDU because I chose to move that encoding to a higher level (above 
//	segmentation).
//

void BACnetRouter::Indication( const BACnetNPDU &pdu )
{
	int						i
	;
	BACnetOctet				*msg, *mptr
	;
	
	// build a buffer as big as the longest header plus the pdu
	msg = new BACnetOctet[ 10 + pdu.pduAddr.addrLen + pdu.pduLen ];
	mptr = msg;
	
	// all messages start with the version
	*mptr++ = 0x01;
	
	// figure out which header to build
	switch (pdu.pduAddr.addrType) {
		case nullAddr:
			throw (-1); // can't send to a null address
			
		case localStationAddr:
		case localBroadcastAddr:
			// find the endpoint considered the local network
			if ((adapterListLen == 0) || (adapterList[0]->adapterNet != -1))
				throw (-1); // no local network defined
			
			// build the header, append the APDU
			*mptr++ = 0x00 + (pdu.pduExpectingReply << 2) + (pdu.pduNetworkPriority);
			memcpy( mptr, pdu.pduData, pdu.pduLen );
			mptr += pdu.pduLen;
			
			// send the packet along to the local endpoint
			adapterList[0]->
				Request(
					BACnetNPDU( pdu.pduAddr, msg, (mptr - msg)
						, pdu.pduExpectingReply, pdu.pduNetworkPriority
						)
					);
			break;
			
		case remoteStationAddr:
			// check for an adapter for the DNET
			for (i = 0; i < adapterListLen; i++)
				if (adapterList[i]->adapterNet == pdu.pduAddr.addrNet)
					break;
			if (i < adapterListLen) {
				// this is really a local network, but application layer doesn't know it
				*mptr++ = 0x00 + (pdu.pduExpectingReply << 2) + (pdu.pduNetworkPriority);
				memcpy( mptr, pdu.pduData, pdu.pduLen );
				mptr += pdu.pduLen;
				
				// build a PDU, making a new localStationAddr
				BACnetNPDU
					npdu( BACnetAddress( pdu.pduAddr.addrAddr, pdu.pduAddr.addrLen )
						, msg, (mptr - msg)
						, pdu.pduExpectingReply, pdu.pduNetworkPriority
						);
				
				// send it along
				adapterList[0]->Request( npdu );
			} else {
				// build a message to be routed
				*mptr++ = 0x20 + (pdu.pduExpectingReply << 2) + (pdu.pduNetworkPriority);
				*mptr++ = (pdu.pduAddr.addrNet >> 8);							// DNET
				*mptr++ = (pdu.pduAddr.addrNet & 0x00FF);
				*mptr++ = pdu.pduAddr.addrLen;									// DLEN
				memcpy( mptr, pdu.pduAddr.addrAddr, pdu.pduAddr.addrLen );		// DADR
				mptr += pdu.pduAddr.addrLen;
				*mptr++ = 0xFF;													// hopcount
				
				memcpy( mptr, pdu.pduData, pdu.pduLen );
				mptr += pdu.pduLen;
				
				// check for a router for the DNET
				for (i = 0; i < routerListLen; i++)
					if (routerList[i].routerNet == pdu.pduAddr.addrNet)
						break;
				if (i < routerListLen) {
					// build a PDU, using the correct localStationAddr
					BACnetNPDU
						npdu( routerList[i].routerAddr, msg, (mptr - msg)
							, pdu.pduExpectingReply, pdu.pduNetworkPriority
							);
					
					// send it along
					routerList[i].routerAdapter->Request( npdu );
				} else {
					// build a PDU, using a broadcast address and send it to everybody
					// in the hope that somebody will pick it up and carry it along
					BACnetNPDU
						npdu( BACnetAddress( localBroadcastAddr ), msg, (mptr - msg)
							, pdu.pduExpectingReply, pdu.pduNetworkPriority
							);
			
					// send it to all adapters
					for (i = 0; i < adapterListLen; i++)
						adapterList[i]->Request( npdu );
				}
			}
			break;
			
		case remoteBroadcastAddr:
			// check for an adapter for the DNET
			for (i = 0; i < adapterListLen; i++)
				if (adapterList[i]->adapterNet == pdu.pduAddr.addrNet)
					break;
			if (i < adapterListLen) {
				// this is really a local broadcast
				*mptr++ = 0x00 + (pdu.pduExpectingReply << 2) + (pdu.pduNetworkPriority);
				memcpy( mptr, pdu.pduData, pdu.pduLen );
				mptr += pdu.pduLen;
				
				// build a PDU, making a new localBroadcastAddr
				BACnetNPDU
					npdu( BACnetAddress( localBroadcastAddr ), msg, (mptr - msg)
						, pdu.pduExpectingReply, pdu.pduNetworkPriority
						);
				
				// send it along
				adapterList[0]->Request( npdu );
			} else {
				// build a message to be broadcast by the real router
				*mptr++ = 0x20 + (pdu.pduExpectingReply << 2) + (pdu.pduNetworkPriority);
				*mptr++ = (pdu.pduAddr.addrNet >> 8);		// DNET
				*mptr++ = (pdu.pduAddr.addrNet & 0x00FF);		
				*mptr++ = 0;									// DLEN
				*mptr++ = 0xFF;		// hop count
				memcpy( mptr, pdu.pduData, pdu.pduLen );
				mptr += pdu.pduLen;
				
				// check for a router for the DNET
				for (i = 0; i < routerListLen; i++)
					if (routerList[i].routerNet == pdu.pduAddr.addrNet)
						break;
				if (i < routerListLen) {
					// build a PDU, using the correct localStationAddr
					BACnetNPDU
						npdu( routerList[i].routerAddr, msg, (mptr - msg)
							, pdu.pduExpectingReply, pdu.pduNetworkPriority
							);
					
					// send it along
					routerList[i].routerAdapter->Request( npdu );
				} else {
					// build a PDU, using a broadcast address and send it to everybody
					// in the hope that somebody will pick it up and carry it along
					BACnetNPDU
						npdu( BACnetAddress( localBroadcastAddr ), msg, (mptr - msg)
							, pdu.pduExpectingReply, pdu.pduNetworkPriority
							);
			
					// send it to all adapters
					for (i = 0; i < adapterListLen; i++)
						adapterList[i]->Request( npdu );
				}
			}
			break;
			
		case globalBroadcastAddr:
			// build a message to be broadcast on all endpoints
			*mptr++ = 0x20 + (pdu.pduExpectingReply << 2) + (pdu.pduNetworkPriority);
			*mptr++ = 0xFF;		// DNET
			*mptr++ = 0xFF;
			*mptr++ = 0;			// DLEN
			*mptr++ = 0xFF;		// hop count
			memcpy( mptr, pdu.pduData, pdu.pduLen );
			mptr += pdu.pduLen;
			
			// build a PDU, using a local broadcast address
			BACnetNPDU
				npdu( BACnetAddress( localBroadcastAddr )
					, msg, (mptr - msg)
					, pdu.pduExpectingReply, pdu.pduNetworkPriority
					);
			
			// send it to all adapters
			for (i = 0; i < adapterListLen; i++)
				adapterList[i]->Request( npdu );
			break;
	}
	
	// toss the buffer
	delete[] msg;
}
