
#ifndef _JDBObject
#define _JDBObject

#ifndef _JDB
#include "JDB.hpp"
#endif

#define _JDBObjDebug		0

typedef unsigned long objId;					// all object ID's are long ints
const int kObjIdSize = sizeof( objId );			// hopefully four bytes

const int kMinObjSize = 8;						// minimum object size
const int kMaxObjSize = 65534;					// maximum object size

#define makeId(b,o)			(((objId)(b) << 8) | (objId)(o))
#define blockOfId(id)		(((objId)(id) >> 8) & 0x00FFFFFF)
#define objOfId(id)			((objId)(id) & 0x000000FF)

const int kJDBObjBlockPad = 8;					// data padding
const int kJDBDefaultObjBlockAllocSize = 2048;	// default object block allocation size

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif

struct JDBObjBlock {
	long			blockFree;					// free space in this block
	unsigned char	blockID;					// next available object ID
	char			blockFilled;				// no available object ID's
	char			blockData[kJDBObjBlockPad];	// actual size from block file
	};

typedef JDBObjBlock *JDBObjBlockPtr;
const int kJDBObjBlockSize = sizeof( JDBObjBlock ) - kJDBObjBlockPad;

struct JDBObj {
	unsigned short	objSize;		// total object size
	unsigned char	objType;		// some type code
	unsigned char	objID;			// ID of object
	};

typedef JDBObj *JDBObjPtr;
const int kJDBObjSize = sizeof( JDBObj );

const unsigned char JDBRefType = 1;

struct JDBRef : JDBObj {
	objId			refObjID;		// ID of actual object
	};

typedef JDBRef *JDBRefPtr;
const int kJDBRefSize = sizeof( JDBRef );

struct JDBObjMgrDesc : public JDBDescObj {
	int				descAllocSize;		// allocation size for blocks
	blockNumber		descUsedBlock;		// block number of free space list
	};

typedef JDBObjMgrDesc *JDBObjMgrDescPtr;
const int kJDBObjMgrDescSize = sizeof( JDBObjMgrDesc );

struct JDBObjUsed {
	int				usedBlock;		// block with free bytes
	int				usedFree;		// how many are free
	};

typedef JDBObjUsed *JDBObjUsedPtr;
const int kJDBObjUsedSize = sizeof( JDBObjUsed );

#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

class JDBObjMgr {
	protected:
		JDBObjMgrDescPtr	mgrDesc;						// descriptor pointer
		
		JDBObjPtr LocateObject(objId id, JDBObjBlockPtr obp, int blkSize);	// unprotected locator
	
		int GetBlockFree( blockNumber &blck, int size );		// find free space
		int SetBlockFree( blockNumber blck, int size );			// update free space
		
	public:
		JDBPtr			mgrFile;						// associated file
		
		JDBObjMgr(JDBPtr jbfp);							// associate with a file
		~JDBObjMgr(void);								//
		
		int NewObject(objId *id, JDBObjPtr op, int size);	// creates a new object
		int DeleteObject(objId id);							// frees space used in database
		
		int ReadObject(objId id, JDBObjPtr op);			// reads object data from block
		int WriteObject(objId id, JDBObjPtr op);		// writes object to block
		
		JDBObjPtr NewObject( objId *id, int size );		// create a new dynamic obj
		JDBObjPtr GetObject(objId id, int pad = 0);			// ptr to dynamic obj
		void FreeObject( JDBObjPtr op );				// release a dynamic obj
		void ResizeObject( JDBObjPtr *op, int size );	// change size of dynamic obj
		
#if _JDBObjDebug
		void Debug(void);
#endif
	};

typedef JDBObjMgr *JDBObjMgrPtr;
const int kJDBObjMgrSize = sizeof( JDBObjMgr );

#endif
