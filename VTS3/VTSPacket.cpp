// VTSPacket.cpp: implementation of the VTSPacketKill class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vts.h"
#include "VTSPacket.h"
#include "VTSDoc.h"
#include "PI.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

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
//MAD_DB	: packetPortID(0)				// port identifier
	: packetProtocolID(0)			// protocol identifier for decoding
	, packetFlags(0)				// protocol specific flags
	, packetType(txData)			// not really transmit
	, packetSource(nullAddr)		// source address
	, packetDestination(nullAddr)	// destination address
//MAD_DB	, packetDataID(0)				// no assigned data (yet)
{
    ::GetSystemTimeAsFileTime( &packetTime );	// gets current time
	memset(m_szPortName, 0, sizeof(m_szPortName));
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

void VTSPacket::NewDataRef( BACnetOctetPtr data, int len, bool fOwned /* = false */ )
{
	// toss current data iff owned
	if (ownData && packetData)
		delete[] packetData;

	// this is not owned data
	ownData = fOwned;			// MAD_DB

	// refer to the new data
	packetLen = len;
	packetData = data;
}


void VTSPacket::operator =( const VTSPacket& pkt)
{
	memcpy(&packetHdr, &pkt.packetHdr, sizeof(packetHdr));
	NewData(pkt.packetData, pkt.packetLen);
}


LPCSTR VTSPacket::GetTimeStampString()
{
	FILETIME	locFileTime;
	SYSTEMTIME	st;
	static char		theTime[16];

	::FileTimeToLocalFileTime( &packetHdr.packetTime, &locFileTime );
	::FileTimeToSystemTime( &locFileTime, &st );
	sprintf( theTime, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	// Warning... must do something with this before calling this method again
	// or data will be overwritten

	return theTime;
}


LPCSTR VTSPacket::GetAddressString( VTSDoc * pdoc, VTSPacketType vtspkttype, int nLen /* = -1 */ )
{
	static char szNameBuff[128];		// ought to be large enough?
	LPCSTR		pszName = NULL;

	BACnetAddress * paddr = vtspkttype == rxData ? &packetHdr.packetSource : &packetHdr.packetDestination;

	memset(szNameBuff, 0, sizeof(szNameBuff));

	if( packetHdr.packetType == vtspkttype )
	{
		// attempt to resolve name... if no doc supplied, we can't, but no harm done.
		if ( pdoc != NULL )
			pszName = pdoc->AddrToName( *paddr, packetHdr.m_szPortName );

		if ( pszName == NULL )
		{
			switch(packetHdr.packetProtocolID)
			{
				case BACnetPIInfo::ipProtocol:		

					pszName = BACnetIPAddr::AddressToString(paddr->addrAddr);
					break;

				case BACnetPIInfo::ethernetProtocol:
				case BACnetPIInfo::arcnetProtocol:
				case BACnetPIInfo::mstpProtocol:
				case BACnetPIInfo::ptpProtocol:
				case BACnetPIInfo::msgProtocol:
				default:

					{
					BACnetAddr bacnetAddr(paddr);
					pszName = bacnetAddr.ToString();
					}
			}
		}

		if ( nLen != -1 )
			sprintf( szNameBuff, "%-*.*s", nLen+1, nLen, pszName );
		else
			strcpy(szNameBuff, pszName);
	}

	return szNameBuff;
}
