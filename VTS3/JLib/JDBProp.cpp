
#include "stdafx.h"

#include "JConfig.hpp"
#include "JError.hpp"
#include "JMemory.hpp"
#include "JDBProp.hpp"

#if _JDBPropObjDebug
#include <iostream.h>
#endif

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JDBProp::JDBProp
//

JDBProp::JDBProp( JDBObjMgrPtr mp, bool defer )
	: propMgr( mp )
	, propDeferredWrite(defer), propModified(false)
{
	// make it
	ThrowIfNil_( propPtr = (JDBPropObjPtr)propMgr->NewObject( &objID, kJDBPropHdrSize ) );
	propPtr->propPad = 0;
	
	// save it
	if (!propDeferredWrite)
		propMgr->WriteObject( objID, propPtr );
}

//
//	JDBProp::JDBProp
//

JDBProp::JDBProp( JDBObjMgrPtr mp, objId id, bool defer )
	: propMgr( mp ), objID( id )
	, propDeferredWrite(defer), propModified(false)
{
	// get it
	propPtr = (JDBPropObjPtr)propMgr->GetObject( id );
}

//
//	JDBProp::~JDBProp
//

JDBProp::~JDBProp( void )
{
	if (propModified)
		propMgr->WriteObject( objID, propPtr );
	
	if (propPtr)
		propMgr->FreeObject( propPtr );
}

//
//	JDBProp::ReadProp
//

void JDBProp::ReadProp( int pid, void *buff, int *len )
{
	int		srcLen
	;
	char	*src
	;
	
	// error checking
	ThrowIfNil_( propPtr );
	ThrowIf_( (pid > 128) || (pid < -127) );
	ThrowIf_( buff == 0 );
	
	// find the property
	src = propPtr->propData;
	srcLen = propPtr->objSize - kJDBPropHdrSize - propPtr->propPad;
	while (srcLen > 0)
		if (*(src+1) == pid)
			break;
		else
		if (*(src+1) > pid) {
			*len = 0;
			return;
		} else {
			srcLen -= *src;
			src += *src;
		}
	
	// copy the data
	if (srcLen > 0) {
		*len = *src - 2;
		JBlockMove( (JPtr)(src + 2), (JPtr)buff, *len );
	} else
		*len = 0;
}

//
//	JDBProp::WriteProp
//

