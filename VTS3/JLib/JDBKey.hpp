
#ifndef _JDBKey
#define _JDBKey

#include "JDB.hpp"
#include "JDBObj.hpp"

#define _JDBKeyDebug		0

enum keyMethod
		{	kNoMatch
		,	kLessThan
		,	kLessEqual
		,	kEqual
		,	kNotEqual
		,	kGreaterThan
		,	kGreaterEqual
		};

enum
	{	err_Parm		= 'parm'
	};

const unsigned char JDBKeyDescType = 3;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif

struct JDBKeyDesc : JDBObj {
	short			keyOptions;			// key option bits
	long			keyCount;			// record count
	short			keyDepth;			// how deep is the key tree
	blockNumber		keyRootID;			// block id of root block
	short			keyL0BF;			// level zero blocking factor
	short			keyL1BF;			// level one blocking factor
	};

typedef JDBKeyDesc *JDBKeyDescPtr;
const int kJDBKeyDescSize = sizeof( JDBKeyDesc );

struct JDBKeyRecord {
	long			key;				// key value
	long			data;				// data value
	};

typedef JDBKeyRecord *JDBKeyRecordPtr;
const int kJDBKeyRecordSize = sizeof( JDBKeyRecord );

struct JDBKeyBlock {
	blockNumber		kblkPrev;			// previous block in chain
	blockNumber		kblkNext;			// next block in chain
	short			kblkCount;			// how many valid records
	JDBKeyRecord	kblkKey[1];			// array of keys
	};

typedef JDBKeyBlock *JDBKeyBlockPtr;
const int kJDBKeyBlockSize = sizeof( JDBKeyBlock ) - kJDBKeyRecordSize;

#if defined(powerc) || defined (__powerc)
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

const int kJDBKeyDefaultBlockingFactor = (kBlockSize / kJDBKeyRecordSize);

class JDBKeyMgr {
		friend class JDBKey;
		
	protected:
		JDBPtr				mgrFile;
		
	public:
		JDBObjMgrPtr		mgrObjMgr;
		
		JDBKeyMgr( JDBObjMgrPtr );
		~JDBKeyMgr( void );
		
		int NewKey(JDBKey &key
			, int L0BF = kJDBKeyDefaultBlockingFactor
			, int L1BF = kJDBKeyDefaultBlockingFactor
			);
		int GetKey(JDBKey &key, objId id);
		
		int DeleteKey(objId id);
	};

typedef JDBKeyMgr *JDBKeyMgrPtr;
const int kJDBKeyMgrSize = sizeof( JDBKeyMgr );

typedef int (*JDBKeyCompFn)(long,long);

class JDBKey {
		friend class JDBKeyMgr;
	
	protected:
		JDBKeyMgrPtr		keyMgr;					// my manager
		JDBKeyDesc			keyDesc;				// copy of descriptor
		
		struct keyStackElem {
			int				stkIndex;
			blockNumber		stkBlock;
			};
		
		JDBKeyCompFn		keyComp;				// comparison function
		keyStackElem		*keyStack;				// list of useful offsets
		
		void LoadStack(long key);					// construct list
		void LoadStackZero(void);					// same without compare
		void UnloadStack(void);						// finished with list
		
	public:
		objId				objID;					// descriptor id
		
		JDBKey( JDBKeyCompFn fn );				// unbound object, useless
		~JDBKey(void);
		
		int NewKey(long key, long data);			// add (key,data) to key
		int DeleteKey(long key, long data);			// remove instance
		int ReadKey(long key, long *data);			// copy out the data
		int WriteKey(long key, long data);			// change the data
		
		typedef int (*searchKeyFn)(long key, long data);
		int SearchKey(keyMethod op1, long key1, keyMethod op2, long key2, searchKeyFn fn);
	};

typedef JDBKey *JDBKeyPtr;
const int kJDBKeySize = sizeof( JDBKey );

int LongKeyComp(long a, long b);					// keys as longs
int UnsignedKeyComp(long a, long b);				// keys as unsigned
int FloatKeyComp(long a, long b);					// keys as floats

#endif
