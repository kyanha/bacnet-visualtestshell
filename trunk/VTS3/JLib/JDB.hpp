#ifndef _JDB
#define _JDB

#include "JFile.hpp"
#include "JMemory.hpp"

#define _JDBDebug				0
#define _JDBDebugDataCache		0
#define _JDBDebugFreeSpace		0
#define _JDBDebugJVBlock		0
#define _JDBDebugJVBlockCount	0
#define _JDBDebugBlockMgmt		0

//
//	This class implements a block file.  This type of file is divided up into contiguous
//	sections called blocks of an arbitrary size.  Once a block ID is assigned it sticks 
//	to the block even though it may change size and physical location.
//
//	The largest block is kMaxBlockSize * kBlockSize, slightly less than 32Mb.  The block 
//	identifier is a long so the file capacity could be 2^32 times the largest block, but 
//	in the object database world only the upper 24 bits are available.  The maximum size
//	could 512Tb, but that isn't possible.  On the Macintosh the maximum file size is a long, 
//	"only" 4Gb, and on the VAX the file size and current position is 48 bits so there 
//	could be larger files, 256Tb.
//

typedef long blockNumber;							// talk about blocks with these
typedef unsigned short blockSize;					// block sizes
typedef unsigned long transactionId;				// transaction ID

const int			kBlockSize = 512;				// bytes for smallest block
const blockSize		kMaxBlockSize = 65534;			// largest block that can be allocated
const blockSize		kBlockEOL = 65535;				// end of free list indicator

const int blockNumberSize = sizeof( blockNumber );	// compiler knows its four bytes
const blockNumber kNotABlock = 0xFFFFFFFF;			// reserved for flags

//
//	Declare the JDB class forward.  These other cooperating classes will need 
//	pointers to some kind of database.
//

class JDB;
typedef JDB *JDBPtr, **JDBHandle;

//
//	A block space record is used to describe where a block is physically located in a
//	file.
//

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif
struct JDBSpaceRec {
	blockNumber		loc;						// physical location
	blockSize		size;						// number of composite blocks
	
#if (!NT_Platform)
	JDBSpaceRec( blockNumber x = 0, blockSize y = 0 )
		: loc(x), size(y)
		{}
#endif
	};
#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif
	
typedef JDBSpaceRec *JDBSpaceRecPtr, **JDBSpaceRecHandle;
const int kJDBSpaceRecSize = sizeof( JDBSpaceRec );

//
//	A Block Allocation Record is used to describe where a block is physically 
//	located in a file.  Warning: the blockNumber is always a physical addr, 
//	and the location of a block can change as long as the directory entry 
//	referencing it is updated.
//

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif
struct JDBBlockAllocRec : public JDBSpaceRec {
	transactionId	transID;
	int				bytesUsed;
	
#if (!NT_Platform)
	JDBBlockAllocRec( blockNumber w = 0, blockSize x = 0, transactionId y = 0, int z = 0 )
		: JDBSpaceRec(w,x), transID(y), bytesUsed(z)
		{}
#endif
	};
#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

typedef JDBBlockAllocRec *JDBBlockAllocPtr, **JDBBlockAllocHandle;
const int kJDBBlockAllocRecSize = sizeof( JDBBlockAllocRec );

//
//	The file descriptor is a way to uniquely identify each file and keep track of
//	its structure.  It is always physical block zero in the file and doesn't get 
//	written to disk until a transaction completes.
//

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif
struct JDBDescRec {
	transactionId	transID;				// last successful transaction
	unsigned char	fileType;				// 0=Mac, 1=PC, ...
	unsigned char	fileVersion;			// simple version for converting old formats
	short			freeBytes;				// bytes available in descriptor block
	short			structCacheSize;		// structure cache records maintained
	short			dataCacheSize;			// data cache records maintained
	short			dirLevels;				// depth of directory tree
	blockNumber		dirLoc;					// block of top of tree
	blockNumber		nextBlock;				// identifier for next unused block
	JDBSpaceRec		freeList;				// location of free list
	};
#if Mac_Platform
#pragma options align=reset
#endif

