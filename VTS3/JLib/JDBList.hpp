
#ifndef _JDBList
#define _JDBList

#include "JDB.hpp"
#include "JDBObj.hpp"

#define _JDBListDebug		0

const unsigned char JDBListObjTypeType = 3;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif

struct JDBListObj : JDBObj {
	int			listElemSize;						// element size
	};

typedef JDBListObj *JDBListObjPtr;
const int kJDBListObjSize = sizeof( JDBListObj );

#if defined(powerc) || defined (__powerc)
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

class JDBListMgr {
		friend class JDBList;
		
	protected:
		JDBPtr				mgrFile;
		JDBObjMgrPtr		mgrObjMgr;
		
	public:
		JDBListMgr( JDBObjMgrPtr );
		~JDBListMgr( void );
		
		int NewList( JDBList &lst, int eSize );
		int GetList( JDBList &lst, objId id );
		
		int DeleteList( objId id );
	};

typedef JDBListMgr *JDBListMgrPtr;
const int kJDBListMgrSize = sizeof( JDBListMgr );

typedef int (*JDBListCompFnPtr)( const void *, const void * );

const int kJDBListEnd = 0x7FFFFFFF;					// end-of-list index

class JDBList {
		friend class JDBListMgr;
		
	protected:
		JDBObjMgrPtr		listObjMgr;				// object manager
		JDBListObjPtr		listPtr;				// pointer to object
		
	public:
		objId				objID;					// object id
		
		JDBList( void );							// unbound object, useless
		~JDBList( void );
		
		int Length( void );							// number of elements
		
		int NewElem( int *indx, void *data );		// creates a new object
		int DeleteElem( int indx );					// frees space used in database
		void DeleteAll( void );						// delete all elements
		
		int ReadElem( int indx, void *data );		// reads object data from block
		int WriteElem( int indx, void *data );		// writes object to block

		int FindElem( const void *data, JDBListCompFnPtr fnp = 0 );		// look for an element, -1 not found
		
		int Sort( JDBListCompFnPtr fnp );			// put elements in order
	};

typedef JDBList *JDBListPtr;
const int kJDBListSize = sizeof( JDBList );

#endif
