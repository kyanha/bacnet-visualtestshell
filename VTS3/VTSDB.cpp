// VTSDB.cpp: implementation of the VTSDB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTS.h"

#include "JConfig.hpp"

#include "VTSDB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const char *gVTSPortTypes[6] =
	{ "Null"
	, "IP"
	, "Ethernet"
	, "ARCNET"
	, "MS/TP"
	, "PTP"
	};

//
//	VTSPacketHeader::VTSPacketHeader
//
//	The packet header contains information about the packet: where it 
//	came from, where it was going, how to interpret the contents, etc.
//	It is stored in the packet list array in the database, while the 
//	packet contents (which vary in length) are stored in JDBOctetStr 
//	objects.
//

VTSPacketHeader::VTSPacketHeader( void )
	: packetPortID(0)				// port identifier
	, packetProtocolID(0)			// protocol identifier for decoding
	, packetFlags(0)				// protocol specific flags
	, packetType(txData)			// not really transmit
	, packetSource(nullAddr)		// source address
	, packetDestination(nullAddr)	// destination address
	, packetDataID(0)				// no assigned data (yet)
{
    ::GetSystemTimeAsFileTime( &packetTime );	// gets current time
}

//
//	VTSPacket::VTSPacket
//
//	If the packet is constructed with known data (the length is non-zero 
//	and the data is not null) then the packet just uses that information 
//	rather than make a copy of it.  The idea is to keep the amount of 
//	buffer creation and memory copies to a minimum.
//
//	In the ReadPacket function, the packet is created without data, then 
//	new data is provided once the information is available from the database.
//	Because the database object is released, it is important to make a copy.
//	The application may use the same packet object in multiple reads (like 
//	in a search loop).
//

VTSPacket::VTSPacket( int len, BACnetOctetPtr data )
	: packetHdr()					// empty header
	, ownData(false)				// packet does not own data
	, packetLen(0)					// number of octets in data
	, packetData(0)					// pointer to data
{
	// make a reference to the data
	if (len != 0) {
		packetLen = len;
		packetData = data;
	}
}

//
//	VTSPacket::~VTSPacket
//
//	If the packet owns its data pointer, delete it.
//

VTSPacket::~VTSPacket( void )
{
	// toss current data iff owned
	if (ownData && packetData)
		delete[] packetData;
}

//
//	VTSPacket::NewData
//
//	This function is called when the packet contents should reflect a new 
//	record from the database.
//

void VTSPacket::NewData( BACnetOctetPtr data, int len )
{
	// toss current data iff owned
	if (ownData && packetData)
		delete[] packetData;

	// take ownership of the pointer
	ownData = true;

	// make a copy of the data
	packetLen = len;
	packetData = new BACnetOctet[ len ];
	memcpy( packetData, data, len );
}

//
//	VTSPacket::NewDataRef
//
//	This function is called when the packet contents should reflect new data
//	and the application knows that the packet will be destroyed at the time 
//	the packet goes away (no dangling pointers).
//

void VTSPacket::NewDataRef( BACnetOctetPtr data, int len )
{
	// toss current data iff owned
	if (ownData && packetData)
		delete[] packetData;

	// this is not owned data
	ownData = false;

	// refer to the new data
	packetLen = len;
	packetData = data;
}

//
//	VTSDB::Init
//

void VTSDB::Init( void )
{
	int		stat
	;
	
	// let the database set itself up
	JDB::Init();

	// build some managers for objects and arrays
	pObjMgr = new JDBObjMgr( this );
	pArrayMgr = new JDBArrayMgr( pObjMgr );
	pListMgr = new JDBListMgr( pObjMgr );

	// allocate a descriptor
	dbDesc = (VTSDescObjPtr)AllocDesc( kVTSDescSig, kVTSDescObjSize );

	// create the port list
	if ((stat = pListMgr->NewList( dbPortList, kObjIdSize )) != 0) {
		return;
	}

	// save the list ID in the descriptor
	dbDesc->portListID = dbPortList.objID;

	// create the name list
	if ((stat = pListMgr->NewList( dbNameList, kVTSNameDescSize )) != 0) {
		return;
	}

	// save the list ID in the descriptor
	dbDesc->nameListID = dbNameList.objID;

	// create the packet list
	if ((stat = pArrayMgr->NewArray( dbPacketList, kVTSPacketHeaderSize, 4096 / kVTSPacketHeaderSize )) != 0) {
		return;
	}

	// save the packet list ID in the descriptor
	dbDesc->packetListID = dbPacketList.objID;
}

//
//	VTSDB::Open
//

void VTSDB::Open( void )
{
	int		stat
	;
	
	// open the database
	JDB::Open();

	// build some managers for objects and arrays
	pObjMgr = new JDBObjMgr( this );
	pArrayMgr = new JDBArrayMgr( pObjMgr );
	pListMgr = new JDBListMgr( pObjMgr );
	
	// get a pointer to our descriptor
	dbDesc = (VTSDescObjPtr)GetDesc( kVTSDescSig );

	// load the port list
	if ((stat = pListMgr->GetList( dbPortList, dbDesc->portListID )) != 0)
		return;

	// load the name list
	if ((stat = pListMgr->GetList( dbNameList, dbDesc->nameListID )) != 0)
		return;

	// load the packet array
	if ((stat = pArrayMgr->GetArray( dbPacketList, dbDesc->packetListID )) != 0)
		return;
}

//
//	VTSDB::Close
//

