
#include "stdafx.h"
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

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

#if (__DECCXX)
#include "cvtdef.h"
#include <ssdef.h>

extern "C" {
int cvt$convert_float( const void *, int, void *, int, int );
}
#endif

#include "Props.h"
#include "BACnet.hpp"

#define nPRIO 16


#ifndef _MSC_VER

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

#define	VTSScanner		1

#if VTSScanner

namespace NetworkSniffer {
	extern char *BACnetObjectType[];
}

#include "VTS.h"
#include "ScriptBase.h"
#include "ScriptKeywords.h"

#endif

const char * apszBinaryPVNames[] = {"INACTIVE", "ACTIVE"};


// Declare matching prototypes for globals defined in ScriptExecutor...
bool Match( int op, int a, int b );
//madanner 9/24/02, needed for expanded BACnetUsigned internal value
//bool Match( int op, unsigned long a, unsigned long b );
bool Match( int op, unsigned long a, unsigned long b );
bool Match( int op, float a, float b );
bool Match( int op, double a, double b );
bool Match( int op, CTime &timeThis, CTime &timeThat );
LPCSTR OperatorToString(int iOperator);


bool ValuesEqual( int v1, int v2 );
bool ValuesGreater( int v1, int v2 );
bool ValuesLess( int v1, int v2 );


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

BACnetAddress & BACnetAddress::operator =( const BACnetAddress &arg )
{
	addrType = arg.addrType;
	addrNet = arg.addrNet;
	addrLen = arg.addrLen;
	memcpy( addrAddr, arg.addrAddr, addrLen );
	return *this;
}

int operator ==( const BACnetAddress &addr1, const BACnetAddress &addr2 )
{
	int			i
	;
	
	// address types must match
	if (addr1.addrType != addr2.addrType)
		return 0;
	
	// remote broadcast and remote station have a network, localStation and remote
	// station have an address.
	switch (addr1.addrType) {
		case nullAddr:
		case localBroadcastAddr:
		case globalBroadcastAddr:
			break;
			
		case remoteBroadcastAddr:
			if (addr1.addrNet != addr1.addrNet) return 0;
			break;
			
		case remoteStationAddr:
			if (addr1.addrNet != addr1.addrNet) return 0;
		case localStationAddr:
			if (addr1.addrLen != addr2.addrLen) return 0;
			for (i = 0; i < addr1.addrLen; i++)
				if (addr1.addrAddr[i] != addr2.addrAddr[i])
					return 0;
			break;
			
		default:
			throw_(1); // no other address types allowed
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


IMPLEMENT_DYNAMIC(BACnetEncodeable, CObject)

//
//	BACnetEncodeable
//

void BACnetEncodeable::Encode( char * ) const
{
	throw_(2) /*not implemented */;
}

void BACnetEncodeable::Decode( const char * )
{
	throw_(3) /*not implemented */;
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


const char * BACnetEncodeable::ToString() const
{
	// User internal buffer and return to caller...  Data won't last too long so if you're going to use
	// more than one of these in calling parameters... Use:  CString(this->ToString()) so the data
	// will be allocated and copied out before the next call blows away the string.

	static char buffer[40];		// should be enough for primitive values
	Encode(buffer);
	return buffer;
}


int BACnetEncodeable::DataType()
{
	ASSERT(0);		// shouldn't be called here...
	return 0;
}


BACnetEncodeable * BACnetEncodeable::clone()
{
	ASSERT(0);		// shouldn't be called here...
	return NULL;
}


bool BACnetEncodeable::PreMatch(int iOperator )
{
	return iOperator == '?=' || iOperator == '>>';
}



// Match is called by default from most all of the classes.  It really just fails and formats the error

bool BACnetEncodeable::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	CString strError;

	if ( pstrError != NULL )
	{
		CString str1(ToString());	// have to do this because ToString uses a static buff... call it twice and you're hosed

		strError.Format(IDS_SCREX_COMPFAILTYPE, str1, OperatorToString(iOperator), rbacnet.ToString() );
		*pstrError += strError;
	}

	return false;
}


// Some types can only test for equality/inequality...  
// Make this available at this level of the class structure

bool BACnetEncodeable::EqualityRequiredFailure( BACnetEncodeable & rbacnet, int iOperator, CString * pstrError )
{
	if ( iOperator != '!=' && iOperator != '=' )
	{
		CString strError, str1(ToString());
		strError.Format(IDS_SCREX_COMPEQREQ, rbacnet.ToString(), OperatorToString(iOperator), str1);
		*pstrError = strError;
		return true;
	}

	return false;
}





IMPLEMENT_DYNAMIC(BACnetNull, BACnetEncodeable)

//
//	BACnetNull
//

BACnetNull::BACnetNull( BACnetAPDUDecoder & dec )
{
	Decode(dec);
}


BACnetNull::BACnetNull( )
{
}


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
		throw_(4) /* not enough data */;
	
	// suck out the tag
	BACnetOctet	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// verify its a null
	if (((tag & 0x08) == 0) && ((tag & 0xF0) != 0x00))
		throw_(5) /* mismatched data type */;
}

void BACnetNull::Encode( char *enc ) const
{
	strcpy( enc, ToString() );
}

void BACnetNull::Decode( const char *dec )
{
	if (stricmp( dec, ToString()) != 0)
		throw_(6) /* null must be 'null' */;
}

const char * BACnetNull::ToString() const
{
	return "Null";
}


int BACnetNull::DataType()
{
	return enull;
}

BACnetEncodeable * BACnetNull::clone()
{
	return new BACnetNull();
}


bool BACnetNull::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetNull)));

	if ( EqualityRequiredFailure(rbacnet, iOperator, pstrError) )
		return false;

	bool fMatch = ((BACnetInteger &) rbacnet).IsKindOf(RUNTIME_CLASS(BACnetNull)) == TRUE;

	if ( fMatch && iOperator == '='  ||  !fMatch &&  iOperator == '!=' )
		return true;

	return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);
}



//==========================================================================

IMPLEMENT_DYNAMIC(BACnetAddr, BACnetEncodeable)


BACnetAddr::BACnetAddr()
{
}


BACnetAddr::BACnetAddr( BACnetOctet * paMAC, unsigned int nMACLen )
{
	AssignAddress(0, paMAC, nMACLen);
}


BACnetAddr::BACnetAddr( unsigned int nNet, BACnetOctet * paMAC, unsigned int nMACLen )
{
	AssignAddress(nNet, paMAC, nMACLen);
}


BACnetAddr::BACnetAddr( BACnetAPDUDecoder & dec )
{
	Decode(dec);
}


void BACnetAddr::Encode( BACnetAPDUEncoder& enc, int context )
{
	BACnetUnsigned bacnetNet(m_bacnetAddress.addrNet);
	BACnetOctetString bacnetMAC(m_bacnetAddress.addrAddr, m_bacnetAddress.addrLen);

	bacnetNet.Encode(enc, context);
	bacnetMAC.Encode(enc, context);
}


void BACnetAddr::Decode( BACnetAPDUDecoder& dec )
{
	BACnetUnsigned bacnetNet(dec);
	BACnetOctetString bacnetMAC(dec);

	AssignAddress(bacnetNet.uintValue, bacnetMAC.strBuff, bacnetMAC.strBuffLen);
}


void BACnetAddr::AssignAddress(unsigned int nNet, BACnetOctet * paMAC, unsigned int nMACLen )
{
	if ( nMACLen == 0 )
	{
		// broadcast
		if ( nNet == 0 )
			m_bacnetAddress.LocalBroadcast();
		else
			m_bacnetAddress.RemoteBroadcast(nNet);
	}
	else
	{
		// specific address
		if ( nNet == 0 )
			m_bacnetAddress.LocalStation(paMAC, nMACLen);
		else
			m_bacnetAddress.RemoteStation(nNet, paMAC, nMACLen);
	}
}


BACnetEncodeable * BACnetAddr::clone()
{
	return new BACnetAddr(m_bacnetAddress.addrNet, m_bacnetAddress.addrAddr, m_bacnetAddress.addrLen);
}


BACnetAddr & BACnetAddr::operator =( const BACnetAddr & arg )
{
	m_bacnetAddress = arg.m_bacnetAddress;
	return *this;
}

const char * BACnetAddr::ToString() const
{
	TRACE0("ToString() for BACnetAddr not implemented");
	ASSERT(0);
	return NULL;
}


IMPLEMENT_DYNAMIC(BACnetBoolean, BACnetEncodeable)

//
//	BACnetBoolean
//

BACnetBoolean::BACnetBoolean( int bvalu )
	: boolValue(bvalu ? bTrue : bFalse)
{
}



BACnetBoolean::BACnetBoolean( BACnetAPDUDecoder & dec )
{
	Decode(dec);
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
		throw(7) /* not enough data */;
	
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
			throw_(8) /* bad length */;
		
		// check for more data
		if (dec.pktLength < 1)
			throw_(9);
		
		boolValue = (eBACnetBoolean)(dec.pktLength--,*dec.pktBuffer++);
	}
}


void BACnetBoolean::Encode( char *enc ) const
{
	strcpy( enc, ToString() );
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
			throw_(10) /* unknown keyword */;
	}
}


const char * BACnetBoolean::ToString() const
{
	return (boolValue ? "Set" : "Reset");
}


int BACnetBoolean::DataType()
{
	return ebool;
}

BACnetEncodeable * BACnetBoolean::clone()
{
	return new BACnetBoolean(boolValue);
}


BACnetBoolean & BACnetBoolean::operator =( const BACnetBoolean & arg )
{
	boolValue = arg.boolValue;
	return *this;
}


bool BACnetBoolean::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetBoolean)));

	if ( EqualityRequiredFailure(rbacnet, iOperator, pstrError) )
		return false;

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetBoolean))  || 
		 !::Match(iOperator, boolValue, ((BACnetBoolean &) rbacnet).boolValue ) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}



IMPLEMENT_DYNAMIC(BACnetEnumerated, BACnetEncodeable)

//
//	BACnetEnumerated
//

BACnetEnumerated::BACnetEnumerated( int evalu /* = 0*/, const char ** papNameList /* = NULL */, int nListSize /* = 0 */ )
	: enumValue( evalu ), m_papNameList(papNameList), m_nListSize(nListSize)
{
}


BACnetEnumerated::BACnetEnumerated( BACnetAPDUDecoder & dec )
				 :m_papNameList(NULL), m_nListSize(0)
{
	Decode(dec);
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
		throw_(11) /* mismatched data type */;
	
	// copy out the data
	rslt = 0;
	while (tag.tagLVT) {
		rslt = (rslt << 8) + (dec.pktLength--,*dec.pktBuffer++);
		tag.tagLVT -= 1;
	}
	
	// save the result
	enumValue = rslt;
}

void BACnetEnumerated::Encode( char *enc ) const
{
	Encode( enc, m_papNameList, m_nListSize );
}

void BACnetEnumerated::Encode( char *enc, const char **table, int tsize ) const
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
	Decode( dec, m_papNameList, m_nListSize );
}

void BACnetEnumerated::Decode( const char *dec, const char **table, int tsize )
{
	if (isdigit(*dec)) {										// explicit number
		// integer encoding
		for (enumValue = 0; *dec; dec++)
			if (!isdigit(*dec))
				throw_(12) /* invalid character */;
			else
				enumValue = (enumValue * 10) + (*dec - '0');
	} else
	if (!table)
		throw_(13) /* no translation available */;
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
			throw_(14) /* no matching translation */;
	}
}


int BACnetEnumerated::DataType()
{
	return et;
}

BACnetEncodeable * BACnetEnumerated::clone()
{
	return new BACnetEnumerated(enumValue, m_papNameList, m_nListSize);
}


BACnetEnumerated & BACnetEnumerated::operator =( const BACnetEnumerated & arg )
{
	enumValue = arg.enumValue;
	m_nListSize = arg.m_nListSize;
	m_papNameList = arg.m_papNameList;
	return *this;
}


bool BACnetEnumerated::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

//	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetEnumerated)));

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetEnumerated))  || 
		 !::Match(iOperator, enumValue, ((BACnetEnumerated &) rbacnet).enumValue) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}



//
//	BACnetUnsigned
//

IMPLEMENT_DYNAMIC(BACnetUnsigned, BACnetEncodeable)

// madanner 9/24/02... required change to unsigned long to handle, well, longs.
//BACnetUnsigned::BACnetUnsigned( unsigned int uivalu )

BACnetUnsigned::BACnetUnsigned( unsigned long uivalu )
	: uintValue( uivalu )
{
}


BACnetUnsigned::BACnetUnsigned( BACnetAPDUDecoder & dec )
{
	Decode(dec);
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
		throw_(15) /* mismatched data type */;
	
	// copy out the data
	rslt = 0;
	while (tag.tagLVT) {
		rslt = (rslt << 8) + (dec.pktLength--,*dec.pktBuffer++);
		tag.tagLVT -= 1;
	}
	
	// save the result
	uintValue = rslt;
}

void BACnetUnsigned::Encode( char *enc ) const
{
	// simple, effective
	// madanner 9/24/02, changed to handle unsigned long case
//	sprintf( enc, "%u", uintValue );
	sprintf( enc, "%lu", uintValue );
}


