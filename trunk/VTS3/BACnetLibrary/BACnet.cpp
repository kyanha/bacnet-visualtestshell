
#include "stdafx.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#if (__DECCXX)
#include "cvtdef.h"
#include <ssdef.h>

extern "C" {
int cvt$convert_float( const void *, int, void *, int, int );
}
#endif

#include "BACnet.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#else

int stricmp( const char *, const char * );

int stricmp( const char *a, const char *b )
{
	while (*a && *b) {
		int cmp = (tolower(*b++) - tolower(*a++));
		if (cmp != 0)
			return (cmp < 0 ? -1 : 1);
	}
	if (*a)
		return 1;
	else
	if (*b)
		return -1;
	else
		return 0;
}

#endif

#define	VTSScanner		0

#if VTSScanner

namespace NetworkSniffer {
	extern char *BACnetObjectType[];
}

#include "ScriptBase.h"
#include "ScriptKeywords.h"

#endif

//
//	BACnetAddress
//

BACnetAddress::BACnetAddress( const unsigned char *addr, const unsigned short len )
	: addrType(localStationAddr), addrNet(0), addrLen(len)
{
	if (len && addr)
		memcpy( addrAddr, addr, addrLen );
}

BACnetAddress::BACnetAddress( const unsigned short net, const unsigned char *addr, const unsigned short len )
	: addrType(remoteStationAddr), addrNet(net), addrLen(len)
{
	if (len && addr)
		memcpy( addrAddr, addr, addrLen );
}

BACnetAddress::BACnetAddress( const BACnetAddressType typ, const unsigned short net, const unsigned char *addr, const unsigned short len )
	: addrType(typ), addrNet(net), addrLen(len)
{
	if (len && addr)
		memcpy( addrAddr, addr, addrLen );
}

void BACnetAddress::LocalStation( const unsigned char *addr, const unsigned short len )
{
	addrType = localStationAddr;
	addrNet = 0;
	addrLen = len;
	memcpy( addrAddr, addr, addrLen );
}

void BACnetAddress::RemoteStation( const unsigned short net, const unsigned char *addr, const unsigned short len )
{
	addrType = remoteStationAddr;
	addrNet = net;
	addrLen = len;
	memcpy( addrAddr, addr, addrLen );
}

void BACnetAddress::LocalBroadcast( void )
{
	addrType = localBroadcastAddr;
	addrNet = 0;
	addrLen = 0;
}

void BACnetAddress::RemoteBroadcast( const short net )
{
	addrType = remoteBroadcastAddr;
	addrNet = net;
	addrLen = 0;
}

void BACnetAddress::GlobalBroadcast( void )
{
	addrType = globalBroadcastAddr;
	addrNet = 0;
	addrLen = 0;
}

int operator ==( const BACnetAddress &addr1, const BACnetAddress &addr2 )
{
	// address types must match
	if (addr1.addrType != addr2.addrType)
		return 0;
	
	// remote broadcast and remote station have a network, localStation and remote
	// station have an address.
	switch (addr1.addrType) {
		case remoteBroadcastAddr:
			if (addr1.addrNet != addr1.addrNet) return 0;
			break;
			
		case remoteStationAddr:
			if (addr1.addrNet != addr1.addrNet) return 0;
		case localStationAddr:
			if (addr1.addrLen != addr2.addrLen) return 0;
			for (int i = 0; i < addr1.addrLen; i++)
				if (addr1.addrAddr[i] != addr2.addrAddr[i])
					return 0;
	}
	
	// must be equal
	return 1;
}

#if _TSMDebug
//
//	operator <<(ostream &strm,const BACnetAddress &addr)
//

ostream &operator <<(ostream &strm,const BACnetAddress &addr)
{
	const static char hex[] = "0123456789ABCDEF"
	;
	int		i
	;
	
	strm << '[';
	
	switch (addr.addrType) {
		case nullAddr:
			break;
			
		case remoteStationAddr:
			strm << addr.addrNet << ':';
		case localStationAddr:
			strm << "0x";
			for (i = 0; i < addr.addrLen; i++) {
				strm << hex[ addr.addrAddr[i] >> 4 ];
				strm << hex[ addr.addrAddr[i] & 0x0F ];
			}
			break;
			
		case localBroadcastAddr:
			strm << '*';
			break;
			
		case remoteBroadcastAddr:
			strm << addr.addrNet << ":*";
			break;
			
		case globalBroadcastAddr:
			strm << "*:*";
			break;
	}
	
	strm << ']';
	
	return strm;
}
#endif

//
//	BACnetEncodeable
//

void BACnetEncodeable::Encode( char * )
{
	throw (-1) /*not implemented */;
}

void BACnetEncodeable::Decode( const char * )
{
	throw (-1) /*not implemented */;
}

void BACnetEncodeable::Peek( BACnetAPDUDecoder& dec )
{
	int					saveLen = dec.pktLength
	;
	const BACnetOctet	*saveBuff = dec.pktBuffer
	;
	
	// use regular decoder
	Decode( dec );

	// restore pointer and length
	dec.pktLength = saveLen;
	dec.pktBuffer = saveBuff;
}

//
//	BACnetNull
//

void BACnetNull::Encode( BACnetAPDUEncoder &enc, int context )
{
	// check for space
	enc.CheckSpace( 1 );
	
	// encode it
	if (context != kAppContext)
		enc.pktBuffer[enc.pktLength++] = ((BACnetOctet)context << 4) + 0x08;
	else
		enc.pktBuffer[enc.pktLength++] = 0x00;
}

void BACnetNull::Decode( BACnetAPDUDecoder &dec )
{
	// enough for the tag byte?
	if (dec.pktLength < 1)
		throw (-1) /* not enough data */;
	
	// suck out the tag
	BACnetOctet	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// verify its a null
	if (((tag & 0x08) == 0) && ((tag & 0xF0) != 0x00))
		throw (-1) /* mismatched data type */;
}

void BACnetNull::Encode( char *enc )
{
	strcpy( enc, "Null" );
}

void BACnetNull::Decode( const char *dec )
{
	if (stricmp( dec, "Null") != 0)
		throw (-1) /* null must be 'null' */;
}

//
//	BACnetBoolean
//

BACnetBoolean::BACnetBoolean( int bvalu )
	: boolValue(bvalu ? bTrue : bFalse)
{
}

void BACnetBoolean::Encode( BACnetAPDUEncoder& enc, int context )
{
	//
	//	Note: A context tagged boolean is not encoded the same as an
	//	application tagged boolean.  See clause 20.2.3.
	//
	if (context == kAppContext) {
		enc.CheckSpace( 1 );
		
		enc.pktBuffer[enc.pktLength++] = 0x10 + (boolValue == bFalse ? 0x00 : 0x01);
	} else {
		enc.CheckSpace( 2 );
		
		enc.pktBuffer[enc.pktLength++] = ((BACnetOctet)context << 4) + 0x09;
		enc.pktBuffer[enc.pktLength++] = (boolValue == bFalse ? 0x00 : 0x01);
	}
}

void BACnetBoolean::Decode( BACnetAPDUDecoder &dec )
{
	BACnetOctet	tag
	;
	
	// enough for the tag byte?
	if (dec.pktLength < 1)
		throw (-1) /* not enough data */;
	
	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// it could be application tagged
	if (tag == 0x10) {
		boolValue = bFalse;
	} else
	if (tag == 0x11) {
		boolValue = bTrue;
	} else {
		// verify context tagged and length
		if ((tag & 0x0F) != 0x09)
			throw (-1) /* bad length */;
		
		// check for more data
		if (dec.pktLength < 1)
			throw (-1);
		
		boolValue = (eBACnetBoolean)(dec.pktLength--,*dec.pktBuffer++);
	}
}

void BACnetBoolean::Encode( char *enc )
{
	if (boolValue)
		strcpy( enc, "Set" );
	else
		strcpy( enc, "Reset" );
}

void BACnetBoolean::Decode( const char *dec )
{
	// check for a keyword
	switch (*dec++) {
		case '1':
		case 'S': case 's':				// Set
		case 'T': case 't':				// True
		case 'E': case 'e':				// Enable
			boolValue = bTrue;
			break;

		case '0':
		case 'R': case 'r':				// Reset
		case 'F': case 'f':				// False
		case 'D': case 'd':				// Disable
			boolValue = bFalse;
			break;

		case 'O': case 'o':
			boolValue = (eBACnetBoolean)((*dec == 'N') || (*dec == 'n'));	// On (all others are Off)
			break;

		default:
			throw (-1) /* unknown keyword */;
	}
}

//
//	BACnetEnumerated
//

BACnetEnumerated::BACnetEnumerated( int evalu )
	: enumValue( evalu )
{
}

void BACnetEnumerated::Encode( BACnetAPDUEncoder& enc, int context )
{
	int				len
	;
	unsigned long	valuCopy
	;
	
	// reduce the value to the smallest number of octets
	len = 4;
	valuCopy = (unsigned long)enumValue;
	while ((len > 1) && ((valuCopy & 0xFF000000) == 0)) {
		len -= 1;
		valuCopy = (valuCopy << 8);
	}
	
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, len ).Encode( enc );
	else
		BACnetAPDUTag( enumeratedAppTag, len ).Encode( enc );
	
	// fill in the data
	while (len--) {
		enc.pktBuffer[enc.pktLength++] = (valuCopy >> 24) & 0xFF;
		valuCopy = (valuCopy << 8);
	}
}

