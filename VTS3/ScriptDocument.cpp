// ScriptDocument.cpp : implementation of the ScriptDocument class
//

#include "stdafx.h"

#include "ScriptDocument.h"
#include "ScriptPacket.h"
#include "ScriptKeywords.h"
#include "ScriptExecutor.h"

#include "md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ScriptDocument

IMPLEMENT_DYNCREATE(ScriptDocument, CDocument)

BEGIN_MESSAGE_MAP(ScriptDocument, CDocument)
	//{{AFX_MSG_MAP(ScriptDocument)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

//
//	ScriptDocument::ScriptDocument
//

ScriptDocument::ScriptDocument()
	: m_editData(0)
	, m_pSelectedTest(0)
	, m_bExecBound(false)
{
	// TODO: add one-time construction code here
}

//
//	ScriptDocument::~ScriptDocument
//

ScriptDocument::~ScriptDocument()
{
}

//
//	ScriptDocument::OnNewDocument
//

BOOL ScriptDocument::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

//
//	ScriptDocument::OnCloseDocument
//

void ScriptDocument::OnCloseDocument( void )
{
	// if there is a test running (or halted) kill it
	if (m_bExecBound)
		gExecutor.Kill();

	// let the base class continue
	CDocument::OnCloseDocument();
}

//
//	ScriptDocument::Serialize
//
//	The content of the script is simply the content of the file, and since 
//	CEdit already knows how to load and store its content through its own
//	serialize member function, just pass it along.
//

void ScriptDocument::Serialize(CArchive& ar)
{
	((CEditView*)m_editData)->SerializeRaw(ar);
}

//
//	ScriptDocument::AssertValid
//

#ifdef _DEBUG
void ScriptDocument::AssertValid() const
{
	CDocument::AssertValid();
}
#endif

//
//	ScriptDocument::Dump
//

#ifdef _DEBUG
void ScriptDocument::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif

//
//	ScriptDocument::CheckSyntax
//

