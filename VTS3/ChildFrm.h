// ChildFrm.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDFRM_H__BDE65086_B82F_11D3_BE52_00A0C95A9812__INCLUDED_)
#define AFX_CHILDFRM_H__BDE65086_B82F_11D3_BE52_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FrameContext.h"
/////////////////////////////////
#include "DockingDetailViewBar.h"
#include "DockingHexViewBar.h"
/////////////////////////////////
// forward definitions
class CSummaryView;
class CDetailView;
class CHexView;


class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Attributes
public:
	CSplitterWnd	m_wndSplit1;
	CSplitterWnd	m_wndSplit2;        // embedded in the first
	CFrameContext*	m_frameContext;		// frame context

	CSummaryView*	m_pSummaryView;
	CDetailView*	m_pDetailView;
	CHexView*		m_pHexView;
	CDockingDetailViewBar* m_pwndDetailViewBar;
	CDockingHexViewBar* m_pwndHexViewBar;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CChildFrame)
	afx_msg void OnCancelMode();
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPorts(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditNames(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDevices(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFirstFrame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewLastFrame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewNextFrame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewPrevFrame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSend(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnEditPorts();
	afx_msg void OnEditNames();
	afx_msg void OnEditDevices();
	afx_msg void OnEditPreferences();
	afx_msg void OnViewFirstFrame();
	afx_msg void OnViewPrevFrame();
	afx_msg void OnViewNextFrame();
	afx_msg void OnViewLastFrame();
	afx_msg void OnSendNewPacket();
	afx_msg void OnSendSelectPort(UINT uID);
	afx_msg void OnSendSelectPacket(UINT uID);
	//add by Hu Meng responding to new menu items
	afx_msg void OnViewDetail();
	afx_msg void OnViewHex();
	afx_msg void OnUpdateViewDetail(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewHex(CCmdUI* pCmdUI);
	//end 6.24
	// Added by Yajun Zhou, 2002-7-24
	afx_msg void OnFileExport();
	/////////////////////////////////
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__BDE65086_B82F_11D3_BE52_00A0C95A9812__INCLUDED_)
