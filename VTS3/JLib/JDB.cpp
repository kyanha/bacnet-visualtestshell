
#include "stdafx.h"

#include <stdio.h>
#include <string.h>

#include "JConfig.hpp"
#include "JError.hpp"
#include "JDB.hpp"
#include "JMemory.hpp"

#if _JDBDebug
#include <iostream.h>
#endif

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#pragma mark ---------- Constructor

//
//	JDB::JDB
//

JDB::JDB( void )
{
	fileStructCache = (JDBStructCachePtr)nil;
	fileFree = (JDBSpaceAllocHandle)JNewHandle( 0 );
	fileReserved = (JDBSpaceAllocHandle)JNewHandle( 0 );
	
	fileDesc = (JDBDescPtr)nil;
	fileDescSize = 1;
	
	fileDataCache = 0;
	fileDataSize = kJDBDefaultDataCacheSize;
	fileDataAge = 1;
	
	transInProgress = false;
}

//
//	JDB::~JDB
//

JDB::~JDB( void )
{
	// make sure the file is closed!
	JDisposeHandle( (JHandle)fileFree );
	JDisposeHandle( (JHandle)fileReserved );
}

#pragma mark ---------- Basic File Management

//
//	JDB::Init
//

void JDB::Init( void )
{
	JDBStructMgmtPtr	cp
	;
	
	// allocate a descriptor
	ThrowIfMemFail_( fileDesc = (JDBDescPtr)(new char[fileDescSize * kBlockSize]) );
	
	// initialize the file descriptor
	fileDesc->transID = 1;						// last successful transaction
	fileDesc->structCacheSize = kJDBDefaultStructCacheSize;	// struct cache records maintained
	fileDesc->dataCacheSize = fileDataSize;		// data cache records maintained
	fileDesc->fileType = 0;						// this is a Macintosh file
	fileDesc->fileVersion = 1;					// start with version one
	
	// free bytes for other header information
	fileDesc->freeBytes = (fileDescSize * kBlockSize) - kJDBDescRecSize;
	
	// no transaction in progress
	transInProgress = false;
	
	// allocate the structure cache
	AllocStructCache( fileDesc->structCacheSize );
	
	// allocate the data cache
	AllocDataCache( fileDataSize );
	
	// set up the free space description
	fileDesc->freeList.loc = 1;						// location of free list
	fileDesc->freeList.size = 1;					// starts out needing one block
	JSetHandleSize( (JHandle)fileFree, 2 * kJDBBlockAllocRecSize );
	(*fileFree)->transID = fileDesc->transID;		// sync with file
	(*fileFree)->len = 1;							// only one element here
	(*fileFree)->table[0].loc = 2;					// starts at block 2
	(*fileFree)->table[0].size = kBlockEOL;			// end of free list indicator (always!)
	
	// initialize the first BAT record
	ThrowIfMemFail_( cp = new JDBStructMgmt( this ) );
	cp->InitBATRec( 0 );
	(*fileStructCache).Put( cp, cp->cacheBlock );	// cache it!
	
	fileDesc->dirLevels = 1;						// only one level
	fileDesc->dirLoc = cp->cacheBlock;				// where is top of tree
	fileDesc->nextBlock = 0;						// identifier for next unused block
	
	// save the descriptor
	ThrowIfError_( Write( 0, fileDescSize * kBlockSize, fileDesc ) );
	
	// initialize the reserved space to empty
	FlushReservedSpace();
	
	// write all this stuff
	Flush();
}

//
//	JDB::Open
//

void JDB::Open( void )
{
	// allocate a descriptor
	ThrowIfMemFail_( fileDesc = (JDBDescPtr)(new char[fileDescSize * kBlockSize]) );
	
	// read in the file descriptor
	ThrowIfError_( Read( 0, fileDescSize * kBlockSize, fileDesc ) );
	
	// allocate the correct size empty cache
	AllocStructCache( fileDesc->structCacheSize );
	
	// allocate the data cache
	fileDataSize = fileDesc->dataCacheSize;
	AllocDataCache( fileDataSize );
	
	// load the free space from disk
	LoadFreeSpace();
	
	// initialize the reserved space to empty
	FlushReservedSpace();
	
	// no transaction in progress
	transInProgress = false;
}

//
//	JDB::Close
//

void JDB::Close( void )
{
	// make sure everything gets flushed
	JDB::Flush();
	
	// now the file can really be closed
	JFile::Close();

	// toss the file descriptor buffer
	if (fileDesc)
		delete[] (char *)fileDesc;
	
	// toss the structure and data cache
	if (fileStructCache)
		delete fileStructCache;
	if (fileDataCache)
		delete[] fileDataCache;
	
	// toss the free and reserved space lists
	JSetHandleSize( (JHandle)fileFree, 0 );
	JSetHandleSize( (JHandle)fileReserved, 0 );
}

//
//	JDB::Flush
//
//	Called when the contents of the file should be saved on the disk.
//

void JDB::Flush( void )
{
	FlushDataCache();				// save modified data blocks
	FlushStructCache();				// save modified cache records
	SaveFreeSpace( false );			// save the free space
	
	// save the descriptor unless a transaction is in progress
	if (!transInProgress)	
		ThrowIfError_( Write( 0, fileDescSize * kBlockSize, fileDesc ) );
	
	JFile::Flush();
}

#pragma mark ---------- Block Management

//
//	JDB::GetFreeDataCacheSlot
//

JDBDataCacheSlotPtr	JDB::GetFreeDataCacheSlot( void )
{
	JDBDataCacheSlotPtr		dcsp = 0
	,						curp = fileDataCache
	;
	
	// find the oldest unreferenced block
	for (int i = 0; i < fileDataSize; i++, curp++ )
		if (!curp->dataRef)
			if ((!dcsp) || (curp->dataAge < dcsp->dataAge))
				dcsp = curp;
	if (!dcsp)
		Throw_( -1 );
	
	// see if it was modified
	if (dcsp->dataModified) {
		if ((dcsp->dataID != -2) && transInProgress)
			if (dcsp->dataAlloc.transID != fileDesc->transID) {	// moved?
#if _JDBDebugDataCache
				cout << dcsp->dataID << " reserved " << dcsp->dataAlloc.loc << ':' << dcsp->dataAlloc.size;
#endif
				PutSpace( fileReserved, dcsp->dataAlloc );
				
				JDBBlockMgmt	mgmt( this, dcsp->dataID )
				;
				JDBSpaceRec		newSpace = ExtractFreeSpace( dcsp->dataAlloc.size )
				;
				
				dcsp->dataAlloc.loc =		mgmt.mgmtAlloc.loc =		newSpace.loc;
				dcsp->dataAlloc.size =		mgmt.mgmtAlloc.size =		newSpace.size;
				dcsp->dataAlloc.transID =	mgmt.mgmtAlloc.transID =	fileDesc->transID;
#if _JDBDebugDataCache
				cout << " moved to " << newSpace.loc << ':' << newSpace.size << endl;
#endif
			}
		
		// update the transaction number
		dcsp->dataAlloc.transID = fileDesc->transID;
		
		// write out the data
		JHLock( (JHandle)dcsp->dataData );
		WriteBlockData( dcsp->dataID, dcsp->dataAlloc, dcsp->dataAlloc.bytesUsed, *dcsp->dataData );
		JHUnlock( (JHandle)dcsp->dataData );
		
		// block is no longer modified
		dcsp->dataModified = false;
	}
	
	// make it the youngest referenced block
	dcsp->dataAge = fileDataAge++;
	
	// return the slot pointer
	return dcsp;
}