void ScriptDocument::CheckSyntax( void )
{
	int					curCaseLevel = 0
	;
	bool				setupMode = false, isSend = false, expectRequired = false
	,					isEnv
	;
	ScriptScanner		scan( m_editData )
	;
	ScriptToken			tok
	;
	ScriptBasePtr		curBase = new ScriptBase()
	;
	ScriptCasePtr		curCase, newCase
	;
	ScriptDependencyPtr	newDep
	;
	ScriptReferencePtr	newRef
	;
	ScriptSectionPtr	curSection = 0
	;
	ScriptTestPtr		curTest = 0, newTest
	;
	ScriptPacketPtr		prevPacket = 0, prevGroup = 0, newPacket
	;

	// calculate the digest
	CalcDigest();

	// remember if this is the current environment
	isEnv = (m_pParmList == gCurrentEnv);

	// set up the parameter list for cleaning later
	m_pParmList->Mark( isEnv );

	try {
		for (;;) {
			scan.NextLine( tok );
			if (tok.tokenType == scriptEOF)
				break;
			if (tok.tokenType == scriptEOL) {
				if (curTest) {
					// add description to test
				}
				continue;
			}

			// look for the variable name
			if (tok.tokenType != scriptKeyword) {
				if (!curSection)
					throw "SETUP/SECTION keyword expected";
				if (!curTest)
					throw "TEST keyword expected";

				// look for test content
				throw "DEPENDENCIES/REFERENCE/SEND/EXPECT expected";
			}

			// check for setup mode
			switch (tok.tokenSymbol) {
				case kwSETUP:
					setupMode = true;
					break;

				case kwSECTION:
					// get the section title
					scan.NextTitle( tok );
					if (tok.tokenValue.IsEmpty())
						throw "Section title expected";

					// create a new section and add it
					curSection = new ScriptSection( tok.tokenValue );
					curBase->Append( curSection );

					// save the line number
					curSection->baseLineStart = tok.tokenLine;
					curSection->baseLineCount = 1;
					
					// clean up previous test iff there was one
					if (curTest)
						SequenceTest( curTest );

					// reset the test
					curTest = 0;
					setupMode = false;
					break;

				case kwTEST:
					// make sure a section is defined
					if (!curSection)
						throw "Section required before test definition";

					// get the test number
					scan.NextTitle( tok );
					if (tok.tokenValue.IsEmpty())
						throw "Test number expected";

					// create a new test and add it to the section
					newTest = new ScriptTest( tok.tokenValue );
					curSection->Append( newTest );
					TRACE1( "Test %08X\n", newTest );

					// save the line number
					newTest->baseLineStart = tok.tokenLine;
					newTest->baseLineCount = 1;
					
					// clean up and chain from the previous test
					if (curTest) {
						SequenceTest( curTest );
						curTest->testNext = newTest;
					}

					// switch context to new test, no previous packet
					curTest = newTest;
					prevPacket = 0;

					// the level zero case group is the test
					curCase = curTest;
					curCase->caseGroup = 0;
					curCase->caseSubgroup = 0;
					curCaseLevel = 0;

					// no longer in setup mode
					setupMode = false;

					// EXPECT not necessary
					expectRequired = false;
					break;

				case kwDEPS:
				case kwDEPENDENCIES:
					// make sure a section is defined
					if (!curTest)
						throw "Test required before dependencies";

					// get the test number
					scan.NextTitle( tok );
					if (tok.tokenValue.IsEmpty())
						throw "Test number expected";

					// create a new test and add it to the section
					newDep = new ScriptDependency( tok.tokenValue );
					curTest->Append( newDep );
					TRACE1( "Deps %08X\n", newDep );

					// save the line number
					newDep->baseLineStart = tok.tokenLine;
					newDep->baseLineCount = 1;
					
					break;

				case kwREF:
				case kwREFERENCE:
					// make sure a test is defined
					if (!curTest)
						throw "Test required before references";

					// get the clause
					scan.NextTitle( tok );
					if (tok.tokenValue.IsEmpty())
						throw "Clause expected";

					// create a new reference and add it to the test
					newRef = new ScriptReference( tok.tokenValue );
					curTest->Append( newRef );
					TRACE1( "Ref %08X\n", newRef );

					// save the line number
					newRef->baseLineStart = tok.tokenLine;
					newRef->baseLineCount = 1;
					
					break;

				case kwSEND:
				case kwTRANSMIT:
					// make sure a test is defined
					if (!curTest)
						throw "Test required before packets";
					if (expectRequired)
						throw "EXPECT required after CASE statement";

					// create a new test and add it to the section
					newPacket =
						new ScriptPacket( ScriptPacket::sendPacket
								, ScriptPacket::rootPacket
								, 0
								);
					prevGroup = newPacket;
					newPacket->packetLevel = curCaseLevel;
					curCase->Append( newPacket );

					// look for AFTER or (
					scan.Next( tok );
					if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == '('))
						newPacket->packetDelay = 0;
					else
					if ((tok.tokenType == scriptKeyword) && (tok.tokenSymbol == kwAFTER)) {
						// look for value
						scan.Next( tok );
						if (!tok.IsInteger(newPacket->packetDelay))
							throw "Packet delay expected";
						if (newPacket->packetDelay < 0)
							throw "Delay must be a non-negative value";
						if (newPacket->packetDelay > kMaxPacketDelay)
							throw "Maximum delay exceeded";
						scan.Next( tok );
						if ((tok.tokenType != scriptSymbol) || (tok.tokenSymbol != '('))
							throw "Open parenthesis '(' expected";
					} else
						throw "Open parenthesis '(' expected";
					TRACE2( "Packet %08X (Send, %d)\n", newPacket, newPacket->packetDelay );

					// now parse the contents
					isSend = true;
					ParsePacket( scan, tok, newPacket, isSend );

#if 0
					// chain together
					if (prevPacket)
						prevPacket->packetNext = newPacket;
#endif
					// start a new chain
					prevPacket = newPacket;
					break;

				case kwEXPECT:
				case kwRECEIVE:
					// make sure a test is defined
					if (!curTest)
						throw "Test required before packets";

					// create a new test and add it to the section
					newPacket =
						new ScriptPacket( ScriptPacket::expectPacket
								, ScriptPacket::rootPacket
								, 0
								);
					prevGroup = newPacket;
					newPacket->packetLevel = curCaseLevel;
					curCase->Append( newPacket );

					// look for BEFORE or (
					scan.Next( tok );
					if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == '('))
						newPacket->packetDelay = kMaxPacketDelay;
					else
					if ((tok.tokenType == scriptKeyword) && (tok.tokenSymbol == kwBEFORE)) {
						// look for value
						scan.Next( tok );
						if (!tok.IsInteger(newPacket->packetDelay))
							throw "Packet delay expected";
						if (newPacket->packetDelay < 0)
							throw "Delay must be a non-negative value";
						if (newPacket->packetDelay > kMaxPacketDelay)
							throw "Maximum delay exceeded";
						scan.Next( tok );
						if ((tok.tokenType != scriptSymbol) || (tok.tokenSymbol != '('))
							throw "Open parenthesis '(' expected";
					} else
						throw "Open parenthesis '(' expected";
					TRACE2( "Packet %08X (Expect, %d)\n", newPacket, newPacket->packetDelay );

					// now parse the contents
					isSend = false;
					ParsePacket( scan, tok, newPacket, isSend );

					// other statements available now
					expectRequired = false;

