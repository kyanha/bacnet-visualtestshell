
#include "stdafx.h"

#include "JConfig.hpp"
#include "JError.hpp"
#include "JMemory.hpp"

#include <stdlib.h>
#if UNIX_Platform
#include <string.h>
#endif

#include "JDBKey.hpp"

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JDBKeyMgr::JDBKeyMgr
//

JDBKeyMgr::JDBKeyMgr( JDBObjMgrPtr omp )
	: mgrObjMgr(omp), mgrFile( omp->mgrFile )
{
}

//
//	JDBKeyMgr::~JDBKeyMgr
//

JDBKeyMgr::~JDBKeyMgr(void)
{
}

//
//	JDBKeyMgr::NewKey
//

int JDBKeyMgr::NewKey(JDBKey &key, int L0BF, int L1BF)
{
	int				stat
	;
	objId			id
	;
	
	key.keyMgr = this;
	
	// allocate an array descriptor
	if ((stat = mgrObjMgr->NewObject( &id, &key.keyDesc, kJDBKeyDescSize )) != 0)
		return stat;
	
	// initialize it
	key.objID = id;
	key.keyDesc.objType = JDBKeyDescType;
	key.keyDesc.keyOptions = 0;
	key.keyDesc.keyCount = 0;
	key.keyDesc.keyL0BF = L0BF;
	key.keyDesc.keyL1BF = L1BF;
	
	// allocate a top level block
	key.keyDesc.keyDepth = 1;
	key.keyDesc.keyRootID = mgrFile->NewBlock( kJDBKeyBlockSize + L0BF * kJDBKeyRecordSize );
	
	// save the descriptor
	if ((stat = mgrObjMgr->WriteObject( id, &key.keyDesc )) != 0)
		return stat;
	
	return 0;
}

//
//	JDBKeyMgr::GetKey
//

int JDBKeyMgr::GetKey(JDBKey &key, objId id)
{
	int				stat
	;
	
	key.objID = id;
	key.keyMgr = this;
	
	// get the array descriptor
	if ((stat = mgrObjMgr->ReadObject( id, &key.keyDesc )) != 0)
		return stat;
	
	return 0;
}
		
//
//	JDBKeyMgr::DeleteKey
//

int JDBKeyMgr::DeleteKey(objId id)
{
	int				stat
	;
	
	// should delete the block tree here
	
	// delete the array descriptor
	if ((stat = mgrObjMgr->DeleteObject( id )) != 0)
		return stat;
	
	return 0;
}

//
//	JDBKey::JDBKey
//

JDBKey::JDBKey( JDBKeyCompFn fn )
	: keyMgr(0), objID(0), keyComp(fn)
{
}

//
//	JDBKey::~JDBKey
//

JDBKey::~JDBKey(void)
{
}

//
//	JDBKey::LoadStack
//

void JDBKey::LoadStack(long key)
{
	int				curLevel, offset
	,				keyStackSize
	;
	blockNumber		blck
	;
	JVBlockPtr		bp
	;
	JDBKeyBlockPtr	kbp
	;
	
	keyStackSize = keyDesc.keyDepth;
	ThrowIfMemFail_( keyStack = new keyStackElem[ keyDesc.keyDepth ] );
	
	// init empty stack
	blck = keyDesc.keyRootID;
	for (curLevel = keyStackSize - 1; curLevel >= 0; curLevel--) {
		keyStack[curLevel].stkBlock = blck;
		ThrowIfMemFail_( bp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, blck ) );
		
		for (offset = 0; offset < kbp->kblkCount; offset++)
			if ((*keyComp)(key,kbp->kblkKey[offset].key) <= 0)	// (key <= kBlock->kblkKey[offset].key)
				break;
		if (offset < kbp->kblkCount)
			blck = kbp->kblkKey[offset].data;
		else
			blck = kbp->kblkKey[offset-1].data;
		
		keyStack[curLevel].stkIndex = offset;
		
		delete bp;
	}
}