typedef JDBDescRec *JDBDescPtr, **JDBDescHandle;
const int kJDBDescRecSize = sizeof( JDBDescRec );

//
//	Different software components need a place to store global information that 
//	may depend on the transaction.  Descriptor objects are available, they are 
//	small slices of the descriptor record.
//

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif
struct JDBDescObj {
	short			objLen;					// total size of descriptor object
	short			objSig;					// object signature
	};
#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

typedef JDBDescObj *JDBDescObjPtr, **JDBDescObjHandle;
const int kJDBDescObjSize = sizeof( JDBDescObj );

//
//	A directory block is used as an intermediate node in a tree where the descriptor
//	record is the root and BAT's are the leaves.  It only contains a list of physical 
//	block locations for the next level down in the tree or the BAT table.
//

const int kJDBDirTableSize = (kBlockSize - 4) / blockNumberSize;

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif
struct JDBDirRec {
	transactionId	transID;						// last successful transaction
	blockNumber		table[kJDBDirTableSize];		// next level down
	};
#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

typedef JDBDirRec *JDBDirPtr, **JDBDirHandle;
const int kJDBDirRecSize = sizeof( JDBDirRec );

//
//	The leaf nodes in the directory tree is an array of block allocation 
//	records.
//

const int kJDBBATTableSize = ((kBlockSize - sizeof(transactionId)) / kJDBBlockAllocRecSize);

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(2)
#endif
struct JDBBATRec {
	transactionId		transID;					// last successful transaction
	JDBBlockAllocRec	table[kJDBBATTableSize];	// table of block data
	};
#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

typedef JDBBATRec *JDBBATPtr, **JDBBATHandle;
const int kJDBBATRecSize = sizeof( JDBBATRec );

//
//	Munger operations need to know how big the header is at the front of the BAT 
//	record to compute the offset properly.
//

const int kJDBBATRecHeaderSize = sizeof(unsigned long) + sizeof(short);

//
//	The list of free and reserved space in the file is maintained as a sorted list
//	of locations and sizes.  It has a similar structure as a BAT block except it 
//	doesn't need tansaction ID's.  It is maintained as a handle and isn't written to 
//	disk until a transaction is about to complete.
//

const int kJDBSpaceHeaderSize = sizeof(unsigned long) + sizeof(short);
const int kJDBSpaceTableSize = ((kBlockSize - kJDBSpaceHeaderSize) / kJDBSpaceRecSize);

#if Mac_Platform
#pragma options align=mac68k
#endif
#if NT_Platform
#pragma pack(push,2)
#endif
struct JDBSpaceAllocRec {
	transactionId	transID;						// last successful transaction
	short			len;							// count of table entries
	JDBSpaceRec		table[kJDBSpaceTableSize];		// table of space data
	};
#if Mac_Platform
#pragma options align=reset
#endif
#if NT_Platform
#pragma pack(pop)
#endif

typedef JDBSpaceAllocRec *JDBSpaceAllocPtr, **JDBSpaceAllocHandle;
const int kJDBSpaceAllocRecSize = sizeof( JDBSpaceAllocRec );

//
//	Each file keeps a cache of directory and BAT blocks to speed up access.  The file
//	descriptor record should be considered part of the cache but is accessed so often 
//	that it is kept on its own.
//
//	When the cache is asked for a block that does not exist the code will create a 
//	new block, mark it unmodified, and read in the data from the file.  The destructor 
//	will save the record if it is marked modified.
//
//	Each cache object needs to know what file it is a part of.  If the cache needs to
//	delete an object and it has been modified, the distructor must understand where the
//	data is going to be written.
//

class JDBStructMgmt {
	public:
		JDBPtr			cacheFile;					// what file this belongs to
		blockNumber		cacheBlock;					// block number of record stored
		Boolean			cacheModified;				// changes have not been saved
		
		union {
			JDBDirRec	cacheDir;					// directory record
			JDBBATRec	cacheBAT;					// BAT record, one or the other!
		};
		
		JDBStructMgmt( JDBPtr fp );					// allocate freely available
		JDBStructMgmt( JDBPtr fp, blockNumber blk );// get a specific block
		~JDBStructMgmt( void );
		
