
#include "stdafx.h"

#include "JError.hpp"
#include "JConfig.hpp"
#include "JMemory.hpp"
#include "JDBRelation.hpp"

#if _JDBRelationDebug
#include <iostream>
#endif

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JDBRelationMgr::JDBRelationMgr
//

JDBRelationMgr::JDBRelationMgr( JDBKeyMgrPtr kmp )
	: mgrKeyMgr(kmp), mgrObjMgr( kmp->mgrObjMgr )
{
}

//
//	JDBRelationMgr::~JDBRelationMgr
//

JDBRelationMgr::~JDBRelationMgr( void )
{
}

//
//	JDBRelationMgr::NewRelation
//

int JDBRelationMgr::NewRelation( JDBRelation &rel, JDBRelationType rType )
{
	objId	id
	;
	
	rel.relationMgr = this;
	
	// allocate the descriptor
	ReturnIfError_( mgrObjMgr->NewObject( &id, &rel.relationDesc, kJDBRelationDescSize ) );
	
	// initialize it
	rel.objID = id;
	rel.relationDesc.objType = kJDBRelationDescType;
	rel.relationDesc.relationType = rel.relationType = rType;
	
	// allocate the key
	ReturnIfError_( mgrKeyMgr->NewKey( rel.relationKey ) );
	
	// store the key ID in the descriptor
	rel.relationDesc.relationKeyID = rel.relationKey.objID;
	
	// save the descriptor
	ReturnIfError_( mgrObjMgr->WriteObject( rel.objID, &rel.relationDesc ) );
	
	// success
	return 0;
}

//
//	JDBRelationMgr::GetRelation
//

int JDBRelationMgr::GetRelation( JDBRelation &rel, objId id )
{
	rel.relationMgr = this;
	rel.objID = id;
	
	// get the descriptor
	ReturnIfError_( mgrObjMgr->ReadObject( id, &rel.relationDesc ) );
	
	// lift the type to make it accessable
	rel.relationType = (JDBRelationType)rel.relationDesc.relationType;
	
	// get the key
	ReturnIfError_( mgrKeyMgr->GetKey( rel.relationKey, rel.relationDesc.relationKeyID ) );
	
	// success
	return 0;
}

//
//	JDBRelationMgr::DeleteRelation
//

int JDBRelationMgr::DeleteRelation( objId id )
{
	JDBRelationDesc		desc
	;
	
	// get the descriptor
	ReturnIfError_( mgrObjMgr->ReadObject( id, &desc ) );
	
	// delete the key
	ReturnIfError_( mgrKeyMgr->DeleteKey( desc.relationKeyID ) );
	
	// delete the array descriptor
	ReturnIfError_( mgrObjMgr->DeleteObject( id ) );
	
	// success
	return 0;
}

//
//	JDBRelation::JDBRelation
//

JDBRelation::JDBRelation( void )
	: relationMgr(0), relationKey(LongKeyComp)
{
}

//
//	JDBRelation::~JDBRelation
//

JDBRelation::~JDBRelation( void )
{
}

//
//	JDBRelation::AddRelation
//

void JDBRelation::AddRelation( objId a, objId b )
{
	long	oldID
	;
	
	if (relationType == oneToOne) {
		if (relationKey.ReadKey( a, &oldID ) == -1) {
			ThrowIfError_( relationKey.NewKey( a, b ) );
		} else {
			ThrowIfError_( relationKey.DeleteKey( oldID, a ) );
			ThrowIfError_( relationKey.WriteKey( a, b ) );
		}
	} else
		ThrowIfError_( relationKey.NewKey( a, b ) );
				
	if ((relationType == oneToOne) || (relationType == oneToMany)) {
		if (relationKey.ReadKey( b, &oldID ) == -1) {
			ThrowIfError_( relationKey.NewKey( b, a ) );
		} else {
			ThrowIfError_( relationKey.DeleteKey( oldID, b ) );
			ThrowIfError_( relationKey.WriteKey( b, a ) );
		}
	} else
		ThrowIfError_( relationKey.NewKey( b, a ) );
}

//
//	JDBRelation::DeleteRelation
//

void JDBRelation::DeleteRelation( objId a, objId b )
{
	ThrowIfError_( relationKey.DeleteKey( a, b ) );
	ThrowIfError_( relationKey.DeleteKey( b, a ) );
}

//
//	JDBRelation::GetRelationCount
//

int JDBRelation::GetRelationCount( objId id )
{
	// set up for the scan
	relationCountMode = true;
	relationCount = 0;
	
	// scan for matches
	ThrowIfError_( relationKey.SearchKey( kEqual, id, kNoMatch, 0, JDBRelationSearch ) );
	
	// success
	return relationCount;
}

//
//	JDBRelation::GetRelation
//

int JDBRelation::GetRelation( objId a, objId *bp, int indx )
{
	int		stat
	;
	
	// set up for the scan
	relationCountMode = false;
	relationCount = indx;
	
	// scan for matches
	stat = relationKey.SearchKey( kEqual, a, kNoMatch, 0, JDBRelationSearch );
	
	// check for a match
	if (stat == 1) {
		*bp = relationRslt;
		return 0;
	} else
	if (stat == 0)
		return -1 /* no relation */;
	else
		return stat;
}

//
//	JDBRelation::JDBRelationSearch
//

bool	JDBRelation::relationCountMode;
int		JDBRelation::relationCount;
objId	JDBRelation::relationRslt;

int JDBRelation::JDBRelationSearch( long, long data )
{
	if (relationCountMode)
		relationCount += 1;
	else
	if (relationCount-- == 0) {
		relationRslt = *(objId *)&data;
		return 1;
	}
	
	return 0;
}

//
//	JDBRelation::GetRelationList
//

int		**JDBRelation::relationList;
int		JDBRelation::relationListCount;

int *JDBRelation::GetRelationList( objId a )
{
	int		stat, *rslt
	;
	
	// set up for the scan
	relationList = (int **)JNewHandle( 0 );
	relationListCount = 0;
	
	// scan for matches
	stat = relationKey.SearchKey( kEqual, a, kNoMatch, 0, JDBRelationListSearch );
	
	rslt = new int[relationListCount + 1];
	rslt[0] = relationListCount;
	JBlockMove(	(JPtr)*relationList, (JPtr)(rslt + 1), (relationListCount * sizeof(long)) );
	JDisposeHandle( (JHandle)relationList );
	
	return rslt;
}

//
//	JDBRelation::JDBRelationListSearch
//

int JDBRelation::JDBRelationListSearch( long, long data )
{
	relationListCount += 1;
	JSetHandleSize( (JHandle)relationList, (relationListCount * sizeof(long)) );
	(*relationList)[relationListCount - 1] = data;
	
	return 0;
}

//
//	JDBRelation::Debug
//

#if _JDBRelationDebug

int JDBRelationDebugDump( long a, long b );

void JDBRelation::Debug( void )
{
	cout << "----- Relation Debug -----" << endl;
	ThrowIfError_( relationKey.SearchKey( kNotEqual, 0, kNoMatch, 0, JDBRelationDebugDump ) );
	cout << endl;
}

int JDBRelationDebugDump( long a, long b )
{
	static char buff[32]
	;
	
	sprintf( buff , "\t[ %08X:%08X ]", a, b );
	cout << buff << endl;
	
	return 0;
}

#endif