//
//	JDB::GetDataCacheSlot
//

JDBDataCacheSlotPtr	JDB::GetDataCacheSlot( blockNumber id )
{
	JDBDataCacheSlotPtr		dcsp = fileDataCache
	;
	
	for (int i = 0; i < fileDataSize; i++, dcsp++ )
		if (dcsp->dataID == id)
			return dcsp;
	
	return 0;
}

//
//	JDB::ReadBlockData
//

int JDB::ReadBlockData( blockNumber id, const JDBSpaceRec sr, int bytes, void *data )
{
#if _JDBDebugDataCache
	cout << "JDB::ReadBlockData(" << id << ',' << sr.loc << ',' << bytes << ",...)" << endl;
#else
#pragma unused (id)
#endif
	
	return Read( sr.loc * kBlockSize, bytes, data );
}

//
//	JDB::WriteBlockData
//

int JDB::WriteBlockData( blockNumber id, const JDBSpaceRec sr, int bytes, void *data )
{
#if _JDBDebugDataCache
	cout << "JDB::WriteBlockData(" << id << ',' << sr.loc << ',' << bytes << ",...)" << endl;
#else
#pragma unused (id)
#endif
	
	return Write( sr.loc * kBlockSize, bytes, data );
}

//
//	JDB::NewBlock
//

blockNumber JDB::NewBlock( const int size )
{
	JDBBlockMgmt	mgmt( this, fileDesc->nextBlock )
	;
	blockNumber		rsltID
	;
	
	rsltID = fileDesc->nextBlock;
	fileDesc->nextBlock = mgmt.mgmtAlloc.loc;
	
	JDBSpaceRec		newSpace = ExtractFreeSpace( (size + kBlockSize) / kBlockSize )
	;
	
	mgmt.mgmtAlloc.loc =		newSpace.loc;
	mgmt.mgmtAlloc.size =		newSpace.size;
	mgmt.mgmtAlloc.transID =	fileDesc->transID;
	mgmt.mgmtAlloc.bytesUsed =	size;
	
#if _JDBDebugDataCache
	cout << "new block allocated at " << newSpace.loc << ':' << newSpace.size << endl;
#endif
	
	int			buffSize = (newSpace.size * kBlockSize) / sizeof(int)
	,			*buff, *c
	;
	
	// create a temporary buffer
	ThrowIfMemFail_( buff = new int[ buffSize ] );
	
	// flush the contents
	c = buff;
	for (int i = 0; i < buffSize; i++)
		*c++ = 0;
	
	// write this to the file
	WriteBlockData( rsltID, newSpace, buffSize * sizeof(int), buff );
	
	// done with buffer
	delete[] buff;
	
	return rsltID;
}

//
//	JDB::DeleteBlock
//

void JDB::DeleteBlock( const blockNumber id )
{
	JDBBlockMgmt	mgmt( this, id )
	;
	
	// reserve/release current space
	if (transInProgress)
		PutSpace( fileReserved, mgmt.mgmtAlloc );
	else
		PutSpace( fileFree, mgmt.mgmtAlloc );
	
	// store the current free block in this one
	mgmt.mgmtAlloc.loc =		fileDesc->nextBlock;
	mgmt.mgmtAlloc.size =		0;
	mgmt.mgmtAlloc.transID =	0;
	mgmt.mgmtAlloc.bytesUsed =	0;
	
	// this becomes first free block
	fileDesc->nextBlock = id;
}

#pragma mark ---------- Space Management

//
//	JDB::LoadFreeSpace
//
//	Read in the current free space description from disk.
//

void JDB::LoadFreeSpace( void )
{
	long	freeSize
	;
	
	// get the current free space description
	freeSize = fileDesc->freeList.size * kBlockSize;
	JSetHandleSize( (JHandle)fileFree, freeSize );
	
	// read the whole thing
	ThrowIfError_( Read( fileDesc->freeList.loc * kBlockSize, freeSize, *fileFree ) );
	
	// trim the free space handle down to only what it needs to be
	freeSize = kJDBSpaceHeaderSize + (*fileFree)->len * kJDBSpaceRecSize;
	JSetHandleSize( (JHandle)fileFree, freeSize );
}

//
//	JDB::ExtractFreeSpace
//
//	Given a space handle and a number of blocks, this function finds the first
//	free space section with enough space and returns a JDBBlockAllocRec describing it.
//	It modifies the handle to remove this space from the list.  This method 
//	may extend the file to allocate more physical space.
//
//	The last table entry in this list is always the location of the end of the file 
//	a little larger than the largest possible block that can be allocated.  It makes the 
//	search and MergeSpace() easier.
//

JDBSpaceRec JDB::ExtractFreeSpace( blockSize size )
{
	int			i, len
	;
	JDBSpaceRec	*freelist
	,			rslt
	;
	
	//
	//	Scan for the first available space.  This scan operation could easily be changed to
	//	best available but isn't worth the effort until you work with very large blocks that 
	//	differ only slightly in size leading to fragmentation.
	//
	freelist = (*fileFree)->table;
	len = (*fileFree)->len;
	for ( i = 0; i < len; i++ )
		if (size <= freelist[i].size)
			break;
	
	//
	//	Prepare the result.
	//
	rslt.loc = freelist[i].loc;
	rslt.size = size;
	
	//
	//	Move the position up passed the space just found.  If this is the last entry don't
	//	subtract the allocated space from the kBlockEOL indicator.
	//
	freelist[i].loc += size;
	if (i != (len - 1)) {
		freelist[i].size -= size;
		//
		// If this is an exact fit, delete the table entry.
		//
		if (freelist[i].size == 0) {
			JMunger( (JHandle)fileFree, kJDBSpaceHeaderSize + i * kJDBSpaceRecSize
			,		(JPtr)nil, kJDBSpaceRecSize, (JPtr)-1, 0
			);
			(*fileFree)->len -= 1;
		}
	} else
		// change the files EOF indicator to add the newly allocated blocks.
		SetSize( freelist[i].loc * kBlockSize );
	
	return rslt;
}

//
//	JDB::PutSpace
//
//	Given a space handle and JDBBlockAllocRec, this function adds the section to the
//	list maintaining it in sorted order by physical block number and combining
//	blocks whenever it can.  This works with free space and reserved space lists.
//