		void	InitDirRec( void );					// initialize as a directory record
		void	InitBATRec( blockNumber start );	// first record is this offset
		
		void	Flush( void );						// easy way to save a modified block
	};

typedef JDBStructMgmt *JDBStructMgmtPtr, **JDBStructMgmtHandle;
const int kJDBStructMgmtSize = sizeof( JDBStructMgmt );

//
//	The following cache object is for (private) use by JDB for dealing with the
//	directory tree of blocks.
//

class JDBStructCache {
	private:
		struct cacheSlot {
			blockNumber			id;								// block number
			JDBStructMgmtPtr	elem;							// pointer to block
		};
		
		int			cacheSize;									// number of records in cache
		cacheSlot	*cacheData;									// pointer to array of slots
		
	public:
		JDBStructCache( int sz );
		~JDBStructCache( void );
		
		JDBStructMgmtPtr Get( blockNumber id );					// get a particular block
		void	Put( JDBStructMgmtPtr elem, blockNumber id );	// put the object into the cache
		void	Remove( blockNumber id );						// remove block from cache
		
		void	Flush( void );									// flush every element
	};

typedef JDBStructCache *JDBStructCachePtr;
const int kJDBStructCacheSize = sizeof( JDBStructCache );
const int kJDBDefaultStructCacheSize = 8;						// starting record count

//
//	The following cache object is for (private) use by JDB for dealing with the
//	data cache of blocks.
//

class JVBlock;

struct JDBDataCacheSlot {
	blockNumber			dataID;							// block number
	JDBBlockAllocRec	dataAlloc;						// physical loc, size, transID
	JHandle				dataData;						// handle to block
	Boolean				dataModified;					// set when modified
	int					dataAge;						// when block was accessed
	JVBlock				*dataRef;						// reference list

	JDBDataCacheSlot( void );
	~JDBDataCacheSlot( void );
	};

typedef JDBDataCacheSlot *JDBDataCacheSlotPtr, **JDBDataCacheSlotHandle;
const int kJDBDataCacheSlotSize = sizeof( JDBDataCacheSlot );
const int kJDBDefaultDataCacheSize = 8;					// default cache size

//
//	Block Management functions are used to move up and down the directory tree easily.  Instances
//	of this class are only used in the block functions of the file, AllocBlock(), FreeBlock() and
//	operator[](), and also in the Write(), Modified() and SetSize() methods of JVBlock.
//

const int maxDirLevels = 5;					// allows for 84*127^4 = 21,852,149,844 BAT's

class JDBBlockMgmt {
	public:
		struct mgmtTableRec {
			short				offset;
			JDBStructMgmtPtr	cp;
		};
		
		JDBPtr				mgmtFile;						// what file this belongs to
		JDBBlockAllocRec	mgmtAlloc;						// copy of dir allocation record
		mgmtTableRec		mgmtTable[maxDirLevels];		// table of cache pointers
		
		JDBBlockMgmt( JDBPtr fp, blockNumber blk );			// set up for a specific block
		~JDBBlockMgmt( void );								// clean up changes
	};

typedef JDBBlockMgmt *JDBBlockMgmtPtr, **JDBBlockMgmtHandle;
const int kJDBBlockMgmtSize = sizeof( JDBBlockMgmt );

//
//	JVBlock
//
//	Blocks are large sections of a JDB.  Blocks may be allocated with various 
//	sizes and may change size during normal operations.  Note that the block number
//	has nothing to do with the size or location of the block in the file.
//
//	The Read(void) and Write(void) methods are overridden to allow integration into 
//	higher layer structures and notification when changes are written back to disk, 
//	respectively.  Alloc() and Free() are virtual for the same reason.
//
//	Code that uses blocks WILL NOT make crude and unjust assumptions about the size of
//	a block so changing the size of a block will never do nasty things.  The SetSize 
//	method is virtual so subclasses can be notified when the application has 
//	requested a block size change.  If this function is overridden, JVBlock::SetSize()
//	should be called (before for "pre-change" processing and after for "post-change").
//	It is possible but unlikely that a subclass of JVBlock will disallow size changes.
//
//	WARNING:  The block data is a handle.  There is no way for this code to know how
//	much of a block is actually data, so it will always be divisible by kBlockSize.  
//	This isn't as nasty as it first might seem, a block is usually divided up into 
//	smaller objects.
//

