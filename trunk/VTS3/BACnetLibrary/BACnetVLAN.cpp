
#include "stdafx.h"

#define _BACnetVLANDebug		0

#if _BACnetVLANDebug
#include <iostream>
#endif

#include <stdlib.h>
#include <string.h>

#include "BACnet.hpp"
#include "BACnetVLAN.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	BACnetVLANMsg::BACnetVLANMsg
//

BACnetVLANMsg::BACnetVLANMsg( BACnetVLANPtr lan, const BACnetAddress &source, const BACnetNPDU &pdu )
	: BACnetTask( oneShotDeleteTask, 0 )
	, msgLAN(lan), msgSource(source), msgDestination(pdu.pduAddr)
{
	// copy the message data
	msgLen = pdu.pduLen;
	msgData = new BACnetOctet[ pdu.pduLen ];
	memcpy( msgData, pdu.pduData, pdu.pduLen );
	
	// ready to go
	InstallTask();
}

//
//	BACnetVLANMsg::~BACnetVLANMsg
//
//	The constructor made a local copy of the PDU data, so when this object gets 
//	deleted, the copy needs to go away as well.
//

BACnetVLANMsg::~BACnetVLANMsg( void )
{
	// dispose of the copy of the data
	delete[] msgData;
}

//
//	BACnetVLANMsg::ProcessTask
//

void BACnetVLANMsg::ProcessTask( void )
{
	// process it
	msgLAN->ProcessMessage( this );
}

//
//	BACnetVLANNode::Indication
//

void BACnetVLANNode::Indication( const BACnetNPDU &pdu )
{
	// create a new VLAN message, it will get queued behind all of the other BACnetTask objects
	new BACnetVLANMsg( nodeLAN, nodeAddr, pdu );
}

//
//	BACnetVLAN::BACnetVLAN
//

BACnetVLAN::BACnetVLAN( int id )
	: vlanID( id ), nodeListSize(0)
{
}

//
//	BACnetVLAN::NewNode
//

BACnetVLANNodePtr BACnetVLAN::NewNode( void )
{
	BACnetOctet			addrByte = nodeListSize
	;
	BACnetAddress		addr( &addrByte, 1 )
	;
	BACnetVLANNodePtr	rslt
	;
	
	rslt = new BACnetVLANNode( addr, this );
	nodeList[nodeListSize++] = rslt;
	
	return rslt;
}

//
//	BACnetVLAN::ProcessMessage
//

void BACnetVLAN::ProcessMessage( BACnetVLANMsgPtr msg )
{
	int			i
	;
	BACnetNPDU	resp( msg->msgSource, msg->msgData, msg->msgLen )
	;
	
#if _BACnetVLANDebug
	const static char hex[] = "0123456789ABCDEF";
	cout << "VLAN " << vlanID << ": ";
	cout << "src = " << msg->msgSource;
	cout << ", dst = " << msg->msgDestination;
	cout << ", msg = ";
	for (int i = 0; i < msg->msgLen; i++) {
		cout << hex[ (msg->msgData[i] >> 4) & 0x0F ];
		cout << hex[ msg->msgData[i] & 0x0F ];
		cout << '.';
	}
	cout << endl;
#endif
	
	switch (msg->msgDestination.addrType) {
		case localStationAddr:
			for (i = 0; i < nodeListSize; i++)
				if (nodeList[i]->nodeAddr == msg->msgDestination) {
					nodeList[i]->Response( resp );
					break;
				}
			break;
			
		case localBroadcastAddr:
			for (i = 0; i < nodeListSize; i++)
				if (!(msg->msgSource == nodeList[i]->nodeAddr))
					nodeList[i]->Response( resp );
			break;
			
		default:
#if _BACnetVLANDebug
			cout << "VLAN Addressing error" << endl;
#endif
			break;
	}
}