#if 0
					// chain together
					if (prevPacket)
						prevPacket->packetNext = newPacket;
#endif
					// start a new chain
					prevPacket = newPacket;
					break;

				case kwAND:
					// make sure a test is defined
					if (!curTest)
						throw "Test required";
					if (expectRequired)
						throw "EXPECT required after CASE statement";
					if (!prevGroup)
						throw "SEND or EXPECT required";

					// create a new test and add it to the section
					newPacket =
						new ScriptPacket( prevPacket->packetType
								, ScriptPacket::andPacket
								, prevGroup
								);
					prevGroup = newPacket;
					newPacket->packetLevel = curCaseLevel;
					curCase->Append( newPacket );

					// look for AFTER/BEFORE or (
					scan.Next( tok );
					if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == '('))
						newPacket->packetDelay = prevPacket->packetDelay;
					else
					if ((tok.tokenType == scriptKeyword) &&
							(  ((prevGroup->packetType == ScriptPacket::sendPacket) && (tok.tokenSymbol == kwAFTER))
							|| ((prevGroup->packetType == ScriptPacket::expectPacket) && (tok.tokenSymbol == kwBEFORE))
							))
						{
						// look for value
						scan.Next( tok );
						if (!tok.IsInteger(newPacket->packetDelay))
							throw "Packet delay expected";
						if (newPacket->packetDelay < 0)
							throw "Delay must be a non-negative value";
						if (newPacket->packetDelay > kMaxPacketDelay)
							throw "Maximum delay exceeded";
						scan.Next( tok );
						if ((tok.tokenType != scriptSymbol) || (tok.tokenSymbol != '('))
							throw "Open parenthesis '(' expected";
					} else
						throw "Open parenthesis '(' expected";
					TRACE2( "Packet %08X (And, %d)\n", newPacket, newPacket->packetDelay );

					// now parse the contents
					ParsePacket( scan, tok, newPacket, isSend );

					// chain from previous packet
					prevPacket->packetNext = newPacket;
					prevPacket = newPacket;
					break;

				case kwOR:
					// make sure a test is defined
					if (!curTest)
						throw "Test required";
					if (expectRequired)
						throw "EXPECT required after CASE statement";
					if (!prevGroup)
						throw "SEND or EXPECT required";

					// create a new test and add it to the section
					newPacket = new ScriptPacket( prevPacket->packetType
								, ScriptPacket::orPacket
								, prevGroup
								);
					prevGroup = newPacket;
					newPacket->packetLevel = curCaseLevel;
					curCase->Append( newPacket );

					// look for AFTER/BEFORE or (
					scan.Next( tok );
					if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == '('))
						newPacket->packetDelay = prevPacket->packetDelay;
					else
					if ((tok.tokenType == scriptKeyword) &&
							(  ((prevGroup->packetType == ScriptPacket::sendPacket) && (tok.tokenSymbol == kwAFTER))
							|| ((prevGroup->packetType == ScriptPacket::expectPacket) && (tok.tokenSymbol == kwBEFORE))
							))
						{
						// look for value
						scan.Next( tok );
						if (!tok.IsInteger(newPacket->packetDelay))
							throw "Packet delay expected";
						if (newPacket->packetDelay < 0)
							throw "Delay must be a non-negative value";
						if (newPacket->packetDelay > kMaxPacketDelay)
							throw "Maximum delay exceeded";
						scan.Next( tok );
						if ((tok.tokenType != scriptSymbol) || (tok.tokenSymbol != '('))
							throw "Open parenthesis '(' expected";
					} else
						throw "Open parenthesis '(' expected";
					TRACE2( "Packet %08X (Or, %d)\n", newPacket, newPacket->packetDelay );

					// now parse the contents
					ParsePacket( scan, tok, newPacket, isSend );

					// chain from previous packet
					prevPacket->packetNext = newPacket;
					prevPacket = newPacket;
					break;

				case kwCASE:
					if (!curTest)
						throw "Test required";

					// get the case number
					scan.NextTitle( tok );
					if (tok.tokenValue.IsEmpty())
						throw "Case title expected";

					// create a new case and add it to the section
					newCase = new ScriptCase( tok.tokenValue );
					
					// figure out what level is being described
					int		level
					;
					LPCSTR	src
					;

					for (src = tok.tokenValue; *src; src++)
						if (!isalnum(*src) && (*src != '.')) {
							delete newCase;
							throw "Invalid character in case label";
						}

					src = tok.tokenValue;
					level = 0;
					for (;;) {
						// can't start with a dot
						if (*src == '.') {
							delete newCase;
							throw "Invalid case label form";
						}

						// skip over the word
						while (*src && (*src != '.'))
							src += 1;

						// count this word as a level
						level += 1;

						if (*src)
							src += 1;
						else
							break;
					}

					if (level == curCaseLevel) {
						// same level
						newCase->baseParent = curCase->baseParent;
						newCase->caseGroup = curCase->caseGroup;
					} else
					if (level == (curCaseLevel + 1)) {
						// indent
						curCaseLevel += 1;
						newCase->baseParent = curCase;
						newCase->caseGroup = newCase;

						// tell the parent
						curCase->caseSubgroup = newCase;
					} else
					if (level == (curCaseLevel - 1)) {
						// parent level
						curCaseLevel -= 1;
						curCase = (ScriptCasePtr)curCase->baseParent;
						newCase->baseParent = curCase->baseParent;
						newCase->caseGroup = ((ScriptCasePtr)curCase->baseParent)->caseSubgroup;
					} else {
						delete newCase;
						throw "Invalid case level";
					}

					newCase->baseParent->Append( newCase );
					TRACE2( "Case %08X, group %08X\n", newCase, newCase->caseGroup );
					curCase = newCase;
					curCase->caseSubgroup = 0;

					// next statement must be EXPECT
					expectRequired = true;
					break;

				case kwENDCASE:
					if (!curTest)
						throw "Test required";
					if (expectRequired)
						throw "EXPECT required after CASE statement";
					if (curCaseLevel == 0)
						throw "Mismatched end case";

					// shift back a level
					curCaseLevel -= 1;
					curCase = (ScriptCasePtr)curCase->baseParent;
					TRACE0( "Endcase\n" );

					// reset the child group
					curCase->caseSubgroup = 0;
					break;

				default:
					if (!setupMode)
						throw "Unrecognized keyword";

					// save it and get the next token
					CString name = tok.tokenValue;
					CString valu = "";
					scan.Next( tok );

					// check for optional '='
					if (tok.tokenType == scriptSymbol) {
						if (tok.tokenSymbol != '=')
							throw "Assignment operator '=' expected";
						scan.Next( tok );
					}

					for (;;) {
						// get the value and save it
						if ((tok.tokenType != scriptValue) && (tok.tokenType != scriptKeyword)
								 && (tok.tokenType != scriptReference))
							throw "Parameter value expected";

						// save the value
						valu += tok.tokenValue;
						scan.Next( tok );

						// check for EOL
						if (tok.tokenType == scriptEOL)
							break;

						// check for more
						if ((tok.tokenType != scriptSymbol) || (tok.tokenSymbol != ','))
							throw "End-of-line or comma expected";
						valu += ", ";

						// get ready for next value
						scan.Next( tok );
					}

					// add it to the parameter list
					m_pParmList->Add( name, valu, tok.tokenValue, tok.tokenLine, isEnv );
					break;
			}
		}

		// sequence the last test
		if (curTest)
			SequenceTest( curTest );
	}
	catch (char *errMsg) {
		m_editData->SetSel( tok.tokenOffset, tok.tokenOffset+tok.tokenLength );
		AfxMessageBox( errMsg );
		delete curBase;
		curBase = 0;
	}

	// delete those vars not found during this parsing phase
	m_pParmList->Release();

	// if this is the current environment, tell everyone else to match
	if (isEnv) {
		for (int i = 0; i < gScriptParmLists.Length(); i++) {
			ScriptParmListPtr splp = gScriptParmLists[i];

			if (splp != gCurrentEnv)
				splp->MatchEnv();
		}
	} else
	// if there is a current environment, match to it 
	if (gCurrentEnv)
		m_pParmList->MatchEnv();

	// bind to the new content (if there is any)
	m_pContentTree->Bind( curBase );
}