void BACnetEnumerated::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
	int				rslt
	;
	
	// extract the tag
	tag.Decode( dec );
	
	// check the type
	if (!tag.tagClass && (tag.tagNumber != enumeratedAppTag))
		throw (-1) /* mismatched data type */;
	
	// copy out the data
	rslt = 0;
	while (tag.tagLVT) {
		rslt = (rslt << 8) + (dec.pktLength--,*dec.pktBuffer++);
		tag.tagLVT -= 1;
	}
	
	// save the result
	enumValue = rslt;
}

void BACnetEnumerated::Encode( char *enc )
{
	Encode( enc, 0, 0 );
}

void BACnetEnumerated::Encode( char *enc, const char **table, int tsize )
{
	int		valu = enumValue
	;

	if ((enumValue < 0) || !table || (enumValue >= tsize))
		sprintf( enc, "%d", valu );
	else
		strcpy( enc, table[enumValue] );
}

void BACnetEnumerated::Decode( const char *dec )
{
	Decode( dec, 0, 0 );
}

void BACnetEnumerated::Decode( const char *dec, const char **table, int tsize )
{
	if (isdigit(*dec)) {										// explicit number
		// integer encoding
		for (enumValue = 0; *dec; dec++)
			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			else
				enumValue = (enumValue * 10) + (*dec - '0');
	} else
	if (!table)
		throw (-1) /* no translation available */;
	else {
		enumValue = 0;
		while (enumValue < tsize) {
			if (strncmp(dec,*table,strlen(dec)) == 0)
				break;
			if (stricmp(dec,*table) == 0)
				break;
			table += 1;
			enumValue += 1;
		}
		if (enumValue >= tsize)
			throw (-1) /* no matching translation */;
	}
}

//
//	BACnetUnsigned
//

BACnetUnsigned::BACnetUnsigned( unsigned int uivalu )
	: uintValue( uivalu )
{
}

void BACnetUnsigned::Encode( BACnetAPDUEncoder& enc, int context )
{
	int				len
	;
	unsigned long	valuCopy
	;
	
	// reduce the value to the smallest number of bytes
	len = 4;
	valuCopy = uintValue;
	while ((len > 1) && ((valuCopy & 0xFF000000) == 0)) {
		len -= 1;
		valuCopy = (valuCopy << 8);
	}
	
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, len ).Encode( enc );
	else
		BACnetAPDUTag( unsignedIntAppTag, len ).Encode( enc );
	
	// fill in the data
	while (len--) {
		enc.pktBuffer[enc.pktLength++] = (valuCopy >> 24) & 0xFF;
		valuCopy = (valuCopy << 8);
	}
}

void BACnetUnsigned::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
	unsigned long	rslt
	;
	
	// extract the tag
	tag.Decode( dec );
	
	// check the type
	if (!tag.tagClass && (tag.tagNumber != unsignedIntAppTag))
		throw (-1) /* mismatched data type */;
	
	// copy out the data
	rslt = 0;
	while (tag.tagLVT) {
		rslt = (rslt << 8) + (dec.pktLength--,*dec.pktBuffer++);
		tag.tagLVT -= 1;
	}
	
	// save the result
	uintValue = rslt;
}

void BACnetUnsigned::Encode( char *enc )
{
	// simple, effective
	sprintf( enc, "%u", uintValue );
}

void BACnetUnsigned::Decode( const char *dec )
{
	unsigned int	t
	;

	// figure out what encoding to use
	if (isdigit(*dec)) {										// nnn
		// integer encoding
		for (uintValue = 0; *dec; dec++)
			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			else
				uintValue = (uintValue * 10) + (*dec - '0');
	} else
	if ( ((dec[0] == 'D') && (dec[1] == '\''))					// D'nnn'
		|| ((dec[0] == 'd') && (dec[1] == '\''))
		) {
		// decimal encoding
		dec += 2;
		if (((strlen(dec) - 1) % 3) != 0)			// must be triplet
			throw (-1) /* must be triplet */;
		for (uintValue = 0; *dec != '\''; ) {
			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			t = (*dec++ - '0');

			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			t = (t * 10) + (*dec++ - '0');

			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			t = (t * 10) + (*dec++ - '0');

			uintValue = (uintValue * 256) + t;
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'x'))					// 0xFF, X'FF', &xFF
		|| ((dec[0] == 'X') && (dec[1] == '\''))
		|| ((dec[0] == 'x') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'x'))
		|| ((dec[0] == '&') && (dec[1] == 'X'))
		) {
		// hex encoding
		dec += 2;
		for (uintValue = 0; *dec && (*dec != '\''); dec++) {
			if (!isxdigit(*dec))
				throw (-1) /* invalid character */;
			uintValue = (uintValue * 16) + (isdigit(*dec) ? (*dec - '0') : (*dec - 'A' + 10));
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'o'))					// 0o377, O'377', &O377
		|| ((dec[0] == 'O') && (dec[1] == '\''))
		|| ((dec[0] == 'o') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'O'))
		|| ((dec[0] == '&') && (dec[1] == 'o'))
		) {
		// octal encoding
		dec += 2;
		for (uintValue = 0; *dec && (*dec != '\''); dec++) {
			if ((*dec < '0') || (*dec > '7'))
				throw (-1) /* invalid character */;
			uintValue = (uintValue * 16) + (*dec - '0');
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'b'))					// 0b11111111, B'11111111', &B11111111
		|| ((dec[0] == 'B') && (dec[1] == '\''))
		|| ((dec[0] == 'b') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'B'))
		|| ((dec[0] == '&') && (dec[1] == 'b'))
		) {
		// binary encoding
		dec += 2;
		for (uintValue = 0; *dec && (*dec != '\''); dec++) {
			if ((*dec < '0') || (*dec > '1'))
				throw (-1) /* invalid character */;
			uintValue = (uintValue * 2) + (*dec - '0');
		}
	} else
		throw (-1) /* unknown or invalid encoding */;
}

//
//	BACnetInteger
//

BACnetInteger::BACnetInteger( int ivalu )
	: intValue( ivalu )
{
}

void BACnetInteger::Encode( BACnetAPDUEncoder& enc, int context )
{
	int		len
	,		valuCopy
	;
	
	// reduce the value to the smallest number of bytes, be careful about 
	// the next upper bit down being sign extended
	len = 4;
	valuCopy = intValue;
	while (len > 1) {
		if ((intValue >= 0) && ((valuCopy & 0xFF800000) != 0x00000000))
			break;
		if ((intValue < 0) && ((valuCopy & 0xFF800000) != 0xFF800000))
			break;
		
		len -= 1;
		valuCopy = (valuCopy << 8);
	}
	
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, len ).Encode( enc );
	else
		BACnetAPDUTag( integerAppTag, len ).Encode( enc );
	
	// fill in the data
	while (len--) {
		enc.pktBuffer[enc.pktLength++] = (valuCopy >> 24) & 0x0FF;
		valuCopy = (valuCopy << 8);
	}
}

void BACnetInteger::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
	int				rslt
	;
	
	// extract the tag
	tag.Decode( dec );
	
	// check the type
	if (!tag.tagClass && (tag.tagNumber != integerAppTag))
		throw (-1) /* mismatched data type */;
	
	// check for sign extension
	if ((*dec.pktBuffer & 0x80) != 0)
		rslt = -1;
	else
		rslt = 0;
	
	// copy out the data
	while (tag.tagLVT) {
		rslt = (rslt << 8) + (dec.pktLength--,*dec.pktBuffer++);
		tag.tagLVT -= 1;
	}
	
	// save the result
	intValue = rslt;
}

void BACnetInteger::Encode( char *enc )
{
	// simple, effective
	sprintf( enc, "%d", intValue );
}

void BACnetInteger::Decode( const char *dec )
{
	bool				negValue = false
	;
	int					t
	;

	// look for a sign
	if (*dec == '-') {
		negValue = true;
		dec += 1;
	} else
	if (*dec == '+')
		dec += 1;

	// figure out what encoding to use
	if (isdigit(*dec)) {										// nnn
		// integer encoding
		for (intValue = 0; *dec; dec++)
			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			else
				intValue = (intValue * 10) + (*dec - '0');
	} else
	if ( ((dec[0] == 'D') && (dec[1] == '\''))					// D'nnn'
		|| ((dec[0] == 'd') && (dec[1] == '\''))
		) {
		// decimal encoding
		dec += 2;
		if (((strlen(dec) - 1) % 3) != 0)			// must be triplet
			throw (-1) /* must be triplet */;
		for (intValue = 0; *dec != '\''; ) {
			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			t = (*dec++ - '0');

			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			t = (t * 10) + (*dec++ - '0');

			if (!isdigit(*dec))
				throw (-1) /* invalid character */;
			t = (t * 10) + (*dec++ - '0');

			intValue = (intValue * 256) + t;
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'x'))					// 0xFF, X'FF', &xFF
		|| ((dec[0] == 'X') && (dec[1] == '\''))
		|| ((dec[0] == 'x') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'x'))
		|| ((dec[0] == '&') && (dec[1] == 'X'))
		) {
		// hex encoding
		dec += 2;
		for (intValue = 0; *dec && (*dec != '\''); dec++) {
			if (!isxdigit(*dec))
				throw (-1) /* invalid character */;
			intValue = (intValue * 16) + (isdigit(*dec) ? (*dec - '0') : (*dec - 'A' + 10));
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'o'))					// 0o377, O'377', &O377
		|| ((dec[0] == 'O') && (dec[1] == '\''))
		|| ((dec[0] == 'o') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'O'))
		|| ((dec[0] == '&') && (dec[1] == 'o'))
		) {
		// octal encoding
		dec += 2;
		for (intValue = 0; *dec && (*dec != '\''); dec++) {
			if ((*dec < '0') || (*dec > '7'))
				throw (-1) /* invalid character */;
			intValue = (intValue * 16) + (*dec - '0');
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'b'))					// 0b11111111, B'11111111', &B11111111
		|| ((dec[0] == 'B') && (dec[1] == '\''))
		|| ((dec[0] == 'b') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'B'))
		|| ((dec[0] == '&') && (dec[1] == 'b'))
		) {
		// binary encoding
		dec += 2;
		for (intValue = 0; *dec && (*dec != '\''); dec++) {
			if ((*dec < '0') || (*dec > '1'))
				throw (-1) /* invalid character */;
			intValue = (intValue * 2) + (*dec - '0');
		}
	} else
		throw (-1) /* unknown or invalid encoding */;

	// update for sign
	if (negValue)
		intValue = (intValue * -1);
}