void JDB::PutSpace( JDBSpaceAllocHandle sph, const JDBSpaceRec sr )
{
	int			i, len
	;
	JDBSpaceRec	*list
	;
	
	if (sr.size == 0)
		return;
	
	list = (*sph)->table;
	len = (*sph)->len;
	
	// look for the first place where we might have a fit
	for (i = 0; i < len; i++)
		if (sr.loc <= list[i].loc + list[i].size)
			break;
	
	//
	//	First case, check to see if we've passed the end of the list.  This will never
	//	happen while we are dealing with the free list, only the reserved list.  Append
	//	the new entry to the end.
	//
	if (i == len) {
		// this is an insert operation where the location is at the end of the handle.
		JMunger( (JHandle)sph, kJDBSpaceHeaderSize + i * kJDBSpaceRecSize
		,		(JPtr)-1, 0, (JPtr)&sr, kJDBSpaceRecSize
		);
		(*sph)->len += 1;
		return;
	}
	
	//
	//	Second case, the new entry comes right after one that is already in the list.  Add
	//	the new size to the existing size.
	//
	if (sr.loc == list[i].loc + list[i].size) {
		list[i].size += sr.size;
		
		// see if it can be combined with the next entry
		if ((i+1 < len) && (list[i].loc + list[i].size == list[i+1].loc)) {
			//
			//	Add the length of the next entry to the current one.  If we are dealing
			//	with the end of the list, set list[i] to be the new end and continue,
			//	deleting the next element.
			//
			if (list[i+1].size == kBlockEOL)
				list[i].size = kBlockEOL;
			else
			//
			//	If the sum of the two elements will be bigger than the maximum allowable
			//	size, leave the two alone.  This may lead to some fragmentation.
			//
			if (((long)list[i].size + (long)list[i+1].size) > kMaxBlockSize)
				return;
			else
			//
			//	No more tests, add them up and delete [i+1], blocks are combined.
			//
			list[i].size += list[i+1].size;
			
			// this is a delete operation.
			JMunger( (JHandle)sph, kJDBSpaceHeaderSize + (i + 1) * kJDBSpaceRecSize
			,		(JPtr)nil, kJDBSpaceRecSize, (JPtr)-1, 0
			);
			(*sph)->len -= 1;
		}
		return;
	}
	
	//
	//	Third case, new entry is just before [i].  Move the start back as far as it can go.
	//	If there would be a size overflow insert it by itself.  The idea is that there will
	//	be many blocks of the same size moving around during a transaction and for these huge
	//	blocks it's better to leave them alone.
	//
	if (sr.loc + sr.size == list[i].loc) {
		//
		//	If we are dealing with the end of the free list, move the loc to the new shorter
		//	end, that's all we can do.
		//
		if (list[i].size == kBlockEOL) {
			list[i].loc = sr.loc;
			return;
		} else
		//
		//	Check for maximum allowable size.  We will fall through to the insert.
		//
		if (((long)sr.size + (long)list[i].size) > kMaxBlockSize)
			;
		else
		//
		//	Simple combine will work here.  Move the list location back and add in the new
		//	entry size.
		//
		{
			list[i].loc = sr.loc;
			list[i].size += sr.size;
			return;
		}
	}
	
	//
	//	Last case, new entry is sitting in the middle of no place and can't be combined 
	//	with any other block.  This JMunger() call is an insert operation, can't you tell? :-)
	//
	JMunger(	(JHandle)sph, kJDBSpaceHeaderSize + i * kJDBSpaceRecSize
	,			(JPtr)-1, 0, (JPtr)&sr, kJDBSpaceRecSize
	);
	(*sph)->len += 1;
}

//
//	JDB::FlushReservedSpace
//
//	At the end of a transaction we are done with the reserved space list and it
//	should be set to an empty list.  This is called when the transaction is finalized
//	or canceled.  Just to be useful, this is also called when the reserved space
//	handle needs to be initialized.
//

void JDB::FlushReservedSpace( void )
{
	JSetHandleSize( (JHandle)fileReserved, kJDBBlockAllocRecSize );
	(*fileReserved)->transID = 0;
	(*fileReserved)->len = 0;
}

//
//	JDB::MergeSpace
//
//	This function merges the reserved space into the free space, combining it 
//	whenever possible.  This is called during the tail end of a transaction when 
//	it is complete.
//

void JDB::MergeSpace( void )
{
	int			i, len
	;
	JDBSpaceRec	*list
	;
	
	JHLock( (JHandle)fileReserved );
	
	len = (*fileReserved)->len;
	list = (*fileReserved)->table;
	for (i = 0; i < len; i++)
		PutSpace( fileFree, list[i] );
	
	JHUnlock( (JHandle)fileReserved );
	
	FlushReservedSpace();
}

//
//	JDB::SaveFreeSpace
//
//	When a modified free space list needs to be written to the file this function
//	is called to do it.  If a transaction is in progress a new location for it is
//	requested, it will be free when the transaction is completed.
//
//	If the transaction ID of the free space matches the file this function has 
//	already been called and the old location doesn't have to be reserved.
//

void JDB::SaveFreeSpace( int complete )
{
	int				stat, hsize
	;
	blockSize		reqSize
	;
	JDBSpaceRec		newLoc
	;
		
	// figure out how much space we need, allow for merge
	reqSize = (JGetHandleSize((JHandle)fileFree) + JGetHandleSize((JHandle)fileReserved)) / kBlockSize + 1;
	
	// allow for a transaction
	if (transInProgress && ((*fileFree)->transID != fileDesc->transID)) {
#if _JDBDebugFreeSpace
		cout << "free space reserved "
			<< fileDesc->freeList.loc << ':' << fileDesc->freeList.size;
#endif
		PutSpace( fileReserved, fileDesc->freeList );
		newLoc = ExtractFreeSpace( reqSize );
#if _JDBDebugFreeSpace
		cout << ", moved to " << newLoc.loc << ':' << newLoc.size << endl;
#endif
		fileDesc->freeList = newLoc;
	} else
	if (fileDesc->freeList.size != reqSize) {
		PutSpace( fileFree, fileDesc->freeList );
		fileDesc->freeList = ExtractFreeSpace( reqSize );
	}
	
	// see if the reserved space should be merged in
	if (complete)
		MergeSpace();
	
	// make sure it has a current transaction ID
	(*fileFree)->transID = fileDesc->transID;
	
	// Round the handle size up to a block size.  This has to happen 
	// because if the current transaction in progress is canceled, the
	// LoadFreeSpace() function wont know how much to read and will assume 
	// that it can get at least some multiple of kBlockSize bytes.  It will 
	// trim it down.
	hsize = JGetHandleSize( (JHandle)fileFree );
	JSetHandleSize( (JHandle)fileFree, hsize + (kBlockSize - (hsize % kBlockSize)) );
	
	// write the free space
	JHLock( (JHandle)fileFree );
	stat =
		Write(	fileDesc->freeList.loc * kBlockSize
		,		JGetHandleSize( (JHandle)fileFree )
		,		*fileFree
		);
	JHUnlock( (JHandle)fileFree );
	
	// set it back to its correct size
	JSetHandleSize( (JHandle)fileFree, hsize );

	ThrowIfError_( stat );
}

