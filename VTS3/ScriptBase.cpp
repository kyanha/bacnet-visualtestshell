// ScriptBase.cpp: implementation of the ScriptBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTS.h"
#include "ScriptBase.h"
#include "ScriptKeywords.h"
#include "ScriptParmList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//
//	Define the following as '1' if signed constants other than integers
//	and floating point values are allowed.  For example, if '-0xF' is an 
//	invalid form of '-15', then define it as '1'.
//

#define	InvalidSignedConst	0

//
//	ScriptBase::ScriptBase
//

ScriptBase::ScriptBase()
	: baseType( scriptNull )
	, baseLineStart(0), baseLineCount(0)
	, baseImage(0), baseStatus(0)
	, baseParent(0)
{
	baseLabel = _T("");
}

//
//	ScriptBase::~ScriptBase
//
//	When a parent is deleted, destroy all of the component elements.
//	These components will not be a part of any other datastructures.
//

ScriptBase::~ScriptBase()
{
	for (POSITION pos = GetHeadPosition(); pos; )
		delete (ScriptBasePtr)GetNext( pos );
}

//
//	ScriptBase::Append
//

void ScriptBase::Append( ScriptBasePtr sbp )
{
	// add to the end of the list
	AddTail( sbp );

	// link the item to the parent
	sbp->baseParent = this;
}

//
//	ScriptBase::Remove
//

void ScriptBase::Remove( int indx )
{
	POSITION	pos = FindIndex( indx )
	;

	ASSERT( pos != NULL );
	delete (ScriptBasePtr)GetAt( pos );
	RemoveAt( pos );
}

//
//	ScriptBase::Length
//

int ScriptBase::Length( void )
{
	return CList<ScriptBasePtr,ScriptBasePtr>::GetCount();
}

//
//	ScriptBase::Child
//

ScriptBasePtr ScriptBase::Child( int indx )
{
	POSITION	pos = FindIndex( indx )
	;

	if (pos != NULL )
		return (ScriptBasePtr)GetAt( pos );
	else
		return 0;
}

//
//	ScriptToken::ScriptToken
//

ScriptToken::ScriptToken( void )
	: tokenType( scriptEOF )
	, tokenEnc( scriptIntegerEnc )
	, tokenLine( -1 )
	, tokenOffset(0), tokenLength(0)
	, tokenValue(""), tokenSymbol(0)
	, m_nIndex(-1)
{
	pTokenIndex = NULL;
}

//
//	ScriptToken::ScriptToken
//

ScriptToken::ScriptToken( const ScriptToken& cpy )
	: tokenType( cpy.tokenType )
	, tokenEnc( cpy.tokenEnc )
	, tokenLine( cpy.tokenLine )
	, tokenOffset( cpy.tokenOffset) , tokenLength( cpy.tokenLength )
	, tokenValue( cpy.tokenValue ), tokenSymbol( cpy.tokenSymbol )
	, m_nIndex(-1)
{
	pTokenIndex = NULL;
	SetIndex(cpy.pTokenIndex);
}


void ScriptToken::SetIndex( ScriptToken * ptoken )
{
	KillIndex();
	if ( ptoken != NULL )
		pTokenIndex = new ScriptToken(*ptoken);
}


//
//	ScriptToken::operator =
//

ScriptToken::operator =( const ScriptToken& cpy )
{
	tokenType = cpy.tokenType;
	tokenLine = cpy.tokenLine;
	tokenOffset = cpy.tokenOffset;
	tokenLength = cpy.tokenLength;
	tokenValue = cpy.tokenValue;
	tokenSymbol = cpy.tokenSymbol;
	SetIndex(cpy.pTokenIndex);
}


// Kills index token chain

void ScriptToken::KillIndex()
{
	if ( pTokenIndex != NULL )
		delete pTokenIndex;
}


//
//	ScriptToken::~ScriptToken
//

ScriptToken::~ScriptToken( void )
{
	KillIndex();
}

//
//	ScriptToken::IsInteger
//

