
#ifndef _JDBArrayMgr
#define _JDBArrayMgr

#include "JDB.hpp"
#include "JDBObj.hpp"

#define _JDBArrayDebug		0

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif

const unsigned char JDBArrayDescType = 2;

struct JDBArrayDesc : JDBObj {
	long			arryCount;		// total valid elements
	long			arryLen;		// how many elements have been added
	long			arryNRec;		// next available slot
	short			arrySize;		// big are elements
	short			arryDepth;		// how deep is the directory tree
	short			arryL0BF;		// level zero blocking factor
	short			arryL1BF;		// level one blocking factor
	blockNumber		arryRootID;		// block id of root block
	};

typedef JDBArrayDesc *JDBArrayDescPtr;
const int kJDBArrayDescSize = sizeof( JDBArrayDesc );

struct JDBArrayBlock {
	blockNumber		arryBlock[1];	// id's of subordinate blocks
	};

typedef JDBArrayBlock *JDBArrayBlockPtr;
const int kJDBArrayBlockSize = sizeof( JDBArrayBlock );

#if defined(powerc) || defined (__powerc)
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

class JDBArrayMgr {
		friend class JDBArray;
		
	protected:
		JDBPtr				mgrFile;
		void DeleteArrayHelper( int depth, blockNumber blckID, int bf );
		
	public:
		JDBObjMgrPtr		mgrObjMgr;
		
		JDBArrayMgr( JDBObjMgrPtr );
		~JDBArrayMgr(void);
		
		int NewArray(JDBArray &ary, int rSize, int L0BF = 0, int L1BF = 0);
		int GetArray(JDBArray &ary, objId id);
		
		int DeleteArray(objId id);
	};

typedef JDBArrayMgr *JDBArrayMgrPtr;
const int kJDBArrayMgrSize = sizeof( JDBArrayMgr );

class JDBArray {
		friend class JDBArrayMgr;
	
	protected:
		JDBArrayMgrPtr		arrMgr;					// my manager
		JDBArrayDesc		arrDesc;				// copy of descriptor
		
		struct aryStackElem {
			int				stkIndex;
			blockNumber		stkBlock;
			};
		
		aryStackElem	*arrStack;					// list of useful offsets
		void LoadStack(int rnum);					// construct list
		void UnloadStack(void);						// finished with list
		
	public:
		objId				objID;					// descriptor id
		
		JDBArray(void);								// unbound object, useless
		~JDBArray(void);
		
		int NewElem(int *rnum, void *data);			// creates a new object
		int DeleteElem(int rnum);					// frees space used in database
		
		int ReadElem(int rnum, void *data);			// reads object data from block
		int WriteElem(int rnum, void *data);		// writes object to block
		
		int Count( void );							// number of elements
		int AllocCount( void );						// number of slots
	};

typedef JDBArray *JDBArrayPtr;
const int kJDBArraySize = sizeof( JDBArray );

#endif
