
#include "stdafx.h"

#include "JConfig.hpp"
#include "JDBOctetStringObj.hpp"

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JDBOctetString::JDBOctetString
//

JDBOctetString::JDBOctetString( JDBObjMgrPtr mp, objId id )
	: strMgr( mp )
{
	strPtr = (JDBOctetStringObjPtr)strMgr->GetObject( id );
}

//
//	JDBOctetString::JDBOctetString
//

JDBOctetString::JDBOctetString( JDBObjMgrPtr mp, unsigned char *src, int len )
	: strMgr( mp )
{
	int		objlen, padlen
	;
	
	// figure out how big to make the object
	objlen = kJDBObjSize + len + 1;
	padlen = 4 - (objlen % 4);
	objlen += padlen;
	
	// make it
	strPtr = (JDBOctetStringObjPtr)strMgr->NewObject( &objID, objlen );

	// first char is amount object padded
	strPtr->strData[0] = padlen;

	// copy the rest of the data
	memcpy( strPtr->strData + 1, src, len );
	
	// save it
	strMgr->WriteObject( objID, strPtr );
}

//
//	JDBOctetString::~JDBOctetString
//

JDBOctetString::~JDBOctetString( void )
{
	strMgr->FreeObject( strPtr );
}

//
//	JDBOctetString::GetLength
//

int JDBOctetString::GetLength( void )
{
	return (strPtr->objSize - kJDBObjSize - strPtr->strData[0] - 1);
}

//
//	JDBOctetString::GetData
//

void JDBOctetString::GetData( unsigned char *dst, int buflen )
{
	int copylen = GetLength()
	;

	copylen = (buflen < copylen ? buflen : copylen);
	memcpy( dst, strPtr->strData + 1, copylen );
}

//
//	JDBOctetString::GetDataPtr
//
//	Most disgusting function.  Imagine reaching into the middle of an object and 
//	mucking around with the contents without going through a member function!  In 
//	the future this should be a pointer to a const char so at least it can't be 
//	changed.  But that really doesn't make it any better.
//

unsigned char * JDBOctetString::GetDataPtr( void )
{
	return strPtr->strData + 1;
}

//
//	JDBOctetString::SetData
//

void JDBOctetString::SetData( unsigned char *src, int len )
{
	int		objlen, padlen
	;
	
	// figure out how big to make the object
	objlen = kJDBObjSize + len + 1;
	padlen = 4 - (objlen % 4);
	objlen += padlen;
	
	// change the size
	strMgr->ResizeObject( (JDBObjPtr *)&strPtr, objlen );

	// first char is amount object padded
	strPtr->strData[0] = padlen;

	// copy the rest of the data
	memcpy( strPtr->strData + 1, src, len );
	
	
	// save it
	strMgr->WriteObject( objID, strPtr );
}