void BACnetUnsigned::Decode( const char *dec )
{
	unsigned int	t
	;

	// figure out what encoding to use
	//Moved by Yajun Zhou, 2002-8-16
	//Moved to line 647
	//	if (isdigit(*dec)) {										// nnn
	//		// integer encoding
	//		for (uintValue = 0; *dec; dec++)
	//			if (!isdigit(*dec))
	//				throw_(16) /* invalid character */;
	//			else
	//				uintValue = (uintValue * 10) + (*dec - '0');
	//	} else
	////////////////////////////////////////
	if ( ((dec[0] == 'D') && (dec[1] == '\''))					// D'nnn'
		|| ((dec[0] == 'd') && (dec[1] == '\''))
		) {
		// decimal encoding
		dec += 2;
		if (((strlen(dec) - 1) % 3) != 0)			// must be triplet
			throw_(17) /* must be triplet */;
		for (uintValue = 0; *dec != '\''; ) {
			if (!isdigit(*dec))
				throw_(18) /* invalid character */;
			t = (*dec++ - '0');

			if (!isdigit(*dec))
				throw_(19) /* invalid character */;
			t = (t * 10) + (*dec++ - '0');

			if (!isdigit(*dec))
				throw_(20) /* invalid character */;
			t = (t * 10) + (*dec++ - '0');

			uintValue = (uintValue * 256) + t;
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'x'))
		|| ((dec[0] == 'X') && (dec[1] == '\''))
		|| ((dec[0] == 'x') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'x'))
		|| ((dec[0] == '&') && (dec[1] == 'X'))
		) {
		// hex encoding
		dec += 2;
		for (uintValue = 0; *dec && (*dec != '\''); dec++) {
			if (!isxdigit(*dec))
				throw_(21) /* invalid character */;
			uintValue = (uintValue * 16) + (isdigit(*dec) ? (*dec - '0') : (*dec - 'A' + 10));
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'o'))
		|| ((dec[0] == 'O') && (dec[1] == '\''))
		|| ((dec[0] == 'o') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'O'))
		|| ((dec[0] == '&') && (dec[1] == 'o'))
		) {
		// octal encoding
		dec += 2;
		for (uintValue = 0; *dec && (*dec != '\''); dec++) {
			if ((*dec < '0') || (*dec > '7'))
				throw_(21) /* invalid character */;
			//Modified by Yajun Zhou, 2002-8-16
			//uintValue = (uintValue * 16) + (*dec - '0');
			uintValue = (uintValue * 8) + (*dec - '0');
			///////////////////////////////////
		}
	} else
	if ( ((dec[0] == '0') && (dec[1] == 'b'))
		|| ((dec[0] == 'B') && (dec[1] == '\''))
		|| ((dec[0] == 'b') && (dec[1] == '\''))
		|| ((dec[0] == '&') && (dec[1] == 'B'))
		|| ((dec[0] == '&') && (dec[1] == 'b'))
		) {
		// binary encoding
		dec += 2;
		for (uintValue = 0; *dec && (*dec != '\''); dec++) {
			if ((*dec < '0') || (*dec > '1'))
				throw_(22) /* invalid character */;
			uintValue = (uintValue * 2) + (*dec - '0');
		}
	} else
	//Moved by Yajun Zhou, 2002-8-16
	if (isdigit(*dec)) {										// nnn
		// integer encoding
		for (uintValue = 0; *dec; dec++)
			if (!isdigit(*dec))
				throw_(16) /* invalid character */;
			else
				uintValue = (uintValue * 10) + (*dec - '0');
	} else
	//////////////////////////////////
		throw_(23) /* unknown or invalid encoding */;
}


int BACnetUnsigned::DataType()
{
	return ud;
}

BACnetEncodeable * BACnetUnsigned::clone()
{
	return new BACnetUnsigned(uintValue);
}


bool BACnetUnsigned::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetUnsigned)));

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetUnsigned))  ||  
		 !::Match(iOperator, uintValue, ((BACnetUnsigned &) rbacnet).uintValue) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}



IMPLEMENT_DYNAMIC(BACnetInteger, BACnetEncodeable)

//
//	BACnetInteger
//

BACnetInteger::BACnetInteger( int ivalu )
	: intValue( ivalu )
{
}


BACnetInteger::BACnetInteger( BACnetAPDUDecoder & dec )
{
	Decode(dec);
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
		throw_(24) /* mismatched data type */;
	
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

void BACnetInteger::Encode( char *enc ) const
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
	//Moved by Yajun Zhou, 2002-8-17
	//Moved to line 834
	//	if (isdigit(*dec)) {										// nnn
	//		// integer encoding
	//		for (intValue = 0; *dec; dec++)
	//			if (!isdigit(*dec))
	//				throw_(25) /* invalid character */;
	//			else
	//				intValue = (intValue * 10) + (*dec - '0');
	//	} else
	////////////////////////////////////////////
	if ( ((dec[0] == 'D') && (dec[1] == '\''))					// D'nnn'
		|| ((dec[0] == 'd') && (dec[1] == '\''))
		) {
		// decimal encoding
		dec += 2;
		if (((strlen(dec) - 1) % 3) != 0)			// must be triplet
			throw_(26) /* must be triplet */;
		for (intValue = 0; *dec != '\''; ) {
			if (!isdigit(*dec))
				throw_(27) /* invalid character */;
			t = (*dec++ - '0');

			if (!isdigit(*dec))
				throw_(28) /* invalid character */;
			t = (t * 10) + (*dec++ - '0');

			if (!isdigit(*dec))
				throw_(29) /* invalid character */;
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
				throw_(30) /* invalid character */;
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
				throw_(31) /* invalid character */;
			//Modified by Yajun Zhou, 2002-8-17
			//intValue = (intValue * 16) + (*dec - '0');
			intValue = (intValue * 8) + (*dec - '0');
			/////////////////////////////////////////
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
				throw_(32) /* invalid character */;
			intValue = (intValue * 2) + (*dec - '0');
		}
	} else
	//Moved by Yajun Zhou, 2002-8-17
	if (isdigit(*dec)) {										// nnn
		// integer encoding
		for (intValue = 0; *dec; dec++)
			if (!isdigit(*dec))
				throw_(25) /* invalid character */;
			else
				intValue = (intValue * 10) + (*dec - '0');
	} else
	//////////////////////////////////////////////////
		throw_(33) /* unknown or invalid encoding */;

	// update for sign
	if (negValue)
		intValue = (intValue * -1);
}


int BACnetInteger::DataType()
{
	return sw;
}

BACnetEncodeable * BACnetInteger::clone()
{
	return new BACnetInteger(intValue);
}


bool BACnetInteger::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetInteger)));

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetInteger)) ||
		 !::Match(iOperator, intValue, ((BACnetInteger &) rbacnet).intValue) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}



IMPLEMENT_DYNAMIC(BACnetReal, BACnetEncodeable)

//
//	BACnetReal
//

BACnetReal::BACnetReal( float rvalu )
	: realValue( rvalu )
{
}



BACnetReal::BACnetReal( BACnetAPDUDecoder & dec )
{
	Decode(dec);
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
#ifdef ENDIAN_SWAP
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
		throw_(34) /* mismatched data type */;
	if (tag.tagLVT != 4)
		throw_(35) /* four bytes of data expected */;
	
	// copy out the data
#if (__DECCXX)
	cvt$convert_float( dec.pktBuffer, CVT$K_IEEE_S
		, &realValue, CVT$K_VAX_F
		, CVT$M_ROUND_TO_NEAREST + CVT$M_BIG_ENDIAN
		);
	dec.pktBuffer += 4;
	dec.pktLength -= 4;
#else
#ifdef ENDIAN_SWAP
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

void BACnetReal::Encode( char *enc ) const
{
	// simple, effective
	sprintf( enc, "%f", realValue );
}

void BACnetReal::Decode( const char *dec )
{
	// check for valid format
	if (sscanf( dec, "%f", &realValue ) != 1)
		throw_(36) /* format error */;
}


int BACnetReal::DataType()
{
	return flt;
}

BACnetEncodeable * BACnetReal::clone()
{
	return new BACnetReal(realValue);
}


bool BACnetReal::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	// Don't assert here because we might be comparing Real value with Null value from priority array
	// This is normal and assert is getting in the way.

//	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetReal)));

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetReal)) || 
		 !::Match(iOperator, realValue, ((BACnetReal &) rbacnet).realValue ) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}




IMPLEMENT_DYNAMIC(BACnetDouble, BACnetEncodeable)

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
#ifdef ENDIAN_SWAP
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
		throw_(37) /* mismatched data type */;
	if (tag.tagLVT != 8)
		throw_(38) /* eight bytes of data expected */;
	
	// copy out the data
#if (__DECCXX)
	cvt$convert_float( dec.pktBuffer, CVT$K_IEEE_T
		, &doubleValue, CVT$K_VAX_G
		, CVT$M_ROUND_TO_NEAREST + CVT$M_BIG_ENDIAN
		);
	dec.pktBuffer += 8;
	dec.pktLength -= 8;
#else
#ifdef ENDIAN_SWAP
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

void BACnetDouble::Encode( char *enc ) const
{
	// simple, effective
	sprintf( enc, "%lf", doubleValue );
}

void BACnetDouble::Decode( const char *dec )
{
	// check for valid format
	if (sscanf( dec, "%lf", &doubleValue ) != 1)
		throw_(39) /* format error */;
}


int BACnetDouble::DataType()
{
	return flt;
}

BACnetEncodeable * BACnetDouble::clone()
{
	return new BACnetDouble(doubleValue);
}


bool BACnetDouble::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetDouble)));

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetDouble)) ||
		 !::Match(iOperator, doubleValue, ((BACnetDouble &) rbacnet).doubleValue ) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}



IMPLEMENT_DYNAMIC(BACnetCharacterString, BACnetEncodeable)

//
//	BACnetCharacterString
//

BACnetCharacterString::BACnetCharacterString( char *svalu )
	: strEncoding(0)
{
	Initialize(svalu);
}


void BACnetCharacterString::Initialize( char * svalu )
{
	strLen = (svalu ? strlen(svalu) : 0);
	strBuff = new BACnetOctet[strLen];
	if (svalu)
		memcpy( strBuff, svalu, (size_t)strLen );
}


BACnetCharacterString::BACnetCharacterString( BACnetAPDUDecoder & dec )
					  :strEncoding(0)
{
	Initialize(NULL);
	Decode(dec);
}


BACnetCharacterString::BACnetCharacterString( BACnetCharacterString & cpy )
					  :strEncoding(cpy.strEncoding)
{
	Initialize((char *) cpy.strBuff);
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
	for (unsigned i = 0; i < strLen; i++)
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
		throw_(40) /* mismatched data type */;

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

void BACnetCharacterString::Encode( char *enc ) const
{
	static char hex[] = "0123456789ABCDEF"
	;

	// check for the simple case
	if (strEncoding == 0) {
		*enc++ = '"';

		// simple contents
		const char *src = (char *)strBuff;
		for (unsigned i = 0; i < strLen; i++, src++)
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
		for (unsigned i = 0; i < strLen; i++) {
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
				throw_(41) /* encoding type keyword expected */;
			else
			if ((encType < 0) || (encType > 255))
				throw_(42) /* out of range */;
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
			throw_(43) /* ASCII or hex string expected */;
#else
		throw_(44) /* not implemented */;
#endif
	}
}


int BACnetCharacterString::DataType()
{
	return s132;
}

BACnetEncodeable * BACnetCharacterString::clone()
{
	return new BACnetCharacterString(*this);
}


bool BACnetCharacterString::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetCharacterString)));

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetCharacterString)) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	// check encoding
	if ( strEncoding != ((BACnetCharacterString &) rbacnet).strEncoding )
	{
		pstrError->Format(IDS_SCREX_COMPFAILSTRENCODING, ((BACnetCharacterString &) rbacnet).strEncoding, strEncoding );
		return false;
	}

	// This may present a problem where the encoding is non ASCII or whatever the heck CString uses...
	// So let's just keep the old stuff around for the sake of it, eh?

	if ( !Match((BACnetCharacterString &) rbacnet, iOperator) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}


bool BACnetCharacterString::Match( BACnetCharacterString & rstring, int iOperator )
{
	if ( PreMatch(iOperator) )
		return true;

	switch(iOperator)
	{
		case '=':	return *this == rstring;
		case '<':	return *this < rstring;
		case '>':	return *this > rstring;
		case '<=':	return *this <= rstring;
		case '>=':	return *this >= rstring;
		case '!=':	return *this != rstring;
		default:
			ASSERT(0);
	}
	return false;
}



bool BACnetCharacterString::operator ==( const BACnetCharacterString & arg )
{
	return (CString(strBuff) == CString(arg.strBuff)) != 0;		// account for nasty Microsoft BOOL stuff
}


bool BACnetCharacterString::operator !=( const BACnetCharacterString & arg )
{
	return (CString(strBuff) != CString(arg.strBuff)) != 0;		// account for nasty Microsoft BOOL stuff
}


bool BACnetCharacterString::operator <=( const BACnetCharacterString & arg )
{
	return (CString(strBuff) <= CString(arg.strBuff)) != 0;		// account for nasty Microsoft BOOL stuff
}


bool BACnetCharacterString::operator <( const BACnetCharacterString & arg )
{
	return (CString(strBuff) < CString(arg.strBuff)) != 0;		// account for nasty Microsoft BOOL stuff
}


bool BACnetCharacterString::operator >=( const BACnetCharacterString & arg )
{
	return (CString(strBuff) >= CString(arg.strBuff)) != 0;		// account for nasty Microsoft BOOL stuff
}


bool BACnetCharacterString::operator >( const BACnetCharacterString & arg )
{
	return (CString(strBuff) > CString(arg.strBuff)) != 0;		// account for nasty Microsoft BOOL stuff
}


/*  Moved from ExpectCharacterString, madanner 10/02
	// verify the encoding
	if (cstrData.strEncoding != scriptData.strEncoding)
		throw "Character string encoding mismatch";

	// verify the value
	minLen = (cstrData.strLen < scriptData.strLen ? cstrData.strLen : scriptData.strLen);

	switch (spep->exprOp) {
		case '<':
			for (i = 0; i < minLen; i++)
				if (cstrData.strBuff[i] >= scriptData.strBuff[i])
					throw "Character string mismatch";
			// ### what about the rest?
			break;
		case '>':
			for (i = 0; i < minLen; i++)
				if (cstrData.strBuff[i] <= scriptData.strBuff[i])
					throw "Character string mismatch";
			// ### what about the rest?
			break;
		case '<=':
			for (i = 0; i < minLen; i++)
				if (cstrData.strBuff[i] < scriptData.strBuff[i])
					throw "Character string mismatch";
			// ### what about the rest?
			break;
		case '>=':
			for (i = 0; i < minLen; i++)
				if (cstrData.strBuff[i] > scriptData.strBuff[i])
					throw "Character string mismatch";
			// ### what about the rest?
			break;
		case '=':
			if (cstrData.strLen != scriptData.strLen)
				throw "Character string mismatch";
			for (i = 0; i < cstrData.strLen; i++)
				if (cstrData.strBuff[i] != scriptData.strBuff[i])
					throw "Character string mismatch";
			break;
		case '!=':
			if (cstrData.strLen != scriptData.strLen)
				break;
			for (i = 0; i < cstrData.strLen; i++)
				if (cstrData.strBuff[i] != scriptData.strBuff[i])
					break;
			if (i >= cstrData.strLen)
				throw "Character string mismatch";
			break;
	}
*/