#pragma mark ---------- Cache Management

//
//	JDB::AllocStructCache
//

void JDB::AllocStructCache( int size )
{
	ThrowIfMemFail_( fileStructCache = new JDBStructCache( size ) );
}

//
//	JDB::FlushStructCache
//

void JDB::FlushStructCache( void )
{
	if (!fileStructCache)
		return;
	
	fileStructCache->Flush();
}

//
//	JDB::AllocDataCache
//

void JDB::AllocDataCache( int size )
{
	ThrowIfMemFail_( fileDataCache = new JDBDataCacheSlot[ size ] );
	fileDataSize = size;
}

//
//	JDB::FlushDataCache
//

void JDB::FlushDataCache( void )
{
	JDBDataCacheSlotPtr		dcsp = fileDataCache
	;
	
	for (int i = 0; i < fileDataSize; i++, dcsp++ )
		if (dcsp->dataModified) {
			if ((dcsp->dataID != -2) && transInProgress)
				if (dcsp->dataAlloc.transID != fileDesc->transID) {	// moved?
#if _JDBDebugDataCache
					cout << dcsp->dataID << " reserved " << dcsp->dataAlloc.loc << ':' << dcsp->dataAlloc.size;
#endif
					PutSpace( fileReserved, dcsp->dataAlloc );
					
					JDBBlockMgmt	mgmt( this, dcsp->dataID )
					;
					JDBSpaceRec		newSpace = ExtractFreeSpace( dcsp->dataAlloc.size )
					;
				
					dcsp->dataAlloc.loc =		mgmt.mgmtAlloc.loc =		newSpace.loc;
					dcsp->dataAlloc.size =		mgmt.mgmtAlloc.size =		newSpace.size;
					dcsp->dataAlloc.transID =	mgmt.mgmtAlloc.transID =	fileDesc->transID;
#if _JDBDebugDataCache
					cout << " moved to " << newSpace.loc << ':' << newSpace.size << endl;
#endif
				}
			
			// always update the transaction number
			dcsp->dataAlloc.transID = fileDesc->transID;
			
			// write out the data
			if (!dcsp->dataRef) {
				JHLock( (JHandle)dcsp->dataData );
				WriteBlockData( dcsp->dataID, dcsp->dataAlloc, dcsp->dataAlloc.bytesUsed, *dcsp->dataData );
				JHUnlock( (JHandle)dcsp->dataData );
			} else
				WriteBlockData( dcsp->dataID, dcsp->dataAlloc, dcsp->dataAlloc.bytesUsed, *dcsp->dataData );
			
			// block is no longer modified
			dcsp->dataModified = false;
		}
}

#pragma mark ---------- Transaction Management

//
//	JDB::StartTransaction
//
//	Starting a transaction means changes to the database are going to be put
//	in currently free space and when it is complete the file descriptor will
//	be written.
//

void JDB::StartTransaction( void )
{
	if (transInProgress)
		Throw_( -1 );	// transaction already in progress
	
	Flush();
	
	transInProgress = true;
	fileDesc->transID += 1;				// will wrap to zero someday
}

//
//	JDB::CancelTransaction
//
//	Canceling a transaction means everthing should reset to the point it was
//	before the transaction started.  This is a slick trick: when the fileStructCache is
//	deleted, all of the modified directory and BAT records will be written to
//	disk in "free" locations (the destructor for a cache record will make sure
//	its saved and knows that a transaction is still in progress).
//

void JDB::CancelTransaction( void )
{
	JVBlockPtr				bp, np
	;
	JDBDataCacheSlotPtr		dcsp = fileDataCache
	;
	
	if (!transInProgress)
		Throw_( -1 );		// transaction not in progress
	
	// dump modified data cache blocks
	for (int i = 0; i < fileDataSize; i++, dcsp++ )
		if (dcsp->dataModified || ((dcsp->dataID != -1) && (dcsp->dataAlloc.transID == fileDesc->transID))) {
			// dereference any blocks (there shouldn't be any)
			for (bp = dcsp->dataRef; bp; bp = np) {
				np = bp->blockRefNext;
				
				bp->blockCache = (JDBDataCacheSlotPtr)nil;
				*bp->blockUserPtr = (JPtr)nil;
				bp->blockRefNext = (JVBlockPtr)nil;
			}
			
			// make it look like it was never used
			dcsp->dataID = -1;
			dcsp->dataAlloc.loc = 0;
			dcsp->dataAlloc.size = 0;
			dcsp->dataAlloc.transID = 0;
			dcsp->dataModified = 0;
			dcsp->dataAge = 0;
			dcsp->dataRef = (JVBlockPtr)nil;
			JSetHandleSize( (JHandle)dcsp->dataData, 0 );
		}
		
	// delete the old structure cache
	delete fileStructCache;
	
	// read in the old file descriptor
	ThrowIfError_( Read( 0, fileDescSize * kBlockSize, fileDesc ) );
	
	// allocate the correct size empty cache
	AllocStructCache( fileDesc->structCacheSize );
	
	// load the free space from disk
	LoadFreeSpace();
	
	// initialize the reserved space to empty
	FlushReservedSpace();
	
	// no transaction in progress
	transInProgress = false;
}

//
//	JDB::CompleteTransaction
//
//	Now a transaction is complete.  SaveFreeSpace() knows that when a transaction is in 
//	progress it should move to a new location if it hasn't already done so.  The last
//	save of the file descriptor must be the last IO to the file.  A better mechanism is
//	to bypass the Finder cache for this last write, but leave it for now.
//

void JDB::CompleteTransaction( void )
{
	if (!transInProgress)
		Throw_( -1 );				// transaction not in progress
	
	FlushDataCache();								// save blocks in data cache
	FlushStructCache();								// save structure cache records
	SaveFreeSpace( true );							// save the free space
	
	transInProgress = false;
	ThrowIfError_( Write( 0, fileDescSize * kBlockSize, fileDesc ) );
	
	JFile::Flush();									// tell file system to flush buffers
}

#pragma mark ---------- Descriptor Maintenance

//
//	JDB::GetDesc
//

JDBDescObjPtr JDB::GetDesc( short sig )
{
	JDBDescObjPtr	cur, dataEnd
	;
	
	cur = (JDBDescObjPtr)((char *)fileDesc + kJDBDescRecSize);
	dataEnd = (JDBDescObjPtr)((char *)fileDesc + fileDescSize*kBlockSize - fileDesc->freeBytes);
	while (cur < dataEnd) {
		if (cur->objSig == sig)
			return cur;
		cur = (JDBDescObjPtr)((char *)cur + cur->objLen);
	}
	
	return 0 /* object not found */;
}

