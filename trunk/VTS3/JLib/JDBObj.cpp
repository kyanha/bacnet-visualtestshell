
#include "stdafx.h"

#include <stdlib.h>
#if UNIX_Platform
#include <string.h>
#endif

#include "JConfig.hpp"
#include "JError.hpp"
#include "JMemory.hpp"
#include "JDBObj.hpp"

#define _JDBDebugRefs		0
#define _JDBDebugDump		0

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JDBObjMgr::JDBObjMgr
//

JDBObjMgr::JDBObjMgr( JDBPtr jbfp )
	: mgrFile( jbfp )
{
	mgrDesc = (JDBObjMgrDescPtr)mgrFile->GetDesc( 0 );
	if (!mgrDesc) {
		mgrDesc = (JDBObjMgrDescPtr)mgrFile->AllocDesc( 0, kJDBObjMgrDescSize );
		mgrDesc->descUsedBlock = mgrFile->NewBlock( 0 );
		mgrDesc->descAllocSize = kJDBDefaultObjBlockAllocSize;
	}
}

//
//	JDBObjMgr::~JDBObjMgr
//

JDBObjMgr::~JDBObjMgr( void )
{
}

//
//	JDBObjMgr::LocateObject
//

JDBObjPtr JDBObjMgr::LocateObject( objId id, JDBObjBlockPtr obp, int blkSize )
{
	int			stat
	;
	JDBObjPtr	cur
	,			limit
	;
	
	cur = (JDBObjPtr)obp->blockData;
	limit = (JDBObjPtr)((char *)obp + blkSize - obp->blockFree);
	while (cur < limit)
		if (cur->objID == objOfId(id))
			goto found;
		else
		if (!cur->objSize) {
			stat = -1 /* corrupted block */;
			break;
		} else
			cur = (JDBObjPtr)(((char *)cur) + cur->objSize);
	
	cur = 0 /* object not found */;
found:
	return cur;
}

//
//	JDBObjMgr::GetBlockFree
//

int JDBObjMgr::GetBlockFree( blockNumber &blck, int size )
{
	int				i, usedLen, allocSize
	;
	JDBObjUsedPtr	used
	;
	JVBlock			usedBlock( mgrFile, (JHandle)&used, mgrDesc->descUsedBlock )
	;
	JVBlockPtr		bp
	;
	JDBObjBlockPtr	obp
	;
	
	// error checking
	ThrowIf_( size < 0 );
	
	// look for a block with enough space (first fit)
	usedLen = usedBlock.GetSize() / kJDBObjUsedSize;
	for (i = 0; i < usedLen; i++)
		if (used[i].usedFree >= size)
			goto okBlock;
	
	// make a place for a new block up front
	usedBlock.SetSize( (usedLen + 1) * kJDBObjUsedSize );
#if Mac_Platform
	BlockMove( (char *)used, ((char *)used) + kJDBObjUsedSize, usedLen * kJDBObjUsedSize );
#endif
#if UNIX_Platform
	memmove( ((char *)used) + kJDBObjUsedSize, (char *)used, usedLen * kJDBObjUsedSize );
#endif
	usedBlock.Modified();
	i = 0;
	
	// allocate a block big enough
	allocSize = mgrDesc->descAllocSize;
	while (size > (allocSize - kJDBObjBlockSize))
		allocSize += kBlockSize;
	
	// allocate a new block
	ThrowIfMemFail_( bp = new JVBlock( mgrFile, (JHandle)&obp, -1, allocSize ) );
	
	obp->blockFree = allocSize - kJDBObjBlockSize;
	obp->blockID = 0;
	obp->blockFilled = 0;
	
	used[i].usedBlock = bp->blockID;
	used[i].usedFree = obp->blockFree;
	
	bp->Modified();
	delete bp;
	
okBlock:
	blck = used[i].usedBlock;
	return 0;
}

//
//	JDBObjMgr::SetBlockFree
//

int JDBObjMgr::SetBlockFree( blockNumber blck, int size )
{
	int				i, usedLen
	;
	JDBObjUsedPtr	used
	;
	JVBlock			usedBlock( mgrFile, (JHandle)&used, mgrDesc->descUsedBlock )
	;
	
	// look for the block in the list
	usedLen = usedBlock.GetSize() / kJDBObjUsedSize;
	for (i = 0; i < usedLen; i++)
		if (used[i].usedBlock == blck) {
			used[i].usedFree = size;
			usedBlock.Modified();
			break;
		}
	
	// might be nice to keep it sorted in decreasing order by size
	
	return 0;
}

