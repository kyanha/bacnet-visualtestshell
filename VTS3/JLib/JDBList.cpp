
#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <cstdlib>

#include "JConfig.hpp"
#include "JError.hpp"
#include "JMemory.hpp"
#include "JDBList.hpp"

#if _JDBListDebug
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
//	JDBListMgr::JDBListMgr
//

JDBListMgr::JDBListMgr( JDBObjMgrPtr omp )
	: mgrFile( omp->mgrFile ), mgrObjMgr( omp )
{
}

//
//	JDBListMgr::~JDBListMgr
//

JDBListMgr::~JDBListMgr( void )
{
}

//
//	JDBListMgr::NewList
//

int JDBListMgr::NewList( JDBList &lst, int eSize )
{
	// forward along a pointer to the object manager
	lst.listObjMgr = mgrObjMgr;
	
	// make it
	lst.listPtr = (JDBListObjPtr)mgrObjMgr->NewObject( &lst.objID, kJDBListObjSize );
	lst.listPtr->listElemSize = eSize;
	
	// save it
	mgrObjMgr->WriteObject( lst.objID, lst.listPtr );
	
	return 0;
}

//
//	JDBListMgr::GetList
//

int JDBListMgr::GetList( JDBList &lst, objId id )
{
	lst.listObjMgr = mgrObjMgr;
	lst.listPtr = (JDBListObjPtr)mgrObjMgr->GetObject( id );
	lst.objID = id;
	
	return 0;
}

//
//	JDBListMgr::DeleteList
//

int JDBListMgr::DeleteList( objId id )
{
	int		stat
	;
	
	// delete the object
	if ((stat = mgrObjMgr->DeleteObject( id )) != 0)
		return stat;
	
	return 0;
}

//
//	JDBList::JDBList
//

JDBList::JDBList( void )
	: listObjMgr(0), listPtr(0), objID(0)
{
}

//
//	JDBList::~JDBList
//

JDBList::~JDBList( void )
{
	if (listPtr)
		listObjMgr->FreeObject( listPtr );
}

//
//	JDBList::Length
//

int JDBList::Length( void )
{
	return ((listPtr->objSize - kJDBListObjSize) / listPtr->listElemSize);
}

//
//	JDBList::NewElem
//

int JDBList::NewElem( int *indx, void *data )
{
	int		len
	;
	char	*dst
	;
	
	// error checking
	ThrowIf_( *indx < 0 );
	
	// calculate the real position
	len = Length();
	if (*indx > len) *indx = len;
	
	// make room
	listObjMgr->ResizeObject( (JDBObjPtr *)&listPtr, listPtr->objSize + listPtr->listElemSize );
	
	// figure out where this data is going to go
	dst = (char *)listPtr + kJDBListObjSize + (*indx * listPtr->listElemSize);
	
	// copy the data out of the way
	if (len != *indx)
		JBlockMove( dst, dst + listPtr->listElemSize
			, (len - *indx) * listPtr->listElemSize
			);
	
	// copy the new data
	JBlockMove( (JPtr)data, (JPtr)dst, listPtr->listElemSize );
	
	// save the object
	ThrowIfError_( listObjMgr->WriteObject( objID, listPtr ) );
	
	// success
	return 0;
}

//
//	JDBList::DeleteElem
//

int JDBList::DeleteElem( int indx )
{
	int		len = Length()
	;
	char	*src
	;
	
	// error checking
	if ((indx < 0) || (indx >= len))
		return -1 /* no such element */;
	
	// figure out what data is going away
	src = (char *)listPtr + kJDBListObjSize + (indx * listPtr->listElemSize);
	
	// copy the data out of the way
	if (len != indx)
		JBlockMove( src + listPtr->listElemSize
			, src, (len - indx) * listPtr->listElemSize
			);
	
	// shrink the object
	listObjMgr->ResizeObject( (JDBObjPtr *)&listPtr, listPtr->objSize - listPtr->listElemSize );
	
	// save the object
	ThrowIfError_( listObjMgr->WriteObject( objID, listPtr ) );
	
	// success
	return 0;
}

//
//	JDBList::ReadElem
//

int JDBList::ReadElem( int indx, void *data )
{
	int		len = Length()
	;
	
	// error checking
	if ((indx < 0) || (indx >= len))
		return -1 /* no such element */;
	
	// copy the data
	JBlockMove( (JPtr)((char *)listPtr + kJDBListObjSize + (indx * listPtr->listElemSize))
		, (JPtr)data, listPtr->listElemSize
		);
	
	// success
	return 0;
}

//
//	JDBList::WriteElem
//

int JDBList::WriteElem( int indx, void *data )
{
	int		len = Length()
	;
	
	// error checking
	ThrowIf_( ((indx < 0) || (indx >= len)) );
	
	// copy the new data
	JBlockMove( (JPtr)data
		, (JPtr)((char *)listPtr + kJDBListObjSize + (indx * listPtr->listElemSize))
		, listPtr->listElemSize
		);
	
	// save the object
	ThrowIfError_( listObjMgr->WriteObject( objID, listPtr ) );
	
	// success
	return 0;
}

//
//	JDBList::FindElem
//

int JDBList::FindElem( const void *data, JDBListCompFnPtr fnp )
{
	int		test, len = Length()
	;
	
	// look for the data
	for (int i = 0; i < len; i++) {
		if (fnp) {
			test = (*fnp)( data, (char *)listPtr + kJDBListObjSize + (i * listPtr->listElemSize) );
		} else {
			test =
				memcmp( (char *)listPtr + kJDBListObjSize + (i * listPtr->listElemSize)
					, data, listPtr->listElemSize
					);
		}
	
		// found a match?
		if (test == 0)
			return i;
	}

	// failed to find a match
	return -1;
}

//
//	JDBList::Sort
//

#ifndef qsort
//typedef int (*_compare_function)(const void*, const void*);                 /* mm 961031 */
//void  qsort(void*, size_t, size_t, _compare_function);                      /* mm 961031 */
#endif

int JDBList::Sort( JDBListCompFnPtr fnp )
{
	int		len = Length()
	;
	
	if (len != 0) {
		qsort( (char *)listPtr + kJDBListObjSize, listPtr->listElemSize, len, fnp );
		ThrowIfError_( listObjMgr->WriteObject( objID, listPtr ) );
	}
	
	return 0;
}

//
//	JDBList::DeleteAll
//

void JDBList::DeleteAll( void )
{
	// make it small
	listObjMgr->ResizeObject( (JDBObjPtr *)&listPtr, kJDBListObjSize );
	
	// save the object
	ThrowIfError_( listObjMgr->WriteObject( objID, listPtr ) );
}