//
//	JDB::AllocDesc
//

JDBDescObjPtr JDB::AllocDesc( short sig, short size )
{
	JDBDescObjPtr	dataEnd
	;
	
	if (size < (sizeof(short) * 2))
		return 0 /* new object size too small */;

	if (size > fileDesc->freeBytes)
		return 0 /* not enough space */;
	
	dataEnd = (JDBDescObjPtr)((char *)fileDesc + fileDescSize*kBlockSize - fileDesc->freeBytes);
	dataEnd->objLen = size;
	dataEnd->objSig = sig;
	
	fileDesc->freeBytes -= size;
	
	return dataEnd;
}

//
//	JDB::ResizeDesc
//

int JDB::ResizeDesc( short sig, short size )
{
	JDBDescObjPtr	curObj, dataEnd, nxtObj
	;
	
	if (size < (sizeof(short) * 2))
		return -1 /* new object size too small */;

	curObj = (JDBDescObjPtr)((char *)fileDesc + kJDBDescRecSize);
	dataEnd = (JDBDescObjPtr)((char *)fileDesc + fileDescSize*kBlockSize - fileDesc->freeBytes);
	while (curObj < dataEnd) {
		if (curObj->objSig == sig)
			break;
		curObj = (JDBDescObjPtr)((char *)curObj + curObj->objLen);
	}
	if (curObj >= dataEnd)
		return -1 /* changing the size of a non-existant object */;
	
	if (curObj->objLen > size) {
		nxtObj = (JDBDescObjPtr)((char *)curObj + curObj->objLen);
		fileDesc->freeBytes += curObj->objLen - size;
		
		// save the new size
		curObj->objLen = size;
		
		// slide the rest of the data down
		if (nxtObj != dataEnd)
			JBlockMove(	(JPtr)nxtObj
				,		(JPtr)(curObj + curObj->objLen)
				,		(char *)nxtObj - (char *)dataEnd
				);
	} else
	if (curObj->objLen < size) {
		// see if the block must expand to create some space
		if ((size - curObj->objLen) > fileDesc->freeBytes)
			return -1 /* not enough space */;
		
		nxtObj = (JDBDescObjPtr)((char *)curObj + curObj->objLen);
		fileDesc->freeBytes -= size - curObj->objLen;
		
		// slide the rest of the data up out of the way
		if (nxtObj != dataEnd)
			JBlockMove(	(JPtr)nxtObj
				,		(JPtr)(curObj + size)
				,		(char *)nxtObj - (char *)dataEnd
				);
		
		// save the new size
		curObj->objLen = size;
	}
	
	return 0;
}

//
//	JDB::DeleteDesc
//

int JDB::DeleteDesc( short sig )
{
	JDBDescObjPtr	curObj, dataEnd, nxtObj
	;
	
	curObj = (JDBDescObjPtr)((char *)fileDesc + kJDBDescRecSize);
	dataEnd = (JDBDescObjPtr)((char *)fileDesc + fileDescSize*kBlockSize - fileDesc->freeBytes);
	while (curObj < dataEnd) {
		if (curObj->objSig == sig)
			break;
		curObj = (JDBDescObjPtr)((char *)curObj + curObj->objLen);
	}
	if (curObj >= dataEnd)
		return -1 /* attempt to delete a non-existant object */;
	
	nxtObj = (JDBDescObjPtr)((char *)curObj + curObj->objLen);
	fileDesc->freeBytes += curObj->objLen;
		
	// slide the rest of the data down
	if (nxtObj != dataEnd)
		JBlockMove(	(JPtr)nxtObj
			,		(JPtr)curObj
			,		(char *)nxtObj - (char *)dataEnd
			);
	
	return 0;
}

#pragma mark ---------- Debug
#if _JDBDebug
//
//	DebugSpace
//

void DebugSpace( JDBSpaceAllocHandle sph );

void DebugSpace( JDBSpaceAllocHandle sph )
{
	int					i
	;
	JDBSpaceAllocPtr	sp
	;
	
	JHLock( (JHandle)sph );
	sp = *sph;
	cout << "    transID  " << sp->transID << endl;
	for (i = 0; i < sp->len; i++)
		cout << "    table[" << i << "] (" << sp->table[i].loc 
			<< ',' << sp->table[i].size << ')' << endl;
	JHUnlock( (JHandle)sph );
	
}

//
//	JDB::DebugBAT
//

void JDB::DebugBAT( blockNumber id, int level )
{
	int					i
	;
	JDBStructMgmtPtr	cp
	;
	
	cout << "    [" << id << "]  ";
	cp = (*fileStructCache).Get( id );
	if (!cp) {
		cout << "Nil!" << endl;
		return;
	}
	if (level) {
		for (i = 0; i < kJDBDirTableSize; i++ ) {
			cout << " " << cp->cacheDir.table[i];
			if (cp->cacheDir.table[i] == 0) {
				cout << " ...";
				break;
			}
		}
		for (i = 0; i < kJDBDirTableSize; i++ )
			if (cp->cacheDir.table[i] == 0)
				break;
			else
				DebugBAT( cp->cacheDir.table[i], level - 1);
		cout << endl;
	} else {
		for (i = 0; i < kJDBBATTableSize; i++ ) {
			if (cp->cacheBAT.table[i].size == 0) {
				cout << " ...";
				break;
			}
			cout
				<< " (" << cp->cacheBAT.table[i].loc
				<< ',' << cp->cacheBAT.table[i].size
				<< ',' << cp->cacheBAT.table[i].transID
				<< ',' << cp->cacheBAT.table[i].bytesUsed
				<<	')';
		}
		cout << endl;
	}
	(*fileStructCache).Put( cp, id );
}

//
//	JDB::Debug
//

void JDB::Debug( void )
{
	JDBDataCacheSlotPtr		dcsp = fileDataCache
	;
	
//	cout << "----- Debug " << fileName << " -----" << endl;
	cout << "----- Debug -----" << endl;
	if (transInProgress)
		cout << "* transaction in progress" << endl;
	
	cout << "fileDesc" << endl;
	cout << "    transID			" << fileDesc->transID << endl;
	cout << "    structCacheSize	" << fileDesc->structCacheSize << endl;
	cout << "    dataCacheSize		" << fileDesc->dataCacheSize << endl;
	cout << "    dirLevels			" << fileDesc->dirLevels << endl;
	cout << "    dirLoc				" << fileDesc->dirLoc << endl;
	cout << "    nextBlock			" << fileDesc->nextBlock << endl;
	cout << "    freeList			(" << fileDesc->freeList.loc << ',' << fileDesc->freeList.size << ')' << endl;
	
	cout << "fileFree" << endl;
	DebugSpace( fileFree );
	cout << "fileReserved" << endl;
	DebugSpace( fileReserved );
	cout << "directory" << endl;
	DebugBAT( fileDesc->dirLoc, fileDesc->dirLevels - 1);
	
	cout << "data cache" << endl;
	for (int i = 0; i < fileDataSize; i++, dcsp++ )
		if (dcsp->dataID == -1)
			cout << "  [" << i << "] "
				<< (dcsp->dataModified ? '*' : ' ') << " unused"
				<< endl;
		else
			cout
				<< "  [" << i << "] "
				<< (dcsp->dataModified ? '*' : ' ') << ' '
				<< dcsp->dataID
				<< " @ (" << dcsp->dataAlloc.loc
				<< ',' << dcsp->dataAlloc.size
				<< ',' << dcsp->dataAlloc.transID
				<< ',' << dcsp->dataAlloc.bytesUsed
				<< ')' << endl;
}