typedef void (*voidPtr);

class JVBlock {
		friend class JDB;								// allow access to data
		friend struct JDBDataCacheSlot;
		
	protected:
		JDBPtr					blockFile;				// member of which file
		JDBDataCacheSlotPtr		blockCache;				// pointer to block information
		JHandle					blockUserPtr;			// user pointer to block data
		JVBlock					*blockRefNext;			// reference list
		
	public:
		blockNumber				blockID;				// public copy of block ID
		
		JVBlock( JDBPtr jbf, JHandle usr, blockNumber id, int size = 0 );	// bind block
		JVBlock( JDBPtr jbf, JHandle usr, JDBBlockAllocRec loc );		// absolute location
		~JVBlock( void );												// release binding
		
		void		Modified( void );					// mark the block modified
		Boolean		isModified( void );					// true when modified
		
		int			GetSize( void );					// return current size
		void		SetSize( int newSize );				// change the size
	};

typedef JVBlock	*JVBlockPtr, **JVBlockHandle;
const int kJVBlockSize = sizeof( JVBlock );

//
//	Now we can describe all the pieces that make up a JDB.
//

class JDB : public JFile {
		friend class JVBlock;							// these have direct access
		friend class JDBStructMgmt;
		friend class JDBBlockMgmt;
		friend class JDBObjMgr;							// for debugging

	private:
		void		LoadFreeSpace( void );
		JDBSpaceRec ExtractFreeSpace( blockSize size );
		void		PutSpace( JDBSpaceAllocHandle sph, const JDBSpaceRec bat );
		void		FlushReservedSpace( void );
		void		MergeSpace( void );
		void		SaveFreeSpace( int complete );
		
		void		AllocStructCache( int size );
		void		FlushStructCache( void );
		
		void		AllocDataCache( int size );
		void		FlushDataCache( void );
		
		JDBDataCacheSlotPtr	GetFreeDataCacheSlot( void );
		JDBDataCacheSlotPtr	GetDataCacheSlot( blockNumber id );
		
		int			ReadBlockData( blockNumber id, const JDBSpaceRec sr, int bytes, void *data );
		int			WriteBlockData( blockNumber id, const JDBSpaceRec sr, int bytes, void *data );
#if _JDBDebug
		void		DebugBAT( blockNumber id, int level );
#endif
		
	protected:
		JDBDescPtr				fileDesc;				// current file descriptor
		short					fileDescSize;			// descriptor size
		JDBSpaceAllocHandle		fileFree;				// free space
		JDBSpaceAllocHandle		fileReserved;			// reserved space
		
		JDBStructCache			*fileStructCache;		// cache of structure management
		JDBDataCacheSlot		*fileDataCache;			// cache of data block descriptors
		int						fileDataSize;			// number of records in data cache
		int						fileDataAge;			// access count for aging data cache
		
	public:
		Boolean					transInProgress;		// transaction in progress
		
		JDB( void );									// initialize empty structures
		virtual ~JDB( void );							// free structures
		
		virtual void Init( void );						// newly created file
		virtual void Open( void );						// existing file
		virtual void Close( void );						// clean up operations
		virtual void Flush( void );						// no buffers sitting around
		
		blockNumber NewBlock( const int size );				// allocate a block
		void		DeleteBlock( const blockNumber id );	// free a block
		
		void	StartTransaction( void );				// keep track of changes
		void	CancelTransaction( void );				// forget changes
		void	CompleteTransaction( void );			// write changes to disk
		
#if _JDBDebug
		virtual void Debug( void );						// generate debugging info
#endif
		
		JDBDescObjPtr GetDesc( short sig );				// find a descriptor record
		JDBDescObjPtr AllocDesc( short sig, short size );// allocate a new descriptor record
		int		ResizeDesc( short sig, short size );	// change a descriptor size
		int		DeleteDesc( short sig );				// delete a descriptor record
	};

const int kJDBSize = sizeof( JDB );

#endif