//
//	JDBKey::LoadStackZero
//

void JDBKey::LoadStackZero(void)
{
	int				curLevel
	,				keyStackSize
	;
	blockNumber		blck
	;
	JVBlockPtr		vkbp
	;
	JDBKeyBlockPtr	kbp
	;
	
	keyStackSize = keyDesc.keyDepth;
	ThrowIfMemFail_( keyStack = new keyStackElem[ keyDesc.keyDepth ] );
	
	blck = keyDesc.keyRootID;
	for (curLevel = keyStackSize - 1; curLevel >= 0; curLevel--) {
		keyStack[curLevel].stkBlock = blck;
		ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, blck ) );
		
		blck = kbp->kblkKey[0].data;
		keyStack[curLevel].stkIndex = 0;
		
		delete vkbp;
	}
}

//
//	JDBKey::UnloadStack
//

void JDBKey::UnloadStack(void)
{
	delete[] keyStack;
}

//
//	JDBKey::NewKey
//

int JDBKey::NewKey(long key, long data)
{
	const int		maxKeyCount = keyDesc.keyL0BF
	;
	int				stat
	,				curLevel, offset
	,				splitIndex
	,				keyStackSize
	;
	long			addKey, akKey, akData
	,				modKey, mkKey
	;
	JDBKeyRecordPtr	keyp
	;
	blockNumber		blck
	;
	JVBlockPtr		vkbp, vnbp, vtbp
	;
	JDBKeyBlockPtr	kbp, nbp, tbp
	;
	
	stat = 0;
	
	keyStackSize = keyDesc.keyDepth;
	ThrowIfMemFail_( keyStack = new keyStackElem[ keyDesc.keyDepth ] );
	
	// travel down the tree, looking for the key
	blck = keyDesc.keyRootID;
	for (curLevel = keyStackSize - 1; curLevel >= 0; curLevel--) {
		ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, blck ) );
		
		// stop when a key is reached greater than the one to add (insert)
		for (offset = 0, keyp = kbp->kblkKey; offset < kbp->kblkCount; offset++, keyp++ )
			if ((*keyComp)(key,keyp->key) < 0)	// (key < keyp->key)
				break;
		
		//
		//	If not on the leaf level and passed the end of the block, rewind
		//	slightly to the last key in the block.
		//
		if (curLevel && (offset == kbp->kblkCount)) {
			offset -= 1;
			keyp -= 1;
		}
		
		// store the match in the stack
		keyStack[curLevel].stkBlock = blck;
		keyStack[curLevel].stkIndex = offset;
		
		// set up for next level down
		blck = keyp->data;
		
		delete vkbp;
	}
	
	//
	//	Add the key.  For each level if the new item is the last one in the 
	//	block then the next level up will need the key changed.
	//	
	//	If there's not enough space to insert the key, split the bucket.  This
	//	means the insertion location needs re-computing for the existing 
	//	level and a new key added to the next level up.
	//
	addKey = 1;
	modKey = 0;
	for (curLevel = 0; curLevel < keyStackSize; curLevel++) {
		ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, keyStack[curLevel].stkBlock ) );
		offset = keyStack[curLevel].stkIndex;
		
		// if key needs modification, do it
		if (modKey) {
			vkbp->Modified();
			kbp->kblkKey[offset].key = mkKey;
			
			// if the last key was modified, the parent must as well
			modKey = (offset == kbp->kblkCount - 1);
		}
		
		//
		//	If nothing more to do, break.  If only modifying the key value loop
		//	around again to the parent level.
		//
		if (!addKey) {
			delete vkbp;
			
			if (!modKey)
				break;
			else
				continue;
		}
		
		// reset the add and modify flags
		addKey = 0;
		modKey = 0;
		
		//
		//	Check to see if there's enough space in the block for the incoming
		//	(key,data) pair.  If not, split the current block into two blocks, 
		//	insert the (key,data) where it should go, and tell the parent block 
		//	it's got a new child.
		//
		if (kbp->kblkCount == maxKeyCount) {
			// initialize a new block
			blck = keyMgr->mgrFile->NewBlock( kJDBKeyBlockSize + keyDesc.keyL0BF * kJDBKeyRecordSize );
			
			ThrowIfMemFail_( vnbp = new JVBlock( keyMgr->mgrFile, (JHandle)&nbp, blck ) );
			
			nbp->kblkPrev = kbp->kblkPrev;
			nbp->kblkNext = vkbp->blockID;
			nbp->kblkCount = 0;
			vnbp->Modified();
			
			// set previous and next links
			blck = kbp->kblkPrev;
			if (blck) {
				ThrowIfMemFail_( vtbp = new JVBlock( keyMgr->mgrFile, (JHandle)&tbp, blck ) );
				
				tbp->kblkNext = vnbp->blockID;
				vtbp->Modified();
				
				delete vtbp;
			}
			kbp->kblkPrev = vnbp->blockID;
			vkbp->Modified();
			
			// calculate the split point
			splitIndex = maxKeyCount / 2;
			
			// copy front half of old block to new one
			JBlockMove( (JPtr)&kbp->kblkKey[0], (JPtr)&nbp->kblkKey[0], splitIndex * kJDBKeyRecordSize );
			nbp->kblkCount = splitIndex;
			
			// slide back half of old block down
			memmove( &kbp->kblkKey[0], &kbp->kblkKey[splitIndex], (kbp->kblkCount - splitIndex) * kJDBKeyRecordSize );
			kbp->kblkCount -= splitIndex;
			
			// add the last key in the new block to the next level up
			keyp = nbp->kblkKey + (nbp->kblkCount - 1);
			addKey = 1;
			akKey  = keyp->key;
			akData = vnbp->blockID;
			
			//
			//	Stack needs to be deeper if the add key request goes up passed
			//	the root.  It's easy to build a new root block here because the 
			//	last key in the new block and the just-split block are available.
			//
			if (curLevel == (keyStackSize-1)) {
				// initialize a new root block
				blck = keyMgr->mgrFile->NewBlock( kJDBKeyBlockSize + keyDesc.keyL0BF * kJDBKeyRecordSize );
				ThrowIfMemFail_( vtbp = new JVBlock( keyMgr->mgrFile, (JHandle)&tbp, blck ) );
				
				tbp->kblkPrev = 0;
				tbp->kblkNext = 0;
				tbp->kblkCount = 2;
				vtbp->Modified();
				
				// add key for new added block
				tbp->kblkKey[0].key = akKey;
				tbp->kblkKey[0].data = akData;
				
				// add key for just-split block
				keyp = kbp->kblkKey + (kbp->kblkCount - 1);
				tbp->kblkKey[1].key = keyp->key;
				tbp->kblkKey[1].data = keyDesc.keyRootID;
				
				// tell the key the new root and that it's deeper
				keyDesc.keyRootID = vtbp->blockID;
				keyDesc.keyDepth += 1;
				
				delete vtbp;
			}
			
			//
			//	Figure out which block to use for the insert.  If the
			//	insert point was before the split the stack needs to 
			//	be patched to use the new block.
			//
			if (offset < splitIndex) {
				keyStack[curLevel].stkBlock = vnbp->blockID;
				delete vkbp;
				
				ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, keyStack[curLevel].stkBlock ) );
			} else
				offset -= splitIndex;
			
			delete vnbp;
		}
		
		//
		//	If this (key,data) touple is going to be the last key in the
		//	block and the key is different, the parent needs to be told 
		//	the new key value.  Otherwise, make space in the block.
		//
		if (offset == kbp->kblkCount) {
			if (kbp->kblkKey[offset-1].key != key) {
				modKey = 1;
				mkKey = key;
			}
		} else
			memmove(	&kbp->kblkKey[offset+1]
			,			&kbp->kblkKey[offset]
			,			(kbp->kblkCount - offset) * kJDBKeyRecordSize
			);
		
		// set the block value
		kbp->kblkKey[offset].key  = key;
		kbp->kblkKey[offset].data = data;
		kbp->kblkCount += 1;
		
		vkbp->Modified();
		delete vkbp;
		
		// don't bother going around again if we're done
		if (!addKey && !modKey)
			break;
		
		//
		//	When a block gets split the key value and data are stashed away.
		//	Stuffing the values back into (key,data) makes the parent level 
		//	processing the same as the current level.
		//
		key = akKey;
		data = akData;
	}
	
	// free the stack
	delete[] keyStack;
	
	// if it was successful, add one to the key count
	if (!stat) {
		keyDesc.keyCount += 1;
		stat = keyMgr->mgrObjMgr->WriteObject( objID, &keyDesc );
	}
	return stat;
}

