
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
//	BACnetAPDUSegment::BACnetAPDUSegment
//
//	This constructor is called when a client or a server TSM has an outbound message
//	that needs segmentation.  It must be a confirmed request or a complex ack, those 
//	are the only ones that can be segmented.
//
//	The lifetime of the buffer is longer than the lifetime of the APDU parameter (which 
//	probably belongs to a client or the workstation object) so a copy is necessary.
//

BACnetAPDUSegment::BACnetAPDUSegment( const BACnetAPDU &apdu, BACnetTSMPtr tp )
	: segTSMPtr(tp)
{
	segAPDU.apduAddr				= apdu.apduAddr;
	segAPDU.apduType				= apdu.apduType;
	segAPDU.apduSeg					= apdu.apduSeg;
	segAPDU.apduMor					= apdu.apduMor;
	segAPDU.apduSA					= apdu.apduSA;
	segAPDU.apduSrv					= apdu.apduSrv;
	segAPDU.apduNak					= apdu.apduNak;
	segAPDU.apduSeq					= apdu.apduSeq;
	segAPDU.apduWin					= apdu.apduWin;
	segAPDU.apduMaxResp				= apdu.apduMaxResp;
	segAPDU.apduService				= apdu.apduService;
	segAPDU.apduInvokeID			= apdu.apduInvokeID;
	segAPDU.apduAbortRejectReason	= apdu.apduAbortRejectReason;
	segAPDU.apduNetworkPriority		= apdu.apduNetworkPriority;
	segAPDU.apduExpectingReply		= apdu.apduExpectingReply;
	
	// make a copy of the variable encoded portion
	if (apdu.pktLength != 0) {
		segBuff = new BACnetOctet[apdu.pktLength];
		segBuffSize = apdu.pktLength;
		segLen = apdu.pktLength;
		
		memcpy( segBuff, apdu.pktBuffer, (size_t)apdu.pktLength );
	} else {
		segBuff = 0;
		segBuffSize = 0;
		segLen = 0;
	}
}

//
//	BACnetAPDUSegment::BACnetAPDUSegment
//

BACnetAPDUSegment::BACnetAPDUSegment( int ssize, BACnetTSMPtr tp )
	: segTSMPtr(tp)
{
	segBuff = new BACnetOctet[ssize];
	segBuffSize = ssize;
	segLen = 0;
}

//
//	BACnetAPDUSegment::~BACnetAPDUSegment
//

BACnetAPDUSegment::~BACnetAPDUSegment( void )
{
	delete[] segBuff;
}

//
//	BACnetAPDUSegment::operator[]
//

const BACnetAPDU& BACnetAPDUSegment::operator[](const int indx)
{
	int				segSize = segTSMPtr->tsmSegmentSize
	;
	
#if _TSMDebug
	cout << "[BACnetAPDUSegment: building segment " << indx << "]" << endl;
#endif
	
	// clear the contents
	segAPDU.pktLength = 0;
	
	switch (segAPDU.apduType) {
		case confirmedRequestPDU:
			segAPDU.apduMaxResp = segTSMPtr->tsmDevice->deviceMaxAPDUSize;
			segAPDU.apduInvokeID = segTSMPtr->tsmInvokeID;
			
			// segmented message?
			if (segTSMPtr->tsmSegmentCount != 1) {
				segAPDU.apduSeg = true;
				segAPDU.apduMor = (indx < (segTSMPtr->tsmSegmentCount - 1)); // more follows
				segAPDU.apduSeq = indx % 256;						// sequence number
				segAPDU.apduWin = segTSMPtr->tsmActualWindowSize;	// window size
			} else {
				segAPDU.apduSeg = false;
				segAPDU.apduMor = false;
			}
			
			// segmented response accepted?
			segAPDU.apduSA = ( (segTSMPtr->tsmSegmentation == segmentedBoth)
					|| (segTSMPtr->tsmSegmentation == segmentedReceive)
					);
			
			// reference the content
			segAPDU.SetBuffer( segBuff + (indx * segSize)
				, (indx < (segLen / segSize)) ? segSize : (segLen % segSize)
				);
			break;
		
		case complexAckPDU:
			segAPDU.apduInvokeID = segTSMPtr->tsmInvokeID;
			
			// segmented message?
			if (segTSMPtr->tsmSegmentCount != 1) {
				segAPDU.apduSeg = true;
				segAPDU.apduMor = (indx < (segTSMPtr->tsmSegmentCount - 1)); // more follows
				segAPDU.apduSeq = indx % 256;						// sequence number
				segAPDU.apduWin = segTSMPtr->tsmActualWindowSize;	// window size
			} else {
				segAPDU.apduSeg = false;
				segAPDU.apduMor = false;
			}
			
			// reference the content
			segAPDU.SetBuffer( segBuff + (indx * segSize)
				, (indx < (segLen / segSize)) ? segSize : (segLen % segSize)
				);
			break;
		
		default:
			throw -1; // other PDU types invalid
	}
	
	return segAPDU;
}

//
//	BACnetAPDUSegment::AddSegment
//

int BACnetAPDUSegment::AddSegment( const BACnetAPDU &apdu )
{
	// check for first packet
	if (segLen == 0) {
		segAPDU.apduAddr				= apdu.apduAddr;
		segAPDU.apduType				= apdu.apduType;
		segAPDU.apduSeg					= apdu.apduSeg;
		segAPDU.apduMor					= apdu.apduMor;
		segAPDU.apduSA					= apdu.apduSA;
		segAPDU.apduSrv					= apdu.apduSrv;
		segAPDU.apduNak					= apdu.apduNak;
		segAPDU.apduSeq					= apdu.apduSeq;
		segAPDU.apduWin					= apdu.apduWin;
		segAPDU.apduMaxResp				= apdu.apduMaxResp;
		segAPDU.apduService				= apdu.apduService;
		segAPDU.apduInvokeID			= apdu.apduInvokeID;
		segAPDU.apduAbortRejectReason	= apdu.apduAbortRejectReason;
		segAPDU.apduNetworkPriority		= apdu.apduNetworkPriority;
		segAPDU.apduExpectingReply		= apdu.apduExpectingReply;

		segAPDU.pktBuffer = segBuff;
		segAPDU.pktLength = 0;
	}
	
	// check for enough space
	if ((segLen + apdu.pktLength) > segBuffSize)
		return -1 /* out of buffer space */;
	
	// copy variable part of APDU into buffer
	memcpy( segBuff + segLen, apdu.pktBuffer, (size_t)apdu.pktLength );
	segLen += apdu.pktLength;
	
	// set the length, might be the last one
	segAPDU.pktLength = segLen;
	
	return 0;
}

//
//	BACnetAPDUSegment::ResultAPDU
//

const BACnetAPDU& BACnetAPDUSegment::ResultAPDU( void )
{
	return segAPDU;
}