//
//	JDBObjMgr::NewObject
//

int JDBObjMgr::NewObject( objId *id, JDBObjPtr op, int size )
{
	int				stat, i
	;
	long			theSize
	;
	blockNumber		blck
	;
	JVBlockPtr		bp
	;
	JDBObjBlockPtr	obp
	;
	JDBObjPtr		cur, temp
	;
	
	// make sure the object is a valid size
	ThrowIf_( (size & 1) != 0 );
	ThrowIf_( ((size < kMinObjSize) || (size > kMaxObjSize)) );
	
teletubbies:
	// search for a good block to use
	if ((stat = GetBlockFree( blck, size )) != 0)
		return stat;
	
	// bind to the block that was found
	ThrowIfMemFail_( bp = new JVBlock( mgrFile, (JHandle)&obp, blck ) );
	
	// point to the free space at the end of the block
	cur = (JDBObjPtr)((char *)obp + bp->GetSize() - obp->blockFree);
	
	// give it an ID
	if (obp->blockID != 255) {
		cur->objID = obp->blockID;
		obp->blockID += 1;
	} else {
		temp = (JDBObjPtr)obp->blockData;
		for (i = 0; i < 256; i++ ) {
			ThrowIf_( temp > cur );		// corrupted block
			
			if (temp == cur) {
				if (i < 254)
					obp->blockID = i + 1;
				cur->objID = i;
				break;
			} else
			if (temp->objID != i) {
				theSize = ((char *)cur) - ((char *)temp);
#if Mac_Platform
				BlockMove( temp, ((char *)temp) + size, theSize );
#endif
#if UNIX_Platform
				memmove( ((char *)temp) + size, temp, theSize );
#endif
				cur = temp;
				cur->objID = i;
				break;
			} else
				temp = (JDBObjPtr)(((char *)temp) + temp->objSize);
		}
		
		// check for full block
		if (i == 256) {
			// mark full
			obp->blockFilled = 1;
			
			// set it to skip until an object is deleted
			if ((stat = SetBlockFree( blck, 0 )) != 0)
				return stat;
			
			// release the binding and try again, again!
			delete bp;
			goto teletubbies;
		}
	}
	
	// return the ID
	*id = makeId( blck, cur->objID );
	
	// set the size
	cur->objSize = size;
	cur->objType = 0;
	JBlockMove( (JPtr)cur, (JPtr)op, kJDBObjSize );
	
	// remove the free space from the block
	obp->blockFree -= size;
	
	// mark the block modified
	bp->Modified();
	
	// save the new amount of free space in the 'used' list
	if (!obp->blockFilled && ((stat = SetBlockFree( blck, obp->blockFree )) != 0))
		return stat;
	
	// release the binding
	delete bp;
	
	// success
	return 0;
}

//
//	JDBObjMgr::DeleteObject
//

int JDBObjMgr::DeleteObject( objId id )
{
	int			stat
	;
	long		size, theSize
	;
	JVBlockPtr		bp
	;
	JDBObjBlockPtr	obp
	;
	JDBObjPtr		cur, limit
	;
	
again:
	// bind to the block
	ThrowIfMemFail_( bp = new JVBlock( mgrFile, (JHandle)&obp, blockOfId(id) ) );
	
	cur = LocateObject( id, obp, bp->GetSize() );
	if (!cur)
		return -3 /* object not found */;
	
	if (cur->objType == JDBRefType)
		id = ((JDBRefPtr)cur)->refObjID;
	else
		id = 0;
	
	limit = (JDBObjPtr)((char *)obp + bp->GetSize() - obp->blockFree);
	
	size = cur->objSize;
	theSize = (((char *)limit) - ((char *)cur)) - size;
	if (theSize == 0)
		obp->blockID = cur->objID;
	else
#if Mac_Platform
		BlockMove( ((char *)cur) + size, cur, theSize );
#endif
#if UNIX_Platform
		memmove( cur, ((char *)cur) + size, theSize );
#endif
	
	// add the free space to the block
	obp->blockFree += size;
	
	// no longer filled
	obp->blockFilled = 0;
	
	// mark the block modified
	bp->Modified();
	
	// save the new amount of free space in the 'used' list
	if ((stat = SetBlockFree( bp->blockID, obp->blockFree )) != 0)
		return stat;
	
	// release the binding
	delete bp;
	
	if (id)
		goto again;
	
	return 0;
}