bool ScriptToken::IsInteger( int &valu, ScriptParmListPtr parms ) const
{
	LPCTSTR				src = tokenValue
	;
	ScriptEncodingType	encType = tokenEnc
	;
	bool				negValue = false
	;
	int					t
	;

	// if it is a keyword, lookup the value
	if (tokenType == scriptKeyword) {
		ScriptParmPtr spp = (parms ? parms->LookupParm( tokenSymbol ) : 0);
		if (!spp)
			return false;
		src = spp->parmValue;

		// figure out what encoding to use
		if ((src[0] == 'D') && (src[1] == '\''))
			encType = scriptDecimalEnc;
		else
		if ((src[0] == '0') && (src[1] == 'x'))
			encType = scriptHexEnc;
		else
		if ((src[0] == 'X') && (src[1] == '\''))
			encType = scriptHexEnc;
		else
		if ((src[0] == '&') && (src[1] == 'x'))
			encType = scriptHexEnc;
		else
		if ((src[0] == '0') && (src[1] == 'o'))
			encType = scriptOctalEnc;
		else
		if ((src[0] == 'O') && (src[1] == '\''))
			encType = scriptOctalEnc;
		else
		if ((src[0] == '&') && (src[1] == 'O'))
			encType = scriptOctalEnc;
		else
		if ((src[0] == '0') && (src[1] == 'b'))
			encType = scriptBinaryEnc;
		else
		if ((src[0] == 'B') && (src[1] == '\''))
			encType = scriptBinaryEnc;
		else
		if ((src[0] == '&') && (src[1] == 'B'))
			encType = scriptBinaryEnc;
		else
			return false;
	} else
	if (tokenType != scriptValue)
		return false;

	// look for a sign
	if (*src == '-') {
		negValue = true;
		src += 1;
	} else
	if (*src == '+')
		src += 1;

	// now parse
	switch (encType) {
		case scriptIntegerEnc:
			for (valu = 0; *src; src++)
				valu = (valu * 10) + (*src - '0');
			break;

		case scriptDecimalEnc:							// D'nnn'
			src += 2;
			if (((strlen(src) - 1) % 3) != 0)			// must be triplet
				return false;
			for (valu = 0; *src != '\''; ) {
				t = (*src++ - '0');
				t = (t * 10) + (*src++ - '0');
				t = (t * 10) + (*src++ - '0');
				valu = (valu * 256) + t;
			}
			break;

		case scriptHexEnc:								// 0xFF, X'FF', &xFF
			src += 2;
			for (valu = 0; *src && (*src != '\''); src++)
				valu = (valu * 16) + (isdigit(*src) ? (*src - '0') : (*src - 'A' + 10));
			break;

		case scriptOctalEnc:							// 0o377, O'377', &O377
			src += 2;
			for (valu = 0; *src && (*src != '\''); src++)
				valu = (valu * 8) + (*src - '0');
			break;

		case scriptBinaryEnc:							// 0b11111111, B'11111111', &B11111111
			src += 2;
			for (valu = 0; *src && (*src != '\''); src++)
				valu = (valu * 2) + (*src - '0');
			break;

		default:
			return false;
	}

	// update for sign
	if (negValue)
		valu = (valu * -1);

	// success
	return true;
}

//
//	ScriptToken::IsInteger
//
//	This function is used to see if the token can be parsed as an integer.  If it is a 
//	keyword, then look it up in the table provided.
//

bool ScriptToken::IsInteger( int &valu, ScriptTranslateTablePtr tp ) const
{
	int		rslt
	;

	// if it is a keyword, lookup the value
	if (tokenType == scriptKeyword) {
		rslt = Lookup( tokenSymbol, tp );
		if (rslt < 0)
			return false;

		// success
		valu = rslt;
		return true;
	}

	// try default processing
	return IsInteger( valu );
}

//
//	ScriptToken::IsPropertyReference
//
//	This function is used to see if the token is a reference to a property.  If it is 
//	the valu will be the property identifier.  Only standard property names are supported, 
//	because otherwise there is no mechanism to match up the type.
//

bool ScriptToken::IsPropertyReference( int &prop ) const
{
	int		rslt
	;

	// if it is a keyword, lookup the value
	if (tokenType == scriptReference) {
		rslt = Lookup( tokenSymbol, ScriptPropertyMap );
		if (rslt < 0)
			return false;

		// success
		prop = rslt;
		return true;
	}

	// sadly, no chance
	return false;
}

//
//	ScriptToken::IsEncodeable
//
//	This function checks to see if the token can be translated into an encodeable type, 
//	one of the BACnet atomic types.
//

bool ScriptToken::IsEncodeable( BACnetEncodeable &enc ) const
{
	try {
		enc.Decode( tokenValue );
	}
	catch (...) {
		return false;
	}

	// success
	return true;
}

//
//	ScriptToken::RemoveQuotes
//
//	Remove the single or double quotes around strings.
//

