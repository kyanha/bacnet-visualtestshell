// ScriptFrame.h : interface of the ScriptFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTFRAME_H__C9E6602B_ABC2_11D4_BEE8_00A0C95A9812__INCLUDED_)
#define AFX_SCRIPTFRAME_H__C9E6602B_ABC2_11D4_BEE8_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxext.h>
#include <afxcview.h>

#include "ScriptDocument.h"
#include "ScriptParmList.h"
#include "ScriptContentTree.h"
#include "ScriptLoadResults.h"

class ScriptFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ScriptFrame)
public:
	ScriptFrame();

	ScriptDocumentPtr	m_pDoc;

	CSplitterWnd		m_wndSplit1;
	CSplitterWnd		m_wndSplit2;

	CEditView			*m_pEditView;
	ScriptParmList		*m_pParmList;
	ScriptContentTree	*m_pContentTree;

	bool	DoInitialSetup( void );			// get ready to run

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ScriptFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void Serialize(CArchive& ar);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ScriptFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(ScriptFrame)
	afx_msg void OnUpdateScriptCheckSyntax(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScriptLoadEPICS(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScriptRun(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScriptHalt(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScriptStep(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScriptStepPass(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScriptStepFail(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScriptKill(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScriptReset(CCmdUI* pCmdUI);
	afx_msg void OnScriptCheckSyntax();
	afx_msg void OnScriptLoadEPICS();
	afx_msg void OnScriptRun();
	afx_msg void OnScriptHalt();
	afx_msg void OnScriptStep();
	afx_msg void OnScriptStepPass();
	afx_msg void OnScriptStepFail();
	afx_msg void OnScriptKill();
	afx_msg void OnScriptReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTFRAME_H__C9E6602B_ABC2_11D4_BEE8_00A0C95A9812__INCLUDED_)