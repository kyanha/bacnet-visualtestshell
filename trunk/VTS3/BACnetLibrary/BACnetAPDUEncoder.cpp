
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
//	big enough.  If some of the content has already been encoded, pass 
//	the number of octets as the length.
//
//	WARNING: Because the encoder object does not own the buffer it can't 
//	verify that there really is enough space.  Unless the application is
//	careful, bad things could happen.
//

BACnetAPDUEncoder::BACnetAPDUEncoder( BACnetOctet *buffPtr, int buffLen )
	: pktBuffer(buffPtr), pktBuffSize(0), pktLength(buffLen)
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
	: pktBuffSize(initBuffSize), pktBuffer(0), pktLength(0)
{
	if (initBuffSize != 0)
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
//	BACnetAPDUEncoder::SetBuffer
//
//	Use this routine to set the buffer pointers to an existing buffer.
//	This is used when building an APDU where the contents are already 
//	inside an NPDU, so there's actually no additional encoding involved.
//

void BACnetAPDUEncoder::SetBuffer( const BACnetOctet *buffer, int len )
{
	pktBuffSize = 0;
	pktBuffer = (BACnetOctetPtr)buffer;		// cast away const
	pktLength  = len;
}

//
//	BACnetAPDUEncoder::NewBuffer
//

void BACnetAPDUEncoder::NewBuffer( int len )
{
	// delete the buffer if we own it
	if (pktBuffSize != 0)
		delete[] pktBuffer;
	
	// create a new one
	pktBuffer = new BACnetOctet[len];
	pktBuffSize = len;
	pktLength = len;
}

//
//	BACnetAPDUEncoder::CheckSpace
//

void BACnetAPDUEncoder::CheckSpace( int len )
{
	// if we don't have a buffer, it's a real problem
	if (!pktBuffer)
		throw -1;
	
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
//	BACnetAPDUEncoder::Append
//

void BACnetAPDUEncoder::Append( BACnetOctet ch )
{
	// if we don't have a buffer, it's a real problem
	if (!pktBuffer)
		throw -1;
	
	// short circuit size check
	if ((pktBuffSize != 0) && ((pktLength + 1) > pktBuffSize))
		CheckSpace( 1 );
	
	// copy it in
	*(pktBuffer + pktLength++) = ch;
}

//
//	BACnetAPDUEncoder::Append
//

void BACnetAPDUEncoder::Append( BACnetOctet *buff, int len )
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