#endif

#pragma mark ---------- Data Cache

//
//	JDBDataCacheSlot::JDBDataCacheSlot
//

JDBDataCacheSlot::JDBDataCacheSlot( void )
	: dataID(-1), dataAlloc(), dataModified(0)
	, dataAge(0), dataRef(0)
{
	dataData = (JHandle)JNewHandle( 0 );
}

//
//	JDBDataCacheSlot::~JDBDataCacheSlot
//

JDBDataCacheSlot::~JDBDataCacheSlot( void )
{
	JVBlockPtr	bp, np
	;
	
	// dereference any blocks (there shouldn't be any)
	for (bp = dataRef; bp; bp = np) {
		np = bp->blockRefNext;
		
		bp->blockCache = (JDBDataCacheSlotPtr)nil;
		*bp->blockUserPtr = (JPtr)nil;
		bp->blockRefNext = (JVBlockPtr)nil;
	}
	
	// toss the handle
	JDisposeHandle( (JHandle)dataData );
}

#pragma mark ---------- Virtual Blocks

//
//	JVBlock::JVBlock
//
//	Allocate a new block.  The next available block identifier is stored in the file 
//	descriptor.  When the management class does it's thing it will allocate directory 
//	and BAT records to make sure the correct BAT record is loaded.
//
//	The BAT location contains what will be the next available block number.  Store that
//	back into the file descriptor.
//
//	Getting free space is pretty simple.
//
//	When the mgmt object is destructed the changes to the BAT will be saved and if there
//	is a transaction in progress the BAT will be moved and the changes will move up the 
//	directory tree.
//

#if _JDBDebugJVBlockCount
int	gJVBlockCount;
#endif

JVBlock::JVBlock( JDBPtr jbf, JHandle usr, blockNumber id, int size )
	: blockFile( jbf ), blockUserPtr( usr ), blockID( id )
{
#if _JDBDebugJVBlock
		cout << "JVBlock bind: id = " << id << ", size = " << size << endl;
#endif
#if _JDBDebugJVBlockCount
		gJVBlockCount += 1;
#endif
	
	if (blockID < 0) {
		// get a free cache slot
		blockCache = jbf->GetFreeDataCacheSlot();
		
		JDBBlockMgmt	mgmt( jbf, jbf->fileDesc->nextBlock )
		;
		
		blockID = blockCache->dataID = blockFile->fileDesc->nextBlock;
		blockFile->fileDesc->nextBlock = mgmt.mgmtAlloc.loc;
		
		if (size == 0) {
			blockCache->dataAlloc.loc =  mgmt.mgmtAlloc.loc =  0;
			blockCache->dataAlloc.size = mgmt.mgmtAlloc.size = 0;
		} else {
			JDBSpaceRec		newSpace = blockFile->ExtractFreeSpace(
								((size % kBlockSize) == 0) ? size / kBlockSize : (size + kBlockSize) / kBlockSize
							)
			;
			
			blockCache->dataAlloc.loc =  mgmt.mgmtAlloc.loc =  newSpace.loc;
			blockCache->dataAlloc.size = mgmt.mgmtAlloc.size = newSpace.size;
		}
		blockCache->dataAlloc.transID = mgmt.mgmtAlloc.transID = blockFile->fileDesc->transID;
		blockCache->dataAlloc.bytesUsed = mgmt.mgmtAlloc.bytesUsed = size;
		
		// set the handle size to contain the data, lock it down
		JSetHandleSize( (JHandle)blockCache->dataData, size );
//		ThrowIfMemError_();
		JHLock( (JHandle)blockCache->dataData );
		
		// bind the user pointer to the data
		if (blockUserPtr)
			*blockUserPtr = *blockCache->dataData;
		
		// clear the data
		char *c = (char *)*blockCache->dataData;
		for (int i = 0; i < size; i++)
			*c++ = 0;
		
		// make this reference the first
		blockCache->dataRef = this;
		blockRefNext = (JVBlockPtr)nil;
		
		// this block is modified
		blockCache->dataModified = true;
	} else {
		// check for slot already available
		blockCache = jbf->GetDataCacheSlot( id );
		if (blockCache) {
			// lock the data, user not expecting it to move
			if (!blockCache->dataRef)
				JHLock( (JHandle)blockCache->dataData );
			
			// add this object to front of reference list
			blockRefNext = blockCache->dataRef;
			blockCache->dataRef = this;
			
			// bind the user pointer to the data
			if (blockUserPtr)
				*blockUserPtr = *blockCache->dataData;
		
			return;
		}
		
		// get a free cache slot
		blockCache = jbf->GetFreeDataCacheSlot();
//		if (!blockCache)
//			; // good time to throw an exception
		
		// build a block management object
		JDBBlockMgmt	mgmt( jbf, id )
		;
		
		// get a copy of the current location, size and transaction ID
		blockCache->dataID = id;
		blockCache->dataAlloc = mgmt.mgmtAlloc;
		
		// set the handle size to contain the data, lock it down
		JSetHandleSize( (JHandle)blockCache->dataData, blockCache->dataAlloc.bytesUsed );
		JHLock( (JHandle)blockCache->dataData );
		
		// bind the user pointer to the data
		if (blockUserPtr)
			*blockUserPtr = *blockCache->dataData;
		
		// read in the data
		blockFile->ReadBlockData( blockCache->dataID
			,	blockCache->dataAlloc
			,	blockCache->dataAlloc.bytesUsed
			,	*blockCache->dataData
			);
		
		// add this object to the reference list
		blockCache->dataRef = this;
		blockRefNext = (JVBlockPtr)nil;
		
		// unmodified block
		blockCache->dataModified = false;
	}
}

//
//	JVBlock::JVBlock
//
//	This constructor is used when a block is located at a known location that's not
//	going to move.  The caller knows the physical location and size, for example, the 
//	file header is a staticly located block at the front of the file.
//