//
//	ScriptDocument::ParsePacket
//

void ScriptDocument::ParsePacket( ScriptScanner& scan, ScriptToken& tok, ScriptPacketPtr spp, bool isSend )
{
	int					eKeyword
	;
	ScriptToken			keyToken
	;
	ScriptPacketExprPtr	spep
	;

	// save the line number
	spp->baseLineStart = tok.tokenLine;

	for (;;) {
		scan.NextLine( tok );
		if (tok.tokenType == scriptEOF)
			throw "Unexpected end of file";
		if (tok.tokenType == scriptEOL)
			continue;

		// if we get ')' we're done
		if (tok.tokenType == scriptSymbol) {
			if (tok.tokenSymbol == ')')
				break;
			throw "Close parenthesis ')' expected";
		}

		// look for the name
		if (tok.tokenType != scriptKeyword)
			throw "Keyword expected";
		eKeyword = tok.tokenSymbol;

		// save this token in case there are problems with the keyword
		keyToken = tok;

		// create an object to hold stuff
		spep = new ScriptPacketExpr;
		spep->exprKeyword = eKeyword;
		spep->exprOp = '=';
		spep->exprIsData = 
//			   (eKeyword == kwNL)
//			|| (eKeyword == kwNLDATA)
			   (eKeyword == kwAL)
			|| (eKeyword == kwALDATA)
			|| (ScriptToken::Lookup( eKeyword, ScriptALMap ) >= 0);
		spep->exprValue = "";

		// save the line number for better error reporting
		spep->exprLine = tok.tokenLine;

		// look for an operator, default '='
		scan.Peek( tok );
		if (tok.tokenType == scriptSymbol) {
			// must be an operator, extract it
			scan.Next( tok );

			// make sure it's valid
			if (	(tok.tokenSymbol == '<')
				||	(tok.tokenSymbol == '<=')
				||	(tok.tokenSymbol == '>')
				||	(tok.tokenSymbol == '>=')
				||	(tok.tokenSymbol == '=')
				||	(tok.tokenSymbol == '!=')
				) {
				if (isSend && (tok.tokenSymbol != '='))
					throw "Equals operator '=' required";
				spep->exprOp = tok.tokenSymbol;
			} else
				throw "Operator expected";
		}

		// data lines (like DATE and TIME) need special parsers, don't bother here
		if (spep->exprIsData) {
			scan.NextTitle( tok );
			spep->exprValue = tok.tokenValue;
		} else {
			// move to next token
			scan.Next( tok );

			// scan in the values
			for (;;) {
				// must be a value
				if ((tok.tokenType != scriptValue) && (tok.tokenType != scriptKeyword) && (tok.tokenType != scriptReference)) {
					delete spep;
					throw "Value required";
				}
				spep->exprValue += tok.tokenValue;

				// move to next token
				scan.Next( tok );

				// check for end-of-line
				if (tok.tokenType == scriptEOL)
					break;

				// should be a comma
				if ((tok.tokenType != scriptSymbol) || (tok.tokenSymbol != ',')) {
					delete spep;
					throw "Comma or end-of-line expected";
				}
				spep->exprValue += ", ";
				scan.Next( tok );
			}
		}

		// all set, add the expression to the packet expression list
		spp->packetExprList.Append( spep );
	}

	// compute line count
	spp->baseLineCount = (tok.tokenLine - spp->baseLineStart) + 1;
}