IMPLEMENT_DYNAMIC(BACnetOctetString, BACnetEncodeable)

//
//	BACnetOctetString::BACnetOctetString
//

BACnetOctetString::BACnetOctetString( void )
	: strLen(0), strBuffLen(0), strBuff(0)
{
}


BACnetOctetString::BACnetOctetString( BACnetAPDUDecoder & dec )
{
	Decode(dec);
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


// this one copies the data... 

BACnetOctetString::BACnetOctetString( const BACnetOctet *bytes, int len)
{
	strLen = strBuffLen = len;
	strBuff = new BACnetOctet[len];
	if (strBuff)
		memcpy( strBuff, bytes, len );
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

void BACnetOctetString::Insert( BACnetOctet *bytes, int len, int position )
{
	// make sure the buffer can handle it
	if (strLen + len > strBuffLen)
		PrepBuffer( strLen + len );

	// move existing data out of the way
	if (pos < strLen)
		memmove( strBuff+position+len, strBuff+position, len );

	// copy in new data
	memcpy( strBuff+position, bytes, len );
	strLen += len;
}

//
//	BACnetOctetString::Insert
//

void BACnetOctetString::Insert( const BACnetOctetString &cpy, int position )
{
	Insert( cpy.strBuff, cpy.strLen, position );
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
		throw_(45) /* mismatched data type */;
	
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

void BACnetOctetString::Encode( char *enc ) const
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
			throw_(46) /* invalid character */;
		upperNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

		c = toupper( *dec++ );
		if (!isxdigit(c))
			throw_(47) /* invalid character */;
		lowerNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

		// stick this on the end
		Append( (upperNibble << 4) + lowerNibble );
	}
}


BACnetEncodeable * BACnetOctetString::clone()
{
	return new BACnetOctetString(*this);
}



bool BACnetOctetString::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetOctetString)));

	if ( EqualityRequiredFailure(rbacnet, iOperator, pstrError) )
		return false;

	bool fMatch = memcmp(strBuff, ((BACnetOctetString &) rbacnet).strBuff, min(((BACnetOctetString &) rbacnet).strBuffLen, strBuffLen)) == 0;

	if ( (fMatch && iOperator == '=') || (!fMatch && iOperator == '!=') )
		return true;

	return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);
}



IMPLEMENT_DYNAMIC(BACnetWeekNDay, BACnetOctetString)


BACnetWeekNDay::BACnetWeekNDay()
			   :BACnetOctetString(3)
{
	Initialize(0xFF, 0xFF, 0xFF);
}



BACnetWeekNDay::BACnetWeekNDay( int nMonth, int nWeekOfMonth, int nDayOfWeek )
			   :BACnetOctetString(3)
{
	Initialize(nMonth, nWeekOfMonth, nDayOfWeek);
}


BACnetWeekNDay::BACnetWeekNDay( BACnetAPDUDecoder& dec )
			   :BACnetOctetString(3)
{
	Decode(dec);
}


void BACnetWeekNDay::Initialize( int nMonth, int nWeekOfMonth, int nDayOfWeek )
{
	m_nMonth = nMonth;
	m_nWeekOfMonth = nWeekOfMonth;
	m_nDayOfWeek = nDayOfWeek;

	ASSERT( (nMonth > 0 && nMonth < 13) || nMonth == 0xFF );
	ASSERT( (nWeekOfMonth > 0 && nWeekOfMonth < 7) || nWeekOfMonth == 0xFF );
	ASSERT( (nDayOfWeek > 0 && nDayOfWeek < 8) || nDayOfWeek == 0xFF );
	LoadBuffer();
}


void BACnetWeekNDay::LoadBuffer()
{
	strBuff[0] = (BACnetOctet) m_nMonth;
	strBuff[1] = (BACnetOctet) m_nWeekOfMonth;
	strBuff[2] = (BACnetOctet) m_nDayOfWeek;
}


void BACnetWeekNDay::UnloadBuffer()
{
	m_nMonth = strBuff[0];
	m_nWeekOfMonth = strBuff[1];
	m_nDayOfWeek = strBuff[2];
}


void BACnetWeekNDay::Encode( BACnetAPDUEncoder& enc, int context )
{
	// just in case the values have changed...
	LoadBuffer();
	BACnetOctetString::Encode(enc,context);
}



void BACnetWeekNDay::Decode( BACnetAPDUDecoder& dec )
{
	BACnetOctetString::Decode(dec);
	UnloadBuffer();
}


void BACnetWeekNDay::Encode( char *enc ) const
{
	sprintf(enc, "%d, %d, %d", m_nMonth, m_nWeekOfMonth, m_nDayOfWeek);
}



BACnetEncodeable * BACnetWeekNDay::clone()
{
	return new BACnetWeekNDay(m_nMonth, m_nWeekOfMonth, m_nDayOfWeek);
}


bool BACnetWeekNDay::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetWeekNDay)));

	if ( EqualityRequiredFailure(rbacnet, iOperator, pstrError) )
		return false;

	bool fMatch = rbacnet.IsKindOf(RUNTIME_CLASS(BACnetWeekNDay)) && 
		          m_nDayOfWeek == ((BACnetWeekNDay &) rbacnet).m_nDayOfWeek &&
		          m_nMonth == ((BACnetWeekNDay &) rbacnet).m_nMonth &&
		          m_nWeekOfMonth == ((BACnetWeekNDay &) rbacnet).m_nWeekOfMonth;

	if ( (iOperator == '=' && !fMatch) || (iOperator == '!=' && fMatch) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}



IMPLEMENT_DYNAMIC(BACnetBitString, BACnetEncodeable)

//
//	BACnetBitString::BACnetBitString
//

BACnetBitString::BACnetBitString( void )
	: bitLen(0), bitBuffLen(0), bitBuff(0)
{
}


BACnetBitString::BACnetBitString( BACnetAPDUDecoder & dec )
	: bitLen(0), bitBuffLen(0), bitBuff(0)
{
	Decode(dec);
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


BACnetBitString::BACnetBitString( int siz, unsigned char * pbits )
	: bitLen(siz), bitBuffLen((siz + 31) / 32), bitBuff(0)
{
	bitBuff = new unsigned long[ bitBuffLen ];
	if (bitBuff)
		LoadBitsFromByteArray(pbits);
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


void BACnetBitString::LoadBitsFromByteArray( unsigned char * pabBits )
{
	ASSERT(pabBits != NULL);

	// bits stored in char array are already in BACnet order, that is, left to right where
	// msb is first flag, etc.

	for ( int i = 0; pabBits != NULL && i < bitLen; i++ )
		SetBit(i, pabBits[i/8] & (unsigned char)(1<<(7-(i%8))) );
}


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

int BACnetBitString::GetBit( int bit ) const
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

bool BACnetBitString::operator ==( BACnetBitString &arg )
{
//	int				i,	siz = (bitBuffLen < arg.bitBuffLen ? bitBuffLen : arg.bitBuffLen);
//	unsigned long	*src = arg.bitBuff,	*dst = bitBuff;
		
	if (bitLen != arg.bitLen)
		return false;

//	for (i = 0; i < siz; i++)
//		if (*dst++ != *src++)
//			return false;

	// ### perhaps last bitBuff element shouldn't always have all bits compared
	// Right!  So let's compare one bit at a time.

	for ( int i = 0; i < bitLen  && GetBit(i) == arg.GetBit(i);  i++ );			// [] doesn't work for some reason

	return i >= bitLen;		// made it through, must be OK
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

#ifdef ENDIAN_SWAP
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
		throw_(48) /* mismatched data type */;
	
	// make sure the destination has enough space
	bLen = ((tag.tagLVT - 1) * 8 - (dec.pktLength--,*dec.pktBuffer++));
 	if (bLen > bitLen)
		SetSize( bLen );
	tag.tagLVT -= 1;
	
	if (!bitBuff)
		throw_(49) /* destination too small */;
	
	// copy out the data
#ifdef ENDIAN_SWAP
	int i = 0, copyLen = tag.tagLVT;

	// don't copy more than copyLen octets
	while (dec.pktLength && copyLen) {
		bitBuff[i] = 0;
		for (int j = 3; (dec.pktLength - 1) > 0 && j >= 0; j--) {
			bitBuff[i] |= (dec.pktLength--,*dec.pktBuffer++) << (j * 8);
			if (!--copyLen)
				break;
		}
		i += 1;
	}
#else
	memcpy( bitBuff, dec.pktBuffer, (size_t)tag.tagLVT );

	bitLen = bLen;
	dec.pktBuffer += tag.tagLVT;
	dec.pktLength -= tag.tagLVT;
#endif
}

void BACnetBitString::Encode( char *enc ) const
{
	*enc++ = 'B';
	*enc++ = '\'';

/*	// encode the content
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
*/

	for (int i = 0; i < bitLen; i++)			// madanner 10/02
		*enc++ = '0' + (int) GetBit(i);

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
			throw_(50) /* invalid character */;
		bit += 1;
	}
}


int BACnetBitString::DataType()
{
	return bits;
}

BACnetEncodeable * BACnetBitString::clone()
{
	BACnetBitString * pbits = new BACnetBitString();
	*pbits = *this;
	return pbits;
}


bool BACnetBitString::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetBitString)));

	if ( EqualityRequiredFailure(rbacnet, iOperator, pstrError) )
		return false;

	bool fMatch = *this == ((BACnetBitString &) rbacnet);

	if ( (iOperator == '=' && !fMatch) || (iOperator == '!=' && fMatch) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}




IMPLEMENT_DYNAMIC(BACnetDate, BACnetEncodeable)

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
	if ( y == 0 && m == 0 && d == 0 )
		year = month = day = DATE_DONT_CARE;

	CalcDayOfWeek();
}


BACnetDate::BACnetDate( BACnetAPDUDecoder & dec )
{
	Decode(dec);
}


bool BACnetDate::IsValid()
{
	if ( dayOfWeek != DATE_DONT_CARE && dayOfWeek != DATE_SHOULDNT_CARE && (dayOfWeek < 1 || dayOfWeek > 7) )
		return false;

	if ( day != DATE_DONT_CARE && day != DATE_SHOULDNT_CARE && (day < 1 || day > 31) )
		return false;

	if ( month != DATE_DONT_CARE && month != DATE_SHOULDNT_CARE && (month < 1 || month > 12 ))
		return false;

	if ( year != DATE_DONT_CARE && year != DATE_SHOULDNT_CARE && (year < 0 || year > 255))
		return false;

	return true;
}

//
//	BACnetDate::CalcDayOfWeek
//

void BACnetDate::CalcDayOfWeek( void )
{
//	int		a, b, c, y, m;
	
	// dont even try with "unspecified" or "don't care" values
	if ((year == DATE_DONT_CARE) || (month == DATE_DONT_CARE) || (day == DATE_DONT_CARE) ||
	    (year == DATE_SHOULDNT_CARE) || (month == DATE_SHOULDNT_CARE) || (day == DATE_SHOULDNT_CARE)) {
		dayOfWeek = DATE_DONT_CARE;
		return;
	}

	// CTime version says 1 = Sunday...  Oops... Convert to 1 = Monday
	dayOfWeek = (Convert().GetDayOfWeek() + 5) % 7 + 1;
	
	// thanks Dr McKenna!
//	a = (month <= 2) ? 1 : 0;
//	y = year + 1900 - a;
//	m = month + 10 * a - 2 * (1 - a);
//	c = y / 100;
//	a = y % 100;
//	b = ((13 * m - 1) / 5) + (a / 4) + (c / 4);
//	dayOfWeek = (b + a + day - 2 * c) % 7;		// 0=Sunday, ...
	
	// translate to BACnet order, 1=Monday, ...
//	dayOfWeek = (dayOfWeek + 6) % 7 + 1;
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
		throw_(51) /* mismatched data type */;
	if (tag.tagLVT != 4)
		throw_(52) /* four bytes of data expected */;
	
	// copy out the data
	year		= *dec.pktBuffer++;
	month		= *dec.pktBuffer++;
	day			= *dec.pktBuffer++;
	dayOfWeek	= *dec.pktBuffer++;
	dec.pktLength -= 4;

	if ( !IsValid() )
		throw_(90);		// invalid values in date
}

