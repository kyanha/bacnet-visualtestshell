
#include "stdafx.h"

#include "JConfig.hpp"
#include "JError.hpp"
#include "JMemory.hpp"

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	Non-Macintosh Memory Management Routines
//
//	These glue routines provide the same function that the Macintosh routines do, except they 
//	are designed to be portable, so they may not be as fast and could lead to fragmented 
//	memory conditions.
//

//
//	JNewHandle
//

JHandle JNewHandle(int len)
{
	JHandleRecPtr	jhrp
	;

	ThrowIfNil_( jhrp = new JHandleRec );
	if (len == 0)
		jhrp->handlePtr = 0;
	else
		ThrowIfNil_( jhrp->handlePtr = new char[len] );
	jhrp->handleSize = len;

	return (JHandle)jhrp;
}

//
//	JDisposeHandle
//

void JDisposeHandle(JHandle jh)
{
	JHandleRecPtr	jhrp = (JHandleRecPtr)jh
	;

	if (jhrp->handlePtr)
		delete[] jhrp->handlePtr;
	delete jhrp;
}

//
//	JLock
//

void JHLock(JHandle)
{
}

//
//	JHUnlock
//

void JHUnlock(JHandle)
{
}

//
//	JSetHandleSize
//

void JSetHandleSize( JHandle jh, int len )
{
	JHandleRecPtr	jhrp = (JHandleRecPtr)jh
	;
	char	*newData
	;

	if (len == 0) {
		if (jhrp->handlePtr)
			delete[] jhrp->handlePtr;
		jhrp->handlePtr = 0;
		jhrp->handleSize = 0;
		return;
	}

	// allocate new buffer space
	ThrowIfNil_( newData = new char[len] );
	
	// copy the old data
	JBlockMove( jhrp->handlePtr, newData, (len < jhrp->handleSize ? len : jhrp->handleSize) );

	// free the old buffer, use the new one
	if (jhrp->handlePtr)
		delete[] jhrp->handlePtr;
	jhrp->handlePtr = newData;

	// set the new size
	jhrp->handleSize = len;
}

//
//	JGetHandleSize
//

int JGetHandleSize(JHandle jh)
{
	return ((JHandleRecPtr)jh)->handleSize;
}

//
//	JMunger
//

void JMunger(JHandle jh, int offset, JPtr p1, int p1len, JPtr p2, int p2len)
{
	JHandleRecPtr	jhrp = (JHandleRecPtr)jh
	;
	char	*newData
	;

	// delete operation?
	if ((p1 == nil) && (p2 == (JPtr)-1) && (p2len == 0)) {
		// delete everything?
		if (p1len == jhrp->handleSize) {
			ThrowIf_( offset != 0 );

			// toss everything
			delete[] jhrp->handlePtr;
			jhrp->handleSize = 0;

			return;
		}

		// just a little error checking
		ThrowIf_( (offset + p1len) > jhrp->handleSize );

		// set up the new buffer
		newData = new char[ jhrp->handleSize - p1len ];
		JBlockMove( jhrp->handlePtr, newData, offset );
		JBlockMove( jhrp->handlePtr + offset + p1len, newData + offset, jhrp->handleSize - offset - p1len );

		// swap the new one with the old one
		delete[] jhrp->handlePtr;
		jhrp->handlePtr = newData;
		jhrp->handleSize = jhrp->handleSize - p1len;

		return;
	}

	// insert/append operation?
	if ((p1 == (JPtr)-1) && (p1len == 0)) {
		if (jhrp->handleSize == 0) {
			ThrowIf_( offset != 0 );
			ThrowIf_( jhrp->handlePtr != nil );

			// create a new buffer
			jhrp->handlePtr = new char[ p2len ];
			JBlockMove( p2, jhrp->handlePtr, p2len );
			jhrp->handleSize = p2len;

			return;
		}

		// just a little error checking
		ThrowIf_( offset > jhrp->handleSize );

		// set up the new buffer
		newData = new char[ jhrp->handleSize + p2len ];
		JBlockMove( jhrp->handlePtr, newData, offset );
		JBlockMove( p2, newData + offset, p2len );
		JBlockMove( jhrp->handlePtr + offset, newData + offset + p2len, jhrp->handleSize - offset );

		// swap the new one with the old one
		delete[] jhrp->handlePtr;
		jhrp->handlePtr = newData;
		jhrp->handleSize = jhrp->handleSize + p2len;

		return;
	}

	// unsupported combination of parameters
	Throw_( -1 );
}

//
//	JBlockMove
//

void JBlockMove( JPtr src, JPtr dst, int len )
{
	memcpy( dst, src, len );
}