CString ScriptToken::RemoveQuotes( void ) const
{
	CString	rslt
	;

	rslt = tokenValue;
	rslt.Delete( 0, 1 );
	rslt.Delete( rslt.GetLength() - 1, 1 );

	return rslt;
}

//
//	ScriptToken::HashCode
//
//	This hash function is used to encode a keyword into some unique value 
//	that can be used in switch statements and looked up in tables very 
//	quickly.  There may be two keywords that map to the same hash value,
//	btu I haven't found any yet!
//

int ScriptToken::HashCode( const char *key )
{ 
#define LARGENUMBER     6293815 
 
	int		index = 1
	;
	DWORD	sum = 0
	,		multiple = LARGENUMBER
	;

	while (*key != '\0') {
		sum += multiple * index++ * (*key++);
		multiple *= LARGENUMBER;
	}

	return sum; 
}

//
//	ScriptToken::Lookup
//
//	This function looks through a translate table for the current token 
//	symbol.  It is used to map keywords into property values, for example.
//	This will return -1 if the value was not found.
//

int ScriptToken::Lookup( int code, ScriptTranslateTablePtr tp )
{
	while (tp->scriptKeyword)
		if (tp->scriptKeyword == code)
			return tp->scriptValue;
		else
			tp += 1;

	return -1;
}


void ScriptToken::ResolveIndex( ScriptParmListPtr parms )
{
	if ( pTokenIndex == NULL )
		return;

	// recurse to start with the ground up... shouldn't be too many.  Don't worry.  Be happy.
	// This is to support things like junk[junkagain[parm]] and such.  Not fully implemented.

	pTokenIndex->ResolveIndex(parms);
		
	CString strError;
	BACnetUnsigned bacnetIndex;

	switch(pTokenIndex->tokenType)
	{
		case scriptValue:

			if ( !pTokenIndex->IsEncodeable(bacnetIndex) )
				strError.Format(IDS_SCREX_INDEXUNSEXP, tokenValue, pTokenIndex->tokenValue);
			else
				m_nIndex = (int) bacnetIndex.uintValue;
			break;

		case scriptReference:

			// not implemented yet
			break;

		case scriptKeyword:

//			Could use this call and that would give us lots of conversion power (hex, binary, dec, octal), but
//			it can't handle things without the base specifier (like '12', etc.).  Too afraid to modify IsInteger
//			to perform that action at this point.  So, look up the parm ourselves.
//			if ( !pTokenIndex->IsInteger(m_nIndex, parms) )

			try {		// may return null, try catches it though
				bacnetIndex.Decode( ((ScriptParmPtr) parms->LookupParm(pTokenIndex->tokenSymbol))->parmValue );
				m_nIndex = (int) bacnetIndex.uintValue;
			}
			catch (...) {
				strError.Format(IDS_SCREX_INDEXUNSVAR, tokenValue, pTokenIndex->tokenValue);
			}
	}


	if ( !strError.IsEmpty() )
		throw CString(strError);
}



//
//	ScriptTokenList::ScriptTokenList
//

ScriptTokenList::ScriptTokenList( void )
{
}

//
//	ScriptTokenList::~ScriptTokenList
//

ScriptTokenList::~ScriptTokenList( void )
{
	for (POSITION pos = GetHeadPosition(); pos; )
		delete (ScriptTokenPtr)GetNext( pos );
}

//
//	ScriptTokenList::Append
//

void ScriptTokenList::Append( const ScriptToken& tok )
{
	// add a copy to the end of the list
	AddTail( new ScriptToken( tok ) );
}

//
//	ScriptTokenList::Length
//

int ScriptTokenList::Length( void )
{
	return CList<ScriptTokenPtr,ScriptTokenPtr>::GetCount();
}

//
//	ScriptTokenList::operator []
//

//madanner 10/24/02, const removed
ScriptToken& ScriptTokenList::operator []( int i )
{
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	return *(ScriptTokenPtr)GetAt( pos );
}

//
//	ScriptScanner::ScriptScanner
//
//	This scanner allows scanning from CEdit text, usually the script source
//	window.
//

ScriptScanner::ScriptScanner( CEdit* ep )
	: scanSource( ep )
	, scanLine( -1 )
	, scanSrc( scanLineBuffer )
{
	scanLineBuffer[0] = 0;
	scanValueBuffer[0] = 0;
}

//
//	ScriptScanner::ScriptScanner
//
//	This form of the ctor allows scanning from any string.
//