void BACnetDate::Encode( char *enc ) const
{
	static char *dow[] = { "", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" }
	;

	// Date is complex data structure... must use brackets to enclose data
	*enc++ = '[';

	// day of week
	if (dayOfWeek == DATE_DONT_CARE)
		*enc++ = '?';
	else if (dayOfWeek == DATE_SHOULDNT_CARE)
		*enc++ = '*';
	else if ((dayOfWeek > 0) && (dayOfWeek < 8)) {
		strcpy( enc, dow[dayOfWeek] );
		enc += 3;
	} else
		*enc++ = '?';

	*enc++ = ' ';

	// month
	if (month == DATE_DONT_CARE)
		*enc++ = '?';
	else if (month == DATE_SHOULDNT_CARE)
		*enc++ = '*';
	else
	{
		sprintf( enc, "%d", month );
		while (*enc) enc++;
	}

	*enc++ = '/';

	// day
	if (day == DATE_DONT_CARE)
		*enc++ = '?';
	else if (day == DATE_SHOULDNT_CARE)
		*enc++ = '*';
	else {
		sprintf( enc, "%d", day );
		while (*enc) enc++;
	}

	*enc++ = '/';

	// year
	if (year == DATE_DONT_CARE) {
		*enc++ = '?';
		*enc = 0;
	} else if (year == DATE_SHOULDNT_CARE) {
		*enc++ = '*';
		*enc = 0;
	} else
		sprintf( enc, "%d", year + 1900 );

	// Date is complex data type so must enclose in brackets for proper parsing
	strcat(enc, "]");
}


void BACnetDate::Decode( const char *dec )
{
	static char *dow[] = { "", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN" }
	;
	char		dowBuff[4]
	;

	// initialize everything to don't care
	dayOfWeek = month = day = year = DATE_DONT_CARE;

	// skip blank on front
	while (*dec && isspace(*dec)) dec++;

	// Date is complex data type so must enclose in brackets
	if ( *dec++ != '[' )
		throw_(110);			// missing start bracket for complex data structures

	// check for dow
	if ( *dec == '*' )
	{
		dayOfWeek = DATE_SHOULDNT_CARE;
		dec += 1;
	}
	else if ( *dec == '?' )
		dec += 1;
	else if (isalpha(*dec)) {
		// They've provided a 3 char day...  read it and test for validity

		for (int j = 0; (j < 3) && isalpha(*dec); j++)
			dowBuff[j] = toupper(*dec++);
		dowBuff[3] = 0;

		for (int i = 1; i < 8 && dayOfWeek == DATE_DONT_CARE; i++)
			if (strcmp(dowBuff,dow[i]) == 0)
				dayOfWeek = i;

		// scanned through... test for validity
		if ( dayOfWeek == DATE_DONT_CARE )
			throw_(91);									// code for bad supplied day (interpreted in caller's context)

		while (*dec!=',' && !isspace(*dec)) dec++;    //Modified by Liangping Xu
	}
	while (*dec && isspace(*dec)) dec++;

	// skip over comma and more space
	if (*dec == ',') {
		dec += 1;
		while (*dec && isspace(*dec)) dec++;
	}

	// check for month
	if ( *dec == '*' )
	{
		month = DATE_SHOULDNT_CARE;
		dec += 1;
	}
	else if ( *dec == '?' )
		dec += 1;
	else {
		for (month = 0; isdigit(*dec); dec++)
			month = (month * 10) + (*dec - '0');

		// they've supplied a month (I think)
		if ( month < 1 || month > 12)
			throw_(92);								// code for bad month, , interpreted in caller's context
	}
	while (*dec && isspace(*dec)) dec++;
	
	// skip over slash and more space
	// make sure slash is there... or (-)
	if (*dec == '/' || *dec == '-') {
		dec += 1;
		while (*dec && isspace(*dec)) dec++;
	}
	else
		throw_(93);									// code for bad date separator, interpreted in caller's context

	// check for day
	if ( *dec == '*' )
	{
		day = DATE_SHOULDNT_CARE;
		dec += 1;
	}
	else if ( *dec == '?' )
		dec += 1;
	else {
		for (day = 0; isdigit(*dec); dec++)
			day = (day * 10) + (*dec - '0');

		// they've supplied a day (I think)
		if ( day < 1 || day > 31)					// doesn't account for month/day invalids (feb 30)
			throw_(94);								// code for bad day, interpreted in caller's context
	}
	while (*dec && isspace(*dec)) dec++;
	
	// skip over slash and more space
	if (*dec == '/' || *dec == '-') {
		dec += 1;
		while (*dec && isspace(*dec)) dec++;
	}
	else
		throw_(93);									// code for bad date separator, interpreted in caller's context

	// check for year
	if ( *dec == '*' )
	{
		year = DATE_SHOULDNT_CARE;
		dec += 1;
	}
	else if ( *dec == '?' )
		dec += 1;
	else {
		int	yr = -1;			// start with no supplied year

		// if they've supplied any number we'll go through this once at least...
		if ( isdigit(*dec) )
			for (yr = 0; isdigit(*dec); dec++)
				yr = (yr * 10) + (*dec - '0');

		// 0..40 -> 2000..2040, 41.. -> 1941..
		// negative = error

		if ( yr < 0 )
			throw_(94);									// code for no supplied year, interpreted in caller's context
		if (yr < 40)
			year = yr + 100;
		else if (yr < 100)
			year = yr;
		else if ((yr >= 1900) && (yr <= (1900 + 254)))
			year = (yr - 1900);
		else
			throw_(95);									// code for bad year, interpreted in caller's context
	}

	// clear white space and look for close bracket
	while (*dec && isspace(*dec)) dec++;
	if ( *dec++ != ']' )
		throw_(111);									// missing close bracket code

	// if we've gotten this far, all values have been read in and are correct
}



CTime BACnetDate::Convert() const
{
	ASSERT( year != DATE_DONT_CARE  && month != DATE_DONT_CARE && day != DATE_DONT_CARE);
	ASSERT( year != DATE_SHOULDNT_CARE  && month != DATE_SHOULDNT_CARE && day != DATE_SHOULDNT_CARE);
	return CTime(year + 1900, month, day, 0, 0, 0);
}



BACnetDate & BACnetDate::operator =( const BACnetDate & arg )
{
	day = arg.day;
	dayOfWeek = arg.dayOfWeek;
	month = arg.month;
	year = arg.year;

	return *this;
}


int BACnetDate::DataType()
{
	return ptDate;
}


BACnetEncodeable * BACnetDate::clone()
{
	return new BACnetDate(year, month, day);
}


bool BACnetDate::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetDate)));

//  Won't work due to don't care cases
//	CTime dateThis(year + 1900, month + 1, day + 1, 0, 0, 0);
//	CTime dateThat(((BACnetDate &) rbacnet).year + 1900, ((BACnetDate &) rbacnet).month + 1, ((BACnetDate &) rbacnet).day + 1, 0, 0, 0);

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetDate)) ||
		 !Match((BACnetDate &) rbacnet, iOperator) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}



bool BACnetDate::Match( BACnetDate & rdate, int iOperator )
{
	if ( PreMatch(iOperator) )
		return true;

	switch(iOperator)
	{
		case '=':	return *this == rdate;
		case '<':	return *this < rdate;
		case '>':	return *this > rdate;
		case '<=':	return *this <= rdate;
		case '>=':	return *this >= rdate;
		case '!=':	return *this != rdate;
		default:
			ASSERT(0);
	}
	return false;
}




bool ValuesEqual( int v1, int v2 )
{
	// if neither of them are don't cares, then we should check their value... even if one of them
	// is shouldn't care. 

	if ( v1 != DATE_DONT_CARE  &&  v2 != DATE_DONT_CARE  &&  v1 != v2 )
		return false;

	// OK... Now test for equality when one is don't care and other is shouldn't care

	if ( (v1 == DATE_SHOULDNT_CARE && v2 != DATE_DONT_CARE && v2 != DATE_SHOULDNT_CARE ) || 
		 (v2 == DATE_SHOULDNT_CARE && v1 != DATE_DONT_CARE && v1 != DATE_SHOULDNT_CARE ) )
		return false;

	return true;
}


bool ValuesGreater( int v1, int v2 )
{
	if ( v1 != DATE_DONT_CARE  &&  v2 != DATE_DONT_CARE  &&
		 v1 != DATE_SHOULDNT_CARE  &&  v2 != DATE_SHOULDNT_CARE  &&  v1 > v2 )
		return true;

	// we've weeded out the don't care case and already tested for greather than...  if it failed...
	// the next best we can do is equal or less than... in either case, the greater than test failed
	// so we should just return false

	return false;
}


bool ValuesLess( int v1, int v2 )
{
	if ( v1 != DATE_DONT_CARE  &&  v2 != DATE_DONT_CARE  &&
		 v1 != DATE_SHOULDNT_CARE  &&  v2 != DATE_SHOULDNT_CARE  &&  v1 < v2 )
		return true;

	// we've weeded out the don't care case and already tested for less than...  if it failed...
	// the next best we can do is equal or greater... in either case, the less than test failed
	// so we should just return false

	return false;
}


bool BACnetDate::operator ==( const BACnetDate & arg )
{
	// test all of the values... if one is out of sorts... then the whole thing isn't equal

	if ( !ValuesEqual(year, arg.year) )
		return false;

	if ( !ValuesEqual(month, arg.month) )
		return false;

	if ( !ValuesEqual(day, arg.day) )
		return false;

	if ( !ValuesEqual(dayOfWeek, arg.dayOfWeek) )
		return false;

	return true;
}




bool BACnetDate::operator !=( const BACnetDate & arg )
{
	// test all values for equality... if one is not equal... then the whole thing is not equal
	// and we don't have to keep testing...
	// We could probably just invert the == operator, but my brain is hurting

	if ( !ValuesEqual(year, arg.year) )
		return true;

	if ( !ValuesEqual(month, arg.month) )
		return true;

	if ( !ValuesEqual(day, arg.day) )
		return true;

	if ( !ValuesEqual(dayOfWeek, arg.dayOfWeek) )
		return true;

	return false;
}



bool BACnetDate::operator <=( const BACnetDate & arg )
{
	if ( !ValuesEqual(year, arg.year) )
		return ValuesLess(year, arg.year);

	// years are now equal or we don't care...
	if ( !ValuesEqual(month, arg.month) )
		return ValuesLess(month, arg.month);

	// years and months are equal or don't care... check day
	if ( !ValuesEqual(day, arg.day) )
		return ValuesLess(day, arg.day);

	// all important values are either equal or don't care... check final day of week
	if ( !ValuesEqual(dayOfWeek, arg.dayOfWeek) )
		return ValuesLess(dayOfWeek, arg.dayOfWeek);

	// values must be equal
	return true;
}


bool BACnetDate::operator <( const BACnetDate & arg )
{
	if ( !ValuesEqual(year, arg.year) )
		return ValuesLess(year, arg.year);

	// years are now equal or we don't care...
	if ( !ValuesEqual(month, arg.month) )
		return ValuesLess(month, arg.month);

	// years and months are equal or don't care... check day
	if ( !ValuesEqual(day, arg.day) )
		return ValuesLess(day, arg.day);

	// all important values are either equal or don't care... check final day of week
	if ( !ValuesEqual(dayOfWeek, arg.dayOfWeek) )
		return ValuesLess(dayOfWeek, arg.dayOfWeek);

	// values must be equal so return false
	return false;
}


bool BACnetDate::operator >=( const BACnetDate & arg )
{
	if ( !ValuesEqual(year, arg.year) )
		return ValuesGreater(year, arg.year);

	// years are now equal or we don't care...
	if ( !ValuesEqual(month, arg.month) )
		return ValuesGreater(month, arg.month);

	// years and months are equal or don't care... check day
	if ( !ValuesEqual(day, arg.day) )
		return ValuesGreater(day, arg.day);

	// all important values are either equal or don't care... check final day of week
	if ( !ValuesEqual(dayOfWeek, arg.dayOfWeek) )
		return ValuesGreater(dayOfWeek, arg.dayOfWeek);

	// values must be equal now
	return true;
}


bool BACnetDate::operator >( const BACnetDate & arg )
{
	if ( !ValuesEqual(year, arg.year) )
		return ValuesGreater(year, arg.year);

	// years are now equal or we don't care...
	if ( !ValuesEqual(month, arg.month) )
		return ValuesGreater(month, arg.month);

	// years and months are equal or don't care... check day
	if ( !ValuesEqual(day, arg.day) )
		return ValuesGreater(day, arg.day);

	// all important values are either equal or don't care... check final day of week
	if ( !ValuesEqual(dayOfWeek, arg.dayOfWeek) )
		return ValuesGreater(dayOfWeek, arg.dayOfWeek);

	// values must be equal so return false
	return false;
}



void BACnetDate::TestDateComps()
{
	bool f;
	int max = 22;
	BACnetDate date1, date2;
	char * t[] = {  "[WED, 11/20/02]", "[WED, 11/20/02]",			// test years
					"[WED, 11/20/02]", "[WED, 11/20/03]",
					"[WED, 11/20/03]", "[WED, 11/20/00]",
					"[WED, 11/20/?]", "[WED, 11/20/?]",
					"[WED, 11/20/03]", "[WED, 11/20/?]",
					"[WED, 11/20/*]", "[WED, 11/20/*]",
					"[WED, 11/20/03]", "[WED, 11/20/*]",
					"[WED, 11/20/*]", "[WED, 11/20/?]",

					"[WED, 11/20/02]", "[WED, 11/21/02]",			// test months
					"[WED, 11/21/02]", "[WED, 11/20/02]",
					"[WED, 11/?/02]", "[WED, 11/?/02]",
					"[WED, 11/?/02]", "[WED, 11/20/02]",
					"[WED, 11/?/02]", "[WED, 11/*/02]",
					"[WED, 11/*/02]", "[WED, 11/20/02]",
					"[WED, 11/*/02]", "[WED, 11/*/02]",

					"[WED, 11/20/02]", "[WED, 12/21/02]",			// test days
					"[WED, 12/20/02]", "[WED, 11/20/02]",
					"[WED, ?/20/02]", "[WED, ?/20/02]",
					"[WED, ?/20/02]", "[WED, 11/20/02]",
					"[WED, ?/20/02]", "[WED, */20/02]",
					"[WED, */20/02]", "[WED, 11/20/02]",
					"[WED, */20/02]", "[WED, */20/02]"
				};


	for ( int i = 0; i < max; i++ )
	{
		date1.Decode( (const char *) t[i*2] );
		date2.Decode( (const char *) t[(i*2)+1] );

		f = date1 == date2;
		f = date1 != date2;
		f = date1 <  date2;
		f = date1 <= date2;
		f = date1 >  date2;
		f = date1 >= date2;
	}
}


IMPLEMENT_DYNAMIC(BACnetTime, BACnetEncodeable)

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



BACnetTime::BACnetTime( BACnetAPDUDecoder & dec )
{
	Decode(dec);
}


bool BACnetTime::IsValid()
{
	if ( hour != DATE_DONT_CARE && hour != DATE_SHOULDNT_CARE && (hour < 0 || hour > 23) )
		return false;

	if ( minute != DATE_DONT_CARE &&  minute != DATE_SHOULDNT_CARE  &&  (minute < 0 || minute > 59) )
		return false;

	if ( second != DATE_DONT_CARE &&  second != DATE_SHOULDNT_CARE &&  (second < 0 || second > 59 ))
		return false;

	if ( hundredths != DATE_DONT_CARE &&  hundredths != DATE_SHOULDNT_CARE &&  (hundredths < 0 ||  hundredths > 99))
		return false;

	return true;
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
		throw_(53) /* mismatched data type */;
	if (tag.tagLVT != 4)
		throw_(54) /* four bytes of data expected */;
	
	// copy out the data
	hour		= *dec.pktBuffer++;
	minute		= *dec.pktBuffer++;
	second		= *dec.pktBuffer++;
	hundredths	= *dec.pktBuffer++;
	dec.pktLength -= 4;

	if ( !IsValid() )
		throw_(100);					// code meaning invalid time values, interpreted by caller's context
}