//
//	BACnetReal
//

BACnetReal::BACnetReal( float rvalu )
	: realValue( rvalu )
{
}

void BACnetReal::Encode( BACnetAPDUEncoder& enc, int context )
{
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, 4 ).Encode( enc );
	else
		BACnetAPDUTag( realAppTag, 4 ).Encode( enc );
	
	// fill in the data
#if (__DECCXX)
	cvt$convert_float( &realValue, CVT$K_VAX_F
		, enc.pktBuffer+enc.pktLength, CVT$K_IEEE_S
		, CVT$M_ROUND_TO_NEAREST + CVT$M_BIG_ENDIAN
		);
	enc.pktLength += 4;
#else
#ifdef _MSC_VER
	unsigned long cpy = *(unsigned long *)&realValue;
	for (int j = 3; j >= 0; j--)
		enc.pktBuffer[enc.pktLength++] = (cpy >> (j * 8)) & 0xFF;
#else
	memcpy( enc.pktBuffer+enc.pktLength, &realValue, (size_t)4 );
	enc.pktLength += 4;
#endif
#endif
}

void BACnetReal::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
		
	// verify the tag can be extracted
	tag.Decode( dec );
	
	// check the type and length
	if (!tag.tagClass && (tag.tagNumber != realAppTag))
		throw (-1) /* mismatched data type */;
	if (tag.tagLVT != 4)
		throw (-1) /* four bytes of data expected */;
	
	// copy out the data
#if (__DECCXX)
	cvt$convert_float( dec.pktBuffer, CVT$K_IEEE_S
		, &realValue, CVT$K_VAX_F
		, CVT$M_ROUND_TO_NEAREST + CVT$M_BIG_ENDIAN
		);
	dec.pktBuffer += 4;
	dec.pktLength -= 4;
#else
#ifdef _MSC_VER
	unsigned long cpy = 0;
	for (int j = 3; dec.pktLength && j >= 0; j--)
		cpy = (cpy << 8) + (dec.pktLength--,*dec.pktBuffer++);
	realValue = *(float *)&cpy;
#else
	memcpy( &realValue, dec.pktBuffer, (size_t)4 );
	dec.pktBuffer += 4;
	dec.pktLength -= 4;
#endif
#endif
}

void BACnetReal::Encode( char *enc )
{
	// simple, effective
	sprintf( enc, "%f", realValue );
}

void BACnetReal::Decode( const char *dec )
{
	// check for valid format
	if (sscanf( dec, "%f", &realValue ) != 1)
		throw (-1) /* format error */;
}

//
//	BACnetDouble
//

BACnetDouble::BACnetDouble( double dvalu )
	: doubleValue( dvalu )
{
}

void BACnetDouble::Encode( BACnetAPDUEncoder& enc, int context )
{
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, 8 ).Encode( enc );
	else
		BACnetAPDUTag( doubleAppTag, 8 ).Encode( enc );
	
	// fill in the data
#if (__DECCXX)
	cvt$convert_float( &doubleValue, CVT$K_VAX_G
		, enc.pktBuffer+enc.pktLength, CVT$K_IEEE_T
		, CVT$M_ROUND_TO_NEAREST + CVT$M_BIG_ENDIAN
		);
	enc.pktLength += 8;
#else
#ifdef _MSC_VER
	union {
		double	src;
		struct {
			unsigned long	t1, t2;
		}	s;
	}		x;

	x.src = doubleValue;
	for (int j = 3; j >= 0; j--)
		enc.pktBuffer[enc.pktLength++] = (x.s.t1 >> (j * 8)) & 0xFF;
	for (int k = 3; k >= 0; k--)
		enc.pktBuffer[enc.pktLength++] = (x.s.t2 >> (k * 8)) & 0xFF;
#else
	memcpy( enc.pktBuffer+enc.pktLength, &doubleValue, (size_t)8 );
	enc.pktLength += 8;
#endif
#endif
}

void BACnetDouble::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
		
	// verify the tag can be extracted
	tag.Decode( dec );
	
	// check the type and length
	if (!tag.tagClass && (tag.tagNumber != doubleAppTag))
		throw (-1) /* mismatched data type */;
	if (tag.tagLVT != 8)
		throw (-1) /* eight bytes of data expected */;
	
	// copy out the data
#if (__DECCXX)
	cvt$convert_float( dec.pktBuffer, CVT$K_IEEE_T
		, &doubleValue, CVT$K_VAX_G
		, CVT$M_ROUND_TO_NEAREST + CVT$M_BIG_ENDIAN
		);
	dec.pktBuffer += 8;
	dec.pktLength -= 8;
#else
#ifdef _MSC_VER
	union {
		double	src;
		struct {
			unsigned long	t1, t2;
		}	s;
	}		x;

	x.src = doubleValue;
	for (int j = 3; dec.pktLength && j >= 0; j--)
		x.s.t1 = (x.s.t1 << 8) + (dec.pktLength--,*dec.pktBuffer++);
	for (int k = 3; dec.pktLength && k >= 0; k--)
		x.s.t2 = (x.s.t2 << 8) + (dec.pktLength--,*dec.pktBuffer++);
	doubleValue = x.src;
#else
	memcpy( &doubleValue, dec.pktBuffer, (size_t)8 );
	dec.pktBuffer += 8;
	dec.pktLength -= 8;
#endif
#endif
}

void BACnetDouble::Encode( char *enc )
{
	// simple, effective
	sprintf( enc, "%lf", doubleValue );
}

void BACnetDouble::Decode( const char *dec )
{
	// check for valid format
	if (sscanf( dec, "%lf", &doubleValue ) != 1)
		throw (-1) /* format error */;
}

//
//	BACnetCharacterString
//

BACnetCharacterString::BACnetCharacterString( char *svalu )
	: strEncoding(0)
{
	strLen = (svalu ? strlen(svalu) : 0);
	strBuff = new BACnetOctet[strLen];
	if (svalu)
		memcpy( strBuff, svalu, (size_t)strLen );
}

BACnetCharacterString::~BACnetCharacterString( void )
{
	delete[] strBuff;
}

void BACnetCharacterString::SetValue( char *svalu, int enc )
{
	// toss the old stuff
	delete[] strBuff;

	// copy in the new
	strEncoding = enc;
	strLen = (svalu ? strlen(svalu) : 0);
	strBuff = new BACnetOctet[strLen];
	if (svalu)
		memcpy( strBuff, svalu, (size_t)strLen );
}

bool BACnetCharacterString::Equals( const char *valu )
{
	// can't compare against non-ASCII strings
	if (strEncoding != 0)
		return false;

	// not the same if different lengths
	if (strlen(valu) != strLen)
		return false;

	// case insensitive, strBuff is not null-terminated
	for (int i = 0; i < strLen; i++)
		if (tolower(strBuff[i]) != tolower(valu[i]))
			return false;

	// success
	return true;
}

void BACnetCharacterString::Encode( BACnetAPDUEncoder& enc, int context )
{
	int				len = strLen + 1
	;
	
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, len ).Encode( enc );
	else
		BACnetAPDUTag( characterStringAppTag, len ).Encode( enc );
	
	// fill in the data
	enc.pktBuffer[enc.pktLength++] = strEncoding;
	len -= 1;
	memcpy( enc.pktBuffer+enc.pktLength, strBuff, (size_t)len );
	enc.pktLength += len;
}

void BACnetCharacterString::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
		
	// verify the tag can be extracted
	tag.Decode( dec );
	
	// check the type and length
	if (!tag.tagClass && (tag.tagNumber != characterStringAppTag))
		throw (-1) /* mismatched data type */;

	// extract the encoding
	strEncoding = (dec.pktLength--,*dec.pktBuffer++);
	
	// skip the encoding and set the length
	tag.tagLVT -= 1;
	strLen = tag.tagLVT;

	// allocate a new buffer
	delete[] strBuff;
	strBuff = new BACnetOctet[ strLen ];

	// copy out the data, null terminated
	memcpy( strBuff, dec.pktBuffer, (size_t)strLen );
	dec.pktBuffer += strLen;
	dec.pktLength -= strLen;
}

