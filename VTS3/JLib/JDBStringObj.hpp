
#ifndef _JDBStringObj
#define _JDBStringObj

#ifndef _JDBObject
#include "JDBObj.hpp"
#endif

#define	_JDBStringObjDebug	0

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif

const int kJDBMinStrLength = 4;
const int kJDBStringObjType = 5;

struct JDBStringObj : JDBObj {
	char			strString[kJDBMinStrLength];	// string data
	};

typedef JDBStringObj *JDBStringObjPtr;
const int kJDBStringObjSize = sizeof( JDBStringObj );

#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

class JDBString {
	protected:
		JDBObjMgrPtr			strMgr;				// object manager
		JDBStringObjPtr			strPtr;				// string object
		
	public:
		objId					objID;
		
		JDBString( JDBObjMgrPtr mp, objId id );		// get an existing object
		JDBString( JDBObjMgrPtr mp, char *src );	// create a new one
		~JDBString( void );
		
		void GetData( char *dst );					// copy value to buffer
		void SetData( char *src );					// change value
		
		operator char *(void);						// protected pointer to value
	};

typedef JDBString *JDBStringPtr;
const int kJDBStringSize = sizeof( JDBString );

#endif