ScriptScanner::ScriptScanner( const char *src )
	: scanSource( 0 )
	, scanLine( -1 )
	, scanSrc( src )
{
}

//
//	ScriptScanner::~ScriptScanner
//

ScriptScanner::~ScriptScanner( void )
{
}

//
//	ScriptScanner::FormatError
//

void ScriptScanner::FormatError( ScriptToken &tok, char *msg )
{
	// set up the token to point to the bad char
	if (scanSource) {
		tok.tokenLine = scanLine;
		tok.tokenOffset = scanSource->LineIndex( scanLine ) + (scanSrc - scanLineBuffer);
		tok.tokenLength = (*scanSrc ? 1 : 0);
	}

	// here it goes
	throw msg;
}

//
//	ScriptScanner::Next
//

#include "ScriptKeywords.h"

void ScriptScanner::Next( ScriptToken& tok )
{
	const char	*tStart
	;
	char		c, *dst
	;
	bool		signFound = false
	;

	// prepare for token value
	tok.tokenLength = 0;
	tok.tokenSymbol = 0;
	dst = scanValueBuffer;
	*dst = 0;

	Deblank();

	// keep track of the start for computing length
	tStart = scanSrc;

	// compute the offset into the source text (unless working from a buffer)
	if (scanSource) {
		tok.tokenLine = scanLine;
		tok.tokenOffset = scanSource->LineIndex( scanLine ) + (scanSrc - scanLineBuffer);
	}

	// check for end-of-line
	if (!*scanSrc) {
		tok.tokenType = scriptEOL;
		tok.tokenLength = 0;
		tok.tokenValue = "";
		return;
	}

	// look for an 'ignoring' comment
	if (*scanSrc == ';') {
		// skip to end of line
		while (*scanSrc) scanSrc++;

		// end-of-line
		tok.tokenType = scriptEOL;
		tok.tokenLength = (scanSrc - tStart);
		tok.tokenValue = "";
		return;
	}

	// look for a 'describing' comment
	if ((*scanSrc == '-') && (*(scanSrc+1) == '-')) {
		// skip the comment chars
		while (*scanSrc == '-')
			scanSrc++;

		Deblank();

		// look for a word
		while (*scanSrc) {
			// copy the word
			while (*scanSrc && (!isspace(*scanSrc)))
				*dst++ = *scanSrc++;

			Deblank();
	
			// copy a blank before the next word
			if (*scanSrc)
				*dst++ = ' ';
		}

		// null terminate the end
		*dst = 0;

		// return an end-of-line token
		tok.tokenType = scriptEOL;
		tok.tokenLength = (scanSrc - tStart);			// actual parsed length, not result length
		tok.tokenValue = scanValueBuffer;

		return;
	}

	// set up for possible error
	tok.tokenType = scriptError;

	// copy at least the first char
	*dst++ = c = toupper(*scanSrc++);
	*dst = 0;

	// special chars are the next easiest
	switch (c) {
		case '=':
		case '?':
        case '*':              //add by Liangping Xu
		case ',':
		case '/':
		case ':':
		case '(':
		case ')':
			tok.tokenType = scriptSymbol;
			tok.tokenSymbol = c;
			break;

		case '<':
			if (*scanSrc == '=') {
				scanSrc += 1;
				tok.tokenType = scriptSymbol;
				tok.tokenSymbol = '<=';
			} else
			if (*scanSrc == '>') {
				scanSrc += 1;
				tok.tokenType = scriptSymbol;
				tok.tokenSymbol = '!=';
			} else {
				tok.tokenType = scriptSymbol;
				tok.tokenSymbol = c;
			}
			break;

		case '>':
			if (*scanSrc == '=') {
				scanSrc += 1;
				tok.tokenType = scriptSymbol;
				tok.tokenSymbol = '>=';
			} else {
				tok.tokenType = scriptSymbol;
				tok.tokenSymbol = c;
			}
			break;

		case '!':
			if (*scanSrc == '=') {
				scanSrc += 1;
				tok.tokenType = scriptSymbol;
				tok.tokenSymbol = '!=';
			}
			break;

		case '+':
		case '-':
			signFound = true;
			// Added by Yajun Zhou, 2002-9-4
			if(*scanSrc == '&')
			{
				*dst++ = c = toupper(*scanSrc++);
				*dst = 0;
				goto FORMAT1;
			}
			if(*scanSrc == '0' && !isdigit(*(scanSrc+1)))
			{
				*dst++ = c = toupper(*scanSrc++);
				*dst = 0;
				break;
			}
			//////////////////////////////////////
			if (!isdigit(*scanSrc))
				FormatError( tok, "Invalid character in numeric constant" );
			break;

		case '&':
FORMAT1:	if (*scanSrc == 'D') {
				tok.tokenType = scriptValue;
				tok.tokenEnc = scriptDecimalEnc;
				*dst++ = *scanSrc++;
				while (isdigit(*scanSrc))
					*dst++ = *scanSrc++;
			} else
			//Modified by Yajun Zhou, 2002-8-16
			//if (*scanSrc == 'H') {
			if (*scanSrc == 'X' ) {
			///////////////////////////////////
				tok.tokenType = scriptValue;
				tok.tokenEnc = scriptHexEnc;
				*dst++ = *scanSrc++;
				while (isxdigit(*scanSrc))
					*dst++ = *scanSrc++;
			} else
			if (*scanSrc == 'B') {
				tok.tokenType = scriptValue;
				tok.tokenEnc = scriptBinaryEnc;
				*dst++ = *scanSrc++;
				while ((*scanSrc == '0') || (*scanSrc == '1'))
					*dst++ = *scanSrc++;
			} else
			if (*scanSrc == 'O') {
				tok.tokenType = scriptValue;
				tok.tokenEnc = scriptOctalEnc;
				*dst++ = *scanSrc++;
				while ((*scanSrc >= '0') && (*scanSrc <= '7'))
					*dst++ = *scanSrc++;
			} else
				;
			if (isalnum(*scanSrc))
				FormatError( tok, "Invalid character in constant" );
			*dst = 0;
			break;

		case '\"':
		case '\'':
			tok.tokenType = scriptValue;
			tok.tokenEnc = scriptASCIIEnc;

			while (*scanSrc && (*scanSrc != c)) {
				if ((*scanSrc == '\\') && (*(scanSrc+1)))
					scanSrc += 1;
				*dst++ = *scanSrc++;
			}
			if (*scanSrc != c)
				FormatError( tok, "Missing close quote" );
			*dst++ = *scanSrc++;
			*dst = 0;
			break;

		case '{':
			tok.tokenType = scriptReference;

			Deblank();

			// get the name
			scanValueBuffer[1] = 0;
			while (isalpha(*scanSrc) || isdigit(*scanSrc) || (*scanSrc == '-') || (*scanSrc == '_')) {
				if ((*scanSrc == '-') && (*(scanSrc+1) == '-'))
					break;
				c = toupper(*scanSrc++);
				*dst++ = (c == '_' ? '-' : c);
			}
			if (!scanValueBuffer[1])
				FormatError( tok, "Missing reference" );
			*dst = 0;

			// compute the hash code
			tok.tokenSymbol = ScriptToken::HashCode( scanValueBuffer + 1 );

			// deal with possible index into property.
			// Only allow indices in property references (as apposed to var references).
			// allow cases of {property[int]}, {property[var]}, {property[{property}]}, {property[{property[int]}]}, etc.
			// Don't allow cases (yet) of var[var], var[int], var[etc.]

			ScanIndexTokens(tok);
			Deblank();

			if ((*dst++ = *scanSrc++) != '}')
				FormatError( tok, "Missing close brace" );

			*dst = 0;
			break;
	}

	// clean up if a token was found
	if (tok.tokenType != scriptError) {
		tok.tokenLength = (scanSrc - tStart);
		tok.tokenValue = scanValueBuffer;
		return;
	}

	// C-style hex constant
	if ((c == '0') && ((*scanSrc == 'x') || (*scanSrc == 'X'))) {
#if InvalidSignedConst
		if (signFound)
			throw "Invalid signed value";
#endif
		tok.tokenType = scriptValue;
		tok.tokenEnc = scriptHexEnc;
		*dst++ = 'x';
		scanSrc += 1;
		while (isxdigit(*scanSrc))
			*dst++ = toupper(*scanSrc++);
		*dst = 0;

		tok.tokenLength = (scanSrc - tStart);
		if (scanValueBuffer[2] == 0)
			FormatError( tok, "Hex digit expected" );
		if (isalnum(*scanSrc))
			FormatError( tok, "Invalid character in hex constant" );

		tok.tokenValue = scanValueBuffer;
		return;
	}

	// C-style binary constant
	if ((c == '0') && ((*scanSrc == 'b') || (*scanSrc == 'B'))) {
#if InvalidSignedConst
		if (signFound)
			throw "Invalid signed value";
#endif
		tok.tokenType = scriptValue;
		tok.tokenEnc = scriptBinaryEnc;
		*dst++ = 'b';
		scanSrc += 1;
		while ((*scanSrc == '0') || (*scanSrc == '1'))
			*dst++ = *scanSrc++;
		*dst = 0;

		tok.tokenLength = (scanSrc - tStart);
		if (scanValueBuffer[2] == 0)
			FormatError( tok, "Binary digit expected" );
		if (isalnum(*scanSrc))
			FormatError( tok, "Invalid character in binary constant" );

		tok.tokenValue = scanValueBuffer;
		return;
	}

	// C-style octal constant
	if ((c == '0') && ((*scanSrc == 'o') || (*scanSrc == 'O'))) {
#if InvalidSignedConst
		if (signFound)
			throw "Invalid signed value";
#endif
		tok.tokenType = scriptValue;
		tok.tokenEnc = scriptOctalEnc;
		*dst++ = 'o';
		scanSrc += 1;
		while ((*scanSrc >= '0') && (*scanSrc <= '7'))
			*dst++ = *scanSrc++;
		*dst = 0;

		tok.tokenLength = (scanSrc - tStart);
		if (scanValueBuffer[2] == 0)
			FormatError( tok, "Octal digit expected" );
		if (isalnum(*scanSrc))
			FormatError( tok, "Invalid character in octal constant" );

		tok.tokenValue = scanValueBuffer;
		return;
	}

	// check for an IP address
	if (isdigit(c)) {
		int			valu = (c - '0')
		;
		const char	*saveSrc = scanSrc
		;
		char		*dst = scanValueBuffer
		;
		bool		validIP = true
		;

		// assume the best
		tok.tokenType = scriptValue;
		tok.tokenEnc = scriptIPEnc;
		*dst++ = c;

		// check for four dotted octets
		for (int i = 0; i < 4; i++, valu = 0) {
			// copy the integer portion
			while (isdigit(*scanSrc)) {
				valu = (valu * 10) + (*scanSrc - '0');
				*dst++ = *scanSrc++;
			}
			if (valu > 255) {
				// nope!
				validIP = false;
				break;
			}
			if ((i != 3) && ((*dst++ = *scanSrc++) != '.')) {
				validIP = false;
				break;
			}
		}

		// check for network size
		if (validIP && (*scanSrc == '/')) {
			*dst++ = *scanSrc++;
			while (isdigit(*scanSrc))
				*dst++ = *scanSrc++;
		}

		// check for port (allow 0xBAC0 encoding)
		if (validIP && (*scanSrc == ':')) {
			*dst++ = *scanSrc++;
			if ((*scanSrc == '0') && (tolower(*(scanSrc+1)) == 'x')) {
				*dst++ = *scanSrc++;
				*dst++ = *scanSrc++;
				while (isxdigit(*scanSrc))
					*dst++ = *scanSrc++;
			} else
				while (isdigit(*scanSrc))
					*dst++ = *scanSrc++;
		}

		// null terminate the results
		*dst = 0;

		// if we made it this far, run with it
		if (validIP) {
			tok.tokenLength = (scanSrc - tStart);
			tok.tokenValue = scanValueBuffer;
			return;
		}

		// rewind
		scanSrc = saveSrc;
	}

	// look for a number
	if (isdigit(c) || (signFound && isdigit(*scanSrc))) {
		tok.tokenType = scriptValue;
		tok.tokenEnc = scriptIntegerEnc;

		// copy the rest of the integer portion
		while (isdigit(*scanSrc))
			*dst++ = *scanSrc++;

		// check for float
		if (*scanSrc == '.') {
			tok.tokenEnc = scriptFloatEnc;
			*dst++ = *scanSrc++;

			if (!isdigit(*scanSrc))
				FormatError( tok, "Invalid number format" );

			while (isdigit(*scanSrc))
				*dst++ = *scanSrc++;

		}

		// check for exponential
		if ((*scanSrc == 'e') || (*scanSrc == 'E')) {
			tok.tokenEnc = scriptFloatEnc;
			*dst++ = *scanSrc++;

			if (*scanSrc == '-')
				*dst++ = *scanSrc++;

			if (!isdigit(*scanSrc))
				FormatError( tok, "Invalid number format" );

			while (isdigit(*scanSrc))
				*dst++ = *scanSrc++;
		}

		// null terminate the results
		*dst = 0;

		// no other alpha chars please
		if (isalpha(*scanSrc))
			FormatError( tok, "Invalid number format" );

		tok.tokenLength = (scanSrc - tStart);
		tok.tokenValue = scanValueBuffer;
		return;
	}

	// return '+' and '-' as symbols, they might be operators
	if (signFound) {
		tok.tokenType = scriptSymbol;
		tok.tokenSymbol = c;
		return;
	}

	// toss all non-alphas
	if (!isalpha(c)) {
		scanSrc -= 1;
		FormatError( tok, "Invalid character" );
	}

	// copy the rest and normalize
	tok.tokenType = scriptKeyword;
	while (isalpha(*scanSrc) || isdigit(*scanSrc) || (*scanSrc == '-') || (*scanSrc == '_')) {
		if ((*scanSrc == '-') && (*(scanSrc+1) == '-'))
			break;
		c = toupper(*scanSrc++);
		*dst++ = (c == '_' ? '-' : c);
	}
	*dst = 0;
	tok.tokenLength = (scanSrc - tStart);
	tok.tokenValue = scanValueBuffer;

	// compute the hash code
	tok.tokenSymbol = ScriptToken::HashCode( scanValueBuffer );

	// deal with possible index into property.
	// Only allow indices in property references (as apposed to var references).
	// allow cases of {property[int]}, {property[var]}, {property[{property}]}, {property[{property[int]}]}, etc.
	// Don't allow cases (yet) of var[var], var[int], var[etc.]
	// If cases of var[x] are allowed, this call to ScanIndexTokens would work... but it's just too messy when resolving
	// things at this point.  In addition, parameters are stored as text and arrays are not allowed or
	// would not be resolved properly.

//	ScanIndexTokens(tok);

	Deblank();

	// check for COMMENT keyword
	if (tok.tokenSymbol == kwCOMMENT) {
		dst = scanValueBuffer;

		// look for a word
		while (*scanSrc) {
			// copy the word
			while (*scanSrc && (!isspace(*scanSrc)))
				*dst++ = *scanSrc++;

			Deblank();

			// copy a blank before the next word
			if (*scanSrc)
				*dst++ = ' ';
		}

		// null terminate the end
		*dst = 0;

		// return an end-of-line token
		tok.tokenType = scriptEOL;
		tok.tokenLength = (scanSrc - tStart);
		tok.tokenValue = scanValueBuffer;

		return;
	}

	// check for hex const string
	if ((tok.tokenSymbol == kwX) && (*scanSrc == '\'')) {
		// normalize
		scanValueBuffer[0] = 'X';

		tok.tokenType = scriptValue;
		tok.tokenEnc = scriptHexEnc;

		*dst++ = *scanSrc++;
		while (isxdigit(*scanSrc))
			*dst++ = toupper(*scanSrc++);

		// set the length and check for close quote
		if (dst == scanValueBuffer + 2)
			FormatError( tok, "Hex digit expected" );
		if (*scanSrc != '\'')
			FormatError( tok, "Invalid hex constant form" );

		*dst++ = *scanSrc++;
		*dst = 0;

		tok.tokenLength = (scanSrc - tStart);
		tok.tokenValue = scanValueBuffer;

		return;
	}

	// check for binary const string
	if ((tok.tokenSymbol == kwB) && (*scanSrc == '\'')) {
		// normalize
		scanValueBuffer[0] = 'B';

		tok.tokenType = scriptValue;
		tok.tokenEnc = scriptBinaryEnc;

		*dst++ = *scanSrc++;
		while ((*scanSrc == '0') || (*scanSrc == '1'))
			*dst++ = *scanSrc++;

		// set the length and check for close quote
		if (dst == scanValueBuffer + 2)
			FormatError( tok, "Binary digit expected" );
		if (*scanSrc != '\'')
			FormatError( tok, "Invalid binary constant form" );

		*dst++ = *scanSrc++;
		*dst = 0;

		tok.tokenLength = (scanSrc - tStart);
		tok.tokenValue = scanValueBuffer;

		return;
	}

	// check for octal const string
	if ((tok.tokenSymbol == kwO) && (*scanSrc == '\'')) {
		// normalize
		scanValueBuffer[0] = 'O';

		tok.tokenType = scriptValue;
		tok.tokenEnc = scriptBinaryEnc;

		*dst++ = *scanSrc++;
		while ((*scanSrc >= '0') && (*scanSrc <= '7'))
			*dst++ = *scanSrc++;
		*dst = 0;

		// set the length and check for close quote
		if (dst == scanValueBuffer + 2)
			FormatError( tok, "Octal digit expected" );
		if (*scanSrc != '\'')
			FormatError( tok, "Invalid octal constant form" );

		*dst++ = *scanSrc++;
		*dst = 0;

		tok.tokenLength = (scanSrc - tStart);
		tok.tokenValue = scanValueBuffer;

		return;
	}

	// no more special cases
	return;
}


