// ScriptPacket.h: interface for the ScriptPacket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTPACKET_H__AC59C5B1_BAFF_11D4_BEF7_00A0C95A9812__INCLUDED_)
#define AFX_SCRIPTPACKET_H__AC59C5B1_BAFF_11D4_BEF7_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BACnet.hpp"

#include "ScriptBase.h"

class ScriptPacket;
typedef ScriptPacket *ScriptPacketPtr;

class ScriptPacketExpr;
typedef ScriptPacketExpr *ScriptPacketExprPtr;

class ScriptPacketExprList;
typedef ScriptPacketExprList *ScriptPacketExprListPtr;

//
//	ScriptPacketExpr
//

const int kMaxScriptExprLen = 8;

class ScriptPacketExpr {
	public:
		ScriptPacketExpr( void );
		~ScriptPacketExpr( void );

		bool IsAssignment(void);						// madanner 11/5/02, is this expression an assignment?
		bool IsDontCare(void);							// is this a don't care assignment?

		int				exprKeyword;					// keyword
		bool			exprIsData;						// true if data keyword
		int				exprOp;							// operator
		CString	 		exprValue;						// list of values
		int				exprLine;						// line number in script
	};


class ScriptIfdefExpr
{
	private:
		ScriptDocument * m_pdocument;

		void ParseForValue(ScriptToken &tok );
		void ResolveToValue( ScriptToken & tok );
		BACnetEncodeable * CreateOperand( ScriptToken & token );

	public:
		ScriptIfdefExpr( ScriptDocument * pdoc );
		ScriptIfdefExpr( ScriptDocument * pdoc, ScriptScanner &scan, ScriptToken & tok );

		bool Evaluate( ScriptToken & tok);
		void Parse( ScriptScanner &scan, ScriptToken &tok );

		ScriptToken		m_tokLValue; 					// value on left of operator
		ScriptToken		m_tokRValue;					// value on right of operator
		int				m_nOp;							// operator
		int				m_nLine;						// line number in script
};


const int kScriptPacketExprSize = sizeof( ScriptPacketExpr );

//
//	ScriptPacketExprList
//

class ScriptPacketExprList : public CList<ScriptPacketExprPtr,ScriptPacketExprPtr> {
	public:
		ScriptPacketExprList( ScriptPacketExprListPtr prev );
		virtual ~ScriptPacketExprList();
	
		ScriptPacketExprListPtr		listPrevious;		// inheritance chain

		// list operations
		void Append( ScriptPacketExprPtr sbp );			// add a child at the end
		void Remove( int indx );						// remove a child

		int Length( void );								// number of children
		ScriptPacketExprPtr Child( int indx );			// child by index
		ScriptPacketExprPtr Find( int keyword );		// specific child

		int FirstData( void );							// index of first app layer elem
	};

const int kScriptPacketExprListSize = sizeof( ScriptPacketExprList );

//
//	ScriptPacket
//

const int kMaxPacketDelay = 1800;						// in ms, 1 minute

class ScriptPacket : public ScriptBase {
	public:
		enum ScriptPacketType
				{ sendPacket
				, expectPacket
				};
		enum ScriptPacketSubtype
				{ rootPacket
				, andPacket
				, orPacket
				};

		ScriptPacket( ScriptPacketType type, ScriptPacketSubtype subtype, ScriptPacketPtr prev );
		~ScriptPacket( void );

		ScriptPacketType			packetType;			// type
		ScriptPacketSubtype			packetSubtype;		// could be root!
		ScriptPacketExprList		packetExprList;		// list of expressions

		int							packetLevel;		// case level
		int							packetDelay;		// BEFORE/AFTER time value (in ms)
		ScriptPacketPtr				packetNext;			// next packet in test sequence

		ScriptPacketPtr				packetPass;			// next when execution passes
		ScriptPacketPtr				packetFail;			// next when it fails
	};

const int kScriptPacketSize = sizeof( ScriptPacket );

#endif // !defined(AFX_SCRIPTPACKET_H__AC59C5B1_BAFF_11D4_BEF7_00A0C95A9812__INCLUDED_)
