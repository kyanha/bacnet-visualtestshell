// ScriptMsgMake.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "ScriptMsgMake.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////


ScriptMsgMake::ScriptMsgMake(ScriptMsgMakeType nType, CScriptMakeDlg * pdlg)
			  :ScriptExecMsg(ScriptExecMsg::msgMakeDlg), m_maketype(nType), m_pdlg(pdlg)
{
}

ScriptMsgMake::~ScriptMsgMake()
{
}