void VTSDB::Close( void )
{
	// all done with the managers
	delete pObjMgr;
	delete pArrayMgr;
	delete pListMgr;

	// pass it along
	JDB::Close();
}

//
//	VTSDB::GetPacketCount
//
//	This simple function returns the number of packets in the database.  Unlike the
//	ReadPacket and WritePacket functions, no changes are made, and the array object 
//	is really bound into the database.  So just to make sure that things like the 
//	cache don't get modified by another thread, lock the writeLock.
//

int VTSDB::GetPacketCount( void )
{
	int			count
	;
	CSingleLock lock( &writeLock )
	;
	
	lock.Lock();
	count = dbPacketList.Count();
	lock.Unlock();

	return count;
}

//
//	VTSDB::DeletePackets
//

void VTSDB::DeletePackets( void )
{
	int					stat
	;
	VTSDBTransaction	trans( this )				// build a transaction wrapper
	;

	// sure would be nice to delete the old ones...

	// create a new packet list
	if ((stat = pArrayMgr->NewArray( dbPacketList, kVTSPacketHeaderSize, 4096 / kVTSPacketHeaderSize )) != 0) {
		return;
	}

	// save the packet list ID in the descriptor
	dbDesc->packetListID = dbPacketList.objID;

	// finished with update
	trans.CompleteTransaction();
}

//
//	VTSDB::ReadPacket
//
//	Read a packet from the database.  Because the cache may move around and change 
//	some pointers, lock it first.  There is no reason to create a transaction because 
//	no modifications will be made.
//

void VTSDB::ReadPacket( int indx, VTSPacket& pkt )
{
	int			stat
	;
	CSingleLock lock( &writeLock )
	;
	
	// make sure no other threads are mucking around
	lock.Lock();

	// read the packet header from the array
	if ((stat = dbPacketList.ReadElem( indx, &pkt.packetHdr )) != 0)
		throw stat;

	// if there's data copy it from the octet string
	if (pkt.packetHdr.packetDataID != 0) {
		// load up the packet data
		JDBOctetStringPtr	osp = new JDBOctetString( pObjMgr, pkt.packetHdr.packetDataID )
		;

		// copy it to the pkt
		pkt.NewData( osp->GetDataPtr(), osp->GetLength() );

		// finished with object
		delete osp;
	} else
		pkt.NewDataRef( 0, 0 );

	// be nice and release it before returning
	lock.Unlock();
}

//
//	VTSDB::WritePacket
//

void VTSDB::WritePacket( int indx, VTSPacket& pkt )
{
	int					stat, rnum
	;
	VTSDBTransaction	trans( this )				// build a transaction wrapper
	;

	// is this updating an existing record?
	if (indx >= 0) {
		VTSPacketHeader		oldHdr
		;
		
		// read the old header
		if ((stat = dbPacketList.ReadElem( indx, &oldHdr )) != 0)
			throw stat;

		// if there is a JDBOctetStringObj there, delete it
		if (oldHdr.packetDataID != 0)
			pObjMgr->DeleteObject( oldHdr.packetDataID );
	}

	// create a new object with the packet contents
	JDBOctetStringPtr	osp = new JDBOctetString( pObjMgr, pkt.packetData, pkt.packetLen )
	;

	// save the object ID into the packet header
	pkt.packetHdr.packetDataID = osp->objID;

	// finished saving contents (already saved when it was created)
	delete osp;

	// write back if this is an update, otherwise append
	if (indx >= 0) {
		if ((stat = dbPacketList.WriteElem( indx, &pkt.packetHdr )) != 0)
			throw stat;
	} else {
		if ((stat = dbPacketList.NewElem( &rnum, &pkt.packetHdr )) != 0)
			throw stat;
	}

	// finished with update
	trans.CompleteTransaction();
}

//
//	VTSDBTransaction::VTSDBTransaction
//
//	Bind to a database and start a transaction.  The writeLock will be locked first, 
//	the the database will be told to start.
//

VTSDBTransaction::VTSDBTransaction( VTSDBPtr dbp )
	: dbPtr(dbp), dbTransInProgress(false)
{
	StartTransaction();
}

//
//	VTSDBTransaction::~VTSDBTransaction
//
//	When this object is destroyed, the transaction context is complete.  If there 
//	was an exception thrown, the application might not have the chance to call 
//	CancelTransaction or CompleteTransaction.  The safe thing to do is cancel it.
//

VTSDBTransaction::~VTSDBTransaction( void )
{
	if (dbTransInProgress)
		CancelTransaction();
}

//
//	VTSDBTransaction::StartTransaction
//

void VTSDBTransaction::StartTransaction( void )
{
	// lock the database
	dbPtr->writeLock.Lock();

	// start the transaction
	dbPtr->StartTransaction();
	dbTransInProgress = true;
}

//
//	VTSDBTransaction::CompleteTransaction
//
//	Let the datbase know that the transaction has completed successfully and then 
//	unlock it for other threads.
//

void VTSDBTransaction::CompleteTransaction( void )
{
	// complete the transaction
	dbPtr->CompleteTransaction();
	dbTransInProgress = false;

	// unlock for other updates
	dbPtr->writeLock.Unlock();
}

//
//	VTSDBTransaction::CancelTransaction
//
//	Something bad happened and the transaction needs to be canceled.  Make sure 
//	it is properly cleaned up before releasing the lock for other threads.
//

void VTSDBTransaction::CancelTransaction( void )
{
	// cancel the transaction
	dbPtr->CancelTransaction();
	dbTransInProgress = false;

	// unlock for other updates
	dbPtr->writeLock.Unlock();
}
