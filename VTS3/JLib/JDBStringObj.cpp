
#include "stdafx.h"

#include "JConfig.hpp"
#include "JDBStringObj.hpp"

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JDBString::JDBString
//

JDBString::JDBString( JDBObjMgrPtr mp, objId id )
	: strMgr( mp )
{
	strPtr = (JDBStringObjPtr)strMgr->GetObject( id );
}

//
//	JDBString::JDBString
//

JDBString::JDBString( JDBObjMgrPtr mp, char *src )
	: strMgr( mp )
{
	int		len
	;
	
	// figure out how big to make the object
	len = kJDBObjSize + strlen( src ) + 1;
	len += 4 - (len % 4);
	
	// make it
	strPtr = (JDBStringObjPtr)strMgr->NewObject( &objID, len );
	strcpy( strPtr->strString, src );
	
	// save it
	strMgr->WriteObject( objID, strPtr );
}

//
//	JDBString::~JDBString
//

JDBString::~JDBString( void )
{
	strMgr->FreeObject( strPtr );
}

//
//	JDBString::GetData
//

void JDBString::GetData( char *dst )
{
	strcpy( dst, strPtr->strString );
}

//
//	JDBString::SetData
//

void JDBString::SetData( char *src )
{
	int		len
	;
	
	// figure out how big to make the object
	len = kJDBObjSize + strlen( src ) + 1;
	len += 4 - (len % 4);
	
	// change the size
	strMgr->ResizeObject( (JDBObjPtr *)&strPtr, len );
	strcpy( strPtr->strString, src );
	
	// save it
	strMgr->WriteObject( objID, strPtr );
}

//
//	JDBString::char *
//

JDBString::operator char *(void)
{
	return strPtr->strString;
}
