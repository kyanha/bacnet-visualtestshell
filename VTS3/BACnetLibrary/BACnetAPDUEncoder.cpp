
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
//	BACnetAPDUEncoder::BACnetAPDUEncoder
//
//	This version of the constructor is used when the application already 
//	has a buffer (probably allocated on the stack) and knows that it is 
//	big enough.
//
//	WARNING: Because the encoder object does not own the buffer it can't 
//	verify that there really is enough space.  Unless the application is
//	careful, bad things could happen.
//

BACnetAPDUEncoder::BACnetAPDUEncoder( BACnetOctet *buffPtr )
	: pktBuffer(buffPtr), pktBuffSize(0), pktLength(0)
{
}

//
//	BACnetAPDUEncoder::BACnetAPDUEncoder
//
//	This version of the constructor builds an object which maintains its own 
//	buffer for the encoded content.  If it runs out of space during encoding 
//	it will increase the buffer size as necessary.
//

BACnetAPDUEncoder::BACnetAPDUEncoder( int initBuffSize )
	: pktBuffSize(initBuffSize), pktLength(0)
{
	pktBuffer = new BACnetOctet[initBuffSize];
}

//
//	BACnetAPDUEncoder::~BACnetAPDUEncoder
//

BACnetAPDUEncoder::~BACnetAPDUEncoder( void )
{
	// delete the buffer if we own it
	if (pktBuffSize != 0)
		delete[] pktBuffer;
}

//
//	BACnetAPDUEncoder::CheckSpace
//

void BACnetAPDUEncoder::CheckSpace( int len )
{
	// if we don't own this buffer, skip checking
	if (pktBuffSize == 0)
		return;
	
	// probably enough space
	if ((pktLength + len) <= pktBuffSize)
		return;
	
	// figure out how much more space we need
	int newSize = pktBuffSize + kDefaultBufferSize;
	while ((pktLength + len) <= newSize)
		newSize += kDefaultBufferSize;
	
	// make a new bigger buffer
	BACnetOctet		*newBuffer
	;
	
	newBuffer = new BACnetOctet[ newSize ];
	if (!newBuffer)
		throw (-1);
	
	// copy the old data
	memcpy( newBuffer, pktBuffer, pktBuffSize );
	pktBuffSize = newSize;
	
	// delete the old buffer, use the new one
	delete[] pktBuffer;
	pktBuffer = newBuffer;
}

//
//	BACnetAPDUEncoder::CopyOctets
//

void BACnetAPDUEncoder::CopyOctets( BACnetOctet *buff, int len )
{
	// make sure there's enough room
	CheckSpace( len );
	
	// copy the data
	memcpy( pktBuffer + pktLength, buff, len );
	pktLength += len;
}

//
//	BACnetAPDUEncoder::Flush
//
//	This removes all of the contents of the encoding without the overhead of mucking 
//	about with the buffer.  If it used to be a large encoding and whatever new stuff 
//	doesn't fill up as much space, too bad, so sad!
//

void BACnetAPDUEncoder::Flush( void )
{
	pktLength = 0;
}