//
//	JDBObjMgr::ReadObject
//

int JDBObjMgr::ReadObject( objId id, JDBObjPtr op )
{
	JVBlockPtr		bp
	;
	JDBObjBlockPtr	obp
	;
	JDBObjPtr		cur
	;
	
again:
	// bind to the block
	ThrowIfMemFail_( bp = new JVBlock( mgrFile, (JHandle)&obp, blockOfId(id) ) );
	
	// locate the object
	cur = LocateObject( id, obp, bp->GetSize() );
	if (!cur)
		return -5 /* object not found */;
	
	if (cur->objType == JDBRefType) {
#if _JDBDebugRefs
		char msg[40];
		sprintf( msg, "(%08X refs to %08X) ", id, ((JDBRefPtr)cur)->refObjID );
		cout << msg;
#endif
		id = ((JDBRefPtr)cur)->refObjID;
		delete bp;
		goto again;
	}
	
	JBlockMove( (JPtr)cur, (JPtr)op, cur->objSize );
	op->objID = (unsigned char)objOfId( id );
	
	delete bp;
	return 0;
}

//
//	JDBObjMgr::WriteObject
//

int JDBObjMgr::WriteObject( objId id, JDBObjPtr op )
{
	int					stat
	,					delta
	;
	objId				newID
	;
	long				theSize
	;
	JVBlockPtr			objBlock, refBlock
	;
	JDBObjBlockPtr		objbp, refbp
	;
	JDBObjPtr			cur, limit
	;
	JDBRefPtr			refcur
	;
	
	refBlock = 0;
	refcur = 0;
	
	// make sure the object has a valid size
	ThrowIf_( (op->objSize & 1) != 0 );
	ThrowIf_( ((op->objSize < kMinObjSize) || (op->objSize > kMaxObjSize)) );
	
	// bind the block with the object
	ThrowIfMemFail_( objBlock = new JVBlock( mgrFile, (JHandle)&objbp, blockOfId(id) ) );
	
	// get a pointer to the object
	cur = LocateObject( id, objbp, objBlock->GetSize() );
	if (!cur)
		return -7 /* object not found */;
	
	// see if it's a reference
	if (cur->objType == JDBRefType) {
		refcur = (JDBRefPtr)cur;
		if (op->objSize <= (objbp->blockFree + kJDBRefSize)) {
			/*
				There's enough space in this block if we replace the reference
				object.   The "real" object it refers to needs to be freed.
			*/
#if _JDBDebugRefs
			char msg1[40];
			sprintf( msg1, "(%08X deleting ref to %08X)", id, refcur->refObjID );
			cout << msg1 << endl;
#endif
			if ((stat = DeleteObject( refcur->refObjID )) != 0)
				return stat /* problem deleting old object */;
		} else {
			/*
				We need more space so chase down the reference.  The current 
				block must be locked because we are going to keep our pointer
				to the reference object and update it later.
			*/
			refBlock = objBlock;
			refbp = objbp;
			
			ThrowIfMemFail_( objBlock = new JVBlock( mgrFile, (JHandle)&objbp, blockOfId(refcur->refObjID) ) );
			
			cur = LocateObject( refcur->refObjID, objbp, objBlock->GetSize() );
			if (!cur)
				return -10 /* object not found */;
		}
	}
	
	delta = op->objSize - cur->objSize;		// positive for needing bytes
	limit = (JDBObjPtr)((char *)objbp + objBlock->GetSize() - objbp->blockFree);
	
	if (delta <= objbp->blockFree) {		// we need LESS than what's available
		if (delta != 0) {
			theSize = ( (char *)limit - (char *)cur ) - cur->objSize;
#if Mac_Platform
			BlockMove(	((char *)cur) + cur->objSize
				,		((char *)cur) + op->objSize
				,		theSize
				);
#endif
#if UNIX_Platform
			memmove(	((char *)cur) + op->objSize
				,		((char *)cur) + cur->objSize
				,		theSize
				);
#endif
			objbp->blockFree -= delta;
			
			if (!objbp->blockFilled)
				if ((stat = SetBlockFree( objBlock->blockID, objbp->blockFree )) != 0)
					return stat;
		}
	} else {								// we need MORE than what's available
		// attempt to create a space
		char *buff
		;
		
		ThrowIfMemFail_( buff = new char[op->objSize] );
		stat = NewObject( &newID, (JDBObjPtr)buff, op->objSize );
		delete[] buff;
		
		// see if it worked
		if (stat)
			return stat /* problems allocating a new object */;
		
		// may already have a reference
		if (refcur) {
#if _JDBDebugRefs
			char msg2[40];
			sprintf( msg2, "(%08X delete old ref to %08X)", id, refcur->refObjID );
			cout << msg2 << endl;
#endif
			if ((stat = DeleteObject( refcur->refObjID )) != 0)
				return stat /* problem freeing old object */;
		} else {
			// check for space
			refcur = (JDBRefPtr)cur;
			if ((kJDBRefSize - refcur->objSize) > objbp->blockFree)
				return -11 /* can't expand to create reference object */;
			
			// slide everything else down
			theSize = (((char *)limit) - ((char *)refcur)) - refcur->objSize;
#if Mac_Platform
			BlockMove(	((char *)refcur) + refcur->objSize
				,		((char *)refcur) + kJDBRefSize
				,		theSize
				);
#endif
#if UNIX_Platform
			memmove(	((char *)refcur) + kJDBRefSize
				,		((char *)refcur) + refcur->objSize
				,		theSize
				);
#endif
			
			// update the free space
			objbp->blockFree += (refcur->objSize - kJDBRefSize);
			if (!objbp->blockFilled)
				if ((stat = SetBlockFree( objBlock->blockID, objbp->blockFree )) != 0)
					return stat;
			
			// the object becomes a reference
			refcur->objSize = kJDBRefSize;
			refcur->objType = JDBRefType;
		}
		
#if _JDBDebugRefs
		char msg3[40];
		sprintf( msg3, "(%08X new ref to %08X)", id, newID );
		cout << msg3 << endl;
#endif
		// mark the reference object block modified
		objBlock->Modified();
		
		// release the old binding
		delete objBlock;
		
		// bind to the block with the new space
		ThrowIfMemFail_( objBlock = new JVBlock( mgrFile, (JHandle)&objbp, blockOfId(newID) ) );
		
		cur = LocateObject( newID, objbp, objBlock->GetSize() );
		if (!cur)
			return -13 /* object not found */;
		
		refcur->refObjID = newID;
		if (refBlock)
			refBlock->Modified();
	}
	
	// save the ID back into the source
	op->objID = cur->objID;
	
	// save the data
	JBlockMove( (JPtr)op, (JPtr)cur, op->objSize );
	
	// mark the block modified
	objBlock->Modified();
	
	// release the block references
	delete objBlock;
	if (refBlock)			// ### May have bug here - see 9/17/00 notes
		delete refBlock;
	
	// success
	return 0;
}