void BACnetTime::Encode( char *enc ) const
{
	// Time is complex data structure... must use brackets to enclose data
	*enc++ = '[';

	// hour
	if (hour == DATE_DONT_CARE)
		*enc++ = '?';
	else if (hour == DATE_SHOULDNT_CARE)
		*enc++ = '*';
	else {
		sprintf( enc, "%02d", hour );
		enc += 2;
	}
	*enc++ = ':';

	// minute
	if (minute == DATE_DONT_CARE)
		*enc++ = '?';
	else if (minute == DATE_SHOULDNT_CARE)
		*enc++ = '*';
	else {
		sprintf( enc, "%02d", minute );
		enc += 2;
	}
	*enc++ = ':';

	// second
	if (second == DATE_DONT_CARE)
		*enc++ = '?';
	else if (second == DATE_SHOULDNT_CARE)
		*enc++ = '*';
	else {
		sprintf( enc, "%02d", second );
		enc += 2;
	}
	*enc++ = '.';

	// hundredths
	if (hundredths == DATE_DONT_CARE) {
		*enc++ = '?';
		*enc = 0;
	} else if (hundredths == DATE_SHOULDNT_CARE) {
		*enc++ = '*';
		*enc = 0;
	}
	else 
		sprintf( enc, "%02d", hundredths );

	// Time is complex data type so must enclose in brackets for proper parsing
	strcat(enc, "]");
}


void BACnetTime::Decode( const char *dec )
{
	// defaults
	hour = minute = second = hundredths = DATE_DONT_CARE;

	// skip blank on front
	while (*dec && isspace(*dec)) dec++;   //add by xlp

	// Time is complex data type so must enclose in brackets
	if ( *dec++ != '[' )
		throw_(110);			// missing start bracket for complex data structures

	// check for hour
	if ( *dec == '*' )
	{
		hour = DATE_SHOULDNT_CARE;
		dec += 1;
	}
	else if ( *dec == '?' )
		dec += 1;
	else {
		// dont' care not specified, they MUST specify hours...
		hour = -1;
		
		if ( isdigit(*dec) )
			for (hour = 0; isdigit(*dec); dec++)
				hour = (hour * 10) + (*dec - '0');

		// test validity and report
		if ( hour < 0 || hour > 23 )
			throw_(101);									// invalid hour specification, interpreted by caller's context
	}
	// add by xlp
	while (*dec && isspace(*dec)) dec++;

	if (*dec == ':')
		dec += 1;
	else
		throw_(102);										// bad time separator, (used to be 55 invalid character)

	// add by xlp
	while (*dec && isspace(*dec)) dec++;

	// check for minute
	if ( *dec == '*' ) 
	{
		minute = DATE_SHOULDNT_CARE;
		dec += 1;
	}
	else if ( *dec == '?' )
		dec += 1;
	else {
		// MUST now supply minute value
		minute = -1;
		
		if ( isdigit(*dec) )
			for (minute = 0; isdigit(*dec); dec++)
				minute = (minute * 10) + (*dec - '0');

		// test validity and report
		if ( minute < 0 || minute > 59 )
			throw_(103);									// invalid minute specification, interpreted by caller's context
	}
	// add by xlp
	while (*dec && isspace(*dec)) dec++;

	if (*dec == ':')
		dec += 1;
	else
		throw_(102);										// bad time separator, (used to be 56 invalid character)

	// add by xlp
	while (*dec && isspace(*dec)) dec++;

	// check for second
	if ( *dec == '*' )
	{
		second = DATE_SHOULDNT_CARE;
		dec += 1;
	}
	else if ( *dec == '?' )
		dec += 1;
	else {
		// MUST now supply the second value
		second = -1;
		
		if ( isdigit(*dec) )
			for (second = 0; isdigit(*dec); dec++)
				second = (second * 10) + (*dec - '0');

		// test validity and report
		if ( second < 0 || second > 59 )
			throw_(104);									// invalid second specification, interpreted by caller's context
	}
	// add by xlp
	while (*dec && isspace(*dec)) dec++;

	// hundredths specification is optional...
	if (*dec == '.')
	{
		dec += 1;

		// so we're now scanning for a valid hundredth number
		while (*dec && isspace(*dec)) dec++;

		if ( *dec == '*' )
		{
			hundredths = DATE_SHOULDNT_CARE;
			dec += 1;
		}
		else if ( *dec == '?' )
			dec += 1;
		else {
			// MUST now supply the hundredth value
			hundredths = -1;
		
			if ( isdigit(*dec) )
				for (hundredths = 0; isdigit(*dec); dec++)
					hundredths = (hundredths * 10) + (*dec - '0');

			// test validity and report
			if ( hundredths < 0 || hundredths > 99 )
				throw_(105);									// invalid hundredth specification, interpreted by caller's context
		}
	}

	// clear white space and look for close bracket
	while (*dec && isspace(*dec)) dec++;
	if ( *dec++ != ']' )
		throw_(111);									// missing close bracket code
}




BACnetTime & BACnetTime::operator =( const BACnetTime & arg )
{
	hour = arg.hour;
	hundredths = arg.hundredths;
	minute = arg.minute;
	second = arg.second;

	return *this;
}


int BACnetTime::DataType()
{
	return ptTime;
}

BACnetEncodeable * BACnetTime::clone()
{
	return new BACnetTime(hour, minute, second, hundredths);
}


bool BACnetTime::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetTime)));

	// must use computed value because CTime doesn't deal with hundredths...
	// doesn't work due to DON'T CAREs
//	long int ltimeThis = hour * 3600 + minute * 60 + second;
//	long int ltimeThat = ((BACnetTime &) rbacnet).hour * 3600 + ((BACnetTime &) rbacnet).minute * 60 + ((BACnetTime &) rbacnet).second;

//	ltimeThis = ltimeThis * 100 + hundredths;
//	ltimeThat = ltimeThat * 100 + ((BACnetTime &) rbacnet).hundredths;

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetTime)) ||
	     !Match((BACnetTime &) rbacnet, iOperator) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}


bool BACnetTime::Match( BACnetTime & rtime, int iOperator )
{
	if ( PreMatch(iOperator) )
		return true;

	switch(iOperator)
	{
		case '=':	return *this == rtime;
		case '<':	return *this < rtime;
		case '>':	return *this > rtime;
		case '<=':	return *this <= rtime;
		case '>=':	return *this >= rtime;
		case '!=':	return *this != rtime;
		default:
			ASSERT(0);
	}
	return false;
}



bool BACnetTime::operator ==( const BACnetTime & arg )
{
	// test all of the values... if one is out of sorts... then the whole thing isn't equal

	if ( !ValuesEqual(hour, arg.hour) )
		return false;

	if ( !ValuesEqual(minute, arg.minute) )
		return false;

	if ( !ValuesEqual(second, arg.second) )
		return false;

	if ( !ValuesEqual(hundredths, arg.hundredths) )
		return false;

	return true;
}


bool BACnetTime::operator !=( const BACnetTime & arg )
{
	// test all values for equality... if one is not equal... then the whole thing is not equal
	// and we don't have to keep testing...

	if ( !ValuesEqual(hour, arg.hour) )
		return true;

	if ( !ValuesEqual(minute, arg.minute) )
		return true;

	if ( !ValuesEqual(second, arg.second) )
		return true;

	if ( !ValuesEqual(hundredths, arg.hundredths) )
		return true;

	return false;
}


bool BACnetTime::operator <=( const BACnetTime & arg )
{
	if ( !ValuesEqual(hour, arg.hour) )
		return ValuesLess(hour, arg.hour);

	// years are now equal or we don't care...
	if ( !ValuesEqual(minute, arg.minute) )
		return ValuesLess(minute, arg.minute);

	// years and months are equal or don't care... check day
	if ( !ValuesEqual(second, arg.second) )
		return ValuesLess(second, arg.second);

	// all important values are either equal or don't care... check final day of week
	if ( !ValuesEqual(hundredths, arg.hundredths) )
		return ValuesLess(hundredths, arg.hundredths);

	// values must be equal
	return true;
}


bool BACnetTime::operator <( const BACnetTime & arg )
{
	if ( !ValuesEqual(hour, arg.hour) )
		return ValuesLess(hour, arg.hour);

	// hours are now equal or we don't care...
	if ( !ValuesEqual(minute, arg.minute) )
		return ValuesLess(minute, arg.minute);

	// hours and minutes are equal or don't care... check second
	if ( !ValuesEqual(second, arg.second) )
		return ValuesLess(second, arg.second);

	// all important values are either equal or don't care... check final second of week
	if ( !ValuesEqual(hundredths, arg.hundredths) )
		return ValuesLess(hundredths, arg.hundredths);

	// values must be equal so return false
	return false;
}


bool BACnetTime::operator >=( const BACnetTime & arg )
{
	if ( !ValuesEqual(hour, arg.hour) )
		return ValuesGreater(hour, arg.hour);

	// hours are now equal or we don't care...
	if ( !ValuesEqual(minute, arg.minute) )
		return ValuesGreater(minute, arg.minute);

	// hours and minutes are equal or don't care... check second
	if ( !ValuesEqual(second, arg.second) )
		return ValuesGreater(second, arg.second);

	// all important values are either equal or don't care... check final second of week
	if ( !ValuesEqual(hundredths, arg.hundredths) )
		return ValuesGreater(hundredths, arg.hundredths);

	// values must be equal now
	return true;
}


bool BACnetTime::operator >( const BACnetTime & arg )
{
	if ( !ValuesEqual(hour, arg.hour) )
		return ValuesGreater(hour, arg.hour);

	// hours are now equal or we don't care...
	if ( !ValuesEqual(minute, arg.minute) )
		return ValuesGreater(minute, arg.minute);

	// hours and minutes are equal or don't care... check second
	if ( !ValuesEqual(second, arg.second) )
		return ValuesGreater(second, arg.second);

	// all important values are either equal or don't care... check final second of week
	if ( !ValuesEqual(hundredths, arg.hundredths) )
		return ValuesGreater(hundredths, arg.hundredths);

	// values must be equal so return false
	return false;
}


void BACnetTime::TestTimeComps()
{
	bool f;
	int max = 7;
	BACnetTime t1, t2;
	char * t[] = {  "[10:11:12.99]", "[10:11:12.99]",
					"[10:11:12.99]", "[10:11:12.50]",
					"[10:11:12.99]", "[10:11:12.*]",
					"[10:11:12.99]", "[10:*:12.50]",
					"[10:?:12.99]", "[10:11:12.50]",
					"[10:*:12.99]", "[10:?:12.50]",
					"[?:11:12.99]", "[12:?:15.50]"
				};


	for ( int i = 0; i < max; i++ )
	{
		t1.Decode( (const char *) t[i*2] );
		t2.Decode( (const char *) t[(i*2)+1] );

		f = t1 == t2;
		f = t1 != t2;
		f = t1 <  t2;
		f = t1 <= t2;
		f = t1 >  t2;
		f = t1 >= t2;
	}
}


//
//	BACnetDateTime ===============================================
//

IMPLEMENT_DYNAMIC(BACnetDateTime, BACnetEncodeable)

BACnetDateTime::BACnetDateTime( void )
{
	ctime = CTime::GetCurrentTime();

	struct tm	*currtime = ctime.GetLocalTm(NULL);
	
	bacnetTime.hour = currtime->tm_hour;
	bacnetTime.minute = currtime->tm_min;
	bacnetTime.second = currtime->tm_sec;
	bacnetTime.hundredths = 0;
	bacnetDate.year = currtime->tm_year;
	bacnetDate.month = currtime->tm_mon + 1;
	bacnetDate.day = currtime->tm_mday;
	bacnetDate.CalcDayOfWeek();
}



BACnetDateTime::BACnetDateTime( int y, int m, int d, int hr, int mn, int sc, int hun )
			   :bacnetDate(y,m,d), bacnetTime(hr,mn,sc,hun)
{
}


BACnetDateTime::BACnetDateTime( BACnetAPDUDecoder & dec )
			   :bacnetDate(DATE_DONT_CARE,DATE_DONT_CARE,DATE_DONT_CARE), bacnetTime(DATE_DONT_CARE,DATE_DONT_CARE,DATE_DONT_CARE,DATE_DONT_CARE)
{
	Decode(dec);
}



void BACnetDateTime::Encode( BACnetAPDUEncoder& enc, int context )
{
	bacnetDate.Encode(enc, context);
	bacnetTime.Encode(enc, context);
}


void BACnetDateTime::Decode( BACnetAPDUDecoder& dec )
{
	bacnetDate.Decode(dec);
	bacnetTime.Decode(dec);
}


void BACnetDateTime::Encode( char *enc ) const
{
	// DateTime is complex data structure... must use brackets to enclose data
	*enc++ = '[';

	bacnetDate.Encode(enc);
	strcat(enc, ", ");
	bacnetTime.Encode(enc + strlen(enc));

	strcat(enc, "]");
}


void BACnetDateTime::Decode( const char *dec )
{
	// skip blank on front
	while (*dec && isspace(*dec)) dec++;

	// Whole thing  is complex data type so must enclose in brackets
	if ( *dec++ != '[' )
		throw_(110);			// missing start bracket for complex data structures

	bacnetDate.Decode(dec);

	// skip blank on front
	while (*dec && isspace(*dec)) dec++;

	// skip over comma and more space
	if (*dec++ != ',')
		throw_(120);			// missing comma

	bacnetTime.Decode(dec);

	// clear white space and look for close bracket
	while (*dec && isspace(*dec)) dec++;
	if ( *dec++ != ']' )
		throw_(111);									// missing close bracket code
}


BACnetDateTime & BACnetDateTime::operator =( const BACnetDateTime & arg )
{
	bacnetDate = arg.bacnetDate;
	bacnetTime = arg.bacnetTime;
	return *this;
}


int BACnetDateTime::DataType()
{
	return dt;
}

BACnetEncodeable * BACnetDateTime::clone()
{
	BACnetDateTime * pdatetime = new BACnetDateTime();
	*pdatetime = *this;
	return pdatetime;
}



bool BACnetDateTime::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetDateTime)));

	// CTimes don't work due to DONT_CARE value.  Too bad too.  CTime would work quite well