//
//	JDBKey::DeleteKey
//

int JDBKey::DeleteKey(long key, long data)
{
	int				stat, rstat, curLevel, offset, thunk
	,				keyStackSize
	;
	long			delKey
	,				modKey, mkKey
	;
	JDBKeyRecordPtr	keyp
	;
	blockNumber		blck
	;
	JVBlockPtr		vkbp, vtbp
	;
	JDBKeyBlockPtr	kbp, tbp
	;
	
	stat = 0;
	rstat = 0;
	vkbp = 0;
	
	keyStackSize = keyDesc.keyDepth;
	ThrowIfMemFail_( keyStack = new keyStackElem[ keyStackSize ] );
	
	//
	//	Travel down the tree, looking for the first occurence, but leave
	//	the exact check for the next loop.
	//
	blck = keyDesc.keyRootID;
	for (curLevel = keyStackSize - 1; curLevel > 0; curLevel--) {
		ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, blck ) );
		keyStack[curLevel].stkBlock = blck;
		
		// find the first occurence
		for (offset = 0, keyp = kbp->kblkKey; offset < kbp->kblkCount; offset++, keyp++ )
			if ((*keyComp)(key,keyp->key) <= 0)	// (key <= keyp->key)
				break;

		keyStack[curLevel].stkIndex = offset;
		
		// prepare for the next level
		if (offset < kbp->kblkCount)
			blck = keyp->data;
		else {
			rstat = -1 /* key not found */;
			goto error;
		}
		
		delete vkbp;
		vkbp = 0;
	}
	
	// linear scan level zero for an exact (key,data) match
	thunk = 0;
	for (;;) {
		ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, blck ) );
		
		// stop if you can't find it
		for (offset = 0, keyp = kbp->kblkKey; offset < kbp->kblkCount; offset++, keyp++ ) {
			if ((*keyComp)(key,keyp->key) < 0) {		// (key < keyp->key)
				rstat = -1 /* key not found */;
				goto error;
			}
			if ((key == keyp->key) && (data == keyp->data))
				break;
		}
		if (offset < kbp->kblkCount) {
			delete vkbp;
			vkbp = 0;
			break;
		}
		
		// try the next block
		thunk = 1;
		blck = kbp->kblkNext;
		
		// unbind from the current block
		delete vkbp;
		vkbp = 0;
		
		if (!blck) {
			rstat = -1 /* key not found */;
			goto error;
		}
	}
	
	// save the level zero location just found
	keyStack[0].stkBlock = blck;
	keyStack[0].stkIndex = offset;
	
	//
	//	Clean up the stack.  As long as thunk is true, some bridging 
	//	had to occur.  The search is specificly looking for the block 
	//	number of the block already found at the next level down.
	//
	for (curLevel = 1; thunk; curLevel++) {
		thunk = 0;
nxtBlock:
		ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, keyStack[curLevel].stkBlock ) );
		
		blck = keyStack[curLevel-1].stkBlock;
		for (offset = keyStack[curLevel].stkIndex; offset < kbp->kblkCount; offset++)
			if (kbp->kblkKey[offset].data == blck)
				break;
		
		blck = kbp->kblkNext;
		delete vkbp;
		vkbp = 0;
		
		if (offset < kbp->kblkCount) {
			// found it!
			keyStack[curLevel].stkIndex = offset;
		} else {
			//
			//	Bad news, set thunk for the parent level and try the 
			//	next block
			//
			thunk = 1;
			
			// off the end of the list is real bad
			if (!blck) {
				rstat = -1 /* corrupted tree */;
				goto error;
			}
			
			// store this in the stack as if it's the right one
			keyStack[curLevel].stkBlock = blck;
			keyStack[curLevel].stkIndex = 0;
			goto nxtBlock;
		}
	}
	
	//
	//	Delete the key.  For each level if the item is the last one in the 
	//	list then the next level up will need the key changed.  If it was the 
	//	only key in the block, the next level up needs the key deleted.
	//
	delKey = 1;
	modKey = 0;
	for (curLevel = 0; curLevel < keyStackSize; curLevel++) {
		ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, keyStack[curLevel].stkBlock ) );
		offset = keyStack[curLevel].stkIndex;
		
		// if key needs modification, do it
		if (modKey) {
			kbp->kblkKey[offset].key = mkKey;
			vkbp->Modified();
			
			// if the last key was modified, the parent must as well
			modKey = (offset == kbp->kblkCount - 1);
		}
		
		//
		//	If nothing more to do, break.  If only modifying the key value loop
		//	around again to the parent level.
		//
		if (!delKey)
			if (!modKey)
				break;
			else
				continue;
		
		// reset the flags
		delKey = 0;
		modKey = 0;
		
		// see if this is the last key in the block
		if (kbp->kblkCount == 1) {
			// the root block doesn't go away, only the count is cleared
			if (curLevel == keyStackSize-1) {
				kbp->kblkCount = 0;
				vkbp->Modified();
				break;
			}
			
			// clean up the linked list at this level
			
			// fix previous block
			if (kbp->kblkPrev) {
				ThrowIfMemFail_( vtbp = new JVBlock( keyMgr->mgrFile, (JHandle)&tbp, kbp->kblkPrev ) );
				
				tbp->kblkNext = kbp->kblkNext;
				vtbp->Modified();
				
				delete vtbp;
			}
			
			// fix next block
			if (kbp->kblkNext) {
				ThrowIfMemFail_( vtbp = new JVBlock( keyMgr->mgrFile, (JHandle)&tbp, kbp->kblkNext ) );
				
				tbp->kblkPrev = kbp->kblkPrev;
				vtbp->Modified();
				
				delete vtbp;
			}
			
			// unbind from the block at this level
			delete vkbp;
			vkbp = 0;
			
			// free the block
			keyMgr->mgrFile->DeleteBlock( keyStack[curLevel].stkBlock );
			
			// tell next level up to delete the key that pointed to this block
			delKey = 1;
		} else {
			if (offset == kbp->kblkCount - 1) {		// last key in block?
				kbp->kblkCount -= 1;
				vkbp->Modified();
				
				// a parent modification is done only when last key different
				mkKey = kbp->kblkKey[offset-1].key;
				modKey = (mkKey != key);
			} else {
				// compress the key out of the block
				memmove(	&kbp->kblkKey[offset]
				,			&kbp->kblkKey[offset+1]
				,			(kbp->kblkCount - offset - 1) * kJDBKeyRecordSize
				);
				kbp->kblkCount -= 1;
				vkbp->Modified();
			}
				
			// unbind from the block at this level
			delete vkbp;
			vkbp = 0;
		}
	}
	