JVBlock::JVBlock( JDBPtr jbf, JHandle usr, JDBBlockAllocRec loc )
	: blockFile( jbf ), blockUserPtr( usr ), blockID( -1 )
{
#if _JDBDebugJVBlockCount
	gJVBlockCount += 1;
#endif
	
	blockCache = jbf->GetFreeDataCacheSlot();
	
	blockCache->dataID = -2;
	blockCache->dataAlloc = loc;
	
	// set the handle size to contain the data, lock it down
	JSetHandleSize( (JHandle)blockCache->dataData, (loc.size * kBlockSize) );
	JHLock( (JHandle)blockCache->dataData );
	
	// bind the user pointer to the data
	if (blockUserPtr)
		*blockUserPtr = *blockCache->dataData;
	
	// read in the data
	blockFile->ReadBlockData( blockCache->dataID, loc, loc.size * kBlockSize, *blockCache->dataData );
	
	// add this block to the reference list
	blockCache->dataRef = this;
	blockRefNext = (JVBlockPtr)nil;
	
	blockCache->dataModified = false;
}

//
//	JVBlock::~JVBlock
//

JVBlock::~JVBlock( void )
{
	JVBlockPtr	*bp
	;
	
#if _JDBDebugJVBlock
	cout << "JVBlock unbind: id = " << blockID << endl;
#endif
#if _JDBDebugJVBlockCount
	gJVBlockCount -= 1;
#endif
	
	if (blockCache) {
		// find this object in the list of references
		bp = &blockCache->dataRef;
		while (*bp && (*bp != this))
			bp = &(*bp)->blockRefNext;
		
		// not found is a serious error
		ThrowIfNil_( *bp );
		
		// remove this object from the list
		*bp = (*bp)->blockRefNext;
		
		// unlock the block if there are no references
		if (!blockCache->dataRef)
			JHUnlock( (JHandle)blockCache->dataData );
	}
}

//
//	JVBlock::Modified
//
//	Mark the block modified.  It doesn't need to get written until the Write() function 
//	is called (and Write() is called by the destructor iff this block is modified).
//

void JVBlock::Modified( void )
{
	blockCache->dataModified = true;
}

//
//	JVBlock::isModified
//
//	Return a flag if this block has been modified and not saved.
//

Boolean JVBlock::isModified( void )
{
	return blockCache->dataModified;
}

//
//	JVBlock::GetSize
//
//	Return the current block size.  Normally the block size is maintained in other data 
//	in the block and the actual size isn't necessary.
//

int JVBlock::GetSize( void )
{
	return blockCache->dataAlloc.bytesUsed;
}

//
//	JVBlock::SetSize
//

void JVBlock::SetSize( int newSize )
{
	// verify this is a bound object
	if (!blockCache)
		return;
	
	// verify this is not an "absolute location" block
	if (blockCache->dataID == -2)
		return;
	
	// see if the size really changed
	if (newSize == blockCache->dataAlloc.bytesUsed)
		return;
	
	// need a management object at this point
	JDBBlockMgmt	mgmt( blockFile, blockCache->dataID )
	;
	
#if _JDBDebugDataCache
		cout << blockCache->dataID;
#endif
	if (blockFile->transInProgress && (blockCache->dataAlloc.transID != blockFile->fileDesc->transID)) {
#if _JDBDebugDataCache
		cout << " reserved " << mgmt.mgmtAlloc.loc << ':' << mgmt.mgmtAlloc.size;
#endif
		blockFile->PutSpace( blockFile->fileReserved, mgmt.mgmtAlloc );
	} else {
#if _JDBDebugDataCache
		cout << " free " << mgmt.mgmtAlloc.loc << ':' << mgmt.mgmtAlloc.size;
#endif
		blockFile->PutSpace( blockFile->fileFree, mgmt.mgmtAlloc );
	}
	
	// update the transaction number
	blockCache->dataAlloc.transID = blockFile->fileDesc->transID;
	
	// get the new location
	JDBSpaceRec		newSpace = blockFile->ExtractFreeSpace( (newSize + kBlockSize) / kBlockSize )
	;
	
	// update the cache location and the management object location
	blockCache->dataAlloc.loc = mgmt.mgmtAlloc.loc = newSpace.loc;
	blockCache->dataAlloc.size = mgmt.mgmtAlloc.size = newSpace.size;
	blockCache->dataAlloc.transID = mgmt.mgmtAlloc.transID = blockFile->fileDesc->transID;
	blockCache->dataAlloc.bytesUsed = mgmt.mgmtAlloc.bytesUsed = newSize;
	blockCache->dataModified = true;
	
#if _JDBDebugDataCache
	cout << " moved to " << newSpace.loc << ':' << newSpace.size << endl;
#endif
	
	// unlock the handle so it can move, change the size, lock it back down
	JHUnlock( (JHandle)blockCache->dataData );
	JSetHandleSize( (JHandle)blockCache->dataData, newSize );
	JHLock( (JHandle)blockCache->dataData );
	
	// update all of the user pointers to the data
	for (JVBlockPtr bp = blockCache->dataRef; bp; bp = bp->blockRefNext)
		*bp->blockUserPtr = *blockCache->dataData;
}

#pragma mark ---------- Structure Management

//
//	JDBStructMgmt::JDBStructMgmt
//

JDBStructMgmt::JDBStructMgmt( JDBPtr fp )
{
	JDBSpaceRec newLoc = fp->ExtractFreeSpace( 1 )
	;
	
	cacheFile = fp;
	cacheBlock = newLoc.loc;
	cacheModified = true;
}

//
//	JDBStructMgmt::JDBStructMgmt
//

JDBStructMgmt::JDBStructMgmt( JDBPtr fp, blockNumber blk )
{
	cacheFile = fp;
	cacheBlock = blk;
	cacheModified = false;
	
	cacheFile->Read( cacheBlock * kBlockSize, kJDBBATRecSize, &cacheBAT );
}

//
//	JDBStructMgmt::~JDBStructMgmt
//

JDBStructMgmt::~JDBStructMgmt( void )
{
	if (cacheModified)
		Flush();
}

//
//	JDBStructMgmt::Flush
//

void JDBStructMgmt::Flush( void )
{
	if (!cacheModified)
		return;
	
	cacheFile->Write( cacheBlock * kBlockSize, kJDBBATRecSize, &cacheBAT );
	cacheModified = false;
}

//
//	JDBStructMgmt::InitDirRec
//

void JDBStructMgmt::InitDirRec( void )
{
	int		i
	;
	
	cacheDir.transID = cacheFile->fileDesc->transID;
	for (i = 0; i < kJDBDirTableSize; i++)
		cacheDir.table[i] = 0;
	cacheModified = true;
}

//
//	JDBStructMgmt::InitBATRec
//

void JDBStructMgmt::InitBATRec( blockNumber start )
{
	int		i
	;
	
	cacheBAT.transID = cacheFile->fileDesc->transID;
	for (i = 0; i < kJDBBATTableSize; i++) {
		cacheBAT.table[i].loc = ++start;
		cacheBAT.table[i].size = 0;
		cacheBAT.table[i].transID = 0;
	}
}