void JDBProp::WriteProp( int pid, const void *buff, int len )
{
	int		srcLen, offset, saveLen, newSize
	;
	char	*src
	;
	
	// error checking
	ThrowIfNil_( propPtr );
	ThrowIf_( (pid > 128) || (pid < -127) );
	ThrowIf_( buff == 0 );
	ThrowIf_( (len < 0) || (len > 126) );
	
	// make sure everything looks OK
	Validate();
	
	// find the property
	src = propPtr->propData;
	srcLen = propPtr->objSize - kJDBPropHdrSize - propPtr->propPad;
	while (srcLen > 0) {
		if (*(src+1) >= pid)
			break;
		else {
			srcLen -= *src;
			src += *src;
		}
	}
	
	if (len == 0) {							// delete operation
		if ((srcLen != 0) && (*(src+1) == pid)) {
			// slide the rest of the data down
			offset = src - propPtr->propData;
			saveLen = srcLen - *src;
			JBlockMove( src + *src, src, saveLen );
			
			// compute the new size, padding as necessary
			newSize = kJDBPropHdrSize + offset + saveLen;
			if ((newSize & 1) != 0) {
				newSize += 1;
				propPtr->propPad = 1;
			} else
				propPtr->propPad = 0;
			
			// resize the object
			propMgr->ResizeObject( (JDBObjPtr *)&propPtr, newSize );
		}
	} else
	if (srcLen == 0) {						// append operation
		// save the offset
		offset = src - propPtr->propData;
		
		// compute the new size, padding as necessary
		newSize = kJDBPropHdrSize + offset + len + 2;
		if ((newSize & 1) != 0) {
			newSize += 1;
			propPtr->propPad = 1;
		} else
			propPtr->propPad = 0;
		
		// resize the object
		propMgr->ResizeObject( (JDBObjPtr *)&propPtr, newSize );
		
		// restore the src
		src = propPtr->propData + offset;
		
		// save the data
		*src++ = len + 2;
		*src++ = pid;
		JBlockMove( (JPtr)buff, (JPtr)src, len );
	} else
	if (*(src+1) != pid) {					// insert operation
		// save the offset
		offset = src - propPtr->propData;
		saveLen = srcLen;
		
		// compute the new size, padding as necessary
		newSize = kJDBPropHdrSize + offset + saveLen + len + 2;
		if ((newSize & 1) != 0) {
			newSize += 1;
			propPtr->propPad = 1;
		} else
			propPtr->propPad = 0;
		
		// resize the object
		propMgr->ResizeObject( (JDBObjPtr *)&propPtr, newSize );
		
		// restore the src
		src = propPtr->propData + offset;
		
		// slide the rest of the data out of the way
		JBlockMove( src, src + len + 2, saveLen );
		
		// save the data
		*src++ = len + 2;
		*src++ = pid;
		JBlockMove( (JPtr)buff, (JPtr)src, len );
	} else
	if (len == (*src + 2)) {				// simple replace
		JBlockMove( (JPtr)buff, (JPtr)(src + 2), len );
	} else {								// replace operation
		// save the offset
		offset = src - propPtr->propData;
		saveLen = srcLen - *src;
		
		// compute the new size, padding as necessary
		newSize = kJDBPropHdrSize + offset + saveLen + len + 2;
		if ((newSize & 1) != 0) {
			newSize += 1;
			propPtr->propPad = 1;
		} else
			propPtr->propPad = 0;
		
		// resize the object
		propMgr->ResizeObject( (JDBObjPtr *)&propPtr, newSize );
		
		// restore the src
		src = propPtr->propData + offset;
		
		// slide the rest of the data out of the way
		JBlockMove( src + *src, src + len + 2, saveLen );
		
		// save the data
		*src++ = len + 2;
		*src++ = pid;
		JBlockMove( (JPtr)buff, (JPtr)src, len );
	}
	
	// make sure everything looks OK
	Validate();
	
	// save the object
	if (!propDeferredWrite)
		ThrowIfError_( propMgr->WriteObject( objID, propPtr ) );
	else
		propModified = true;
}

//
//	JDBProp::Validate
//

void JDBProp::Validate( void )
{
	int		srcLen
	;
	char	*src
	;
	
	// error checking
	ThrowIfNil_( propPtr );
	ThrowIf_( (propPtr->propPad < 0) || (propPtr->propPad > 1) );
	
	// run the list
	src = propPtr->propData;
	srcLen = propPtr->objSize - kJDBPropHdrSize - propPtr->propPad;
	while (srcLen > 0) {
		if ( (*src <= 0) || (*src > srcLen) )
			break;
		srcLen -= *src;
		src += *src;
	}
}

//
//	JDBProp::Debug
//

#if _JDBPropObjDebug
void JDBProp::Debug( void )
{
	int		srcLen, pLen
	;
	char	*src
	;
	
	cout << "JDBProp" << endl;
	cout << "    objSize = " << propPtr->objSize << endl;
	cout << "    objType = " << propPtr->objType << endl;
	cout << "    objID = " << propPtr->objID << endl;
	cout << "    propPad = " << propPtr->propPad << endl;
	
	src = propPtr->propData;
	srcLen = propPtr->objSize - kJDBPropHdrSize - propPtr->propPad;
	while (srcLen > 0) {
		cout << "    [" << (int)*src << "] " << (int)*(src+1) << ':';
		srcLen -= *src;
		
		pLen = *src - 2;
		src += 2;
		
		while (pLen--)
			if ((*src >= ' ') && (*src <= '~'))
				cout << *src++;
			else {
				cout << '.';
				src += 1;
			}
		
		cout << endl;
	}
	cout << endl;
}
#endif