error:
	// unbind if there was some kind of error
	if (vkbp)
		delete vkbp;
	
	// free the stack
	delete[] keyStack;
	
	// if success, decrement the total number of keys
	if (!rstat) {
		keyDesc.keyCount -= 1;
		stat = keyMgr->mgrObjMgr->WriteObject( objID, &keyDesc );
	}
	
	return rstat;
}

//
//	JDBKey::ReadKey
//

int JDBKey::ReadKey(long key, long *data)
{
	int				stat = 0
	;
	JDBKeyRecordPtr	keyp
	;
	JVBlockPtr		vkbp
	;
	JDBKeyBlockPtr	kbp
	;
	
	// load up the blocks in the stack
	LoadStack( key );
	
	ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, keyStack->stkBlock ) );
	
	// make sure we haven't run off the end of the block
	if (keyStack->stkIndex == kbp->kblkCount)
		stat = -1 /* key not found */;
	else {
		// copy the data out of the record
		keyp = kbp->kblkKey + keyStack->stkIndex;
		if (keyp->key == key)
			*data = keyp->data;
		else
			stat = -1 /* key not found */;
	}
	
	delete vkbp;
	
	// unload the stack
	UnloadStack();
	
	return stat;
}

//
//	JDBKey::WriteKey
//

int JDBKey::WriteKey(long key, long data)
{
	int				stat = 0
	;
	JDBKeyRecordPtr	keyp
	;
	JVBlockPtr		vkbp
	;
	JDBKeyBlockPtr	kbp
	;
	
	// load up the blocks in the stack
	LoadStack( key );
	
	// bind to the stack bottom
	ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, keyStack->stkBlock ) );
	
	// make sure we haven't run off the end of the block
	if (keyStack->stkIndex == kbp->kblkCount)
		stat = -1 /* key not found */;
	else {
		// copy the data out of the record
		keyp = kbp->kblkKey + keyStack->stkIndex;
		if (keyp->key == key) {
			keyp->data = data;
			vkbp->Modified();
		} else
			stat = -1 /* key not found */;
	}
	
	// unbind
	delete vkbp;
	
	// unload the stack
	UnloadStack();
	
	return stat;
}