//
//	JDBObjMgr::NewObject
//

JDBObjPtr JDBObjMgr::NewObject( objId *id, int size )
{
	JDBObjPtr	op
	;
	
	ThrowIf_( (size & 1) != 0 );
	ThrowIfMemFail_( op = (JDBObjPtr)(new char[ size ]) );
	ThrowIfError_( NewObject( id, op, size ) );
	
	return op;
}

//
//	JDBObjMgr::GetObject
//

JDBObjPtr JDBObjMgr::GetObject( objId id, int pad )
{
	char			*buff = 0
	;
	JVBlockPtr		bp
	;
	JDBObjBlockPtr	obp
	;
	JDBObjPtr		cur
	;
	
	ThrowIf_( pad < 0 );		// make sure there is at least enough space
	
again:
	// bind to the block
	ThrowIfMemFail_( bp = new JVBlock( mgrFile, (JHandle)&obp, blockOfId(id) ) );
	
	// locate the object
	cur = LocateObject( id, obp, bp->GetSize() );
	if (!cur)
		goto error /* object not found */;
	
	if (cur->objType == JDBRefType) {
#if _JDBDebugRefs
		char msg[40];
		sprintf( msg, "(%08X refs to %08X) ", id, ((JDBRefPtr)cur)->refObjID );
		cout << msg;
#endif
		id = ((JDBRefPtr)cur)->refObjID;
		delete bp;
		goto again;
	}
	
	// allocate a buffer for the data
	ThrowIfMemFail_( buff = new char[ cur->objSize + pad ] );
	
	// copy the data
	JBlockMove( (JPtr)cur, (JPtr)buff, cur->objSize );
	
error:
	// unbind
	delete bp;
	
	// return result
	return (JDBObjPtr)buff;
}

