// ScriptCommand.cpp: implementation of the ScriptCHECKCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptDocument.h"
#include "ScriptPacket.h"
#include "ScriptIfdefHandler.h"
#include "ScriptCommand.h"

#include "ScriptMakeDlg.h"
#include "ScriptMsgMake.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


ScriptMessage::ScriptMessage( ScriptIfdefHandler & ifdefHandler, ScriptScanner & scan, ScriptToken & tok, CString * pstrTestTitle, CWnd * pparent /* = NULL */ )
			  : ScriptCommand(ScriptBase::scriptCheck)
			  , m_pstrTestTitle(pstrTestTitle)
{
	baseStatus = 0;
	m_pParent = pparent;

	baseLineStart = tok.tokenLine;

	scan.Next( tok );
	if ( tok.tokenType != scriptValue || tok.tokenEnc != scriptASCIIEnc )
		throw "CHECK/MAKE statement requires \"Title Text\" following keyword";

	baseLabel = tok.RemoveQuotes();
	Parse(ifdefHandler, scan, tok);

	baseLineCount = baseLineStart - tok.tokenLine;
}


void ScriptMessage::Parse( ScriptIfdefHandler & ifdefHandler, ScriptScanner & scan, ScriptToken & tok )
{
	bool fOpen = false;

	// start looking first for ')'... it might be where we left off or the next line.
	// either way... look for it first.

	scan.Next(tok);

	for(;;)
	{
		if ( tok.tokenType == scriptEOF )
			throw "Unexpected end of file";

		if ( tok.tokenType != scriptEOL )
		{
			if ( !fOpen )
			{
				// we need to check for the first thing...
				if ( tok.tokenType != scriptSymbol ||  tok.tokenSymbol != '(' )
					throw "Open parenthesis '(' expected after CHECK/MAKE title";

				fOpen = true;			// make sure we don't do this again
				scan.Next(tok);			// could look for next string right after (...
				continue;
			}

			if ( tok.tokenEnc == scriptASCIIEnc && tok.tokenType == scriptValue )
			{
				if ( !tok.IsDontCare() )
					m_strText = m_strText + (m_strText.IsEmpty() ? "" : "\n") + tok.RemoveQuotes();
				scan.Next(tok);
			}

			// know when to quit?
			if ( tok.tokenType != scriptEOL )
			{
				if ( tok.tokenType != scriptSymbol ||  tok.tokenSymbol != ')' )
					throw "Closing parenthesis ')' expected after CHECK/MAKE text";

				// found closing deal... move on from here
				break;
			}
		}

		do
			scan.NextLine(tok);
		while ( ifdefHandler.IsIfdefExpression(tok, &scan) || ifdefHandler.IsSkipping() );
	}
}


ScriptCHECKCommand::ScriptCHECKCommand( ScriptIfdefHandler & ifdefHandler, ScriptScanner & scan, ScriptToken & tok, CString * pstrTestTitle, CWnd * pparent /* = NULL */ )
				   :ScriptMessage(ifdefHandler, scan, tok, pstrTestTitle, pparent)
{
	baseType = scriptCheck;

	// set the image list offset and status
	baseImage = 21;
}


ScriptCHECKCommand::~ScriptCHECKCommand()
{
}


bool ScriptCHECKCommand::Execute(CString * pstrError)
{
	CString strFullText;

	strFullText.Format("TEST:  %s\nCHECK: %s\n_____________________________________________\n\n%s\n_____________________________________________\nPass %s?",
						(LPCSTR) *m_pstrTestTitle, baseLabel, m_strText, baseLabel);

	CScriptMakeDlg	dlg(false, strFullText, m_pParent);
	if ( dlg.DoModal() != IDOK )
//	if ( AfxMessageBox(strFullText, MB_YESNO | MB_ICONQUESTION) != IDYES )
	{
		ASSERT(pstrError != NULL);
		if ( pstrError != NULL )
			pstrError->Format("CHECK \"%s\" failed", baseLabel);

		return false;
	}

	return true;
}


ScriptMAKECommand::ScriptMAKECommand( ScriptIfdefHandler & ifdefHandler, ScriptScanner & scan, ScriptToken & tok, CString * pstrTestTitle, CWnd * pparent /* = NULL */ )
				   :ScriptMessage(ifdefHandler, scan, tok, pstrTestTitle, pparent )
{
	baseType = scriptMake;

	// set the image list offset and status
	baseImage = 25;
	m_pdlg = NULL;
	m_fHanging = false;
	m_nDestroyCode = 0;
}


ScriptMAKECommand::~ScriptMAKECommand()
{
	Kill();
}


void ScriptMAKECommand::Kill(int nDestroyCode /* = 1 */ )
{
	if ( m_pdlg != NULL  )
	{
		if ( m_pdlg->IsUp()  && m_pqueue != NULL )
			m_pqueue->Fire( new ScriptMsgMake(ScriptMsgMake::msgmakeDestroy, m_pdlg) );

		m_pdlg = NULL;
	}

	// -1 for failure, 1 for success
	m_nDestroyCode = nDestroyCode;
}



bool ScriptMAKECommand::IsUp()
{
	if ( m_pdlg )
	{
		if ( m_pdlg->IsUp() )
			m_nDestroyCode = 0;
		else
			Kill(m_pdlg->IsSuccess() ? 1 : -1);
	}

	return m_nDestroyCode == 0;
}


bool ScriptMAKECommand::IsSuccess()
{
	IsUp();
	return m_nDestroyCode == 1;
}


void ScriptMAKECommand::SetQueue(ScriptExecMsgQueue * pqueue)
{
	m_pqueue = pqueue;
}



bool ScriptMAKECommand::Execute(CString * pstrError)
{
	CString strFullText;
	bool fResult = true;

	strFullText.Format("TEST:  %s\nMAKE: %s\n_____________________________________________\n\n%s\n_____________________________________________\nPress OK when complete.",
						(LPCSTR) *m_pstrTestTitle, baseLabel, m_strText);

	// should only be called once... but just go with the flow...
	
	ASSERT(m_pdlg == NULL);
	if ( m_pdlg != NULL )
		return false;

	// called with string error pointer if we should do modal...
	if ( m_fHanging  &&  m_pqueue != NULL )
	{
		m_pdlg = new CScriptMakeDlg(true, strFullText, m_pParent );
		m_pqueue->Fire( new ScriptMsgMake(ScriptMsgMake::msgmakeCreate, m_pdlg) );
	}
	else
	{
		CScriptMakeDlg	dlg(true, strFullText, m_pParent );
		if ( dlg.DoModal() != IDOK )
		{
			if ( pstrError != NULL )
				pstrError->Format("MAKE \"%s\" failed", baseLabel);
			fResult = false;
		}
	}

	return fResult;
}