//	CTime dateThis(bacnetDate.year + 1900, bacnetDate.month + 1, bacnetDate.day + 1, bacnetTime.hour, bacnetTime.minute, bacnetTime.second);
//	CTime dateThat(((BACnetDateTime &) rbacnet).bacnetDate.year + 1900, ((BACnetDateTime &) rbacnet).bacnetDate.month + 1, ((BACnetDateTime &) rbacnet).bacnetDate.day + 1,
//		           ((BACnetDateTime &) rbacnet).bacnetTime.hour, ((BACnetDateTime &) rbacnet).bacnetTime.minute, ((BACnetDateTime &) rbacnet).bacnetTime.second);

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetDateTime)) ||
		 !Match((BACnetDateTime &) rbacnet, iOperator) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}


CTime BACnetDateTime::Convert()
{
	ASSERT( bacnetDate.year != DATE_DONT_CARE  && bacnetDate.month != DATE_DONT_CARE && bacnetDate.day != DATE_DONT_CARE);
	ASSERT( bacnetTime.hour != DATE_DONT_CARE  && bacnetTime.minute != DATE_DONT_CARE && bacnetTime.second != DATE_DONT_CARE);

	return CTime(bacnetDate.year + 1900, bacnetDate.month, bacnetDate.day, bacnetTime.hour, bacnetTime.minute, bacnetTime.second);
}



bool BACnetDateTime::Match( BACnetDateTime & rdatetime, int iOperator )
{
	if ( PreMatch(iOperator) )
		return true;

	switch(iOperator)
	{
		case '=':	return *this == rdatetime;
		case '<':	return *this < rdatetime;
		case '>':	return *this > rdatetime;
		case '<=':	return *this <= rdatetime;
		case '>=':	return *this >= rdatetime;
		case '!=':	return *this != rdatetime;
		default:
			ASSERT(0);
	}
	return false;
}



bool BACnetDateTime::operator ==( const BACnetDateTime & arg )
{
	return bacnetDate == arg.bacnetDate  &&  bacnetTime == arg.bacnetTime;
}


bool BACnetDateTime::operator !=( const BACnetDateTime & arg )
{
	return bacnetDate != arg.bacnetDate  &&  bacnetTime != arg.bacnetTime;
}


bool BACnetDateTime::operator <=( const BACnetDateTime & arg )
{
	return bacnetDate < arg.bacnetDate  || (bacnetDate == arg.bacnetDate && bacnetTime <= arg.bacnetTime);
}


bool BACnetDateTime::operator <( const BACnetDateTime & arg )
{
	return bacnetDate < arg.bacnetDate  || (bacnetDate == arg.bacnetDate && bacnetTime < arg.bacnetTime);
}


bool BACnetDateTime::operator >=( const BACnetDateTime & arg )
{
	return bacnetDate > arg.bacnetDate  || (bacnetDate == arg.bacnetDate && bacnetTime >= arg.bacnetTime);
}


bool BACnetDateTime::operator >( const BACnetDateTime & arg )
{
	return bacnetDate > arg.bacnetDate  || (bacnetDate == arg.bacnetDate && bacnetTime > arg.bacnetTime);
}


//
//	BACnetDateTime ===============================================
//

IMPLEMENT_DYNAMIC(BACnetDateRange, BACnetEncodeable)

BACnetDateRange::BACnetDateRange( void )
{
}



BACnetDateRange::BACnetDateRange( int y, int m, int d, int y2, int m2, int d2 )
			   :bacnetDateStart(y,m,d), bacnetDateEnd(y2, m2, d2)
{
}




BACnetDateRange::BACnetDateRange( BACnetAPDUDecoder& dec )
//				:bacnetDateStart(dec), bacnetDateEnd(dec)		// can't guarantee calling order, but we need it :)
{
	Decode(dec);
}



void BACnetDateRange::Encode( BACnetAPDUEncoder& enc, int context )
{
	bacnetDateStart.Encode(enc, context);
	bacnetDateEnd.Encode(enc, context);
}


void BACnetDateRange::Decode( BACnetAPDUDecoder& dec )
{
	bacnetDateStart.Decode(dec);
	bacnetDateEnd.Decode(dec);
}


void BACnetDateRange::Encode( char *enc ) const
{
	// DateRange is complex data structure... must use brackets to enclose data
	*enc++ = '[';

	bacnetDateStart.Encode(enc);
	strcat(enc, ", ");
	bacnetDateEnd.Encode(enc + strlen(enc));

	strcat(enc, "]");
}


void BACnetDateRange::Decode( const char *dec )
{
	// skip blank on front
	while (*dec && isspace(*dec)) dec++;

	// Whole thing  is complex data type so must enclose in brackets
	if ( *dec++ != '[' )
		throw_(110);			// missing start bracket for complex data structures

	bacnetDateStart.Decode(dec);

	// skip blank on front
	while (*dec && isspace(*dec)) dec++;

	// skip over comma and more space
	if (*dec++ != ',')
		throw_(120);			// missing comma

	bacnetDateEnd.Decode(dec);

	// clear white space and look for close bracket
	while (*dec && isspace(*dec)) dec++;
	if ( *dec++ != ']' )
		throw_(111);									// missing close bracket code
}


int BACnetDateRange::DataType()
{
	return dtrange;
}

BACnetEncodeable * BACnetDateRange::clone()
{
	BACnetDateRange * pdaterange = new BACnetDateRange();
	*pdaterange = *this;
	return pdaterange;
}


bool BACnetDateRange::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetDateRange)));

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetDateRange)) ||
		 !Match((BACnetDateRange &) rbacnet, iOperator) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}



BACnetDateRange & BACnetDateRange::operator =( const BACnetDateRange & arg )
{
	bacnetDateEnd = arg.bacnetDateEnd;
	bacnetDateStart = arg.bacnetDateStart;
	return *this;
}




CTimeSpan BACnetDateRange::GetSpan() const
{
	return (CTimeSpan) (bacnetDateEnd.Convert() - bacnetDateStart.Convert());
}


bool BACnetDateRange::Match( BACnetDateRange & rdaterange, int iOperator )
{
	if ( PreMatch(iOperator) )
		return true;

	switch(iOperator)
	{
		case '=':	return *this == rdaterange;
		case '<':	return *this < rdaterange;
		case '>':	return *this > rdaterange;
		case '<=':	return *this <= rdaterange;
		case '>=':	return *this >= rdaterange;
		case '!=':	return *this != rdaterange;
		default:
			ASSERT(0);
	}
	return false;
}



bool BACnetDateRange::operator ==( const BACnetDateRange & arg )
{
	return bacnetDateStart == arg.bacnetDateStart && bacnetDateEnd == arg.bacnetDateEnd;
}


bool BACnetDateRange::operator !=( const BACnetDateRange & arg )
{
	return bacnetDateStart != arg.bacnetDateStart && bacnetDateEnd != arg.bacnetDateEnd;
}


bool BACnetDateRange::operator <=( const BACnetDateRange & arg )
{
	return (GetSpan() <= arg.GetSpan()) != 0;		// for stupid Microsoft BOOL
}


bool BACnetDateRange::operator <( const BACnetDateRange & arg )
{
	return GetSpan() < arg.GetSpan() != 0;		// for stupid Microsoft BOOL
}


bool BACnetDateRange::operator >=( const BACnetDateRange & arg )
{
	return GetSpan() >= arg.GetSpan() != 0;		// for stupid Microsoft BOOL
}


bool BACnetDateRange::operator >( const BACnetDateRange & arg )
{
	return GetSpan() > arg.GetSpan() != 0;		// for stupid Microsoft BOOL
}




IMPLEMENT_DYNAMIC(BACnetObjectIdentifier, BACnetEncodeable)

//
//	BACnetObjectIdentifier::BACnetObjectIdentifier
//

BACnetObjectIdentifier::BACnetObjectIdentifier( int objType, int instanceNum )
	: objID( (objType << 22) + instanceNum )
{
}


BACnetObjectIdentifier::BACnetObjectIdentifier( unsigned int nobjID )
					   :objID(nobjID)
{
}


BACnetObjectIdentifier::BACnetObjectIdentifier( BACnetAPDUDecoder & dec )
{
	Decode(dec);
}



//
//	BACnetObjectIdentifier::SetValue
//

void BACnetObjectIdentifier::SetValue( BACnetObjectType objType, int instanceNum )
{
	objID = ((int)objType << 22) + instanceNum;
}

void BACnetObjectIdentifier::Encode( BACnetAPDUEncoder& enc, int context )
{
	// encode the tag
	if (context != kAppContext)
		BACnetAPDUTag( context, 4 ).Encode( enc );
	else
		BACnetAPDUTag( objectIdentifierAppTag, 4 ).Encode( enc );
	
	// fill in the data
#ifdef ENDIAN_SWAP
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
		throw_(60) /* mismatched data type */;
	if (tag.tagLVT != 4)
		throw_(61) /* four bytes of data expected */;
	
	// copy out the data
#ifdef ENDIAN_SWAP
	for (int j = 3; dec.pktLength && j >= 0; j--)
		objID = (objID << 8) + (dec.pktLength--,*dec.pktBuffer++);
#else
	memcpy( &objID, dec.pktBuffer, (size_t)4 );
	dec.pktBuffer += 4;
	dec.pktLength -= 4;
#endif
}

void BACnetObjectIdentifier::Encode( char *enc ) const
{
	int		objType = (objID >> 22)
	,		instanceNum = (objID & 0x003FFFFF)
	;
	char	typeBuff[32], *s
	;

#if VTSScanner
	if (objType < MAX_DEFINED_OBJ /* sizeof(NetworkSniffer::BACnetObjectType) */)
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
			throw_(62) /* integer expected */;
		if ((objType < 0) || (objType >= 128))
			throw_(63) /* out of range */;
	} else
	if ((tok.tokenType == scriptKeyword) && (tok.tokenSymbol == kwVENDOR)) {
		// next must be a number in the vendor range
		scan.Next( tok );
		if (!tok.IsInteger( objType ))
			throw_(64) /* integer expected */;
		if ((objType < 128) || (objType >= (1 << 10)))
			throw_(65) /* out of range */;
	} else
	if (!tok.IsInteger( objType, ScriptObjectTypeMap ))
		throw_(66) /* object type keyword expected */;
	else
	if ((objType < 0) || (objType >= (1 << 10)))
		throw_(67) /* out of range */;

	// get the next token
	scan.Next( tok );

	// skip the ',' if it was entered
	if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == ','))
		scan.Next( tok );

	// make sure it's an integer
	if (!tok.IsInteger( instanceNum ))
		throw_(68) /* instance expected */;
	if ((instanceNum < 0) || (instanceNum >= (1 << 22)))
		throw_(69) /* out of range */;

	// everything checks out
	objID = (objType << 22) + instanceNum;
}
#else
void BACnetObjectIdentifier::Decode( const char * )
{
	throw_(70) /* not implemented */;
}
#endif


int BACnetObjectIdentifier::DataType()
{
	return ob_id;
}

BACnetEncodeable * BACnetObjectIdentifier::clone()
{
	return new BACnetObjectIdentifier(objID);
}


bool BACnetObjectIdentifier::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetObjectIdentifier)));

	if ( !rbacnet.IsKindOf(RUNTIME_CLASS(BACnetObjectIdentifier))  ||  !::Match(iOperator, (unsigned long) objID, (unsigned long) ((BACnetObjectIdentifier &) rbacnet).objID ) )
		return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);

	return true;
}


BACnetObjectIdentifier & BACnetObjectIdentifier::operator =( const BACnetObjectIdentifier &arg )
{
	objID = arg.objID;
	return *this;
}


//==============================================================================


IMPLEMENT_DYNAMIC(BACnetAddressBinding, BACnetEncodeable)


BACnetAddressBinding::BACnetAddressBinding()
{
}


BACnetAddressBinding::BACnetAddressBinding( unsigned int nobjID, unsigned short nNet, BACnetOctet * paMAC, unsigned short nMACLen )
				     :m_bacnetObjectID(nobjID), m_bacnetAddr(nNet, paMAC, nMACLen)
{
}


BACnetAddressBinding::BACnetAddressBinding( BACnetAPDUDecoder& dec )
//					 :m_bacnetObjectID(dec), m_bacnetAddr(dec)				// can't guarantee calling order this way
{
	Decode(dec);
}


void BACnetAddressBinding::Encode( BACnetAPDUEncoder& enc, int context )
{
	m_bacnetObjectID.Encode(enc, context);
	m_bacnetAddr.Encode(enc, context);
}


void BACnetAddressBinding::Decode( BACnetAPDUDecoder& dec )
{
	m_bacnetObjectID.Decode(dec);
	m_bacnetAddr.Decode(dec);
}



BACnetAddressBinding & BACnetAddressBinding::operator =( const BACnetAddressBinding & arg )
{
	m_bacnetObjectID = arg.m_bacnetObjectID;
	m_bacnetAddr = arg.m_bacnetAddr;
	return *this;
}


int BACnetAddressBinding::DataType()
{
	return dabind;
}

BACnetEncodeable * BACnetAddressBinding::clone()
{
	BACnetAddressBinding * paddrbind = new BACnetAddressBinding();
	*paddrbind = *this;
	return paddrbind;
}


bool BACnetAddressBinding::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetAddressBinding)));

	if ( EqualityRequiredFailure(rbacnet, iOperator, pstrError) )
		return false;

	bool fMatch = rbacnet.IsKindOf(RUNTIME_CLASS(BACnetAddressBinding)) &&
				  m_bacnetAddr.Match(((BACnetAddressBinding &) rbacnet).m_bacnetAddr, iOperator, pstrError )  &&
				  m_bacnetObjectID.Match(((BACnetAddressBinding &) rbacnet).m_bacnetObjectID, iOperator, pstrError);

	if ( (fMatch  && iOperator == '=')  ||  (!fMatch && iOperator == '!=') )
		return true;

	return BACnetEncodeable::Match(rbacnet, iOperator, pstrError);
}



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
		throw_(71) /* not enough data */;
	
	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// check the type
	if ((tag & 0x0F) != 0x0E)
		throw_(72) /* mismatched tag class */;
	
	// check for a big context
	if ((tag & 0xF0) == 0xF0) {
		if (dec.pktLength < 1)
			throw_(73) /* not enough data */;
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
		throw_(74) /* not enough data */;
	
	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// check the type
	if ((tag & 0x0F) != 0x0F)
		throw_(75) /* mismatched tag class */;
	
	// check for a big context
	if ((tag & 0xF0) == 0xF0) {
		if (dec.pktLength < 1)
			throw_(76) /* not enough data */;
		dec.pktLength -= 1;
	}
}