void BACnetCharacterString::Encode( char *enc )
{
	static char hex[] = "0123456789ABCDEF"
	;

	// check for the simple case
	if (strEncoding == 0) {
		*enc++ = '"';

		// simple contents
		const char *src = (char *)strBuff;
		for (int i = 0; i < strLen; i++, src++)
			if (*src < ' ') {
				*enc++ = '\\';
				*enc++ = 'x';
				*enc++ = hex[ *src >> 4 ];
				*enc++ = hex[ *src & 0x0F ];
			} else
			if (*src == '\"') {
				*enc++ = '\\';
				*enc++ = '\"';
			} else
				*enc++ = *src;

		*enc++ = '"';
		*enc = 0;
	} else {
		switch (strEncoding) {
			case 1:
				strcpy( enc, "IBM-MICROSOFT-DBCS, X'" );
				break;
			case 2:
				strcpy( enc, "JIS-C-6226, X'" );
				break;
			case 3:
				strcpy( enc, "UCS-4, X'" );
				break;
			case 4:
				strcpy( enc, "UCS-2, X'" );
				break;
			case 5:
				strcpy( enc, "ISO-8859-1, X'" );
				break;
			default:
				sprintf( enc, "%d, X'", strEncoding );
				break;
		}
		while (*enc) enc++;

		// encode the content
		for (int i = 0; i < strLen; i++) {
			*enc++ = hex[ strBuff[i] >> 4 ];
			*enc++ = hex[ strBuff[i] & 0x0F ];
		}

		*enc++ = '\'';
		*enc = 0;
	}
}

void BACnetCharacterString::Decode( const char *dec )
{
	char		c = 0
	,			*dst
	;
	const char	*src
	;

	// check for explicit ASCII string
	if ((*dec == 'A') && (*(dec+1) == '\''))
		dec += 1;

	// if the first character is a quote, process as a normal string
	if ((*dec == '\"') || (*dec == '\'')) {
		strEncoding = 0;
		c = *dec++;

		// look for the close and count
		strLen = 0;
		src = dec;
		while (*src && (*src != c)) {
			if (*src == '\\') {
				if (tolower(*(src+1)) == 'x')
					src += 4;
				else
					src += 2;
			} else
				src += 1;
			strLen++;
		}

		// allocate a new buffer
		delete[] strBuff;
		strBuff = new BACnetOctet[ strLen ];

		// copy the data
		src = dec;
		dst = (char *)strBuff;
		while (*src && (*src != c))
			if (*src == '\\') {
				if (tolower(*(src+1)) == 'x') {
					src += 2;
					*dst = (isdigit(*src) ? *src - '0' : (toupper(*src) - 'A') + 10) << 4;
					src += 1;
					*dst++ += (isdigit(*src) ? *src - '0' : (toupper(*src) - 'A') + 10);
					src += 1;
				} else {
					src += 1;
					*dst++ = *src++;
				}
			} else
				*dst++ = *src++;
	} else {
#if VTSScanner
		int		encType
		;

		// create a scanner bound to the text
		ScriptScanner	scan( dec );
		ScriptToken		tok;

		// get something
		scan.Next( tok );

		// if a hex string was given, allow default to ASCII
		if ((tok.tokenType == scriptValue) && (tok.tokenEnc == scriptHexEnc))
			strEncoding = 0;
		else {
			if (!tok.IsInteger( encType, ScriptCharacterTypeMap ))
				throw (-1) /* encoding type keyword expected */;
			else
			if ((encType < 0) || (encType > 255))
				throw (-1) /* out of range */;
			strEncoding = encType;

			// get the next token
			scan.Next( tok );

			// skip the comma if it was entered
			if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == ','))
				scan.Next( tok );
		}

		// if the next beast is an ASCII or hex string
		if ((tok.tokenType == scriptValue) && (tok.tokenEnc == scriptASCIIEnc)) {
			int saveEncoding = strEncoding;
			Decode( tok.tokenValue );
			strEncoding = saveEncoding;
		} else
		if ((tok.tokenType == scriptValue) && (tok.tokenEnc == scriptHexEnc)) {
			BACnetOctetString	ostr;
			ostr.Decode( tok.tokenValue );

			// build a new buffer
			delete[] strBuff;
			strLen = ostr.strLen;
			strBuff = new BACnetOctet[ strLen ];
			memcpy( strBuff, ostr.strBuff, (size_t)strLen );
		} else
			throw (-1) /* ASCII or hex string expected */;
#else
		throw (-1) /* not implemented */;
#endif
	}
}

//
//	BACnetOctetString::BACnetOctetString
//

BACnetOctetString::BACnetOctetString( void )
	: strLen(0), strBuffLen(0), strBuff(0)
{
}

//
//	BACnetOctetString::BACnetOctetString
//

BACnetOctetString::BACnetOctetString( int len )
{
	if (len > 0) {
		strLen = 0;
		strBuff = new BACnetOctet[ len ];
		strBuffLen = len;
	} else {
		strLen = strBuffLen = 0;
		strBuff = 0;
	}
}

//
//	BACnetOctetString::BACnetOctetString
//

BACnetOctetString::BACnetOctetString( const BACnetOctetString &cpy )
{
	strLen = strBuffLen = cpy.strLen;
	strBuff = new BACnetOctet[ cpy.strLen ];
	if (strBuff)
		memcpy( strBuff, cpy.strBuff, (size_t)cpy.strLen );
}

//
//	BACnetOctetString::BACnetOctetString
//

BACnetOctetString::BACnetOctetString( BACnetOctet *bytes, int len )
{
	strLen = len;
	strBuff = bytes;
	strBuffLen = 0;
}

//
//	BACnetOctetString::~BACnetOctetString
//

BACnetOctetString::~BACnetOctetString( void )
{
	Flush();
}

//
//	BACnetOctetString::PrepBuffer
//

void BACnetOctetString::PrepBuffer( int size )
{
	BACnetOctet	*newBuff
	;
	const int	blockFactor = 512
	;
	int			newSize = size + (blockFactor - (size % blockFactor))
	;

	// allocate a new buffer big enough
	newBuff = new BACnetOctet[ newSize ];

	// if there is existing data, copy it
	if (strBuff && strLen)
		memcpy( newBuff, strBuff, strLen );

	// if current buffer owned, delete it
	if (strBuff && strBuffLen)
		delete[] strBuff;

	// point to the new buffer
	strBuff = newBuff;
	strBuffLen = newSize;
}

//
//	BACnetOctetString::Flush
//

void BACnetOctetString::Flush( void )
{
	if (strBuff && strBuffLen)
		delete[] strBuff;

	strBuff = 0;
	strLen = strBuffLen = 0;
}

//
//	BACnetOctetString::Append
//

void BACnetOctetString::Append( BACnetOctet byte )
{
	// make sure the buffer can handle it
	if (strLen + 1 > strBuffLen)
		PrepBuffer( strLen + 1 );

	// append the data
	strBuff[strLen++] = byte;
}

//
//	BACnetOctetString::Insert
//

void BACnetOctetString::Insert( BACnetOctet *bytes, int len, int pos )
{
	// make sure the buffer can handle it
	if (strLen + len > strBuffLen)
		PrepBuffer( strLen + len );

	// move existing data out of the way
	if (pos < strLen)
		memmove( strBuff+pos+len, strBuff+pos, len );

	// copy in new data
	memcpy( strBuff+pos, bytes, len );
	strLen += len;
}

//
//	BACnetOctetString::Insert
//

void BACnetOctetString::Insert( const BACnetOctetString &cpy, int pos )
{
	Insert( cpy.strBuff, cpy.strLen, pos );
}

//
//	BACnetOctetString::operator []
//

BACnetOctet &BACnetOctetString::operator [](const int indx)
{
	return strBuff[indx];
}

//
//	BACnetOctetString::Reference
//

void BACnetOctetString::Reference( BACnetOctet *bytes, int len )
{
	if (strBuff && strBuffLen)
		delete[] strBuff;

	strBuff = bytes;
	strLen = len;
	strBuffLen = 0;
}

int BACnetOctetString::Length( void )
{
	return strLen;
}

void BACnetOctetString::Encode( BACnetAPDUEncoder& enc, int context )
{
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, strLen ).Encode( enc );
	else
		BACnetAPDUTag( octetStringAppTag, strLen ).Encode( enc );
	
	// fill in the data
	memcpy( enc.pktBuffer+enc.pktLength, strBuff, (size_t)strLen );
	enc.pktLength += strLen;
}

void BACnetOctetString::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
		
	// verify the tag can be extracted
	tag.Decode( dec );
	
	// check the type and length
	if (!tag.tagClass && (tag.tagNumber != octetStringAppTag))
		throw (-1) /* mismatched data type */;
	
	// check for space
	if ((strBuffLen != 0) && (strBuffLen != tag.tagLVT)) {
		delete[] strBuff;
		strLen = strBuffLen = tag.tagLVT;
		strBuff = new BACnetOctet[ tag.tagLVT ];
	} else
	if (!strBuff) {
		strLen = strBuffLen = tag.tagLVT;
		strBuff = new BACnetOctet[ tag.tagLVT ];
	}
	
	// copy the data
	strLen = tag.tagLVT;
	memcpy( strBuff, dec.pktBuffer, (size_t)strLen );
	dec.pktBuffer += tag.tagLVT;
	dec.pktLength -= tag.tagLVT;
}

void BACnetOctetString::Encode( char *enc )
{
	static char hex[] = "0123456789ABCDEF"
	;

	*enc++ = 'X';
	*enc++ = '\'';

	// encode the content
	for (int i = 0; i < strLen; i++) {
		*enc++ = hex[ strBuff[i] >> 4 ];
		*enc++ = hex[ strBuff[i] & 0x0F ];
	}

	*enc++ = '\'';
	*enc = 0;
}

