#if !defined(AFX_SCRIPTEXECMSG_H__9221BBDD_15A7_400A_84B6_FA0B3269318D__INCLUDED_)
#define AFX_SCRIPTEXECMSG_H__9221BBDD_15A7_400A_84B6_FA0B3269318D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptExecMsg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ScriptExecMsg window

class ScriptExecMsg : public CObject
{
	public:
		enum ScriptMsgType
			{
				msgStatus,
				msgMakeDlg
			};

		ScriptMsgType	m_msgtype;

		ScriptExecMsg( ScriptMsgType msgtype );
		virtual ~ScriptExecMsg();

		ScriptMsgType GetType(void);
};

/////////////////////////////////////////////////////////////////////////////


#endif // !defined(AFX_SCRIPTEXECMSG_H__9221BBDD_15A7_400A_84B6_FA0B3269318D__INCLUDED_)