// madanner 10/24/02
// suck out the values (or references) in the index.
// warning:  recusive through ScripScanner.Next()

void ScriptScanner::ScanIndexTokens( ScriptToken & tok )
{
	Deblank();

	// check to see if we're about to start an index entry
	if ( *scanSrc == '[' )
	{
		// Process the index recursively.  We should end up with the scan buffer advanced to past
		// all of the indices.

		ScriptScanner scanAgain(++scanSrc);
		ScriptToken tokIndex;

		scanAgain.Next(tokIndex);
		scanSrc = scanAgain.scanSrc;

		Deblank();

		if ( *scanSrc++ != ']' )
			FormatError(tokIndex, "Missing close bracket ]");

		tok.SetIndex(new ScriptToken(tokIndex));
	}
}



void ScriptScanner::Deblank()
{
	while (*scanSrc && isspace(*scanSrc))
		scanSrc += 1;
}


//
//	ScriptScanner::Peek
//
//	This function saves a copy of the current source and restores it after calling
//	Next().  It is used to look ahead into the stream for a potential token.
//

void ScriptScanner::Peek( ScriptToken& tok )
{
	const char	*reserveSrc = scanSrc
	;

	// scan the token
	Next( tok );

	// restore the pointer to the next token
	scanSrc = reserveSrc;
}

