
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
//	BACnetAPDU::BACnetAPDU
//

BACnetAPDU::BACnetAPDU( int initBuffSize )
	: BACnetAPDUEncoder( initBuffSize )
	, apduType(abortPDU), apduService(0), apduInvokeID(0), apduAbortRejectReason(0)
	, apduExpectingReply(0), apduNetworkPriority(0)
{
}

//
//	BACnetAPDU::BACnetAPDU
//

BACnetAPDU::BACnetAPDU( BACnetOctet *buffPtr, int buffLen )
	: BACnetAPDUEncoder( buffPtr, buffLen )
	, apduType(abortPDU), apduService(0), apduInvokeID(0), apduAbortRejectReason(0)
	, apduExpectingReply(0), apduNetworkPriority(0)
{
}

//
//	BACnetConfirmedServiceAPDU::BACnetConfirmedServiceAPDU
//
//	Most likely a confirmed service will be built without easy access to the device information record 
//	where information about segmentation support is stored, so build the simplest kind of message.  It 
//	may get modified (the SA bit in the first octet).  If it needs to be segmented, a new header will be 
//	built (see BACnetAPDUSegment::operator[]).
//
//	In addition to SA, the device will also have to modify the max-APDU-length-accepted octet based on 
//	its configuration.
//

BACnetConfirmedServiceAPDU::BACnetConfirmedServiceAPDU( BACnetConfirmedServiceChoice ch )
{
	apduType = confirmedRequestPDU;
	apduService = (int)ch;
	apduExpectingReply = 1;
	
	Append( ((int)apduType) << 4 );			// no segmentation by default
	Append( 0 );							// maximum length
	Append( 0 );							// invoke ID
	Append( apduService );
}

//
//	BACnetUnconfirmedServiceAPDU::BACnetUnconfirmedServiceAPDU
//

BACnetUnconfirmedServiceAPDU::BACnetUnconfirmedServiceAPDU( BACnetUnconfirmedServiceChoice ch )
{
	apduType = unconfirmedRequestPDU;
	apduService = (int)ch;
	
	Append( ((int)apduType) << 4 );
	Append( apduService );
}

//
//	BACnetSimpleAckAPDU::BACnetSimpleAckAPDU
//

BACnetSimpleAckAPDU::BACnetSimpleAckAPDU( BACnetConfirmedServiceChoice ch, BACnetOctet invID )
	: BACnetAPDU( simpleAckBuff, 3 )
{
	apduType = simpleAckPDU;
	apduService = (int)ch;
	apduInvokeID = invID;
	
	simpleAckBuff[0] = ((int)apduType) << 4;
	simpleAckBuff[1] = invID;
	simpleAckBuff[2] = (BACnetOctet)ch;
}

//
//	BACnetComplexAckAPDU::BACnetComplexAckAPDU
//
//	A complex ACK suffers from the same problems that a confirmed request does: at the time the message
//	is being built it's not known if segmentation is supported or if it needs to be segmented.  This 
//	may be rebuilt by a state machine.
//

BACnetComplexAckAPDU::BACnetComplexAckAPDU( BACnetConfirmedServiceChoice ch, BACnetOctet invID )
{
	apduType = complexAckPDU;
	apduService = (int)ch;
	apduInvokeID = invID;
	
	Append( ((int)apduType) << 4 );			// no segmentation by default
	Append( invID );
	Append( apduService );
}

//
//	BACnetComplexAckAPDU::SetInvokeID
//

void BACnetComplexAckAPDU::SetInvokeID( BACnetOctet invID )
{
	apduInvokeID = invID;
	pktBuffer[1] = invID;
}

//
//	BACnetSegmentAckAPDU::BACnetSegmentAckAPDU
//

BACnetSegmentAckAPDU::BACnetSegmentAckAPDU( const BACnetAddress &dest, BACnetOctet invID, int nak, int srv, BACnetOctet seq, BACnetOctet win )
	: BACnetAPDU( segmentAckBuff, 4 )
{
	apduType = segmentAckPDU;
	apduAddr = dest;
	apduInvokeID = invID;
	
	segmentAckBuff[0] = (((int)apduType) << 4) + (nak << 1) + srv;
	segmentAckBuff[1] = invID;
	segmentAckBuff[2] = seq;
	segmentAckBuff[3] = win;
}

//
//	BACnetErrorAPDU::BACnetErrorAPDU
//

BACnetErrorAPDU::BACnetErrorAPDU( BACnetConfirmedServiceChoice ch, BACnetOctet invID )
{
	apduType = errorPDU;
	apduService = (int)ch;
	apduInvokeID = invID;
	
	Append( ((int)apduType) << 4 );
	Append( invID );
	Append( apduService );
}

//
//	BACnetRejectAPDU::BACnetRejectAPDU
//

BACnetRejectAPDU::BACnetRejectAPDU( BACnetOctet invID, BACnetRejectReason reason )
	: BACnetAPDU( rejectBuff, 3 )
{
	apduType = rejectPDU;
	apduInvokeID = invID;
	apduAbortRejectReason = (int)reason;
	
	rejectBuff[0] = ((int)apduType) << 4;
	rejectBuff[1] = invID;
	rejectBuff[2] = apduAbortRejectReason;
}

//
//	BACnetAbortAPDU::BACnetAbortAPDU
//

BACnetAbortAPDU::BACnetAbortAPDU( const BACnetAddress &dest, int srv, BACnetOctet invID, BACnetAbortReason reason )
	: BACnetAPDU( abortBuff, 3 )
{
	apduType = abortPDU;
	apduAddr = dest;
	apduInvokeID = invID;
	apduAbortRejectReason = (int)reason;
	
	abortBuff[0] = (((int)apduType) << 4) + srv;
	abortBuff[1] = invID;
	abortBuff[2] = apduAbortRejectReason;
}
