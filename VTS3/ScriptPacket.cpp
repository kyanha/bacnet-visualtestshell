// ScriptPacket.cpp: implementation of the ScriptPacket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTS.h"
#include "ScriptPacket.h"

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