void BACnetOctetString::Decode( const char *dec )
{
	int		upperNibble, lowerNibble
	;
	char	c
	;

	// toss existing content
	Flush();

	// skip preamble
	if ( ((dec[0] == '0') && (dec[1] == 'x'))					// 0xFF, X'FF', &xFF
		|| ((dec[0] == 'X') && (dec[1] == '\''))
		|| ((dec[0] == 'x') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'x'))
		|| ((dec[0] == '&') && (dec[1] == 'X'))
		)
		dec += 2;

	while (*dec && (*dec != '\'')) {
		c = toupper( *dec++ );
		if (!isxdigit(c))
			throw (-1) /* invalid character */;
		upperNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

		c = toupper( *dec++ );
		if (!isxdigit(c))
			throw (-1) /* invalid character */;
		lowerNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

		// stick this on the end
		Append( (upperNibble << 4) + lowerNibble );
	}
}

//
//	BACnetBitString::BACnetBitString
//

BACnetBitString::BACnetBitString( void )
	: bitLen(0), bitBuffLen(0), bitBuff(0)
{
}

//
//	BACnetBitString::BACnetBitString
//

BACnetBitString::BACnetBitString( int siz )
	: bitLen(siz), bitBuffLen((siz + 31) / 32), bitBuff(0)
{
	bitBuff = new unsigned long[ bitBuffLen ];
	if (bitBuff)
		for (int i = 0; i < bitBuffLen; i++)
			bitBuff[i] = 0;
}

//
//	BACnetBitString::BACnetBitString
//

BACnetBitString::BACnetBitString( const BACnetBitString &cpy )
	: bitLen( cpy.bitLen ), bitBuffLen( cpy.bitBuffLen )
{
	unsigned long	*src, *dst
	;
	
	src = cpy.bitBuff;
	dst = bitBuff = new unsigned long[ bitBuffLen ];
	for (int i = 0; i < bitBuffLen; i++)
		*src++ = *dst++;
}

//
//	BACnetBitString::~BACnetBitString
//

BACnetBitString::~BACnetBitString( void )
{
	delete[] bitBuff;
}

//
//	BACnetBitString::SetSize
//

void BACnetBitString::SetSize( int siz )
{
	int		newBuffLen = (siz + 31) / 32
	;
	
	if (newBuffLen != bitBuffLen) {
		int				i
		;
		unsigned long	*src = bitBuff
		,				*dst
		,				*rslt = new unsigned long[newBuffLen]
		;
		
		dst = rslt;
		for (i = 0; (i < bitBuffLen) && (i++ < newBuffLen); i++)
			*dst++ = *src++;
		while (i++ < newBuffLen)
			*dst++ = 0;
		
		delete[] bitBuff;
		bitBuff = rslt;
		bitBuffLen = newBuffLen;
	}
	bitLen = siz;
}

//
//	BACnetBitString::SetBit
//

void BACnetBitString::SetBit( int bit, int valu )
{
	int		intOffset = (bit / 32)
	,		bitOffset = 31 - (bit % 32)
	;
	
	if (bit >= bitLen)
		SetSize( bit+1 );
	if (bitBuff)
		if (valu)
			bitBuff[intOffset] |= (1 << bitOffset);
		else
			bitBuff[intOffset] &= 0xFFFFFFFF - (1 << bitOffset);
}

//
//	BACnetBitString::ResetBit
//

void BACnetBitString::ResetBit( int bit )
{
	int		intOffset = (bit / 32)
	,		bitOffset = 31 - (bit % 32)
	;
	
	if ((bit < bitLen) && bitBuff)
		bitBuff[intOffset] &= 0xFFFFFFFF - (1 << bitOffset);
}

//
//	BACnetBitString::GetBit
//

int BACnetBitString::GetBit( int bit )
{
	int		intOffset = (bit / 32)
	,		bitOffset = 31 - (bit % 32)
	;
	
	if ((bit >= bitLen) || (!bitBuff))
		return 0;
	
	return ((bitBuff[intOffset] >> bitOffset) & 0x01);
}

//
//	BACnetBitString::operator []
//

const int BACnetBitString::operator [](int bit)
{
	int		intOffset = (bit / 32)
	,		bitOffset = 31 - (bit % 32)
	;
	
	if ((bit >= bitLen) || (!bitBuff))
		return 0;
	
	return ((bitBuff[intOffset] >> bitOffset) & 0x01);
}

//
//	BACnetBitString::operator +=
//

BACnetBitString &BACnetBitString::operator +=( const int bit )
{
	int		intOffset = (bit / 32)
	,		bitOffset = 31 - (bit % 32)
	;
	
	if (bit >= bitLen)
		SetSize( bit+1 );
	if (bitBuff)
		bitBuff[intOffset] |= (1 << bitOffset);
	return *this;
}

//
//	BACnetBitString::operator -=
//

BACnetBitString &BACnetBitString::operator -=( const int bit )
{
	int		intOffset = (bit / 32)
	,		bitOffset = 31 - (bit % 32)
	;
	
	if ((bit < bitLen) && bitBuff)
		bitBuff[intOffset] &= 0xFFFFFFFF - (1 << bitOffset);
	return *this;
}

//
//	BACnetBitString::operator =
//

BACnetBitString &BACnetBitString::operator =( const BACnetBitString &arg )
{
	int		i
	;
	
	if (bitLen < arg.bitLen)
		SetSize( arg.bitLen );
	
	unsigned long	*src = arg.bitBuff
	,				*dst = bitBuff
	;
	
	for (i = 0; i < arg.bitBuffLen; i++)
		*dst++ = *src++;
	while (i++ < bitBuffLen)
		*dst++ = 0;
	bitLen = arg.bitLen;
	
	return *this;
}

//
//	BACnetBitString::operator |=
//

BACnetBitString &BACnetBitString::operator |=( const BACnetBitString &arg )
{
	if (bitLen < arg.bitLen)
		SetSize( arg.bitLen );
	
	unsigned long	*src = arg.bitBuff
	,				*dst = bitBuff
	;
	
	for (int i = 0; i < arg.bitBuffLen; i++)
		*dst++ |= *src++;
	
	return *this;
}

//
//	BACnetBitString::operator &=
//

BACnetBitString &BACnetBitString::operator &=( const BACnetBitString &arg )
{
	int				i
	,				siz = (bitBuffLen < arg.bitBuffLen ? bitBuffLen : arg.bitBuffLen)
	;
	unsigned long	*src = arg.bitBuff
	,				*dst = bitBuff
	;
		
	for (i = 0; i < siz; i++)
		*dst++ &= *src++;
	while (i++ < bitBuffLen)
		*dst++ = 0;
	
	return *this;
}

//
//	BACnetBitString::operator ==
//

bool BACnetBitString::operator ==( const BACnetBitString &arg )
{
	int				i
	,				siz = (bitBuffLen < arg.bitBuffLen ? bitBuffLen : arg.bitBuffLen)
	;
	unsigned long	*src = arg.bitBuff
	,				*dst = bitBuff
	;
		
	if (bitLen != arg.bitLen)
		return false;

	for (i = 0; i < siz; i++)
		if (*dst++ != *src++)
			return false;

	// ### perhaps last bitBuff element shouldn't always have all bits compared

	return true;
}

void BACnetBitString::Encode( BACnetAPDUEncoder& enc, int context )
{
	int				len = (bitLen + 7) / 8 + 1
	;
	
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, len ).Encode( enc );
	else
		BACnetAPDUTag( bitStringAppTag, len ).Encode( enc );
	
	// fill in the data
	enc.pktBuffer[enc.pktLength++] = 8 - (((bitLen + 7) % 8) + 1);
	len -= 1;

#ifdef _MSC_VER
	int i = 0;
	while (len) {
		int cpy = bitBuff[i++];
		for (int j = 3; len && j >= 0; j--, len--)
			enc.pktBuffer[enc.pktLength++] = (cpy >> (j * 8)) & 0xFF;
	}
#else
	memcpy( enc.pktBuffer+enc.pktLength, bitBuff, (size_t)len );
	enc.pktLength += len;
#endif
}

void BACnetBitString::Decode( BACnetAPDUDecoder& dec )
{
	int				bLen
	;
	BACnetAPDUTag	tag
	;
		
	// verify the tag can be extracted
	tag.Decode( dec );
	
	// check the type and length
	if (!tag.tagClass && (tag.tagNumber != bitStringAppTag))
		throw (-1) /* mismatched data type */;
	
	// make sure the destination has enough space
	bLen = ((tag.tagLVT - 1) * 8 - (dec.pktLength--,*dec.pktBuffer++));
	if (bLen > bitLen)
		SetSize( bLen );
	tag.tagLVT -= 1;
	
	if (!bitBuff)
		throw (-1) /* destination too small */;
	
	// copy out the data, null terminated
#ifdef _MSC_VER
	int i = 0;
	while (dec.pktLength) {
		bitBuff[i] = 0;
		for (int j = 3; dec.pktLength && j >= 0; j--)
			bitBuff[i] |= (dec.pktLength--,*dec.pktBuffer++) << (j * 8);
		i += 1;
	}
#else
	memcpy( bitBuff, dec.pktBuffer, (size_t)tag.tagLVT );

	bitLen = bLen;
	dec.pktBuffer += tag.tagLVT;
	dec.pktLength -= tag.tagLVT;
#endif
}

void BACnetBitString::Encode( char *enc )
{
	*enc++ = 'B';
	*enc++ = '\'';

	// encode the content
	if (bitBuff) {
		int bit = 0;
		for (int i = 0; bit < bitLen; i++) {
			int x = bitBuff[i];
			for (int j = 0; (bit < bitLen) && (j < 32); j++) {
				*enc++ = '0' + (x & 0x01);
				x = (x >> 1);
				bit += 1;
			}
		}
	}

	*enc++ = '\'';
	*enc = 0;
}

