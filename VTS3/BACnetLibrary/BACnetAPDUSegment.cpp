
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
	segBuff = new BACnetOctet[apdu.pktLength];
	segBuffSize = apdu.pktLength;
	segLen = apdu.pktLength;
	
	memcpy( segBuff, apdu.pktBuffer, (size_t)apdu.pktLength );
	
	segAPDU.apduAddr				= apdu.apduAddr;
	segAPDU.apduType				= apdu.apduType;
	segAPDU.apduService				= apdu.apduService;
	segAPDU.apduInvokeID			= apdu.apduInvokeID;
	segAPDU.apduAbortRejectReason	= apdu.apduAbortRejectReason;
	segAPDU.apduNetworkPriority		= apdu.apduNetworkPriority;
	segAPDU.apduExpectingReply		= apdu.apduExpectingReply;
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
	,				varLen
	;
	BACnetOctet		*varPart
	;
	
#if _TSMDebug
	cout << "[BACnetAPDUSegment: building segment " << indx << "]" << endl;
#endif
	
	// clear the contents
	segAPDU.pktLength = 0;
	
	switch (segAPDU.apduType) {
		case confirmedRequestPDU:
			segAPDU.Append( segAPDU.apduType << 4 );
			segAPDU.Append( MaxAPDUEncode( segTSMPtr->tsmDevice->deviceMaxAPDUSize ) );
			segAPDU.Append( segTSMPtr->tsmInvokeID );
			
			// segmented message?
			if (segTSMPtr->tsmSegmentCount != 1) {
				segAPDU.pktBuffer[0] += 0x08;
				if (indx < (segTSMPtr->tsmSegmentCount - 1))
					segAPDU.pktBuffer[0] += 0x04;			// more follows
				
				segAPDU.Append( indx % 256 );				// sequence number
				segAPDU.Append( segTSMPtr->tsmActualWindowSize );
			}
			
			// OK to segment the reply?
			if ((segTSMPtr->tsmSegmentation == segmentedBoth)
					|| (segTSMPtr->tsmSegmentation == segmentedReceive))
				segAPDU.pktBuffer[0] += 0x02;				// segmented response accepted
			
			// copy the service
			segAPDU.Append( segBuff[3] );
			
			// skip over the (static) header
			varPart = segBuff + 4;
			varLen = segLen - 4;
			
			// copy the content
			segAPDU.Append( varPart + (indx * segSize)
				, (indx < (varLen / segSize)) ? segSize : (varLen % segSize)
				);
			break;
		
		case complexAckPDU:
			segAPDU.Append( segAPDU.apduType << 4 );
			segAPDU.Append( segTSMPtr->tsmInvokeID );
			
			// segmented message?
			if (segTSMPtr->tsmSegmentCount != 1) {
				segAPDU.pktBuffer[0] += 0x08;
				if (indx < (segTSMPtr->tsmSegmentCount - 1))
					segAPDU.pktBuffer[0] += 0x04;			// more follows
				
				segAPDU.Append( indx % 256 );				// sequence number
				segAPDU.Append( segTSMPtr->tsmActualWindowSize );
			}
			
			// copy the service
			segAPDU.Append( segBuff[2] );
			
			// skip over the (static) header
			varPart = segBuff + 3;
			varLen = segLen - 3;
			
			// copy the content
			segAPDU.Append( varPart + (indx * segSize)
				, (indx < (varLen / segSize)) ? segSize : (varLen % segSize)
				);
			break;
	}
	
fini:
	return segAPDU;
}

//
//	BACnetAPDUSegment::AddSegment
//

int BACnetAPDUSegment::AddSegment( const BACnetAPDU &apdu )
{
	int				varLen
	;
	BACnetOctet		*varPart
	;
	
	// check for first packet
	if (segLen == 0) {
		segAPDU.apduAddr = apdu.apduAddr;
		
		segAPDU.pktBuffer = segBuff;
		segAPDU.pktLength  = 0;
		
		segAPDU.apduType = apdu.apduType;
		segAPDU.apduService = apdu.apduService;
		segAPDU.apduInvokeID = apdu.apduInvokeID;
		segAPDU.apduAbortRejectReason = apdu.apduAbortRejectReason;
		segAPDU.apduExpectingReply = apdu.apduExpectingReply;
		segAPDU.apduNetworkPriority = apdu.apduNetworkPriority;
		
		// save the header, trimmed down
		if (apdu.apduType == confirmedRequestPDU) {
			segBuff[0] = apdu.pktBuffer[0] & 0xF0;
			segBuff[1] = 0;
			segBuff[2] = apdu.apduInvokeID;
			segBuff[3] = apdu.apduService;
			segLen = 4;
		} else
		if (apdu.apduType == complexAckPDU) {
			segBuff[0] = apdu.pktBuffer[0] & 0xF0;
			segBuff[1] = apdu.apduInvokeID;
			segBuff[2] = apdu.apduService;
			segLen = 3;
		}
	}
	
	// check for enough space
	if ((segLen + apdu.pktLength) > segBuffSize)
		return -1 /* out of buffer space */;
	
	// skip static part
	if (apdu.apduType == confirmedRequestPDU) {
		varPart = apdu.pktBuffer + 6;
		varLen = apdu.pktLength - 6;
	} else
	if (apdu.apduType == complexAckPDU) {
		varPart = apdu.pktBuffer + 5;
		varLen = apdu.pktLength - 5;
	}
	
	// copy variable part of APDU into buffer
	memcpy( segBuff + segLen, varPart, (size_t)varLen );
	segLen += varLen;
	
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
