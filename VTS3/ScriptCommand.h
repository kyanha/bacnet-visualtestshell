// ScriptCommand.h: interface for the ScriptCHECKCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTCOMMAND_H__99CE0EF3_EAE9_42D2_AD4C_E0B3F2D056C7__INCLUDED_)
#define AFX_SCRIPTCOMMAND_H__99CE0EF3_EAE9_42D2_AD4C_E0B3F2D056C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ScriptBase.h"
#include "ScriptExecMsg.h"
#include "VTSQueue.h"

class ScriptIfdefHandler;
class CScriptMakeDlg;

//class ScriptScanner;
//class ScriptToken;


class ScriptMessage : public ScriptCommand
{
	protected:
		CString * m_pstrTestTitle;
		CString	m_strText;

	private:
		void Parse( ScriptIfdefHandler & ifdefHandler, ScriptScanner & scan, ScriptToken & tok );

	public:
		ScriptMessage( ScriptIfdefHandler & ifdefHandler, ScriptScanner & scan, ScriptToken & tok, CString * pstrTest );
};


class ScriptCHECKCommand : public ScriptMessage
{
	public:
		ScriptCHECKCommand( ScriptIfdefHandler & ifdefHandler, ScriptScanner & scan, ScriptToken & tok, CString * pstrTest );
		virtual ~ScriptCHECKCommand();

		bool Execute(CString * pstrError);
};



class ScriptMAKECommand : public ScriptMessage
{
	private:
		CScriptMakeDlg		* m_pdlg;
		int					m_nDestroyCode;
		ScriptExecMsgQueue * m_pqueue;

	public:

		bool m_fHanging;			// true if this should be modeless... 

		ScriptMAKECommand( ScriptIfdefHandler & ifdefHandler, ScriptScanner & scan, ScriptToken & tok, CString * pstrTest );
		virtual ~ScriptMAKECommand();

		void SetQueue(ScriptExecMsgQueue * pqueue);

		bool Execute(CString * pstrError);
		void Kill(int nDestroyCode = 1);
		bool IsUp(void);
		bool IsSuccess(void);
};


#endif // !defined(AFX_SCRIPTCOMMAND_H__99CE0EF3_EAE9_42D2_AD4C_E0B3F2D056C7__INCLUDED_)
