
#include "stdafx.h"

#include <stdlib.h>

#if UNIX_Platform
#include <string.h>
#endif

#include "JConfig.hpp"
#include "JError.hpp"
#include "JMemory.hpp"
#include "JDBArray.hpp"

#if _JDBArrayDebug
#include <iostream.h>
#endif

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JDBArrayMgr::JDBArrayMgr
//

JDBArrayMgr::JDBArrayMgr( JDBObjMgrPtr omp )
	: mgrObjMgr(omp), mgrFile( omp->mgrFile )
{
}

//
//	JDBArrayMgr::~JDBArrayMgr
//

JDBArrayMgr::~JDBArrayMgr(void)
{
}

//
//	JDBArrayMgr::NewArray
//

int JDBArrayMgr::NewArray(JDBArray &ary, int rSize, int L0BF, int L1BF)
{
	objId			id
	;
	
	ary.arrMgr = this;
	
	// set up some defaults
	if (L0BF == 0) L0BF = (kBlockSize / rSize);
	if (L1BF == 0) L1BF = (kBlockSize / sizeof(long));

	// allocate an array descriptor
	ReturnIfError_( mgrObjMgr->NewObject( &id, &ary.arrDesc, kJDBArrayDescSize ) );
	
	// initialize it
	ary.objID = id;
	ary.arrDesc.arryCount = 0;
	ary.arrDesc.arryLen = 0;
	ary.arrDesc.arryNRec = -1;
	ary.arrDesc.arrySize = rSize;
	ary.arrDesc.arryL0BF = L0BF;
	ary.arrDesc.arryL1BF = L1BF;
	
	// allocate a level zero block
	ary.arrDesc.arryDepth = 1;
	ary.arrDesc.arryRootID = mgrFile->NewBlock( rSize * L0BF );
#if _JDBArrayDebug
	cout << "NewArray: new root block " << ary.arrDesc.arryRootID << endl;
#endif
	
	// save the descriptor
	ReturnIfError_( mgrObjMgr->WriteObject( id, &ary.arrDesc ) );
	
	return 0;
}

//
//	JDBArrayMgr::GetArray
//

int JDBArrayMgr::GetArray(JDBArray &ary, objId id)
{
	ary.objID = id;
	ary.arrMgr = this;
	
	// get the array descriptor
	ReturnIfError_( mgrObjMgr->ReadObject( id, &ary.arrDesc ) );
	
	return 0;
}
		
//
//	JDBArrayMgr::DeleteArray
//

int JDBArrayMgr::DeleteArray(objId id)
{
	JDBArrayDesc	arrDesc
	;

	// get the array descriptor
	ReturnIfError_( mgrObjMgr->ReadObject( id, &arrDesc ) );
	
	// recursively delete the block tree
	DeleteArrayHelper( arrDesc.arryDepth, arrDesc.arryRootID, arrDesc.arryL1BF );
	
	// delete the array descriptor
	ReturnIfError_( mgrObjMgr->DeleteObject( id ) );
	
	return 0;
}

//
//	JDBArrayMgr::DeleteArrayHelper
//

void JDBArrayMgr::DeleteArrayHelper( int depth, blockNumber blckID, int bf )
{
	JVBlockPtr		bp
	;
	blockNumber		*blckList
	;
	
	if (depth > 1) {
		// get the block
		bp = new JVBlock( mgrFile, (JHandle)&blckList, blckID );
		
		// delete all of the blocks listed
		for (int i = 0; i < bf; i++)
			if (blckList[i] != 0)
				DeleteArrayHelper( depth-1, blckList[i], bf );

		// release the block back
		delete bp;
	}

	// now delete the block
	mgrFile->DeleteBlock( blckID );
}

//
//	JDBArray::JDBArray
//

JDBArray::JDBArray(void)
	: arrMgr(0), objID(0)
{
}

//
//	JDBArray::~JDBArray
//

JDBArray::~JDBArray(void)
{
}

//
//	JDBArray::LoadStack
//

int					*arrStack;				// list of useful offsets

void JDBArray::LoadStack(int rnum)
{
	int				i, divd, bsize
	,				arrStackSize
	;
	blockNumber		blck, *blckList
	;
	JVBlockPtr		bp
	;
		
#if _JDBArrayDebug
	cout << "LoadStack( " << rnum << " )" << endl;
#endif

	arrStackSize = arrDesc.arryDepth + 1;
	ThrowIfMemFail_( arrStack = new JDBArray::aryStackElem[ arrStackSize ] );
	
	/* init empty stack */
	for (i = 0; i < arrStackSize; i++) {
		arrStack[i].stkIndex = 0;
		arrStack[i].stkBlock = 0;
	}
	
	/* load up the offsets */
	divd = arrDesc.arryL0BF;
	for (i = 0; i < arrDesc.arryDepth; i++) {
		arrStack[i].stkIndex = rnum % divd;
		rnum = rnum / divd;
		divd = arrDesc.arryL1BF;
	}
	
	/* make the tree deeper? */
	if (rnum) {
#if _JDBArrayDebug
	cout << "	- tree deeper" << endl;
#endif
		arrDesc.arryDepth += 1;
		
		ThrowIfMemFail_( bp =
			new JVBlock( arrMgr->mgrFile, (JHandle)&blckList
				, -1, arrDesc.arryL1BF * sizeof(blockNumber)
				)
			);
		
		blckList[0] = arrDesc.arryRootID;
		arrDesc.arryRootID = bp->blockID;
		
		bp->Modified();
		delete bp;
		
		/* use next element, should not be allocated */
		arrStack[i].stkIndex = rnum;
		
		i += 1;
	}
	
	/* find all the blocks */
	blck = arrDesc.arryRootID;
	while (i--) {
		ThrowIfMemFail_( bp = new JVBlock( arrMgr->mgrFile, (JHandle)&blckList, blck ) );
		
		arrStack[i].stkBlock = blck;
		blck = blckList[ arrStack[i].stkIndex ];
#if _JDBArrayDebug
	cout << "	- level " << i << ", stkBlock = " << arrStack[i].stkBlock
		<< ", stkIndex = " << arrStack[i].stkIndex << endl;
#endif
		
		if ((!blck) && i) {
			if (i > 1)
				bsize = arrDesc.arryL1BF * sizeof(blockNumber);
			else
				bsize = arrDesc.arryL0BF * arrDesc.arrySize;
				
			blck = arrMgr->mgrFile->NewBlock( bsize );
			blckList[ arrStack[i].stkIndex ] = blck;
			bp->Modified();
		}
		
		delete bp;
	}
}