//
//	JDBKey::SearchKey
//	
//	This routine is very good for scanning a range of key values.  The 
//	search function will be called for every key that matches the search 
//	criteria.
//	
//	The first condition cannot be kNoMatch.  If first condition is kEqual 
//	or kNotEqual the second condition must be kNoMatch.  Other sets of 
//	conditions will be reordered and/or optimized away.
//	
//	This function takes advantage of the fact that level zero has the 
//	keys in ascending order and it's a linked list.  The stack is released  
//	before the search really starts to unlock the blocks in the cache.
//

int gTestMatch[7][3] =
	{ { 1, 1, 1 }	/* kNoMatch */
	, { 1, 0, 0 }	/* kLessThan */
	, { 1, 1, 0 }	/* kLessEqual */
	, { 0, 1, 0 }	/* kEqual */
	, { 1, 0, 1 }	/* kNotEqual */
	, { 0, 0, 1 }	/* kGreaterThan */
	, { 0, 1, 1 }	/* kGreaterEqual */
	};

int JDBKey::SearchKey(keyMethod op1, long key1, keyMethod op2, long key2, searchKeyFn fn)
{
	int				stat, curLevel, strtIndex, tst, slide
	;
	long			xKey
	;
	keyMethod		xop
	;
	JDBKeyRecordPtr	keyp
	;
	blockNumber		blkNum
	;
	JVBlockPtr		vkbp = 0
	;
	JDBKeyBlockPtr	kbp = 0
	;
	
	//
	//	The first condition cannot be kNoMatch.  If first condition is kEqual 
	//	or kNotEqual the second condition must be kNoMatch.  The second condition 
	//	cannot be kEqual or kNotEqual.  Other sets of conditions will be 
	//	reordered and/or optimized away.
	//
	if (op1 == kNoMatch)
		Throw_( err_Parm );
	if (((op1 == kEqual) || (op1 == kNotEqual)) && (op2 != kNoMatch))
		Throw_( err_Parm );
	if ((op2 == kEqual) || (op2 == kNotEqual))
		Throw_( err_Parm );
	
	// optimize the comparison combinations
	if (op2 > op1) {
		xop = op1;    op1 = op2;    op2 = xop;
		xKey = key1;  key1 = key2;  key2 = xKey;
	}
	if ((op1 <= kLessEqual) && (op2 != kNoMatch)) {
		if ((*keyComp)( key1, key2 ) > 0) {
			op1 = op2;
			key1 = key2;
		}
		op2 = kNoMatch;
	} else
	if (op2 >= kGreaterThan) {
		if ((*keyComp)( key1, key2 ) < 0) {
			op1 = op2;
			key1 = key2;
		}
		op2 = kNoMatch;
	}
	
	stat = 0;
	slide = 1;
	
	// load up the blocks in the stack
	if ((op1 <= kLessEqual) || (op1 == kNotEqual))
		LoadStackZero();
	else
		LoadStack( key1 );
	
	// store this stuff for the loop
	blkNum = keyStack->stkBlock;
	strtIndex = keyStack->stkIndex;
	
	// unload the stack to ease up on memory usage
	UnloadStack();
	
	// assume the block changed
	goto nextBlock;
	
nextKey:
	// loop thru all the keys in this block
	keyp = kbp->kblkKey + strtIndex;
	for (curLevel = strtIndex; curLevel < kbp->kblkCount; curLevel++, keyp++ ) {
		switch (op1) {
			case kLessThan:
			case kLessEqual:
			case kEqual:
				tst = (*keyComp)( keyp->key, key1 );
				if (!gTestMatch[ (int)op1 ][ tst + 1 ])
					goto fini;
				break;
			case kNotEqual:
				if ((*keyComp)( keyp->key, key1 ) == 0)
					continue;
				break;
			case kGreaterThan:
				if (slide) {
					if ((*keyComp)( keyp->key, key1 ) > 0)
						slide = 0;
					else
						continue;
				}
				break;
		}
		if (op2 >= kLessThan) {
			tst = (*keyComp)( keyp->key, key2 );
			if (!gTestMatch[ (int)op2 ][ tst + 1 ])
				goto fini;
		}
		
		stat = (*fn)( keyp->key, keyp->data );
		if (stat)
			goto fini;
	}
	blkNum = kbp->kblkNext;
	strtIndex = 0;
	
nextBlock:
	// move to the next block
	if (blkNum) {
		// unbind from the current block, if any
		if (vkbp) delete vkbp;
		ThrowIfMemFail_( vkbp = new JVBlock( keyMgr->mgrFile, (JHandle)&kbp, blkNum ) );
		
		goto nextKey;
	}
	
fini:
	// finished looking for keys
	if (vkbp) delete vkbp;
	
	return stat;
}

//
//	LongKeyComp
//

int LongKeyComp(long a, long b)
{
	if (a < b)
		return -1;
	else
	if (a > b)
		return 1;
	else
		return 0;
}

//
//	UnsignedKeyComp
//

int UnsignedKeyComp(long a, long b)
{
	register unsigned	ua, ub
	;
	
	ua = *(unsigned *)&a;
	ub = *(unsigned *)&b;
	
	if (ua < ub)
		return -1;
	else
	if (ua > ub)
		return 1;
	else
		return 0;
}

//
//	FloatKeyComp
//

int FloatKeyComp(long a, long b)
{
	register float	fa, fb
	;
	
	fa = *(float *)&a;
	fb = *(float *)&b;
	if (fa < fb)
		return -1;
	else
	if (fa > fb)
		return 1;
	else
		return 0;
}