void BACnetBitString::Decode( const char *dec )
{
	int		bit
	;
	char	c
	;

	// toss existing content
	SetSize( 0 );

	// skip preamble
	if ( ((dec[0] == '0') && (dec[1] == 'b'))					// 0b1101, B'1101', &b1101
		|| ((dec[0] == 'B') && (dec[1] == '\''))
		|| ((dec[0] == 'b') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'b'))
		|| ((dec[0] == '&') && (dec[1] == 'B'))
		)
		dec += 2;

	bit = 0;
	while (*dec && (*dec != '\'')) {
		c = *dec++;
		if (c == '0')
			SetBit( bit, 0 );
		else
		if (c == '1')
			SetBit( bit, 1 );
		else
			throw (-1) /* invalid character */;
		bit += 1;
	}
}

//
//	BACnetDate::BACnetDate
//

BACnetDate::BACnetDate( void )
{
	time_t		now
	;
	struct tm	*currtime
	;
	
	time( &now );
	currtime = localtime( &now );
	
	year = currtime->tm_year;
	month = currtime->tm_mon + 1;
	day = currtime->tm_mday;
	
	CalcDayOfWeek();
}

//
//	BACnetDate::BACnetDate
//

BACnetDate::BACnetDate( int y, int m, int d )
	: year(y), month(m), day(d)
{
	CalcDayOfWeek();
}

//
//	BACnetDate::CalcDayOfWeek
//

void BACnetDate::CalcDayOfWeek( void )
{
	int		a, b, c, y, m
	;
	
	// dont even try with "unspecified" or "don't care" values
	if ((year == 255) || (month == 255) || (day == 255)) {
		dayOfWeek = 255;
		return;
	}
	
	// thanks Dr McKenna!
	a = (month <= 2) ? 1 : 0;
	y = year + 1900 - a;
	m = month + 10 * a - 2 * (1 - a);
	c = y / 100;
	a = y % 100;
	b = ((13 * m - 1) / 5) + (a / 4) + (c / 4);
	dayOfWeek = (b + a + day - 2 * c) % 7;		// 0=Sunday, ...
	
	// translate to BACnet order, 1=Monday, ...
	dayOfWeek = (dayOfWeek + 6) % 7 + 1;
}

void BACnetDate::Encode( BACnetAPDUEncoder& enc, int context )
{
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, 4 ).Encode( enc );
	else
		BACnetAPDUTag( dateAppTag, 4 ).Encode( enc );
	
	// fill in the data
	enc.pktBuffer[enc.pktLength++] = year;
	enc.pktBuffer[enc.pktLength++] = month;
	enc.pktBuffer[enc.pktLength++] = day;
	enc.pktBuffer[enc.pktLength++] = dayOfWeek;
}

void BACnetDate::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
		
	// verify the tag can be extracted
	tag.Decode( dec );
	
	// check the type and length
	if (!tag.tagClass && (tag.tagNumber != dateAppTag))
		throw (-1) /* mismatched data type */;
	if (tag.tagLVT != 4)
		throw (-1) /* four bytes of data expected */;
	
	// copy out the data
	year		= *dec.pktBuffer++;
	month		= *dec.pktBuffer++;
	day			= *dec.pktBuffer++;
	dayOfWeek	= *dec.pktBuffer++;
	dec.pktLength -= 4;
}

void BACnetDate::Encode( char *enc )
{
	static char *dow[] = { "", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" }
	;

	// day of week
	if (dayOfWeek == 255)
		*enc++ = '?';
	else
	if ((dayOfWeek > 0) && (dayOfWeek < 8)) {
		strcpy( enc, dow[dayOfWeek] );
		enc += 3;
	} else
		*enc++ = '?';
	*enc++ = ' ';

	// month
	if (month == 255)
		*enc++ = '?';
	else {
		sprintf( enc, "%d", month );
		while (*enc) enc++;
	}
	*enc++ = '/';

	// day
	if (day == 255)
		*enc++ = '?';
	else {
		sprintf( enc, "%d", day );
		while (*enc) enc++;
	}
	*enc++ = '/';

	// year
	if (year == 255) {
		*enc++ = '?';
		*enc = 0;
	} else
		sprintf( enc, "%d", year + 1900 );
}

void BACnetDate::Decode( const char *dec )
{
	static char *dow[] = { "", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN" }
	;
	char		dowBuff[4]
	;

	// initialize everything to don't care
	dayOfWeek = month = day = year = 255;

	// skip blank on front
	while (*dec && isspace(*dec)) dec++;

	// check for dow
	if ((*dec == '*') || (*dec == '?'))
		dec += 1;
	else 
	if (isalpha(*dec)) {
		for (int j = 0; (j < 3) && isalpha(*dec); j++)
			dowBuff[j] = toupper(*dec++);
		dowBuff[3] = 0;

		for (int i = 1; i < 8; i++)
			if (strcmp(dowBuff,dow[i]) == 0) {
				dayOfWeek = i;
				break;
			}
		while (*dec && !isspace(*dec)) dec++;
	}
	while (*dec && isspace(*dec)) dec++;

	// skip over comma and more space
	if (*dec == ',') {
		dec += 1;
		while (*dec && isspace(*dec)) dec++;
	}

	// check for month
	if ((*dec == '*') || (*dec == '?'))
		dec += 1;
	else {
		for (month = 0; isdigit(*dec); dec++)
			month = (month * 10) + (*dec - '0');
	}
	if (*dec == '/')
		dec += 1;

	// check for day
	if ((*dec == '*') || (*dec == '?'))
		dec += 1;
	else {
		for (day = 0; isdigit(*dec); dec++)
			day = (day * 10) + (*dec - '0');
	}
	if (*dec == '/')
		dec += 1;

	// check for year
	if ((*dec == '*') || (*dec == '?'))
		dec += 1;
	else
	if (isdigit(*dec)) {
		int	yr;
		for (yr = 0; isdigit(*dec); dec++)
			yr = (yr * 10) + (*dec - '0');

		// 0..40 -> 2000..2040, 41.. -> 1941..
		if (yr < 40)
			year = yr + 100;
		else
		if (yr < 100)
			year = yr;
		else
		if ((yr >= 1900) && (yr <= (1900 + 254)))
			year = (yr - 1900);
		else
			year = 0;
	}
}

//
//	BACnetTime::BACnetTime
//

BACnetTime::BACnetTime( void )
{
	time_t		now
	;
	struct tm	*currtime
	;
	
	time( &now );
	currtime = localtime( &now );
	
	hour = currtime->tm_hour;
	minute = currtime->tm_min;
	second = currtime->tm_sec;
	hundredths = 0;
}

//
//	BACnetTime::BACnetTime
//

BACnetTime::BACnetTime( int hr, int mn, int sc, int hun )
	: hour(hr), minute(mn), second(sc), hundredths(hun)
{
}

void BACnetTime::Encode( BACnetAPDUEncoder& enc, int context )
{
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, 4 ).Encode( enc );
	else
		BACnetAPDUTag( timeAppTag, 4 ).Encode( enc );
	
	// fill in the data
	enc.pktBuffer[enc.pktLength++] = hour;
	enc.pktBuffer[enc.pktLength++] = minute;
	enc.pktBuffer[enc.pktLength++] = second;
	enc.pktBuffer[enc.pktLength++] = hundredths;
}

void BACnetTime::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
		
	// verify the tag can be extracted
	tag.Decode( dec );
	
	// check the type and length
	if (!tag.tagClass && (tag.tagNumber != timeAppTag))
		throw (-1) /* mismatched data type */;
	if (tag.tagLVT != 4)
		throw (-1) /* four bytes of data expected */;
	
	// copy out the data
	hour		= *dec.pktBuffer++;
	minute		= *dec.pktBuffer++;
	second		= *dec.pktBuffer++;
	hundredths	= *dec.pktBuffer++;
	dec.pktLength -= 4;
}

void BACnetTime::Encode( char *enc )
{
	// hour
	if (hour == 255)
		*enc++ = '?';
	else {
		sprintf( enc, "%02d", hour );
		enc += 2;
	}
	*enc++ = ':';

	// minute
	if (minute == 255)
		*enc++ = '?';
	else {
		sprintf( enc, "%02d", minute );
		enc += 2;
	}
	*enc++ = ':';

	// second
	if (second == 255)
		*enc++ = '?';
	else {
		sprintf( enc, "%02d", second );
		enc += 2;
	}
	*enc++ = '.';

	// hundredths
	if (hundredths == 255) {
		*enc++ = '?';
		*enc = 0;
	} else
		sprintf( enc, "%02d", hundredths );
}

void BACnetTime::Decode( const char *dec )
{
	// defaults
	hour = minute = second = hundredths = 255;

	// check for hour
	if ((*dec == '*') || (*dec == '?'))
		dec += 1;
	else
	if (isdigit(*dec)) {
		for (hour = 0; isdigit(*dec); dec++)
			hour = (hour * 10) + (*dec - '0');
	}
	if (*dec == ':')
		dec += 1;
	else
		throw (-1) /* invalid character */;

	// check for minute
	if ((*dec == '*') || (*dec == '?'))
		dec += 1;
	else
	if (isdigit(*dec)) {
		for (minute = 0; isdigit(*dec); dec++)
			minute = (minute * 10) + (*dec - '0');
	}
	if (*dec == ':')
		dec += 1;
	else
		throw (-1) /* invalid character */;

	// check for second
	if ((*dec == '*') || (*dec == '?'))
		dec += 1;
	else
	if (isdigit(*dec)) {
		for (second = 0; isdigit(*dec); dec++)
			second = (second * 10) + (*dec - '0');
	}
	if (*dec == '.')
		dec += 1;
	else
		throw (-1) /* invalid character */;

	// check for hundredths
	if ((*dec == '*') || (*dec == '?'))
		dec += 1;
	else
	if (isdigit(*dec)) {
		hundredths = (*dec++ - '0') * 10;
		if (isdigit(*dec))
			hundredths += (*dec++ - '0');
		while (isdigit(*dec))
			dec++;
		if (*dec)
			throw (-1) /* invalid character */;
	} else
		throw (-1) /* invalid character */;
}