//
//	JDBArray::UnloadStack
//

void JDBArray::UnloadStack(void)
{
	delete[] arrStack;
}

//
//	JDBArray::NewElem
//

int JDBArray::NewElem(int *rnum, void *data)
{
	int			stat
	;
	char		*dest
	;
	JVBlockPtr	bp
	;
	
	if (arrDesc.arryNRec >= 0) {
		// use space held by record since deleted
		LoadStack( arrDesc.arryNRec );
		
		// pass the record number back
		*rnum = arrDesc.arryNRec;
		
		// suck it off the front of the SLL
		ThrowIfMemFail_( bp = new JVBlock( arrMgr->mgrFile, (JHandle)&dest, arrStack->stkBlock ) );
		dest += (arrDesc.arrySize * arrStack->stkIndex);
		
		arrDesc.arryNRec = *(long *)dest;
		JBlockMove( (JPtr)data, (JPtr)dest, arrDesc.arrySize );
		
		bp->Modified();
		delete bp;
		
		// unload the stack
		UnloadStack();
	} else {
		// no free rnums, get a new one on the end
		LoadStack( arrDesc.arryLen );
		
		// pass the record number back
		*rnum = arrDesc.arryLen;
		
		// tack the data on the end
		ThrowIfMemFail_( bp = new JVBlock( arrMgr->mgrFile, (JHandle)&dest, arrStack->stkBlock ) );
		
		JBlockMove( (JPtr)data, (JPtr)(dest + arrDesc.arrySize * arrStack->stkIndex), arrDesc.arrySize );
		
		bp->Modified();
		delete bp;
		
		arrDesc.arryLen += 1;
		
		// unload the stack
		UnloadStack();
	}
	arrDesc.arryCount += 1;
	
	// write the array descriptor
	stat = arrMgr->mgrObjMgr->WriteObject( objID, &arrDesc );
	
	// return the status
	return stat;
}

//
//	JDBArray::DeleteElem
//

int JDBArray::DeleteElem(int rnum)
{
	int			stat, rstat
	;
	char		*dest
	;
	JVBlockPtr	bp
	;
	
	if (rnum >= arrDesc.arryLen) {
		stat = -1 /* record never allocated */;
		goto fini;
	}
	
	LoadStack( rnum );
	
	// stuff it in the front of the SLL
	ThrowIfMemFail_( bp = new JVBlock( arrMgr->mgrFile, (JHandle)&dest, arrStack->stkBlock ) );
	dest += (arrDesc.arrySize * arrStack->stkIndex);
	
	*(long *)dest = arrDesc.arryNRec;
	bp->Modified();
	
	delete bp;
	
	arrDesc.arryNRec = rnum;
	arrDesc.arryCount -= 1;
	
	// unload the stack
	UnloadStack();
	
fini:
	if ((rstat = arrMgr->mgrObjMgr->WriteObject( objID, &arrDesc )) != 0)
		if (!stat)
			stat = rstat;
	
	return stat;
}

//
//	JDBArray::ReadElem
//

int JDBArray::ReadElem(int rnum, void *data)
{
	int			stat = 0
	;
	char		*dest
	;
	JVBlockPtr	bp
	;
	
	if (rnum >= arrDesc.arryLen) {
		stat = -1 /* record never allocated */;
		goto fini;
	}
	
	// load up the stack
	LoadStack( rnum );
	
	// copy out the data
	bp = new JVBlock( arrMgr->mgrFile, (JHandle)&dest, arrStack->stkBlock );
	
	JBlockMove( (JPtr)(dest + (arrDesc.arrySize * arrStack->stkIndex)), (JPtr)data, arrDesc.arrySize );
	
	delete bp;
	
	// unload the stack
	UnloadStack();
	
fini:
	return stat;
}

//
//	JDBArray::WriteElem
//

int JDBArray::WriteElem(int rnum, void *data)
{
	int			stat = 0
	;
	char		*dest
	;
	JVBlockPtr	bp
	;
	
	if (rnum >= arrDesc.arryLen) {
		stat = -1 /* record never allocated */;
		goto fini;
	}
	
	// load up the stack
	LoadStack( rnum );
	
	// copy the data
	ThrowIfMemFail_( bp = new JVBlock( arrMgr->mgrFile, (JHandle)&dest, arrStack->stkBlock ) );
	dest += (arrDesc.arrySize * arrStack->stkIndex);
	
	JBlockMove( (JPtr)data, (JPtr)dest, arrDesc.arrySize );
	
	bp->Modified();
	delete bp;
	
	// unload the stack
	UnloadStack();
	
fini:
	return stat;
}

//
//	JDBArray::Count
//

int JDBArray::Count( void )
{
	return arrDesc.arryCount;
}

//
//	JDBArray::AllocLength
//

int JDBArray::AllocCount( void )
{
	return arrDesc.arryLen;
}