IMPLEMENT_DYNAMIC(BACnetBinaryPriV, BACnetEnumerated)

BACnetBinaryPriV::BACnetBinaryPriV( int nValue )
			   :BACnetEnumerated(nValue)
{
}



void BACnetBinaryPriV::Encode( char *enc ) const
{
	BACnetEnumerated::Encode(enc, apszBinaryPVNames, 2);
}


void BACnetBinaryPriV::Decode( const char *dec )
{
	BACnetEnumerated::Decode(dec, apszBinaryPVNames, 2);
}


IMPLEMENT_DYNAMIC(BACnetObjectContainer, BACnetEncodeable)

BACnetObjectContainer::BACnetObjectContainer()
{
	pbacnetTypedValue = NULL;
}




BACnetObjectContainer::BACnetObjectContainer( BACnetEncodeable * pbacnetEncodeable )
{
	pbacnetTypedValue = NULL;
	SetObject(pbacnetEncodeable);
}


void BACnetObjectContainer::Decode( BACnetAPDUDecoder& dec )
{
	ASSERT(pbacnetTypedValue != NULL);

	if ( pbacnetTypedValue != NULL )
		pbacnetTypedValue->Decode(dec);
}


void BACnetObjectContainer::Encode( BACnetAPDUEncoder& enc, int context )
{
	ASSERT(pbacnetTypedValue != NULL);

	if ( pbacnetTypedValue != NULL )
		pbacnetTypedValue->Encode(enc, context);
}


void BACnetObjectContainer::Encode( char *enc ) const
{
	ASSERT(pbacnetTypedValue != NULL);

	if ( pbacnetTypedValue != NULL )
		pbacnetTypedValue->Encode(enc);
}


void BACnetObjectContainer::Decode( const char *dec )
{
	ASSERT(pbacnetTypedValue != NULL);

	if ( pbacnetTypedValue != NULL )
		pbacnetTypedValue->Decode(dec);
}



const char * BACnetObjectContainer::ToString() const
{
	return pbacnetTypedValue == NULL ? NULL : pbacnetTypedValue->ToString();
}


void BACnetObjectContainer::SetObject( BACnetEncodeable * pBACnetEncodeable )
{
	if ( pbacnetTypedValue != NULL )
		delete pbacnetTypedValue;

	pbacnetTypedValue = pBACnetEncodeable;
}



bool BACnetObjectContainer::IsObjectType( CRuntimeClass * pruntimeclass )
{
	ASSERT(pruntimeclass != NULL);

	return pbacnetTypedValue == NULL ? false : (pbacnetTypedValue->IsKindOf(pruntimeclass) != 0);	// accounts for BOOL(int) cases to convert to bool.
}


CRuntimeClass * BACnetObjectContainer::GetObjectType()
{
	return pbacnetTypedValue == NULL ? NULL : pbacnetTypedValue->GetRuntimeClass();
}


BACnetEncodeable *  BACnetObjectContainer::GetObject()
{
	return pbacnetTypedValue;
}


BACnetObjectContainer::~BACnetObjectContainer()
{
	SetObject(NULL);
}



//====================================================================

IMPLEMENT_DYNAMIC(BACnetPriorityValue, BACnetObjectContainer)

BACnetPriorityValue::BACnetPriorityValue()
{
}



BACnetPriorityValue::BACnetPriorityValue( BACnetAPDUDecoder & dec )
					:BACnetObjectContainer()
{
	Decode(dec);
}



BACnetPriorityValue::BACnetPriorityValue( BACnetEncodeable * pbacnetEncodeable )
					:BACnetObjectContainer(pbacnetEncodeable)
{
}


void BACnetPriorityValue::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag		tagNullTest;

	// Peek for received null value of zero, don't advance APDU pointer
	tagNullTest.Peek(dec);
	CreateTypedObject(tagNullTest.tagNumber);

	BACnetObjectContainer::Decode(dec);
}


void BACnetPriorityValue::CreateTypedObject(BACnetApplicationTag tag)
{
	switch( tag )
	{
		case nullAppTag:			SetObject(new BACnetNull());			break;
		case unsignedIntAppTag:		SetObject(new BACnetUnsigned());		break;
		case realAppTag:			SetObject(new BACnetReal());			break;
		case enumeratedAppTag:		SetObject(new BACnetBinaryPriV());		break;
		default:

			TRACE1("Attempt to create BACnetEncodeable type %d from Tag", tag );
			ASSERT(0);
			SetObject(NULL);
	}
}


int BACnetPriorityValue::DataType()
{
	return prival;
}


BACnetEncodeable * BACnetPriorityValue::clone()
{
	return new BACnetPriorityValue( pbacnetTypedValue == NULL ? (BACnetEncodeable *) NULL : pbacnetTypedValue->clone());
}


bool BACnetPriorityValue::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetPriorityValue)));
	ASSERT(pbacnetTypedValue != NULL);

	return rbacnet.IsKindOf(RUNTIME_CLASS(BACnetPriorityValue))  &&
		   pbacnetTypedValue->Match(*((BACnetPriorityValue &)rbacnet).GetObject(), iOperator, pstrError);
}



//====================================================================

IMPLEMENT_DYNAMIC(BACnetCalendarEntry, BACnetObjectContainer)

BACnetCalendarEntry::BACnetCalendarEntry()
{
}



BACnetCalendarEntry::BACnetCalendarEntry( BACnetAPDUDecoder & dec )
					:BACnetObjectContainer()
{
	Decode(dec);
}



BACnetCalendarEntry::BACnetCalendarEntry( BACnetEncodeable * pbacnetEncodeable )
					:BACnetObjectContainer(pbacnetEncodeable)
{
}


void BACnetCalendarEntry::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag		tagTestType;

	tagTestType.Decode(dec);

	// Tag has 0, 1, 2 for date, range, weeknday

    switch (tagTestType.tagNumber)
	{
		case 0:					SetObject( new BACnetDate(dec) );	break;
		case 1:					SetObject( new BACnetDateRange(dec) ); break;
		case 2:					SetObject( new BACnetWeekNDay(dec) ); break;
		default:				TRACE0("INVALID type in encoded stream for CalendarEntry");
								ASSERT(0);
	}
}


int BACnetCalendarEntry::DataType()
{
	return calent;
}


BACnetEncodeable * BACnetCalendarEntry::clone()
{
	return new BACnetCalendarEntry( pbacnetTypedValue == NULL ? (BACnetEncodeable *) NULL : pbacnetTypedValue->clone());
}


bool BACnetCalendarEntry::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetCalendarEntry)));
	ASSERT(pbacnetTypedValue != NULL);

	return rbacnet.IsKindOf(RUNTIME_CLASS(BACnetCalendarEntry))  &&
		    pbacnetTypedValue->Match(*((BACnetCalendarEntry &)rbacnet).GetObject(), iOperator, pstrError);
}


//====================================================================

IMPLEMENT_DYNAMIC(BACnetTimeStamp, BACnetObjectContainer)

BACnetTimeStamp::BACnetTimeStamp()
{
}



BACnetTimeStamp::BACnetTimeStamp( BACnetAPDUDecoder & dec )
				:BACnetObjectContainer()
{
	Decode(dec);
}



BACnetTimeStamp::BACnetTimeStamp( BACnetEncodeable * pbacnetEncodeable )
					:BACnetObjectContainer(pbacnetEncodeable)
{
}


void BACnetTimeStamp::Decode( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag		tagTestType;

	tagTestType.Decode(dec);

	// Tag has 0, 1, 2 for date, range, weeknday

    switch (tagTestType.tagNumber)
	{
		case 0:					SetObject( new BACnetTime(dec) );	break;
		case 1:					SetObject( new BACnetUnsigned(dec) ); break;
		case 2:					SetObject( new BACnetDateTime(dec) ); break;
		default:				TRACE0("INVALID type in encoded stream for TimeStamp");
								ASSERT(0);
	}
}


int BACnetTimeStamp::DataType()
{
	return TSTMP;
}



BACnetEncodeable * BACnetTimeStamp::clone()
{
	return new BACnetTimeStamp( pbacnetTypedValue == NULL ? (BACnetEncodeable *) NULL : pbacnetTypedValue->clone());
}


bool BACnetTimeStamp::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetTimeStamp)));
	ASSERT(pbacnetTypedValue != NULL);

	return rbacnet.IsKindOf(RUNTIME_CLASS(BACnetTimeStamp))  &&
		    pbacnetTypedValue->Match(*((BACnetTimeStamp &)rbacnet).GetObject(), iOperator, pstrError);
}




//====================================================================



IMPLEMENT_DYNAMIC(BACnetGenericArray, BACnetEncodeable)


BACnetGenericArray::BACnetGenericArray()
{
}



BACnetGenericArray::BACnetGenericArray( int nSize )
{
	m_apBACnetObjects.SetSize(nSize);
}



void BACnetGenericArray::Decode( BACnetAPDUDecoder& dec )
{
	ClearArray();
	BACnetUnsigned bacnetUnsigned(dec);
	m_apBACnetObjects.SetSize(bacnetUnsigned.uintValue);
}


void BACnetGenericArray::Encode( BACnetAPDUEncoder& enc, int context )
{
	BACnetUnsigned bacnetSize(m_apBACnetObjects.GetSize());
	bacnetSize.Encode(enc, context);

	for ( int i = 0; i < m_apBACnetObjects.GetSize(); i++)
		if ( m_apBACnetObjects[i] != NULL )
			m_apBACnetObjects[i]->Encode(enc, context);
}


void BACnetGenericArray::Encode( char *enc ) const
{
	ASSERT(0); // not implemented yet
}


void BACnetGenericArray::Decode( const char *dec )
{
	ASSERT(0); // not implemented yet
}


const char * BACnetGenericArray::ToString() const
{
	ASSERT(0); // not implemented yet
	return NULL;
}


BACnetEncodeable * BACnetGenericArray::operator[](int nIndex) const
{
	ASSERT(nIndex < m_apBACnetObjects.GetSize());
	return m_apBACnetObjects[nIndex];
}


int BACnetGenericArray::Add( BACnetEncodeable * pbacnetEncodeable )
{
	return m_apBACnetObjects.Add(pbacnetEncodeable);
}



BACnetEncodeable & BACnetGenericArray::operator[](int nIndex)
{
	return *m_apBACnetObjects[nIndex];
}



BACnetGenericArray::~BACnetGenericArray()
{
	ClearArray();
}


int BACnetGenericArray::GetSize()
{
	return m_apBACnetObjects.GetSize();
}


void BACnetGenericArray::ClearArray()
{
	for ( int i = 0; i < m_apBACnetObjects.GetSize(); i++)
		if ( m_apBACnetObjects[i] != NULL )
			delete m_apBACnetObjects[i];

	m_apBACnetObjects.RemoveAll();
}


bool BACnetGenericArray::Match( BACnetEncodeable &rbacnet, int iOperator, CString * pstrError )
{
	if ( PreMatch(iOperator) )
		return true;

//	ASSERT(rbacnet.IsKindOf(RUNTIME_CLASS(BACnetGenericArray)));

	CString strError;

	// want to compare entire array...  Better make sure lengths are the same
	if ( GetSize() != ((BACnetGenericArray &) rbacnet).GetSize() )
	{
		if ( pstrError != NULL )
			pstrError->Format(IDS_SCREX_COMPARRAYSIZE, GetSize(), ((BACnetGenericArray &) rbacnet).GetSize() );

		return false;
	}

	int nIndex;
	for ( nIndex = 0; nIndex < GetSize() && 
			m_apBACnetObjects[nIndex]->Match(((BACnetGenericArray &)rbacnet)[nIndex], iOperator, &strError);
			nIndex++ );

	if ( pstrError != NULL  &&  !strError.IsEmpty() )
	{
		char szBuff[20];

		sprintf(szBuff, "[%d] ", nIndex);
		*pstrError += szBuff + strError;
	}

	return strError.IsEmpty() != 0;		// account for stupid Windows BOOL define
}



//====================================================================
//====================================================================



IMPLEMENT_DYNAMIC(BACnetPriorityArray, BACnetGenericArray)

BACnetPriorityArray::BACnetPriorityArray()
{
}



BACnetPriorityArray::BACnetPriorityArray( BACnetAPDUDecoder& dec )
					:BACnetGenericArray(nPRIO)
{
	Decode(dec);
}


BACnetPriorityArray::BACnetPriorityArray( float * paPriority, int nMax, float fNull )
{
	for ( int i = 0; i < nMax; i++ )
		if ( paPriority[i] == fNull )
			Add(new BACnetPriorityValue(new BACnetNull()));
		else
			Add(new BACnetPriorityValue(new BACnetReal(paPriority[i])));
}


BACnetPriorityArray::BACnetPriorityArray( int * paPriority, int nMax, int bNull )
{
	for ( int i = 0; i < nMax; i++ )
		if ( paPriority[i] == bNull )
			Add(new BACnetPriorityValue(new BACnetNull()));
		else
			Add(new BACnetPriorityValue(new BACnetBinaryPriV(paPriority[i])));
}


BACnetPriorityArray::BACnetPriorityArray( unsigned short * paPriority, int nMax, unsigned short uNull )
{
	for ( int i = 0; i < nMax; i++ )
		if ( paPriority[i] == uNull )
			Add(new BACnetPriorityValue(new BACnetNull()));
		else
			Add(new BACnetPriorityValue(new BACnetUnsigned(paPriority[i])));
}


void BACnetPriorityArray::Decode( BACnetAPDUDecoder& dec )
{
//	BACnetGenericArray::Decode(dec);		// no on priority arrays... assumed to be 16

	for ( int i = 0; i < GetSize(); i++)
		m_apBACnetObjects[i] = new BACnetPriorityValue(dec);
}



BACnetPriorityValue * BACnetPriorityArray::operator[](int nIndex) const
{
	return (BACnetPriorityValue *) m_apBACnetObjects[nIndex];
}



