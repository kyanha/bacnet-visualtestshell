
#ifndef _JDBPropObj
#define _JDBPropObj

#ifndef _JDBObject
#include "JDBObj.hpp"
#endif

#define _JDBPropObjDebug	0

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif

const int kJDBPropObjType = 6;
const int kJDBPropObjDataSize = 4;

struct JDBPropObj : JDBObj {
	long						propPad;
	char						propData[kJDBPropObjDataSize];
	};

typedef JDBPropObj *JDBPropObjPtr;
const int kJDBPropObjSize = sizeof( JDBPropObj );
const int kJDBPropHdrSize = kJDBPropObjSize - kJDBPropObjDataSize;

#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

class JDBProp {
	protected:
		JDBObjMgrPtr			propMgr;				// object manager
		JDBPropObjPtr			propPtr;				// property object
		bool					propDeferredWrite;		// postpone write until destructor
		bool					propModified;
		
		void Validate( void );							// check everything over
		
	public:
		objId					objID;					// object id of prop object
		
		JDBProp( JDBObjMgrPtr mp, bool defer = false );				// create a new one
		JDBProp( JDBObjMgrPtr mp, objId id, bool defer = false );	// get an existing object
		~JDBProp( void );
		
		void ReadProp( int pid, void *buff, int *len );			// copy value to buffer
		void WriteProp( int pid, const void *buff, int len );	// change value
		
#if _JDBPropObjDebug
		void Debug( void );
#endif
	};

typedef JDBProp *JDBPropPtr;
const int kJDBPropSize = sizeof( JDBProp );

#endif
