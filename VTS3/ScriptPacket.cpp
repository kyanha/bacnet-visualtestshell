// ScriptPacket.cpp: implementation of the ScriptPacket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTS.h"
#include "ScriptDocument.h"
#include "ScriptParmList.h"
#include "ScriptPacket.h"
#include "ScriptKeywords.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//
//	ScriptPacketExpr::ScriptPacketExpr
//

ScriptPacketExpr::ScriptPacketExpr( void )
	: exprKeyword(0)
	, exprIsData(false)
	, exprOp(0)
	, exprValue("")
{
}


bool ScriptPacketExpr::IsAssignment()
{
	return exprOp == '>>';
}


bool ScriptPacketExpr::IsDontCare()
{
	return exprOp == '?=';
}


//
//	ScriptPacketExpr::~ScriptPacketExpr
//

ScriptPacketExpr::~ScriptPacketExpr( void )
{
}


//===========================================================

ScriptIfdefExpr::ScriptIfdefExpr( ScriptDocument * pdoc )
	: m_pdocument(pdoc), m_tokLValue(), m_tokRValue(), m_nOp(0), m_nLine(0)
{
}


ScriptIfdefExpr::ScriptIfdefExpr( ScriptDocument * pdoc, ScriptScanner & scan, ScriptToken &tok )
	: m_pdocument(pdoc), m_tokLValue(), m_tokRValue(), m_nOp(0), m_nLine(0)
{
	Parse(scan, tok);
}



// We need to pass tok here so error reported shows proper stuff in editor on throw

bool ScriptIfdefExpr::Evaluate(ScriptToken & tok)
{
	CString				strError;

	// Need to contain dynamically allocated objects so throws don't, well, throw off the delete :)

	// Set assignment so editor reports proper section on throw
	tok = m_tokLValue;
	BACnetAnyValue		bacnet1(CreateOperand(m_tokLValue));

	// Set assignment so editor reports proper section on throw
	tok = m_tokRValue;
	BACnetAnyValue		bacnet2(CreateOperand(m_tokRValue));

	// if we're using any don't cares...  we should just say "fine!"
	if ( bacnet1.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetNull)) || bacnet2.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetNull)) )
		return true;

	// types can be either char, int, bool or unsigned (means keyword) or undefined var
	// cannot mix and match types...

	if ( strcmp(bacnet1.GetObjectType()->m_lpszClassName, bacnet2.GetObjectType()->m_lpszClassName) != 0 )
		throw "Unable to compare different data types in IF expression";

	// remember we're using unsigned values to compare enums like (I-AM-ROUTER-TO-NETWORK) so we have to
	// report an error if the script is attempting <, >, <= or >=

	if ( (bacnet1.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetUnsigned)) || bacnet1.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetBoolean))) &&  m_nOp != '=' && m_nOp != '!=' )
		throw "Operators <, <=, >, >= are unsupported for keyword and boolean comparisons";

	return bacnet1.GetObject()->Match(*(bacnet2.GetObject()), m_nOp, &strError);
}



BACnetEncodeable * ScriptIfdefExpr::CreateOperand( ScriptToken & token )
{
	CString	strError;
	int		nValue;			

	ResolveToValue(token);

	if ( token.IsDontCare() )			// deal with don't cares... caller must know BACnetNull == don't care
		return new BACnetNull();

	if ( token.tokenEnc == scriptASCIIEnc )
		return new BACnetCharacterString(token.RemoveQuotes());

	if ( token.tokenEnc == scriptIntegerEnc )
	{
		if ( token.tokenType == scriptKeyword )
		{
			// Value after resolving through parameters is still a keyword... this means it's either
			// a boolean, intended enumeration or an undefined variable.

			if ( (nValue = token.Lookup(token.tokenSymbol, ScriptBooleanMap)) >= 0 )
				return new BACnetBoolean(nValue);

			return new BACnetUnsigned(token.tokenSymbol);
		}

		if ( token.tokenType == scriptValue && token.IsInteger(nValue) )
			return new BACnetInteger(nValue);
	}

	strError.Format("Unsupported operand type (%s) for conditional expression", token.tokenValue );
	throw CString(strError);
	return NULL;
}


void ScriptIfdefExpr::ResolveToValue( ScriptToken & tok )
{
	if ( tok.tokenType == scriptKeyword )
	{
		ScriptParm *  pp;
	
		if ( (pp = m_pdocument->m_pParmList->LookupParm( tok.tokenSymbol )) != 0 )
		{
			ScriptScanner	scanner(pp->parmValue);
			ScriptToken		token;

			scanner.Next(token);
			ResolveToValue(token);
			tok = token;
		}
	}
}