//
//	ScriptDocument::CalcDigest
//

void ScriptDocument::CalcDigest( void )
{
	int				lineCount
	;
	unsigned short	lineLen
	;
	MD5_CTX			md
	;
	char			lineBuffer[ 256 ]
	;

	TRACE1( "Doc: %s\n", this->GetPathName() );

	MD5Init( &md );

	lineCount = m_editData->GetLineCount();
	for (int i = 0; i < lineCount; i++) {
		// suck the line into a private buffer
		lineLen = m_editData->GetLine( i, lineBuffer, sizeof(lineBuffer) );
		MD5Update( &md, (unsigned char *)lineBuffer, lineLen );
	}

	MD5Final( &md );

	// save the digest
	memcpy( m_digest, md.digest, sizeof(md.digest) );
}

//
//	ScriptDocument::SequenceTest
//
//	This function chains all of the packets pass and fail pointers so the execution has 
//	very little work to do figuring out what's next.
//

void ScriptDocument::SequenceTest( ScriptTestPtr stp )
{
	ScriptPacketPtr		pChain
	;

	stp->testFirstPacket = SequenceLevel( stp, 0, pChain );
}

//
//	ScriptDocument::SequenceLevel
//
//	General idea: start at the end of the group of packets and work backwards.  For
//	each root element (a SEND or EXPECT), sequence all of the related packets going 
//	forwards (AND and OR).  Save a pointer to the first element of the last disjunctive
//	group (the last OR packet) so it can be chained across case groups.
//
//	Return a pointer to the start of the group.  If the start of the group is an EXPECT, 
//	then the chain pointer will be important to the next level up.
//