//
//	BACnetObjectIdentifier::BACnetObjectIdentifier
//

BACnetObjectIdentifier::BACnetObjectIdentifier( int objType, int instanceNum )
	: objID( (objType << 22) + instanceNum )
{
}

//
//	BACnetObjectIdentifier::SetValue
//

void BACnetObjectIdentifier::SetValue( int objType, int instanceNum )
{
	objID = (objType << 22) + instanceNum;
}

void BACnetObjectIdentifier::Encode( BACnetAPDUEncoder& enc, int context )
{
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, 4 ).Encode( enc );
	else
		BACnetAPDUTag( objectIdentifierAppTag, 4 ).Encode( enc );
	
	// fill in the data
#ifdef _MSC_VER
	for (int j = 3; j >= 0; j--)
		enc.pktBuffer[enc.pktLength++] = (objID >> (j * 8)) & 0xFF;
#else
	memcpy( enc.pktBuffer+enc.pktLength, &objID, (size_t)4 );
	enc.pktLength += 4;
#endif
}

void BACnetObjectIdentifier::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;
		
	// verify the tag can be extracted
	tag.Decode( dec );
	
	// check the type and length
	if (!tag.tagClass && (tag.tagNumber != objectIdentifierAppTag))
		throw (-1) /* mismatched data type */;
	if (tag.tagLVT != 4)
		throw (-1) /* four bytes of data expected */;
	
	// copy out the data
#ifdef _MSC_VER
	for (int j = 3; dec.pktLength && j >= 0; j--)
		objID = (objID << 8) + (dec.pktLength--,*dec.pktBuffer++);
#else
	memcpy( &objID, dec.pktBuffer, (size_t)4 );
	dec.pktBuffer += 4;
	dec.pktLength -= 4;
#endif
}

void BACnetObjectIdentifier::Encode( char *enc )
{
	int		objType = (objID >> 22)
	,		instanceNum = (objID & 0x003FFFFF)
	;
	char	typeBuff[32], *s
	;

#if VTSScanner
	if (objType < 18 /* sizeof(NetworkSniffer::BACnetObjectType) */)
		s = NetworkSniffer::BACnetObjectType[objType];
	else
#endif
	if (objType < 128)
		sprintf( s = typeBuff, "RESERVED %d", objType );
	else
		sprintf( s = typeBuff, "VENDOR %d", objType );

	sprintf( enc, "%s, %d", s, instanceNum );
}

#if VTSScanner
void BACnetObjectIdentifier::Decode( const char *dec )
{
	int		objType, instanceNum
	;

	// create a scanner bound to the text
	ScriptScanner	scan( dec );
	ScriptToken		tok;

	// get something
	scan.Next( tok );

	if ((tok.tokenType == scriptKeyword) && (tok.tokenSymbol == kwRESERVED)) {
		// next must be a number in the reserved range
		scan.Next( tok );
		if (!tok.IsInteger( objType ))
			throw (-1) /* integer expected */;
		if ((objType < 0) || (objType >= 128))
			throw (-1) /* out of range */;
	} else
	if ((tok.tokenType == scriptKeyword) && (tok.tokenSymbol == kwVENDOR)) {
		// next must be a number in the vendor range
		scan.Next( tok );
		if (!tok.IsInteger( objType ))
			throw (-1) /* integer expected */;
		if ((objType < 128) || (objType >= (1 << 10)))
			throw (-1) /* out of range */;
	} else
	if (!tok.IsInteger( objType, ScriptObjectTypeMap ))
		throw (-1) /* object type keyword expected */;
	else
	if ((objType < 0) || (objType >= (1 << 10)))
		throw (-1) /* out of range */;

	// get the next token
	scan.Next( tok );

	// skip the ',' if it was entered
	if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == ','))
		scan.Next( tok );

	// make sure it's an integer
	if (!tok.IsInteger( instanceNum ))
		throw (-1) /* instance expected */;
	if ((instanceNum < 0) || (instanceNum >= (1 << 22)))
		throw (-1) /* out of range */;

	// everything checks out
	objID = (objType << 22) + instanceNum;
#else
void BACnetObjectIdentifier::Decode( const char * )
{
	throw (-1) /* not implemented */;
}
#endif

//
//	BACnetOpeningTag
//

void BACnetOpeningTag::Encode( BACnetAPDUEncoder& enc, int context )
{
	if (context < 15) {
		enc.CheckSpace( 1 );
		enc.pktBuffer[enc.pktLength++] = ((context & 0x0F) << 4) + 0x0E;
	} else {
		enc.CheckSpace( 2 );
		enc.pktBuffer[enc.pktLength++] = 0xFE;
		enc.pktBuffer[enc.pktLength++] = (context & 0xFF);
	}
}

void BACnetOpeningTag::Decode( BACnetAPDUDecoder& dec )
{
	BACnetOctet	tag
	;
	
	// enough for the tag byte?
	if (dec.pktLength < 1)
		throw (-1) /* not enough data */;
	
	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// check the type
	if ((tag & 0x0F) != 0x0E)
		throw (-1) /* mismatched tag class */;
	
	// check for a big context
	if ((tag & 0xF0) == 0xF0) {
		if (dec.pktLength < 1)
			throw (-1) /* not enough data */;
		dec.pktLength -= 1;
	}
}

//
//	BACnetClosingTag
//

void BACnetClosingTag::Encode( BACnetAPDUEncoder& enc, int context )
{
	if (context < 15) {
		enc.CheckSpace( 1 );
		enc.pktBuffer[enc.pktLength++] = ((context & 0x0F) << 4) + 0x0F;
	} else {
		enc.CheckSpace( 2 );
		enc.pktBuffer[enc.pktLength++] = 0xFF;
		enc.pktBuffer[enc.pktLength++] = (context & 0xFF);
	}
}

void BACnetClosingTag::Decode( BACnetAPDUDecoder& dec )
{
	BACnetOctet	tag
	;
	
	// enough for the tag byte?
	if (dec.pktLength < 1)
		throw (-1) /* not enough data */;
	
	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// check the type
	if ((tag & 0x0F) != 0x0F)
		throw (-1) /* mismatched tag class */;
	
	// check for a big context
	if ((tag & 0xF0) == 0xF0) {
		if (dec.pktLength < 1)
			throw (-1) /* not enough data */;
		dec.pktLength -= 1;
	}
}

//
//	BACnetAPDUTag::BACnetAPDUTag
//

BACnetAPDUTag::BACnetAPDUTag( BACnetTagClass tclass, BACnetApplicationTag tnum, int tlen )
	: tagClass(tclass), tagNumber(tnum), tagLVT(tlen)
{
}

BACnetAPDUTag::BACnetAPDUTag( BACnetApplicationTag tnum, int tlen )
	: tagClass(applicationTagClass), tagNumber(tnum), tagLVT(tlen)
{
}

BACnetAPDUTag::BACnetAPDUTag( int context, int tlen )
	: tagClass(contextTagClass), tagNumber((BACnetApplicationTag)context), tagLVT(tlen)
{
}

//
//	BACnetAPDUTag::Encode
//
//	Note that the context is unused, but required to override the pure virtual 
//	member function of BACnetEncodeable.
//

void BACnetAPDUTag::Encode( BACnetAPDUEncoder& enc, int )
{
	int			len = 0
	;
	BACnetOctet	tnum
	;
	
	// compute the tag length, including the data
	len = 1;
	if ((tagClass == openingTagClass) || (tagClass == closingTagClass))
		;
	else
	if ((tagClass == applicationTagClass) && (tagNumber == booleanAppTag))
		;
	else {
		// if we are context specific, use the context tag number
		tnum = (BACnetOctet)tagNumber;
		
		// extra big tag number?
		if (tnum >= 15)
			len += 1;
		
		// long lengths?
		if (tagLVT < 5)
			;
		else
		if (tagLVT <= 253)
			len += 1;
		else
		if (tagLVT <= 65535)
			len += 3;
		else
			len += 5;
		
		// add the rest of the data
		len += tagLVT;
	}
	
	// check to see if there's enough space
	enc.CheckSpace( len );
	
	// check for special encoding of open and close tags
	if (tagClass == openingTagClass) {
		enc.pktBuffer[enc.pktLength++] = (((BACnetOctet)tagNumber & 0x0F) << 4) + 0x0E;
		return;
	}
	if (tagClass == closingTagClass) {
		enc.pktBuffer[enc.pktLength++] = (((BACnetOctet)tagNumber & 0x0F) << 4) + 0x0F;
		return;
	}
	
	// check for context encoding
	tnum = (BACnetOctet)tagNumber;
	if (tagClass == contextTagClass)
		enc.pktBuffer[enc.pktLength] = 0x08;
	else
		enc.pktBuffer[enc.pktLength] = 0x00;
	
	// this first byte is a killer
	enc.pktBuffer[enc.pktLength++] +=
		(((tnum < 15) ? tnum : 0x0F) << 4)
		+ ((tagLVT < 5) ? tagLVT : 0x05)
		;
	if (tnum >= 15)
		enc.pktBuffer[enc.pktLength++] = tnum;
	
	// really short lengths already done
	if (tagLVT < 5)
		;
	else {
		if (tagLVT <= 253) {
			// byte lengths
			enc.pktBuffer[enc.pktLength++] = tagLVT;
		} else
		if (tagLVT <= 65535) {
			// short lengths
			enc.pktBuffer[enc.pktLength++] = 254;
			enc.pktBuffer[enc.pktLength++] = (tagLVT >> 8) & 0x0FF;
			enc.pktBuffer[enc.pktLength++] = tagLVT & 0x0FF;
		} else {
			// long lengths
			enc.pktBuffer[enc.pktLength++] = 255;
			enc.pktBuffer[enc.pktLength++] = (tagLVT >> 24) & 0x0FF;
			enc.pktBuffer[enc.pktLength++] = (tagLVT >> 16) & 0x0FF;
			enc.pktBuffer[enc.pktLength++] = (tagLVT >>  8) & 0x0FF;
			enc.pktBuffer[enc.pktLength++] = tagLVT & 0x0FF;
		}
	}
}

