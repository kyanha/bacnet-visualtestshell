//******************************************************************
// 	Author:			Yajun Zhou
//	Date:			2002-4-22
//******************************************************************
#if !defined(AFX_SCRIPTEDIT_H__530C4E4B_3567_4332_81F6_84274DFEA53C__INCLUDED_)
#define AFX_SCRIPTEDIT_H__530C4E4B_3567_4332_81F6_84274DFEA53C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptEdit.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// ScriptEdit view
#include "LineNumCtrl.h"

class ScriptEdit : public CEditView
{
protected:
	ScriptEdit();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(ScriptEdit)

// Attributes
public:
	
// Operations
public:
	void SetLine(int nLineIndex,LPTSTR lpszString);
	int GetCurLineIndex();
	void GotoLine(int nLineIndex);
	void SetDefaultFont();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ScriptEdit)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~ScriptEdit();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	int m_nTempDigit;
	void ScrollCurLnVisible(UINT nChar);
	BOOL IsCurLnVisible();
	void UpdateRect(int nCurrentY);
	void UpdateTwoLine();
	void UpdateOneLine(int nCurrentY);
	void UpdateWholeEdit();
	void DisplayLnNum();
	//{{AFX_MSG(ScriptEdit)
	afx_msg void OnSetfocus();
	afx_msg void OnEditGotoline();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChange();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHscroll();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVscroll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CLineNumCtrl m_LineNumCtrl;
	CPoint m_ptCurPoint;
	CEdit* m_pEdit;
	int m_nMargin;
	int m_nClientX;
	int m_nClientY;

	int m_nCurrentLine;
	int m_nFirstVisibleLn;
	int m_nVisibleLnCount;
	int m_nLineCount;
	
	bool m_bDelete;
	bool m_bInput;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTEDIT_H__530C4E4B_3567_4332_81F6_84274DFEA53C__INCLUDED_)