BACnetPriorityValue & BACnetPriorityArray::operator[](int nIndex)
{
	return (BACnetPriorityValue &) *m_apBACnetObjects[nIndex];
}



//====================================================================



IMPLEMENT_DYNAMIC(BACnetCalendarArray, BACnetGenericArray)



BACnetCalendarArray::BACnetCalendarArray()
{
}


BACnetCalendarArray::BACnetCalendarArray( BACnetAPDUDecoder& dec )
{
	Decode(dec);
}



void BACnetCalendarArray::Decode( BACnetAPDUDecoder& dec )
{
	BACnetGenericArray::Decode(dec);

	for ( int i = 0; i < m_apBACnetObjects.GetSize(); i++)
		m_apBACnetObjects[i] = new BACnetCalendarEntry(dec);
}



BACnetCalendarEntry * BACnetCalendarArray::operator[](int nIndex) const
{
	return (BACnetCalendarEntry *) m_apBACnetObjects[nIndex];
}



BACnetCalendarEntry & BACnetCalendarArray::operator[](int nIndex)
{
	return  (BACnetCalendarEntry &) *m_apBACnetObjects[nIndex];
}



//====================================================================


IMPLEMENT_DYNAMIC(BACnetTextArray, BACnetGenericArray)


BACnetTextArray::BACnetTextArray( char * paText[], int nMax /* = -1 */ )
{
	int nSize = 0;

	while ( paText[nSize] != NULL  && (nMax == -1 || nSize < nMax) )
		m_apBACnetObjects.Add(new BACnetCharacterString(paText[nSize++]));
}


// Constructor for only one array element

BACnetTextArray::BACnetTextArray( char * pText )
{
	m_apBACnetObjects.Add(new BACnetCharacterString(pText));
}


BACnetTextArray::BACnetTextArray( BACnetAPDUDecoder& dec )
{
	Decode(dec);
}



void BACnetTextArray::Decode( BACnetAPDUDecoder& dec )
{
	// Don't need unsigned at beginning of decode stream to tell us how many strings follow
	// We'll figure that out by encountering a null in place of a string

//	BACnetGenericArray::Decode(dec);

	BACnetAPDUTag	tag;

	while ( true )
	{
		tag.Peek(dec);
		if ( tag.tagNumber != characterStringAppTag )
			break;

		Add(new BACnetCharacterString(dec));
	}
}


BACnetCharacterString * BACnetTextArray::operator[](int nIndex) const
{
	return (BACnetCharacterString *) m_apBACnetObjects[nIndex];
}



BACnetCharacterString & BACnetTextArray::operator[](int nIndex)
{
	return  (BACnetCharacterString &) *m_apBACnetObjects[nIndex];
}


//====================================================================


IMPLEMENT_DYNAMIC(BACnetUnsignedArray, BACnetGenericArray)


// nMax, if specified, says don't check for zero to stop...

BACnetUnsignedArray::BACnetUnsignedArray( unsigned char paUnsigned[], int nMax /* = -1 */ )
{
	int nSize = 0;

	while ( (paUnsigned[nSize] != 0 || nMax != -1)  && (nMax == -1 || nSize < nMax) )
		m_apBACnetObjects.Add(new BACnetUnsigned( (unsigned int) paUnsigned[nSize++]));
}


BACnetUnsignedArray::BACnetUnsignedArray( unsigned short paUnsigned[], int nMax /* = -1 */ )
{
	// Same as for chars but, well, different.
	// This will probably go away if I change the EPICS internal store to have 'word' types
	// for alarm_values and fault_values in the MSI and MSO object, like in the MSV object.

	int nSize = 0;

	while ( (paUnsigned[nSize] != 0  || nMax != -1)  && (nMax == -1 || nSize < nMax) )
		m_apBACnetObjects.Add(new BACnetUnsigned( (unsigned int) paUnsigned[nSize++]));
}


BACnetUnsignedArray::BACnetUnsignedArray( BACnetAPDUDecoder& dec )
{
	Decode(dec);
}


void BACnetUnsignedArray::Decode( BACnetAPDUDecoder& dec )
{
	BACnetGenericArray::Decode(dec);

	for ( int i = 0; i < m_apBACnetObjects.GetSize(); i++)
		m_apBACnetObjects[i] = new BACnetUnsigned(dec);
}


BACnetUnsigned * BACnetUnsignedArray::operator[](int nIndex) const
{
	return (BACnetUnsigned *) m_apBACnetObjects[nIndex];
}



BACnetUnsigned & BACnetUnsignedArray::operator[](int nIndex)
{
	return  (BACnetUnsigned &) *m_apBACnetObjects[nIndex];
}



IMPLEMENT_DYNAMIC(BACnetObjectIDList, BACnetGenericArray)



BACnetObjectIDList::BACnetObjectIDList()
{
}


BACnetObjectIDList::BACnetObjectIDList( int nSize )
					:BACnetGenericArray(nSize)
{
}


BACnetObjectIDList::BACnetObjectIDList( BACnetAPDUDecoder& dec )
{
	Decode(dec);
}



void BACnetObjectIDList::Decode( BACnetAPDUDecoder& dec )
{
	// Object ID lists suck out of the stream until a non-object ID tag is found...
	BACnetAPDUTag tag;

//	BACnetGenericArray::Decode(dec);		// don't use unsigned int in front of list to determine size

	while (true)
	{
		tag.Peek(dec);
		if ( tag.tagNumber != objectIdentifierAppTag )
			break;

		Add(new BACnetObjectIdentifier(dec));
	}
}


BACnetObjectIdentifier * BACnetObjectIDList::operator[](int nIndex) const
{
	return (BACnetObjectIdentifier *) m_apBACnetObjects[nIndex];
}



BACnetObjectIdentifier & BACnetObjectIDList::operator[](int nIndex)
{
	return  (BACnetObjectIdentifier &) *m_apBACnetObjects[nIndex];
}



//====================================================================

IMPLEMENT_DYNAMIC(BACnetAnyValue, BACnetObjectContainer)

BACnetAnyValue::BACnetAnyValue()
{
	SetType(0);
}




BACnetAnyValue::BACnetAnyValue( BACnetEncodeable * pbacnetEncodeable )
					:BACnetObjectContainer(pbacnetEncodeable)
{
	SetType(0);
}


void BACnetAnyValue::SetType(int nNewType)
{
	m_nType = nNewType;
}


int BACnetAnyValue::GetType()
{
	return m_nType;
}


void BACnetAnyValue::SetObject( int nNewType, BACnetEncodeable * pbacnetEncodeable )
{
	SetType(nNewType);
	BACnetObjectContainer::SetObject(pbacnetEncodeable);
}


// Used to compute type instead of set it...

void BACnetAnyValue::SetObject( BACnetEncodeable * pbacnetEncodeable )
{
	SetType(pbacnetEncodeable->DataType());
	BACnetObjectContainer::SetObject(pbacnetEncodeable);
}




bool BACnetAnyValue::CompareToEncodedStream( BACnetAPDUDecoder & dec, int iOperator, LPCSTR lpstrValueName )
{
	CString strThrowMessage;

	if ( GetObject() == NULL )		// no data found
		strThrowMessage.Format(IDS_SCREX_COMPEPICSNULL, lpstrValueName);

	else if ( dec.pktBuffer == NULL )
		strThrowMessage.Format(IDS_SCREX_COMPDATANULL, lpstrValueName);

	else
	{
		// Have to use type value here because we don't know how to decode the stream...\
		// The type tells us what kind of object to attempt to reconstitute

		switch (GetType())
		{
			case u127:		// 1..127 ---------------------------------
			case u16:		// 1..16 ----------------------------------
			case ud:		// unsigned dword -------------------------
			case uw:		// unsigned word --------------------------

				BACnetUnsigned(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case ssint:		// short signed int -----------------------		// actually the same type
			case sw:		// signed word ----------------------------

				BACnetInteger(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case flt:		// float ----------------------------------------

				BACnetReal(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case pab:		// priority array bpv ---------------------		deal with index cases (-1=all, 0=element count, base 1=index
			case paf:		// priority array flt ---------------------
			case pau:		// priority array unsigned ----------------

				BACnetPriorityArray(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case ebool:		// boolean enumeration ---------------------------------

				BACnetBoolean(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case bits:		// octet of 1 or 0 flags

				BACnetBitString(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case ob_id:		// object identifier

				BACnetObjectIdentifier(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case s10:		// char [10] --------------------------------------------
			case s32:		// char [32]
			case s64:		// char [64]
			case s132:		// char [132]

				BACnetCharacterString(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;
       
			case enull:		// null enumeration ------------------------------------

				BACnetNull(dec).Match(*GetObject(), iOperator, &strThrowMessage);
				break;

			case et:		// generic enumation ----------------------------------

				BACnetEnumerated(dec).Match(*GetObject(), iOperator, &strThrowMessage);
				break;

			case ptDate:	// date ------------------------------------------------

				BACnetDate(dec).Match(*GetObject(), iOperator, &strThrowMessage);
				break;

			case ptTime:	// time -------------------------------------------------

				BACnetTime(dec).Match(*GetObject(), iOperator, &strThrowMessage);
				break;

			case dt:		// date/time stamp -------------------------------------

				BACnetDateTime(dec).Match(*GetObject(), iOperator, &strThrowMessage);
				break;

			case dtrange:	// range of dates ---------------------------------------

				BACnetDateRange(dec).Match(*GetObject(), iOperator, &strThrowMessage);
				break;

			case calist:	// array of calendar entries -----------------------------

				BACnetCalendarArray(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case dabind:	// device address binding --------------------------------

				BACnetAddressBinding(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case lobj:		// array of object identifiers ----------------------------

				BACnetObjectIDList(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case uwarr:		// unsigned array ------------------------------------------
			case stavals:	// list of unsigned ----------------------------------------

				BACnetUnsignedArray(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case statext:
			case actext:	// character string array ----------------------------------

				BACnetTextArray(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case prival:	// single priority value----------------------------------

				BACnetPriorityValue(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case calent:	// single calendar entry ----------------------------------

				BACnetCalendarEntry(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			case TSTMP:		// time stamp, could be multiple type---------------------

				BACnetTimeStamp(dec).Match(*GetObject(), iOperator, &strThrowMessage );
				break;

			default:

				strThrowMessage.Format(IDS_SCREX_COMPUNSUPPORTED, GetType() );
		}
    }


	if ( !strThrowMessage.IsEmpty() )
	{
		CString strFailString;
		strFailString.Format(IDS_SCREX_COMPFAIL, lpstrValueName, (LPCSTR) strThrowMessage );

		throw CString(strFailString);
	}

	return true;
}





/*  CASES NOT IMPLEMENTED YET...  Soon to follow (9/02) 

		case raslist: // list of readaccessspecs
             p= eRASLIST(p,(BACnetReadAccessSpecification far*)msg->pv);
			break;

		case act: // action array
             p= eACT(p,(BACnetActionCommand far**)msg->pv, msg->Num,msg->ArrayIndex);
			break;

		case vtcl: // vt classes
				 p= eVTCL(p,(BACnetVTClassList far*)msg->pv);
			break;

		case evparm: // event parameter
				 p= eEVPARM(p,(BACnetEventParameter far*)msg->pv);
			break;

		case skeys: // session keys
				 p= eSKEYS(p,(BACnetSessionKey far*)msg->pv);
			break;

		case tsrecip: // time synch recipients
		case recip: // recipient
				 p= eRECIP(p,(BACnetRecipient far*)msg->pv);
			break;

		case reciplist: // list of BACnetDestination
				 p= eRECIPLIST(p,(BACnetDestination far*)msg->pv);
			break;

		case xsched:    // exception schedule: array[] of specialevent
				 p= eXSCHED(p,(BACnetExceptionSchedule far*)msg->pv,msg->ArrayIndex);
			break;

		case wsched: // weekly schedule: array[7] of list of timevalue
				 p= eWSCHED(p,(BACnetTimeValue far**)msg->pv,7,msg->ArrayIndex);
			break;

		case propref: // list of object prop refs
		case lopref:  // list of object prop refs
				 p= eLOPREF(p,(BACnetObjectPropertyReference far*)msg->pv);
			break;

		case setref: // setpoint reference
				 p= eSETREF(p,(BACnetObjectPropertyReference far*)msg->pv);
			break;

		case vtse: // list of active  vt sessions (parse type) 
				 p= eVTSE(p,(BACnetVTSession far*)msg->pv);
			break;

*/



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
			enc.pktBuffer[enc.pktLength++] = DATE_DONT_CARE;
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
		throw_(77) /* not enough data */;
	
	tag = (dec.pktLength--,*dec.pktBuffer++);
	
	// extract the type
	tagClass = (BACnetTagClass)((tag >> 3) & 0x01);
	
	// extract the number
	tagNumber = (BACnetApplicationTag)(tag >> 4);
	if (tagNumber == 0x0F) {
		if (dec.pktLength < 1)
			throw_(78) /* not enough data */;
		tagNumber = (BACnetApplicationTag)(dec.pktLength--,*dec.pktBuffer++);
	}
	
	// extract the length
	tagLVT = (tag & 0x07);
	if (tagLVT < 5)
		;
	else
	if (tagLVT == 5) {
		if (dec.pktLength < 1)
			throw_(79) /* not enough data */;
		tagLVT = (dec.pktLength--,*dec.pktBuffer++);
		if (tagLVT == 254) {
			if (dec.pktLength < 2)
				throw_(80) /* not enough data */;
			tagLVT = (dec.pktLength--,*dec.pktBuffer++);
			tagLVT = (tagLVT << 8) + (dec.pktLength--,*dec.pktBuffer++);
		} else
		if (tagLVT == 255) {
			if (dec.pktLength < 4)
				throw_(81) /* not enough data */;
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
		throw_(82);
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

//
//	IsBound
//

bool IsBound( BACnetNetClientPtr cp, BACnetNetServerPtr sp )
{
	return ((cp->clientPeer == sp) && (sp->serverPeer == cp));
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
	if (!clientPeer) throw_(83);
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
	if (!serverPeer) throw_(84);
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

//
//	IsBound
//

bool IsBound( BACnetAppClientPtr cp, BACnetAppServerPtr sp )
{
	return ((cp->clientPeer == sp) && (sp->serverPeer == cp));
}
