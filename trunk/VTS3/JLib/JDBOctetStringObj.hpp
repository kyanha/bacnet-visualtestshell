
#ifndef _JDBOctetStringObj
#define _JDBOctetStringObj

#ifndef _JDBObject
#include "JDBObj.hpp"
#endif

#define	_JDBOctetStringObjDebug	0

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif

const int kJDBMinStrLength = 4;
const int kJDBStringObjType = 5;

struct JDBOctetStringObj : JDBObj {
	unsigned char		strData[kJDBMinStrLength];	// string data
	};

typedef JDBOctetStringObj *JDBOctetStringObjPtr;
const int kJDBOctetStringObjSize = sizeof( JDBOctetStringObj );

#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

class JDBOctetString {
	protected:
		JDBObjMgrPtr			strMgr;				// object manager
		JDBOctetStringObjPtr	strPtr;				// string object
		
	public:
		objId					objID;
		
		JDBOctetString( JDBObjMgrPtr mp, objId id );		// get an existing object
		JDBOctetString( JDBObjMgrPtr mp, unsigned char *src, int len );	// create a new one
		~JDBOctetString( void );
		
		int GetLength( void );								// how much data
		void GetData( unsigned char *dst, int bufflen );	// copy value to buffer
		unsigned char *GetDataPtr(void);					// pointer to data

		void SetData( unsigned char *src, int len );		// change value
	};

typedef JDBOctetString *JDBOctetStringPtr;
const int kJDBOctetStringSize = sizeof( JDBOctetString );

#endif
