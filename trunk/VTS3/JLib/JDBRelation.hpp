#ifndef _JDBRelation
#define _JDBRelation

#ifndef _JDB
#include "JDB.hpp"
#endif

#ifndef _JDBObj
#include "JDBObj.hpp"
#endif

#ifndef _JDBKey
#include "JDBKey.hpp"
#endif

#define _JDBRelationDebug		0

//
//	JDBRelationType
//

enum JDBRelationType { oneToOne, oneToMany, manyToMany };

//
//	JDBRelationDesc
//

const int kJDBRelationDescType = 6;

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif

struct JDBRelationDesc : JDBObj {
	short		relationType;
	objId		relationKeyID;
	};

typedef JDBRelationDesc *JDBRelationDescPtr;
const int kJDBRelationDescSize = sizeof( JDBRelationDesc );

#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

//
//	JDBRelationMgr
//

class JDBRelationMgr {
		friend class JDBRelation;
		
	protected:
		JDBObjMgrPtr		mgrObjMgr;
		
	public:
		JDBKeyMgrPtr		mgrKeyMgr;
		
		JDBRelationMgr( JDBKeyMgrPtr kmp );
		~JDBRelationMgr( void );
		
		int NewRelation( JDBRelation &rel, JDBRelationType rType );
		int GetRelation( JDBRelation &rel, objId id );
		
		int DeleteRelation( objId id );
	};

typedef JDBRelationMgr *JDBRelationMgrPtr;
const int kJDBRelationMgrSize = sizeof( JDBRelationMgr );

//
//	JDBRelation
//

class JDBRelation {
		friend class JDBRelationMgr;
		
	private:
		static bool			relationCountMode;
		static int			relationCount;
		static objId		relationRslt;
		static int			JDBRelationSearch( long key, long data );
		
		static int			**relationList;
		static int			relationListCount;
		static int			JDBRelationListSearch( long key, long data );
		
	protected:
		JDBRelationMgrPtr	relationMgr;		// manager
		JDBRelationDesc		relationDesc;		// relation descriptor
		JDBKey				relationKey;		// the actual data
		
	public:
		objId				objID;
		JDBRelationType		relationType;
		
		JDBRelation( void );						// unbound, useless
		~JDBRelation( void );
		
		void AddRelation( objId a, objId b );
		void DeleteRelation( objId a, objId b );
		
		int GetRelationCount( objId id );
		int GetRelation( objId a, objId *b, int indx = 0 );
		int *GetRelationList( objId a );
		
#if _JDBRelationDebug
		void Debug( void );
#endif
	};

typedef JDBRelation *JDBRelationPtr;
const int kJDBRelationSize = sizeof( JDBRelation );

#endif
