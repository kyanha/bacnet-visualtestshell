
#include "stdafx.h"

#include <string.h>

#include "BACnet.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	BACnetAPDUDecoder
//

BACnetAPDUDecoder::BACnetAPDUDecoder( const BACnetOctet *buffer, int len )
	: pktBuffer(buffer), pktLength(len)
{
}

BACnetAPDUDecoder::BACnetAPDUDecoder( const BACnetAPDUEncoder &enc )
	: pktBuffer(enc.pktBuffer), pktLength(enc.pktLength)
{
}

BACnetAPDUDecoder::BACnetAPDUDecoder( const BACnetNPDU &npdu )
	: pktBuffer(npdu.pduData), pktLength(npdu.pduLen)
{
}

//
//	BACnetAPDUDecoder::SetBuffer
//

void BACnetAPDUDecoder::SetBuffer( const BACnetOctet *buffer, int len )
{
	pktBuffer = buffer;
	pktLength = len;
}

//
//	ExamineTag
//

void BACnetAPDUDecoder::ExamineTag( BACnetAPDUTag &t )
{
	const BACnetOctet	*pkt = pktBuffer		// save the packet pointer
	;
	int					len = pktLength			// save the packet length
	;
	
	// call the (destructive) decoder
	t.Decode( *this );
	
	// restore the pointer and length
	pktBuffer = pkt;
	pktLength = len;
}

//
//	BACnetAPDUDecoder::CopyOctets
//
//	A crude but effective way of copying out a chunk of information.  Note that 
//	this does not account for tagged data, nor does it check to make sure there 
//	is actually enough data in the buffer to copy out.  This routine is very 
//	seldom called, and when it is it's usually for a very short buffer (like 
//	one octet) so it is not worth the overhead of calling memcpy().
//

void BACnetAPDUDecoder::CopyOctets( BACnetOctet *buff, int len )
{
	while (len--)
		*buff++ = (pktLength--,*pktBuffer++);
}

//
//	BACnetAPDUDecoder::ExtractData
//
//	This routine is used to copy the data portion of tagged information into 
//	a buffer supplied by the caller.  The return value is the number of octets
//	copied.
//
//	This is a destructive call (the packet pointer and length is updated to the 
//	next tag).
//

int BACnetAPDUDecoder::ExtractData( BACnetOctet *buffer )
{
	BACnetAPDUTag	tag
	;
	
	// extract the tag
	tag.Decode( *this );
	
	// don't copy data for an application tagged boolean (there isn't any)
	if (!tag.tagClass && (tag.tagNumber == booleanAppTag))
		return 0;
	
	// copy out the data
	CopyOctets( buffer, tag.tagLVT );
	
	// return the length
	return tag.tagLVT;
}

//
//	ostream &operator <<(ostream &strm, const BACnetAPDUDecoder &dec )
//
//	A handy debugging function.
//