ScriptPacketPtr ScriptDocument::SequenceLevel( ScriptBasePtr sbp, ScriptPacketPtr pPass, ScriptPacketPtr &pChain )
{
	int				len = sbp->Length()
	;
	ScriptCasePtr	curCase
	,				nextCase = 0
	;
	ScriptPacketPtr	curPacket, pStart
	,				nextCaseStart = 0
	;

	for (int i = len - 1; i >= 0; i--) {
		ScriptBasePtr cur = sbp->Child( i );

		if (cur->baseType == ScriptBase::scriptPacket) {
			curPacket = (ScriptPacketPtr)cur;

			if (curPacket->packetSubtype == ScriptPacket::rootPacket) {
				if (nextCaseStart) {
					pPass = nextCaseStart;
					nextCase = 0;
					nextCaseStart = 0;
				}

				pChain = SequencePacket( curPacket, pPass );
				pPass = curPacket;
			}
		} else
		if (cur->baseType == ScriptBase::scriptCase) {
			curCase = (ScriptCasePtr)cur;

			// the next case might be in a different group
			if (nextCase && (nextCase->caseGroup != curCase->caseGroup)) {
				pPass = nextCaseStart;
				nextCaseStart = 0;
			}

			// sequence the level
			pStart = SequenceLevel( cur, pPass, pChain );

			// fix the chain to point to the next case
			TRACE2( "%08X->fail = %08X\n", pChain, nextCaseStart );
			pChain->packetFail = nextCaseStart;

			// get ready for another case
			nextCase = curCase;
			nextCaseStart = pStart;
		} else
			;
	}

	// first thing we found was an expect inside a case
	if (nextCaseStart)
		pPass = nextCaseStart;

	return pPass;
}

//
//	ScriptDocument::SequencePacket
//
//	Given a root packet, this procedure sequences all of the AND and OR packets.  It will 
//	return a pointer to the "chain" packet, the first packet in the last OR group.
//

ScriptPacketPtr ScriptDocument::SequencePacket( ScriptPacketPtr spp, ScriptPacketPtr pPass )
{
	ScriptPacketPtr		chain = spp, prev
	;

	// begin a conjunction
	for (;;) {
		// init the basics
		TRACE2( "%08X->pass = %08X\n", spp, pPass );
		spp->packetPass = pPass;
		TRACE2( "%08X->fail = %08X\n", spp, 0 );
		spp->packetFail = 0;

		// link the AND's together
		for (;;) {
			prev = spp;
			spp = spp->packetNext;

			// chain the success path
			if (spp && (spp->packetSubtype == ScriptPacket::andPacket)) {
				TRACE2( "%08X->pass = %08X\n", prev, spp );
				prev->packetPass = spp;
				TRACE2( "%08X->pass = %08X\n", spp, pPass );
				spp->packetPass = pPass;
				TRACE2( "%08X->fail = %08X\n", spp, 0 );
				spp->packetFail = 0;
			} else
				break;
		}

		// got an OR?
		if (spp && (spp->packetSubtype == ScriptPacket::orPacket)) {
			TRACE2( "%08X->fail = %08X\n", chain, spp );
			chain->packetFail = spp;
			chain = spp;
		} else
			break;
	}

	// return a pointer to the packet to chain
	return chain;
}

//
//	ScriptDocument::BindExecutor
//

void ScriptDocument::BindExecutor( void )
{
	m_bExecBound = true;

	// lock the edit window
	m_editData->SetReadOnly( true );
}

//
//	ScriptDocument::UnbindExecutor
//

void ScriptDocument::UnbindExecutor( void )
{
	m_bExecBound = false;

	// unlock the edit window
	m_editData->SetReadOnly( false );
}

//
//	ScriptDocument::Reset
//

void ScriptDocument::Reset( void )
{
	m_pParmList->Reset();
}

//
//	ScriptDocument::SetImageStatus
//

void ScriptDocument::SetImageStatus( ScriptBasePtr sbp, int stat )
{
	// set the image list element to match
	m_pContentTree->m_pTreeCtrl->SetItemImage( sbp->baseItem
		, sbp->baseImage + stat
		, sbp->baseImage + stat
		);
}