#pragma mark ---------- Structure Cache

//
//	JDBStructCache::JDBStructCache
//

JDBStructCache::JDBStructCache( int sz )
{
	cacheSize = sz;
	ThrowIfMemFail_( cacheData = new cacheSlot[sz] );
	
	for (int i = 0; i < cacheSize; i++) {
		cacheData[i].id = kNotABlock;
		cacheData[i].elem = 0;
	}
}

//
//	JDBStructCache::~JDBStructCache
//

JDBStructCache::~JDBStructCache( void )
{
	for (int i = 0; i < cacheSize; i++)
		if (cacheData[i].elem)
			delete cacheData[i].elem;
	
	delete[] cacheData;
}

//
//	JDBStructCache::Get
//

JDBStructMgmtPtr JDBStructCache::Get( blockNumber id )
{
	JDBStructMgmtPtr	rslt
	;
	
	for (int i = 0; i < cacheSize; i++)
		if (cacheData[i].id == id) {
			rslt = cacheData[i].elem;
			for (int j = i; j < cacheSize-1; j++)
				cacheData[j] = cacheData[j + 1];
			
			cacheData[cacheSize-1].id = kNotABlock;
			cacheData[cacheSize-1].elem = 0;
			
			return rslt;
		}
	
	return (JDBStructMgmtPtr)nil;
}

//
//	JDBStructCache::Put
//

void JDBStructCache::Put( JDBStructMgmtPtr elem, blockNumber id )
{
	if (cacheData[cacheSize-1].elem)
		delete cacheData[cacheSize-1].elem;
	
	for (int i = cacheSize-1; i > 0; i -= 1)
		cacheData[i] = cacheData[i - 1];
	
	cacheData[0].id = id;
	cacheData[0].elem = elem;
}

//
//	JDBStructCache::Remove
//

void JDBStructCache::Remove( blockNumber id )
{
	JDBStructMgmtPtr	temp
	;
	
	temp = Get( id );
	if (temp)
		delete temp;
}

//
//	JDBStructCache::Flush
//

void JDBStructCache::Flush( void )
{
	for (int i = 0; i < cacheSize; i++)
		if (cacheData[i].elem)
			cacheData[i].elem->Flush();
}

#pragma mark ---------- Block Management

//
//	JDBBlockMgmt::JDBBlockMgmt
//

JDBBlockMgmt::JDBBlockMgmt( JDBPtr fp, blockNumber blk )
{
	int					tblSize, i
	;
	blockNumber			b
	;
	JDBStructMgmtPtr	cp
	;
	
	mgmtFile = fp;
	
	b = blk;
	tblSize = kJDBBATTableSize;
	for (i = 0; i < fp->fileDesc->dirLevels; i++ ) {
		mgmtTable[i].offset = b % tblSize;
		b /= tblSize;
		tblSize = kJDBDirTableSize;
	}
	
	//
	// If (b != 0) then the tree isn't deep enough.  Allocate a new directory 
	// record, it will be the top of the tree during the destructor.  The first 
	// element should point to the current top, make sure that gets saved.
	//
	if (b != 0) {
		mgmtTable[i].offset = (short)b;
		fp->fileDesc->dirLevels += 1;
		ThrowIfMemFail_( cp = new JDBStructMgmt( fp ) );
		cp->InitDirRec();
		cp->cacheDir.table[0] = fp->fileDesc->dirLoc;
		goto update;
	} else
		b = fp->fileDesc->dirLoc;
	
	//
	//	Move back down the list, which is also down the tree towards the BAT, allocating
	//	directory records and BAT's as necessary.
	//
again:
	i -= 1;
	if (b) {
		cp = (*fp->fileStructCache).Get( b );
		if (!cp)
			ThrowIfMemFail_( cp = new JDBStructMgmt( fp, b ) );
	} else {
		ThrowIfMemFail_( cp = new JDBStructMgmt( fp ) );
		if (i == 0)
			cp->InitBATRec( blk );
		else
			cp->InitDirRec();
	}
	
	//
	//	Save the cache pointer in the management list and keep going.
	//
update:
	mgmtTable[i].cp = cp;
	if (i != 0) {
		b = cp->cacheDir.table[ mgmtTable[i].offset ];
		goto again;
	}
	
	mgmtAlloc = cp->cacheBAT.table[ mgmtTable[i].offset ];
}

//
//	JDBBlockMgmt::~JDBBlockMgmt
//

JDBBlockMgmt::~JDBBlockMgmt( void )
{
	int					i
	;
	blockNumber			b, *blkp
	;
	JDBStructMgmtPtr	cp
	;
	JDBBlockAllocPtr	bap
	;
	
	i = 0;
	
	// See if the BAT record changed.
	cp = mgmtTable[i].cp;
	bap = &cp->cacheBAT.table[ mgmtTable[i].offset ];
	if ((mgmtAlloc.loc != bap->loc) || (mgmtAlloc.size != bap->size)
		|| (mgmtAlloc.transID != bap->transID)
		|| (mgmtAlloc.bytesUsed != bap->bytesUsed)
		) {
		*bap = mgmtAlloc;
		goto update;
	}
	
	// This record needs to be put back into the cache
again:
	b = cp->cacheBlock;
	(*mgmtFile->fileStructCache).Put( cp, b );
	i += 1;
	if (i == mgmtFile->fileDesc->dirLevels) {
		if (mgmtFile->fileDesc->dirLoc != b) {
#if _JDBDebugBlockMgmt
			cout << "structure tree top moved to " << b << endl;
#endif
			mgmtFile->fileDesc->dirLoc = b;
		}
		return;
	}
	
	// See if the directory needs to be updated
	cp = mgmtTable[i].cp;
	blkp = &cp->cacheDir.table[ mgmtTable[i].offset ];
	if (b != *blkp) {
		*blkp = b;
		goto update;
	}
	goto again;
	
	// Something about this record has changed
update:
	cp->cacheModified = 1;
	if (mgmtFile->transInProgress && (cp->cacheBAT.transID != mgmtFile->fileDesc->transID)) {
		JDBSpaceRec oldLoc
		;
		
		oldLoc.loc = cp->cacheBlock;
		oldLoc.size = 1;

		JDBSpaceRec newLoc = mgmtFile->ExtractFreeSpace( 1 )
		;
		
#if _JDBDebugBlockMgmt
		cout << "structure reserved " << oldLoc.loc << ':' << oldLoc.size;
#endif
		mgmtFile->PutSpace( mgmtFile->fileReserved, oldLoc );
		cp->cacheBlock = newLoc.loc;
#if _JDBDebugBlockMgmt
		cout << ", moved to " << newLoc.loc << ':' << newLoc.size << endl;
#endif
	}
	cp->cacheBAT.transID = mgmtFile->fileDesc->transID;
	goto again;
}