//
//	ScriptScanner::NextTitle
//
//	This special parsing function is called to group all of the information to the 
//	end-of-line into the parse token.  Initial and trailing blanks will be removed, 
//	and multiple blanks in the data will be reduced to a single blank.
//

void ScriptScanner::NextTitle( ScriptToken& tok )
{
	const char	*tStart
	;
	char		*dst = scanValueBuffer
	;

	// copy the line number
	tok.tokenLine = scanLine;

	// deblank
	while (*scanSrc && isspace(*scanSrc))
		scanSrc += 1;

	// check for end-of-line
	if ((*scanSrc == ';') || ((*scanSrc == '-') && (*(scanSrc+1) == '-'))) {
		tok.tokenType = scriptEOL;
		tok.tokenLength = 0;
		tok.tokenValue = "";
		return;
	}

	// keep track of the start for computing length
	tStart = scanSrc;

	// compute the offset into the source text
	tok.tokenOffset = scanSource->LineIndex( scanLine ) + (scanSrc - scanLineBuffer);

	// look for a word
	while (*scanSrc) {
		// copy the word
		while (*scanSrc && (!isspace(*scanSrc)))
			*dst++ = *scanSrc++;

		// deblank
		while (*scanSrc && isspace(*scanSrc))
			scanSrc += 1;

		// check for end-of-line
		if ((*scanSrc == ';') || ((*scanSrc == '-') && (*(scanSrc+1) == '-')))
			break;

		// copy a blank before the next word
		if (*scanSrc)
			*dst++ = ' ';
	}

	// null terminate the end
	*dst = 0;

	// return an end-of-line token
	tok.tokenType = scriptTitle;
	tok.tokenLength = (scanSrc - tStart);			// actual parsed length, not result length
	tok.tokenValue = scanValueBuffer;

	return;
}

//
//	ScriptScanner::NextLine
//
//	This member function loads the next line from the script and calls Next() to parse
//	the first token.
//

void ScriptScanner::NextLine( ScriptToken& tok )
{
	int		len
	;

	// if there's no source, bail
	if (!scanSource)
		throw (-1);

	// get the next line
	scanLine += 1;
	if (scanLine >= scanSource->GetLineCount()) {
		tok.tokenType = scriptEOF;
		return;
	}

	// suck the line into a private buffer and null-terminate it
	len = scanSource->GetLine( scanLine, scanLineBuffer, kTokenBufferSize );
	scanLineBuffer[len] = 0;

	// load the next token
	scanSrc = scanLineBuffer;
	Next( tok );
}