ostream &operator <<(ostream &strm, const BACnetAPDUDecoder &dec )
{
	int					i, len
	;
	const static char	hex[] = "0123456789ABCDEF"
	;
	BACnetAPDUDecoder	decCopy = dec
	;
	BACnetAPDUTag		t
	;
	BACnetOctet			buff[1024]
	;
	char				msg[1024]
	;
	
	while (decCopy.pktLength != 0) {
		try {
			decCopy.ExamineTag( t );
//			strm << "tagNumber : " << (int)t.tagNumber << endl;
//			strm << "tagClass : " << (int)t.tagClass << endl;
//			strm << "tagLVT : " << t.tagLVT << endl;
			
			switch (t.tagClass) {
				case contextTagClass:
					strm << "Context specific data (tag " << (int)t.tagNumber << "): ";
					len = decCopy.ExtractData( buff );
					for (i = 0; i < len; i++) {
						strm << hex[(buff[i] >> 4) & 0x0F];
						strm << hex[buff[i] & 0x0F];
						strm << '.';
					}
					strm << endl;
					break;
					
				case openingTagClass: {
					BACnetOpeningTag	openingTag
					;
					
					openingTag.Decode( decCopy );
					strm << "Opening tag : " << (int)t.tagNumber << endl;
					break;
					}
					
				case closingTagClass: {
					BACnetClosingTag	closingTag
					;
					
					closingTag.Decode( decCopy );
					strm << "Closing tag : " << (int)t.tagNumber << endl;
					break;
					}
				
				case applicationTagClass:
					switch (t.tagNumber) {
						case unusedAppTag:
							throw -1; // should never get here
						
						case nullAppTag: {
							BACnetNull		nullValue
							;
							
							nullValue.Decode( decCopy );
							strm << "Null" << endl;
							break;
							}
							
						case booleanAppTag: {
							BACnetBoolean	boolValue
							;
							
							boolValue.Decode( decCopy );
							strm << "Boolean : " << (int)boolValue.boolValue << endl;
							break;
							}
							
						case unsignedIntAppTag: {
							BACnetUnsigned	uintValue
							;
							
							uintValue.Decode( decCopy );
							strm << "Unsigned integer : " << uintValue.uintValue << endl;
							break;
							}
							
						case integerAppTag: {
							BACnetInteger	intValue
							;
							
							intValue.Decode( decCopy );
							strm << "Integer : " << intValue.intValue << endl;
							break;
							}
							
						case realAppTag: {
							BACnetReal	realValue
							;
							
							realValue.Decode( decCopy );
							strm << "Real : " << realValue.realValue << endl;
							break;
							}
							
						case doubleAppTag: {
							BACnetDouble	doubleValue
							;
							
							doubleValue.Decode( decCopy );
							strm << "Double : " << doubleValue.doubleValue << endl;
							break;
							}
							
						case octetStringAppTag:
							strm << "Octet string : ";
							goto suckData;
							
						case characterStringAppTag:
							strm << "Character string : ";
							goto suckData;
							
						case bitStringAppTag:
							strm << "Bit string : ";
							
						suckData:
							len = decCopy.ExtractData( buff );
							for (i = 0; i < len; i++)
								if ((buff[i] > ' ') && (buff[i] <= '~'))
									strm << (char)buff[i] << '.';
								else {
									strm << hex[(buff[i] >> 4) & 0x0F];
									strm << hex[buff[i] & 0x0F];
									strm << '.';
								}
							strm << endl;
							break;
							
						case enumeratedAppTag: {
							BACnetEnumerated	enumValue
							;
							
							enumValue.Decode( decCopy );
							strm << "Enumerated : " << enumValue.enumValue << endl;
							break;
							}
							
						case dateAppTag: {
							BACnetDate			dateValue
							;
							
							dateValue.Decode( decCopy );
							strm << "Date : "
								<< "year = " << dateValue.year << " (" << dateValue.year + 1900 << ")"
								<< ", month = " << dateValue.month
								<< ", day = " << dateValue.day
								<< ", dayOfWeek = " << dateValue.dayOfWeek
								<< endl;
							break;
							}
							
						case timeAppTag: {
							BACnetTime			timeValue
							;
							
							timeValue.Decode( decCopy );
							strm << "Time : "
								<< "hour = " << timeValue.hour
								<< ", minute = " << timeValue.minute
								<< ", second = " << timeValue.second
								<< ", hundredths = " << timeValue.hundredths
								<< endl;
							break;
							}
							
						case objectIdentifierAppTag: {
							BACnetObjectIdentifier	objId
							;
							
							objId.Decode( decCopy );
							objId.Encode( msg );
							strm << "Object Identifier : " << msg << endl;
							break;
							}
                                                        
						case reservedAppTag13:
							strm << "Reserved (13) : ";
							goto suckData;
                                                        
						case reservedAppTag14:
							strm << "Reserved (14) : ";
							goto suckData;
                                                        
						case reservedAppTag15:
							strm << "Reserved (15) : ";
							goto suckData;
					}
				}
		}
		catch (...) {
			strm << "Decoding error" << endl;
			break;
		}
	}
	
	// final blank line
	strm << endl;
	
	return strm;
}
