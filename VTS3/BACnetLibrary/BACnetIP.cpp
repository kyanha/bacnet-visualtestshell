
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "BACnet.hpp"
#include "BACnetIP.hpp"

#ifdef _MSC_VER
#define ENDIAN_SWAP     1
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#if (__DECCXX && __ALPHA)
#define ENDIAN_SWAP     1
#endif

#ifndef ENDIAN_SWAP
#define ENDIAN_SWAP     0
#endif

//
//	BACnetIPAddr::BACnetIPAddr
//

BACnetIPAddr::BACnetIPAddr( void )
{
}

//
//	BACnetIPAddr::BACnetIPAddr
//

BACnetIPAddr::BACnetIPAddr( const BACnetAddress &addr )
	: BACnetAddress( addr )
{
}

//
//	BACnetIPAddr::BACnetIPAddr
//

BACnetIPAddr::BACnetIPAddr( unsigned long host, unsigned short port )
{
	Pack( host, port );
}

//
//	BACnetIPAddr::BACnetIPAddr
//
//	Take an address in the form "x.x.x.x:x" and build a BACnet address 
//	out of it.
//

BACnetIPAddr::BACnetIPAddr( const char *spec )
{
	unsigned long	host
	;
	unsigned short	port
	;
	
	// parse the spec
	BACnetIPAddr::StringToHostPort( spec, &host, 0, &port );
	
	Pack( host, port );
}

//
//	BACnetIPAddr::Pack
//

void BACnetIPAddr::Pack( unsigned long host, unsigned short port )
{
	addrType = localStationAddr;
	addrLen = sizeof(unsigned long) + sizeof(unsigned short);
	Pack( addrAddr, host, port );
}

//
//	BACnetIPAddr::Unpack
//

void BACnetIPAddr::Unpack( unsigned long *hostp, unsigned short *portp )
	const
{
	Unpack( addrAddr, hostp, portp );
}

//
//	BACnetIPAddr::Pack
//

void BACnetIPAddr::Pack( BACnetOctet *rsltp, unsigned long host, unsigned short port )
{
#if ENDIAN_SWAP
	rsltp[3] = (host & 0xFF);	host >>= 8;
	rsltp[2] = (host & 0xFF);	host >>= 8;
	rsltp[1] = (host & 0xFF);	host >>= 8;
	rsltp[0] = (host & 0xFF);
	rsltp[5] = (port & 0xFF);	port >>= 8;
	rsltp[4] = (port & 0xFF);
#else
	*(unsigned long *)rsltp = host;
	*(unsigned short *)(rsltp + 4) = port;
#endif
}

//
//	BACnetIPAddr::Unpack
//

void BACnetIPAddr::Unpack( const BACnetOctet *rsltp, unsigned long *hostp, unsigned short *portp )
{
#if ENDIAN_SWAP
	unsigned long	host
	;
	unsigned short	port
	;

	host = *rsltp++;
	host = (host << 8) + *rsltp++;
	host = (host << 8) + *rsltp++;
	host = (host << 8) + *rsltp++;
	port = *rsltp++;
	port = (port << 8) + *rsltp;

	if (hostp) *hostp = host;
	if (portp) *portp = port;
#else
	if (hostp) *hostp = *(unsigned long *)rsltp;
	if (portp) *portp = *(unsigned short *)(rsltp + 4);
#endif
}

//
//	BACnetIPAddr::StringToHostPort
//

void BACnetIPAddr::StringToHostPort( const char *str, unsigned long *hostp, unsigned long *maskp, unsigned short *portp )
{
	int				n
	;
	unsigned long	host = 0
	;
	unsigned long	subnetmask = 0xFFFFFFFF
	;
	unsigned short	port = kBACnetIPDefaultPort
	;
	
	// first four decimals
	for (int i = 0; i < 4; i++ ) {
		BACnetOctet	byte = 0;
		while (*str && isdigit(*str))
			byte = (byte * 10) + (*str++ - '0');
		host = (host << 8) + byte;
		
		if (*str == '.')
			str += 1;
	}
	
	// check for number of bits in host
	if (*str == '/') {
		str += 1;
		
		// next is mask number
		if (isdigit(*str)) {
			n = 0;
			while (*str && isdigit(*str))
				n = (n * 10) + (*str++ - '0');
			
			if ((n > 0) && (n < 32))
				subnetmask = (subnetmask << (32 - n));
		}
	}
	
	// check for port number
	if (*str == ':') {
		str += 1;
		
		// next is port number
		if (isdigit(*str))
			if (toupper(*(str+1)) == 'X') {
				str += 2;
				for (port = 0; isxdigit(*str); str++)
					port = (port << 4) + (isdigit(*str) ? (*str - '0') : (toupper(*str) - 'A' + 10));
			} else {
				for (port = 0; isdigit(*str); str++)
					port = (port * 10) + (*str - '0');
			}
	}
	
	// return results
	if (hostp) *hostp = host;
	if (maskp) *maskp = subnetmask;
	if (portp) *portp = port;
}

//
//	BACnetIPAddr::operator char *
//

char * BACnetIPAddr::AddressToString( void )
{
	static char	buff[64]
	;
	
	sprintf( buff, "%d.%d.%d.%d:%d"
		, addrAddr[0], addrAddr[1], addrAddr[2], addrAddr[3]
		, (addrAddr[4] * 256) + addrAddr[5]
		);
	
	return buff;
}