//
//	JDBObjMgr::FreeObject
//

void JDBObjMgr::FreeObject( JDBObjPtr op )
{
	delete[] (char *)op;
}

//
//	JDBObjMgr::ResizeObject
//

void JDBObjMgr::ResizeObject( JDBObjPtr *op, int size )
{
	char		*buff
	;
	
	// make sure the request is for a valid size
	ThrowIf_( (size & 1) != 0 );
	ThrowIf_( ((size < kMinObjSize) || (size > kMaxObjSize)) );
	
	// allocate a new buffer
	ThrowIfMemFail_( buff = new char[ size ] );
	
	// copy the current data
	JBlockMove( (JPtr)*op, (JPtr)buff, size );
	
	// delete the old pointer
	delete[] (char *)*op;
	
	// set the new one
	*op = (JDBObjPtr)buff;
	(*op)->objSize = size;
}

//
//	JDBObjMgr::Debug
//

#if _JDBObjDebug

void JDBObjMgr::Debug( void )
{
	char				buff[64]
	;
	JDBDataCacheSlotPtr	dcsp = mgrFile->fileDataCache
	;
	JDBObjBlockPtr		obp
	;
	JDBObjPtr			cur
	,					limit
	;
	
	cout << "----- Dump -----" << endl;
	for (int i = 0; i < mgrFile->fileDataSize; i++, dcsp++ ) {
		if (dcsp->dataID == -1) {
			cout << "  [" << i << "] "
				<< (dcsp->dataModified ? '*' : ' ') << " unused"
				<< endl;
			continue;
		} else
			cout
				<< "  [" << i << "] "
				<< (dcsp->dataModified ? '*' : ' ') << ' '
				<< dcsp->dataID
				<< " @ (" << dcsp->dataAlloc.loc
				<< ',' << dcsp->dataAlloc.size
				<< ',' << dcsp->dataAlloc.transID
				<< ',' << dcsp->dataAlloc.bytesUsed
				<< ')';
		
		obp = (JDBObjBlockPtr)*dcsp->dataData;
		cout << '(' << (int)obp->blockID << ',' << obp->blockFree << ") ";
		if (obp->blockFree > dcsp->dataAlloc.bytesUsed) {
			cout << "??\n";
			continue;
		}
		
		cur = (JDBObjPtr)obp->blockData;
		limit = (JDBObjPtr)((char *)obp + dcsp->dataAlloc.bytesUsed - obp->blockFree);
		while (cur < limit) {
			sprintf( buff, "[%d:%02X", cur->objSize, cur->objID
				);
			cout << buff;
			if (cur->objType == JDBRefType) {
				sprintf( buff, " %08X", ((JDBRefPtr)cur)->refObjID
					);
				cout << buff;
			}
			cout << "] ";
			if (!cur->objSize) {
				cout << "...";
				break;
			}
			cur = (JDBObjPtr)(((char *)cur) + cur->objSize);
		}
		cout << endl;
	}
	cout << endl;
#if 0
	fprintf( stdout, "Block    L M Data\n" );
	for (int i = 0; i < mgrFile->cacheSize; i++) {
		fprintf( stdout, "%08X %d %d "
			, mgrFile->cacheList[i].cBlock
			, mgrFile->cacheList[i].cLocked
			, mgrFile->cacheList[i].cModified
			);
		if (mgrFile->cacheList[i].cBlock < 0) {
			fprintf( stdout, "...\n" );
			break;
		}
		
	}
	fprintf( stdout, "\n" );
#endif
}
#endif
