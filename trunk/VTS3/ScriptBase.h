// ScriptBase.h: interface for the ScriptBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTBASE_H__18DBD511_B069_11D4_BEEA_00A0C95A9812__INCLUDED_)
#define AFX_SCRIPTBASE_H__18DBD511_B069_11D4_BEEA_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

#include "BACnet.hpp"

//
//	ScriptBase
//
//	All of the script elements are derived from this base class that 
//	provides basic container functionality and information to display 
//	the heirarchy in the tree view.
//

class ScriptBase;
typedef ScriptBase *ScriptBasePtr;

class ScriptBase : public CList<ScriptBasePtr,ScriptBasePtr> {
	public:
		enum ScriptBaseType
				{ scriptNull
				, scriptSection
				, scriptTest
				, scriptDependency
				, scriptReference
				, scriptPacket
				, scriptCase
				};

		ScriptBase();
		virtual ~ScriptBase();
	
		ScriptBaseType	baseType;							// runtime type info
		CString			baseLabel;							// label for tree view
		int				baseLineStart, baseLineCount;		// where in source

		HTREEITEM		baseItem;							// item handle when bound
		int				baseImage, baseStatus;				// index and offset of status

		ScriptBasePtr	baseParent;							// pointer to parent

		// list operations
		void Append( ScriptBasePtr sbp );					// add a child at the end
		void Remove( int indx );							// remove a child

		int Length( void );									// number of children
		ScriptBasePtr Child( int indx );					// specific child
	};

const int kScriptBaseSize = sizeof( ScriptBase );

//
//	ScriptSection
//

class ScriptSection : public ScriptBase {
	public:
		ScriptSection( const CString& title );
	};

typedef ScriptSection *ScriptSectionPtr;
const int kScriptSectionSize = sizeof( ScriptSection );

//
//	ScriptDependency
//

class ScriptDependency : public ScriptBase {
	public:
		ScriptDependency( const CString& number );
	};

typedef ScriptDependency *ScriptDependencyPtr;
const int kScriptDependencySize = sizeof( ScriptDependency );


//
//	ScriptReference
//

class ScriptReference : public ScriptBase {
	public:
		ScriptReference( const CString& clause );
	};

typedef ScriptReference *ScriptReferencePtr;
const int kScriptReferenceSize = sizeof( ScriptReference );

//
//	ScriptCase
//

class ScriptCase : public ScriptBase {
	public:
		ScriptCase		*caseGroup;
		ScriptCase		*caseSubgroup;

		ScriptCase( const CString& number );
	};

typedef ScriptCase *ScriptCasePtr;
const int kScriptCaseSize = sizeof( ScriptCase );

//
//	ScriptTest
//

class ScriptPacket;

class ScriptTest : public ScriptCase {
	public:
		ScriptTest( const CString& number );

		ScriptTest		*testNext;					// next test
		ScriptPacket	*testFirstPacket;			// first packet in test
	};

typedef ScriptTest *ScriptTestPtr;
const int kScriptTestSize = sizeof( ScriptTest );

//
//	ScriptToken
//

enum ScriptTokenType
		{ scriptError										// invalid character
		, scriptKeyword										// word
		, scriptTitle										// title string
		, scriptSymbol										// operator or symbol
		, scriptValue										// some kind of value, see encoding
		, scriptReference									// EPICS reference name
		, scriptEOL											// end of line
		, scriptEOF											// end of file
		};

enum ScriptEncodingType
		{ scriptIntegerEnc									// [+|-](0..9)*
		, scriptFloatEnc									// [+|-]N[.N][e[-]N]
		, scriptDecimalEnc									// D'nnn'
		, scriptHexEnc										// 0xFF, X'FF', &xFF
		, scriptOctalEnc									// 0o377, O'377', &O377
		, scriptBinaryEnc									// 0b11111111, B'11111111', &B11111111
		, scriptASCIIEnc									// A'hi', "hi", 'hi'
		, scriptIPEnc										// n.n.n.n[/n][:n]
		};

struct ScriptTranslateTable {
	int		scriptKeyword;
	int		scriptValue;
	};

typedef ScriptTranslateTable *ScriptTranslateTablePtr;
const int kScriptTranslateTableSize = sizeof( ScriptTranslateTable );

//
//	ScriptToken
//

class ScriptParmList;
typedef ScriptParmList *ScriptParmListPtr;

const int kTokenBufferSize = 8192;

class ScriptToken {
	public:
		ScriptToken( void );								// initialize
		~ScriptToken( void );

		ScriptToken( const ScriptToken &cpy );				// copy constructor allowed
		operator =( const ScriptToken &ref );				// assignment allowed

		ScriptTokenType		tokenType;						// token type
		ScriptEncodingType	tokenEnc;						// value encoding
		int					tokenSymbol;					// character code or keyword hash code
		int					tokenLine;						// line number
		int					tokenOffset, tokenLength;		// char offset and length

		CString				tokenValue;						// token value

		bool IsInteger( int &valu, ScriptParmListPtr parms = 0 ) const;	// acceptable integer form with parm lookup
		bool IsInteger( int &valu, ScriptTranslateTablePtr tp ) const;	// with keyword lookup
		bool IsPropertyReference( int &prop ) const;				// with keyword lookup
		bool IsEncodeable( BACnetEncodeable &enc ) const;			// can this be interpreted?

		static int HashCode( const char *key );						// return a hash code
		static int Lookup( int code, ScriptTranslateTablePtr tp );	// find keyword in a table, -1 if not found
	};

typedef ScriptToken *ScriptTokenPtr;
const int kScriptTokenSize = sizeof( ScriptToken );

//
//	ScriptTokenList
//

class ScriptTokenList : public CList<ScriptTokenPtr,ScriptTokenPtr> {
	public:
		ScriptTokenList( void );
		~ScriptTokenList( void );

		void Append( const ScriptToken& tok );				// add a token (makes a copy)

		int Length( void );									// number of tokens
		const ScriptToken& operator []( int i );			// index operator
	};

typedef ScriptTokenList *ScriptTokenListPtr;
const int kScriptTokenListSize = sizeof( ScriptTokenList );

//
//	ScriptScanner
//

class ScriptScanner {
	protected:
		CEdit*			scanSource;							// where this token came from (?)
		int				scanLine;							// line number of source

		char			scanLineBuffer[kTokenBufferSize];	// current line being parsed
		char			scanValueBuffer[kTokenBufferSize];	// current token being built

		void FormatError( ScriptToken &tok, char *msg );	// sets up the token and throws the message

	public:
		const char		*scanSrc;							// ptr to current/next token

		ScriptScanner( CEdit* ep );							// source of parsing
		ScriptScanner( const char *src );					// source of parsing
		~ScriptScanner( void );

		void Next( ScriptToken& tok );						// put the next token in here
		void Peek( ScriptToken& tok );						// non-destructive look into stream
		void NextTitle( ScriptToken& tok );					// the rest of the line is a title
		void NextLine( ScriptToken& tok );					// move to the next line in the source
	};

typedef ScriptScanner *ScriptScannerPtr;
const int kScriptScannerSize = sizeof( ScriptScanner );

#endif // !defined(AFX_SCRIPTBASE_H__18DBD511_B069_11D4_BEEA_00A0C95A9812__INCLUDED_)