void BACnetAPDUTag::Decode( BACnetAPDUDecoder& dec )
{
	BACnetOctet	tag
	;
	
	// enough for the tag byte?
	if (dec.pktLength < 1)
		throw (-1) /* not enough data */;
	
	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// extract the type
	tagClass = (BACnetTagClass)((tag >> 3) & 0x01);
	
	// extract the number
	tagNumber = (BACnetApplicationTag)(tag >> 4);
	if (tagNumber == 0x0F) {
		if (dec.pktLength < 1)
			throw (-1) /* not enough data */;
		tagNumber = (BACnetApplicationTag)(dec.pktLength--,*dec.pktBuffer++);
	}
	
	// extract the length
	tagLVT = (tag & 0x07);
	if (tagLVT < 5)
		;
	else
	if (tagLVT == 5) {
		if (dec.pktLength < 1)
			throw (-1) /* not enough data */;
		tagLVT = (dec.pktLength--,*dec.pktBuffer++);
		if (tagLVT == 254) {
			if (dec.pktLength < 2)
				throw (-1) /* not enough data */;
			tagLVT = (dec.pktLength--,*dec.pktBuffer++);
			tagLVT = (tagLVT << 8) + (dec.pktLength--,*dec.pktBuffer++);
		} else
		if (tagLVT == 255) {
			if (dec.pktLength < 4)
				throw (-1) /* not enough data */;
			tagLVT = (dec.pktLength--,*dec.pktBuffer++);
			tagLVT = (tagLVT << 8) + (dec.pktLength--,*dec.pktBuffer++);
			tagLVT = (tagLVT << 8) + (dec.pktLength--,*dec.pktBuffer++);
			tagLVT = (tagLVT << 8) + (dec.pktLength--,*dec.pktBuffer++);
		}
	} else
	if (tagLVT == 6) {
		tagClass = openingTagClass;
		tagLVT = 0;
	} else
	if (tagLVT == 7) {
		tagClass = closingTagClass;
		tagLVT = 0;
	}
	
	// check for enough data (except for an application tagged boolean)
	if (!tagClass && (tagNumber == booleanAppTag))
		;
	else
	if (dec.pktLength < tagLVT)
		throw (-1);
}

//----------

//
//	BACnetNPDU::BACnetNPDU
//

BACnetNPDU::BACnetNPDU( const BACnetAddress &addr, const BACnetOctet *data, const int len, const int reply, const int priority )
	: pduAddr(addr), pduData(data), pduLen(len)
	, pduExpectingReply(reply), pduNetworkPriority(priority)
{
}

//
//	BACnetNetClient::BACnetNetClient
//

BACnetNetClient::BACnetNetClient( void )
	: clientPeer(0)
{
}

//
//	BACnetNetClient::~BACnetNetClient
//

BACnetNetClient::~BACnetNetClient( void )
{
	if (clientPeer)
		Unbind( this, clientPeer );
}

//
//	BACnetNetClient::Request
//

void BACnetNetClient::Request( const BACnetNPDU &pdu )
{
	if (clientPeer)
		clientPeer->Indication( pdu );
}

//
//	BACnetNetServer::BACnetNetServer
//

BACnetNetServer::BACnetNetServer( void )
	: serverPeer(0)
{
}

//
//	BACnetNetServer::~BACnetNetServer
//

BACnetNetServer::~BACnetNetServer( void )
{
	if (serverPeer)
		Unbind( serverPeer, this );
}

//
//	BACnetNetServer::Response
//

void BACnetNetServer::Response( const BACnetNPDU &pdu )
{
	if (serverPeer)
		serverPeer->Confirmation( pdu );
}

//
//	Bind
//

void Bind( BACnetNetClientPtr cp, BACnetNetServerPtr sp )
{
	cp->clientPeer = sp;
	sp->serverPeer = cp;
}

//
//	Unbind
//

void Unbind( BACnetNetClientPtr cp, BACnetNetServerPtr sp )
{
	cp->clientPeer = 0;
	sp->serverPeer = 0;
}

#if _TSMDebug

//
//	BACnetDebugNPDU::BACnetDebugNPDU
//

const char debugHex[] = "0123456789ABCDEF";

BACnetDebugNPDU::BACnetDebugNPDU( const char *lbl )
	: label(lbl)
{
}

//
//	BACnetDebugNPDU::BACnetDebugNPDU
//

BACnetDebugNPDU::BACnetDebugNPDU( BACnetNetServerPtr sp, const char *lbl )
	: label(lbl)
{
	Bind( this, sp );
}

//
//	BACnetDebugNPDU::Indication
//

void BACnetDebugNPDU::Indication( const BACnetNPDU &pdu )
{
	cout << '(' << label << " '";
	for (int i = 0; i < pdu.pduLen; i++) {
		cout << debugHex[ (pdu.pduData[i] >> 4) & 0x0F ];
		cout << debugHex[ pdu.pduData[i] & 0x0F ];
		cout << '.';
	}
	cout << "' to " << pdu.pduAddr << ')';
	cout << endl;
	
	Request( pdu );
}

//
//	BACnetDebugNPDU::Confirmation
//

void BACnetDebugNPDU::Confirmation( const BACnetNPDU &pdu )
{
	cout << '(' << label << " '";
	for (int i = 0; i < pdu.pduLen; i++) {
		cout << debugHex[ (pdu.pduData[i] >> 4) & 0x0F ];
		cout << debugHex[ pdu.pduData[i] & 0x0F ];
		cout << '.';
	}
	cout << "' from " << pdu.pduAddr << ')';
	cout << endl;
	
	Response( pdu );
}

#endif

//
//	BACnetPort::BACnetPort
//
//	This ctor sets the port status to uninitialized.  It will be up to a derived 
//	class to clear the status when the port is up and running.
//

BACnetPort::BACnetPort( void )
	: portStatus(-1)
{
}

//
//	BACnetPort::~BACnetPort
//
//	This dtor doesn't look like it does much, but it is virtual.  So when a pointer 
//	to a BACnetPort is deleted, the real derived class dtor is called.  All classes 
//	that have a virtual member function should have a virtual dtor.
//

BACnetPort::~BACnetPort( void )
{
}

//
//	BACnetPort::FilterData
//
//	The default for ports is to do no filtering.  This member function allows the 
//	application to get a chance to see what is being sent and received.  It currently 
//	does not allow the derived class to interfere with the processing of the packet,
//	but that might be a nice feature to add.
//

void BACnetPort::FilterData( BACnetOctet *, int, BACnetPortDirection )
{
}

//
//	BACnetPort::PortStatusChange
//
//	This function is called when the port needs to change its status.  It can be overridden 
//	by a derived class to do something with the new status.
//

void BACnetPort::PortStatusChange( void )
{
}

//----------

//
//	BACnetAppClient::BACnetAppClient
//

BACnetAppClient::BACnetAppClient( void )
	: clientPeer(0)
{
}

//
//	BACnetAppClient::~BACnetAppClient
//

BACnetAppClient::~BACnetAppClient( void )
{
	if (clientPeer)
		Unbind( this, clientPeer );
}

//
//	BACnetAppClient::Request
//

void BACnetAppClient::Request( const BACnetAPDU &pdu )
{
	if (!clientPeer) throw -1;
	clientPeer->Indication( pdu );
}

//
//	BACnetAppServer::BACnetAppServer
//

BACnetAppServer::BACnetAppServer( void )
	: serverPeer(0)
{
}

//
//	BACnetAppServer::~BACnetAppServer
//

BACnetAppServer::~BACnetAppServer( void )
{
	if (serverPeer)
		Unbind( serverPeer, this );
}

//
//	BACnetAppServer::Response
//

void BACnetAppServer::Response( const BACnetAPDU &pdu )
{
	if (!serverPeer) throw -1;
	serverPeer->Confirmation( pdu );
}

//
//	Bind
//

void Bind( BACnetAppClientPtr cp, BACnetAppServerPtr sp )
{
	cp->clientPeer = sp;
	sp->serverPeer = cp;
}

//
//	Unbind
//

void Unbind( BACnetAppClientPtr cp, BACnetAppServerPtr sp )
{
	cp->clientPeer = 0;
	sp->serverPeer = 0;
}