void ScriptIfdefExpr::ParseForValue(ScriptToken &tok )
{
	if (tok.tokenType == scriptEOF || tok.tokenType == scriptEOL )
		throw "Unexpected end of line, IF expression expected";

	// should be a keyword (var yet to be resolved) or an actual value
	if ( tok.tokenType != scriptKeyword && tok.tokenType != scriptValue )
		throw "Keyword, script variable or literal value expected";
}



void ScriptIfdefExpr::Parse( ScriptScanner & scan, ScriptToken & tok )
{
	// We're scanning ' VALUE = VALUE ' so first parse the first VAR/Keyword/Value
	scan.Next(tok);

	m_tokLValue = tok;
	ParseForValue(m_tokLValue);

	// OK.. got the first value, next check for operator...
	// use RValue as temp

	scan.Next(tok);
	switch(tok.tokenSymbol)
	{
		case '<':	case'<=':  case '>':  case '>=':  case '=':  case '!=':
			break;
		case '?=':	case '>>':
			throw "Assignement or \"don't care\" operator not supported in IF expression";
		default:
			throw "Expression operator expected (=, !=, <, <=, >, >=)";
	}

	m_nOp = tok.tokenSymbol;

	// now get the right side of the equation...
	scan.Next(tok);

	m_tokRValue = tok;
	ParseForValue(m_tokRValue);
}


//
//	ScriptPacketExprList::ScriptPacketExprList
//

ScriptPacketExprList::ScriptPacketExprList( ScriptPacketExprListPtr prev )
	: listPrevious(prev)
{
}

//
//	ScriptPacketExprList::~ScriptPacketExprList
//

ScriptPacketExprList::~ScriptPacketExprList()
{
	for (POSITION pos = GetHeadPosition(); pos; )
		delete (ScriptPacketExprPtr)GetNext( pos );
}

//
//	ScriptPacketExprList::Append
//

void ScriptPacketExprList::Append( ScriptPacketExprPtr spep )
{
	// add to the end of the list
	AddTail( spep );
}

//
//	ScriptPacketExprList::Remove
//

void ScriptPacketExprList::Remove( int indx )
{
	POSITION	pos = FindIndex( indx )
	;

	ASSERT( pos != NULL );
	delete (ScriptPacketExprPtr)GetAt( pos );
	RemoveAt( pos );
}

//
//	ScriptPacketExprList::Length
//

int ScriptPacketExprList::Length( void )
{
	return CList<ScriptPacketExprPtr,ScriptPacketExprPtr>::GetCount();
}

//
//	ScriptPacketExprList::Child
//

ScriptPacketExprPtr ScriptPacketExprList::Child( int indx )
{
	POSITION	pos = FindIndex( indx )
	;

	if (pos != NULL )
		return (ScriptPacketExprPtr)GetAt( pos );
	else
		return 0;
}

//
//	ScriptPacketExprList::Find
//

ScriptPacketExprPtr ScriptPacketExprList::Find( int keyword )
{
	for (POSITION pos = GetHeadPosition(); pos; ) {
		ScriptPacketExprPtr spep = (ScriptPacketExprPtr)GetNext( pos );

		if (spep->exprKeyword == keyword)
			return spep;
	}

	// if there is a previous packet, continue search
	return (listPrevious ? listPrevious->Find( keyword ) : 0);
}

//
//	ScriptPacketExprList::FirstData
//

int ScriptPacketExprList::FirstData( void )
{
	int		len = Length()
	;

	for (int i = 0; i < len; i++) {
		ScriptPacketExprPtr spep = (ScriptPacketExprPtr)GetAt( FindIndex( i ) );

		if (spep->exprIsData)
			return i;
	}

	// failed
	return -1;
}

//
//	ScriptPacket::ScriptPacket
//

ScriptPacket::ScriptPacket( ScriptPacketType type, ScriptPacketSubtype subtype, ScriptPacketPtr prev )
	: packetType(type)
	, packetSubtype(subtype)
	, packetExprList( prev ? &prev->packetExprList : 0 )
	, packetNext(0), packetDelay( kMaxPacketDelay )
	, packetPass(0), packetFail(0)
{
	// tell the base class this is a packet
	baseType = scriptPacket;

	// set the label based on the type
	switch (packetSubtype) {
		case rootPacket:
			switch (packetType) {
				case sendPacket:
					baseLabel = "Send";
					break;
				case expectPacket:
					baseLabel = "Expect";
					break;
			}
			break;
		case andPacket:
			baseLabel = "And";
			break;
		case orPacket:
			baseLabel = "Or";
			break;
	}

	// set the image, clear the status
	baseImage = 13;
	baseStatus = 0;
}

//
//	ScriptPacket::~ScriptPacket
//

ScriptPacket::~ScriptPacket( void )
{
}
