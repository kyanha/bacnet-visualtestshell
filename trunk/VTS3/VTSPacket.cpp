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

	bErrorDecode = FALSE;
}


/*
VTSPacket::VTSPacket()
{
	ownData = false;
	packetLen = 0;
	packetData = NULL;
}
*/
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

VTSPacket::VTSPacket( const VTSPacket& pkt)
{
	if(this != &pkt)
	{
		memcpy(&packetHdr, &pkt.packetHdr, sizeof(packetHdr));
		ownData = false;
		packetData = 0;
		packetLen = 0;
		NewData(pkt.packetData, pkt.packetLen);
	}
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


LPCSTR VTSPacket::GetAddressString( VTSDoc * pdoc, BOOL bSource, int nLen /* = -1 */ )
{
	static char szNameBuff[128];		// ought to be large enough?
	LPCSTR		pszName = NULL;

	BACnetAddress * paddr = bSource ? &packetHdr.packetSource : &packetHdr.packetDestination;

	memset(szNameBuff, 0, sizeof(szNameBuff));

	
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

	return szNameBuff;
}

void VTSPacket::FindNPDUStartPos(int& npduindex)
{
	if ( packetData == NULL )
		return;

	switch ((BACnetPIInfo::ProtocolType)packetHdr.packetProtocolID)
	{
		case BACnetPIInfo::ipProtocol:
			// new code is using the length specified in the BVLC packet to determine
			// the start of the NPDU.  Submitted in #1261344 by dmrichards on 8/19/2005
			// skip the fake ip header, address (4), and port (2) 
//			{ 
//			   npduindex += 6; //BVLC 
//			   if (packetData && packetData[npduindex] == 0x81) 
//			   { 
//			      npduindex += (packetData[npduindex+2]*256 + packetData[npduindex+3]); 
//				} 
//			} 
//			break; 

			// skip the fake ip header, address (4), and port (2)
			{
				npduindex += 6;
			
				//BVLC
				if (packetData[npduindex] == 0x81)
				{
					npduindex++;

					// extract the function
					int func = packetData[npduindex];

					npduindex += 3; //BVLC Function: 1 bytes; BVLC Length: 2 bytes

					// set the function group
					switch ((BVLCFunction)func)
					{
						case bvlcResult:
						case blvcWriteBroadcastDistributionTable:
						case blvcReadBroadcastDistributionTable:
						case blvcReadBroadcastDistributionTableAck:
						case bvlcRegisterForeignDevice:
						case bvlcReadForeignDeviceTable:
						case bvlcReadForeignDeviceTableAck:
						case bvlcDeleteForeignDeviceTableEntry:
							break;

						case blvcForwardedNPDU:
						npduindex += 6;
						break;

						// dig deeper into these
						case bvlcDistributeBroadcastToNetwork:
						case bvlcOriginalUnicastNPDU:
						case bvlcOriginalBroadcastNPDU:
							break;
					}
				}			
			}
			break;

		case BACnetPIInfo::ethernetProtocol:
			// skip over source (6), destination (6), length (2), and SAP (3)
			npduindex += 17;			
			break;

		case BACnetPIInfo::arcnetProtocol:
			// skip over source (1), destination (1), SAP (3), LLC (1)
			npduindex += 6;			
			break;

		case BACnetPIInfo::mstpProtocol:
			// skip over preamble
			npduindex += 2;			
			break;
		case BACnetPIInfo::msgProtocol:
		case BACnetPIInfo::bakRestoreMsgProtocol:
			npduindex = -1;		// just VTS message, do not contain npdu
			break;
	}
}

BOOL VTSPacket::GetSNET(unsigned short& snet)
{
	int npduindex = 0;	

	FindNPDUStartPos(npduindex);

	if(packetLen - npduindex < 2)
		return FALSE;

	if(!(packetData[npduindex + 1] & 0x08)) //SNET doesn't exist
		return FALSE;

	int index = 2;

	if(packetData[npduindex + 1] & 0x20) //DNET and DADR exist
	{
		index = index + 3 + packetData[npduindex + 4]; 		
	}

	snet = packetData[npduindex + index];	
	snet = (snet << 8) | packetData[npduindex + index + 1];
	
	return TRUE;
}

CString VTSPacket::GetSADRString(VTSDoc * pdoc, BOOL bAlias)
{
	CString sadrStr;
	BACnetAddress sadr;
	int npduindex = 0;

	FindNPDUStartPos(npduindex);

	if(packetLen - npduindex < 2)
		return "";

	if(!(packetData[npduindex + 1] & 0x08)) //SADR doesn't exist
		return "";

	int index = 4;

	if(packetData[npduindex + 1] & 0x20) //DNET and DADR exist
	{
		index = index + 3 + packetData[npduindex + 4]; 		
	}

	unsigned char slen = packetData[npduindex + index];

	if(slen == 0)
		return "";

	if ( slen > 7 )
		return "BAD07";    // error string can not be larger than this
	
	sadr.addrType = localStationAddr;
	sadr.addrLen = slen;
	memcpy(sadr.addrAddr, packetData + npduindex + index + 1, slen);		
	
	if ( pdoc != NULL && bAlias )
		sadrStr = pdoc->AddrToName(sadr, packetHdr.m_szPortName);

	if ( sadrStr == "" )
	{
		BACnetAddr bacnetAddr(&sadr);
		sadrStr = bacnetAddr.ToString();		
	}
	
	return sadrStr;
}

BOOL VTSPacket::GetDNET(unsigned short& dnet)
{
	CString str;
	BACnetAddress dadr;
	int npduindex = 0;

	FindNPDUStartPos(npduindex);

	if(packetLen - npduindex< 2)
		return FALSE;		

	if( !(packetData[npduindex + 1] & 0x20) ) //DNET and DADR don't exist
		return FALSE;

	dnet = packetData[npduindex + 2];	
	dnet = (dnet << 8) | packetData[npduindex + 3];	

	return TRUE;
}

CString VTSPacket::GetDADRString(VTSDoc * pdoc, BOOL bAlias)	
{
	CString dadrStr;
	BACnetAddress dadr;
	int npduindex = 0;

	FindNPDUStartPos(npduindex);

	if (npduindex == -1)
		return "";	// do not contain NPDU

	if(packetLen - npduindex< 2)
		return "";		

	if( !(packetData[npduindex + 1] & 0x20) ) //DNET and DADR don't exist
		return "";

	unsigned char dlen = packetData[npduindex + 4];

	if(dlen == 0)
		return "";	

	if (dlen > 7 )
		return "BAD07";  // probably bad packet do not parse any more of the addressing

	dadr.addrType = localStationAddr;
	dadr.addrLen = dlen;
	memcpy(dadr.addrAddr, packetData + npduindex + 5, dlen);		
	
	if ( pdoc != NULL && bAlias )
		dadrStr = pdoc->AddrToName(dadr, packetHdr.m_szPortName);

	if ( dadrStr == "" )
	{
		BACnetAddr bacnetAddr(&dadr);
		dadrStr = bacnetAddr.ToString();		
	}

	return dadrStr;
}

BOOL VTSPacket::SetDesAddress(BACnetAddress& addr)
{
	switch(packetHdr.packetProtocolID)
	{
	case BACnetPIInfo::ipProtocol:
		{
			if(addr.addrLen != 6)
				return FALSE;
			
			memcpy(packetData, addr.addrAddr, 6);
		}
		break;

	case BACnetPIInfo::ethernetProtocol:
		break;

	case BACnetPIInfo::mstpProtocol:
		break;
	}

	return TRUE;
}

BOOL VTSPacket::SetSNET(unsigned short snet, BOOL bReserveSnet)
{
	BOOL bDnet = FALSE;
	BOOL bSnet = FALSE;
	
	int npduindex = 0;	

	FindNPDUStartPos(npduindex);

	if(packetLen - npduindex < 2)
		return FALSE;

	if(!(packetData[npduindex + 1] & 0x08)) //SNET doesn't exist
	{
		bSnet = FALSE;
	}
	else
	{
		bSnet = TRUE;
	}

	if(packetData[npduindex + 1] & 0x20) //DNET and DADR exist
	{
		bDnet = TRUE;			
	}
	else
	{
		bDnet = FALSE;
	}

	if( bSnet )
	{
		if( bReserveSnet )
		{
			if( bDnet )
			{
				unsigned char dlen = packetData[npduindex + 4];
				packetData[npduindex + 5 + dlen] = (snet & 0xFF00) >> 8;	
				packetData[npduindex + 6 + dlen] = (snet & 0x00FF);
			}
			else
			{
				packetData[npduindex + 2] = (snet & 0xFF00) >> 8;	
				packetData[npduindex + 3] = (snet & 0x00FF);
			}			
		}
		else
		{
			packetData[npduindex + 1] &= 0xF7; //set SNET bit 0

			BACnetOctetPtr oldbuff = packetData;
			int oldPacketLen = packetLen;

			if( bDnet )
			{
				unsigned char dlen = oldbuff[npduindex + 4];
				unsigned char slen = oldbuff[npduindex + 7 + dlen];
				packetLen -= 3 + slen;

				packetData = new BACnetOctet[packetLen];
				memcpy(packetData, oldbuff, npduindex + 5 + dlen);
				memcpy(packetData + npduindex + 5 + dlen, oldbuff + npduindex + 8 + dlen + slen, 
					oldPacketLen - (npduindex + 8 + dlen + slen));
			}
			else
			{
				unsigned char slen = oldbuff[npduindex + 4];
				packetLen -= 3 + slen;

				packetData = new BACnetOctet[packetLen];
				memcpy(packetData, oldbuff, npduindex + 2);
				memcpy(packetData + npduindex + 2, oldbuff + npduindex + 5 + slen, 
					oldPacketLen - (npduindex + 5 + slen));
			}

			delete[] oldbuff;
		}
	}
	else
	{
		if( bReserveSnet )
		{
			packetData[npduindex + 1] |= 0x08; //set SNET bit 1

			BACnetOctetPtr oldbuff = packetData;
			int oldPacketLen = packetLen;

			packetLen += 3;
			packetData = new BACnetOctet[packetLen];

			if( bDnet )
			{
				unsigned char dlen = oldbuff[npduindex + 4];
				
				memcpy(packetData, oldbuff, npduindex + 5 + dlen);
				packetData[npduindex + 5 + dlen] = (snet & 0xFF00) >> 8;
				packetData[npduindex + 6 + dlen] = (snet & 0x00FF);

				packetData[npduindex + 7 + dlen] = 0; //slen = 0

				memcpy(packetData + npduindex + 8 + dlen, oldbuff + npduindex + 5 + dlen,
					oldPacketLen - (npduindex + 5 + dlen));
			}
			else
			{
				memcpy(packetData, oldbuff, npduindex + 2);

				packetData[npduindex + 2] = (snet & 0xFF00) >> 8;	
				packetData[npduindex + 3] = (snet & 0x00FF);

				packetData[npduindex + 4] = 0; //slen = 0

				memcpy(packetData + npduindex + 5, oldbuff + npduindex + 2,
					oldPacketLen - (npduindex + 2));
			}
		}
	}
	

	return TRUE;
}

//bReserveDnet = FALSE, ignore parameter 'dnet', delete DNET/DLEN/DADR from packet if they exit
//
BOOL VTSPacket::SetDNET(unsigned short dnet, BOOL bReserveDnet)
{
	CString str;
	BACnetAddress dadr;
	int npduindex = 0;
	BOOL bDnet = FALSE;
	BOOL bSnet = FALSE;

	FindNPDUStartPos(npduindex);

	if(packetLen - npduindex< 2)
		return FALSE;		

	if( !(packetData[npduindex + 1] & 0x20) ) 
		bDnet = FALSE; //DNET and DADR don't exist
	else
		bDnet = TRUE;

	if(!(packetData[npduindex + 1] & 0x08)) 
		bSnet = FALSE; //SNET doesn't exist
	else
		bSnet = TRUE;

	if( bDnet )
	{
		if( bReserveDnet )
		{
			packetData[npduindex + 2] = (dnet & 0xFF00) >> 8;
			packetData[npduindex + 3] = (dnet & 0x00FF);		
		}
		else
		{
			packetData[npduindex + 1] &= 0xDF; //set DNET bit 0
			unsigned char dlen = packetData[npduindex + 4];

			BACnetOctetPtr oldbuff = packetData;
			int oldPacketLen = packetLen;
			packetLen = packetLen - 4 - dlen; //DNET 2 bytes, DLEN 1 byte, DADR dlen bytes, Hop Count 1 byte
			packetData = new BACnetOctet[packetLen];
			memcpy(packetData, oldbuff, npduindex + 2);

			if( bSnet )
			{
				unsigned char slen = oldbuff[npduindex + 7 + dlen];
				memcpy(packetData + npduindex + 2, oldbuff + npduindex + 5 + dlen, 
				3 + slen);
				memcpy(packetData + npduindex + 5 + slen, oldbuff + npduindex + 9 + dlen + slen,
					oldPacketLen - (npduindex + 9 + dlen + slen));

			}
			else
			{
				memcpy(packetData + npduindex + 2, oldbuff + npduindex + 6 + dlen, 
				oldPacketLen - (npduindex + 6 + dlen)); //do not forget hopcount byte
			}			

			delete[] oldbuff;
		}		
	}
	else
	{
		if( bReserveDnet )
		{
			packetData[npduindex + 1] |= 0x20; //set DNET bit 1
			
			BACnetOctetPtr oldbuff = packetData;
			int oldPacketLen = packetLen;
			packetLen = packetLen + 4; //add DNET and DLEN and Hop Count, 4 byte
			packetData = new BACnetOctet[packetLen];
			memcpy(packetData, oldbuff, npduindex + 2);
			
			packetData[npduindex + 2] = (dnet & 0xFF00) >> 8;
			packetData[npduindex + 3] = (dnet & 0x00FF);
			packetData[npduindex + 4] = 0; //dlen = 0
			
			if( bSnet )
			{			
				unsigned char slen = oldbuff[npduindex + 4];
				memcpy(packetData + npduindex + 5, oldbuff + npduindex + 2, 3 + slen);
				packetData[npduindex + 8 + slen] = 0; //hopcount = 0
				memcpy(packetData + npduindex + 9 + slen, oldbuff + npduindex + 5 + slen,
					oldPacketLen - (npduindex + 5 + slen));
			}
			else
			{
				packetData[npduindex + 5] = 0; //hopcount = 0
				memcpy(packetData + npduindex + 6, oldbuff + npduindex + 2, 
					oldPacketLen - (npduindex + 2));
			}			
			
			delete[] oldbuff;
		}		
	}	
	
	return TRUE;
}

BOOL VTSPacket::SetDADR(unsigned char *dadr, unsigned char len)
{
	int npduindex = 0;

	FindNPDUStartPos(npduindex);

	if(packetLen - npduindex< 2)
		return FALSE;		

	if( !(packetData[npduindex + 1] & 0x20) ) //DNET and DADR don't exist
		return FALSE;

	unsigned char dlen = packetData[npduindex + 4];
	if( dlen == len )
	{
		memcpy(packetData + npduindex + 5, dadr, dlen);
	}
	else
	{
		BACnetOctetPtr oldbuff = packetData;
		int oldPacketLen = packetLen;
		packetLen = packetLen + len - dlen;
		packetData = new BACnetOctet[packetLen];
		memcpy(packetData, oldbuff, npduindex + 4);
		packetData[npduindex + 4] = len;
		memcpy(packetData + npduindex + 5, dadr, len);
		memcpy(packetData + npduindex + 5 + len, oldbuff + npduindex + 5 + dlen, 
			oldPacketLen - (npduindex + 5 + dlen));

		delete[] oldbuff;
	}

	return TRUE;
}

BOOL VTSPacket::SetSADR(unsigned char *sadr, unsigned char len)
{
	int npduindex = 0;

	FindNPDUStartPos(npduindex);

	if(packetLen - npduindex < 2)
		return FALSE;

	if(!(packetData[npduindex + 1] & 0x08)) //SADR doesn't exist
		return FALSE;

	int index = 4;

	if(packetData[npduindex + 1] & 0x20) //DNET and DADR exist
	{
		index = index + 3 + packetData[npduindex + 4]; 		
	}

	unsigned char slen = packetData[npduindex + index];
	if( slen == len )
	{
		memcpy(packetData + npduindex + index + 1, sadr, slen);
	}
	else
	{
		BACnetOctetPtr oldbuff = packetData;
		int oldPacketLen = packetLen;
		packetLen = packetLen + len - slen;
		packetData = new BACnetOctet[packetLen];
		memcpy(packetData, oldbuff, npduindex + index);
		packetData[npduindex + index] = len;
		memcpy(packetData + npduindex + index + 1, sadr, len);
		memcpy(packetData + npduindex + index + 1 + len, oldbuff + npduindex + index + 1 + slen, 
			oldPacketLen - (npduindex + index + 1 + slen));

		delete[] oldbuff;
	}

	return TRUE;
}