
#include "stdafx.h"

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
//	BACnetAPDUDecoder
//

BACnetAPDUDecoder::BACnetAPDUDecoder( const BACnetOctet *buffer, int len )
	: pktBuffer(buffer), pktLength(len)
{
}

BACnetAPDUDecoder::BACnetAPDUDecoder( const BACnetAPDUEncoder &enc )
	: pktBuffer(enc.pktBuffer), pktLength(enc.pktLength)
{
}

//
//	BACnetAPDUDecoder::SetBuffer
//

void BACnetAPDUDecoder::SetBuffer( const BACnetOctet *buffer, int len )
{
	pktBuffer = buffer;
	pktLength = len;
}

//
//	ExamineTag
//

void BACnetAPDUDecoder::ExamineTag(BACnetAPDUTag &t)
{
	const BACnetOctet	*pkt = pktBuffer		// save the packet pointer
	;
	int					len = pktLength			// save the packet length
	;
	
	// call the (destructive) decoder
	t.Decode( *this );
	
	// restore the pointer and length
	pktBuffer = pkt;
	pktLength = len;
}

//
//	BACnetAPDUDecoder::CopyOctets
//
//	A crude but effective way of copying out a chunk of information.  Note that 
//	this does not account for tagged data, nor does it check to make sure there 
//	is actually enough data in the buffer to copy out.  This routine is very 
//	seldom called, and when it is it's usually for a very short buffer (like 
//	one octet) so it is not worth the overhead of calling memcpy().
//

void BACnetAPDUDecoder::CopyOctets( BACnetOctet *buff, int len )
{
	while (len--)
		*buff++ = (pktLength--,*pktBuffer++);
}

//
//	BACnetAPDUDecoder::ExtractData
//
//	This routine is used to copy the data portion of tagged information into 
//	a buffer supplied by the caller.  The return value is the number of octets
//	copied.
//
//	This is a destructive call (the packet pointer and length is updated to the 
//	next tag).
//

int BACnetAPDUDecoder::ExtractData( BACnetOctet *buffer )
{
	BACnetAPDUTag	tag
	;
	
	// extract the tag
	tag.Decode( *this );
	
	// don't copy data for an application tagged boolean (there isn't any)
	if (!tag.tagClass && (tag.tagNumber == booleanAppTag))
		return 0;
	
	// copy out the data
	CopyOctets( buffer, tag.tagLVT );
	
	// return the length
	return tag.tagLVT;
}
