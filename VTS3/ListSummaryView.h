#if !defined(AFX_LISTSUMMARYVIEW_H__B2A66D8C_50B2_4533_A960_50C0FECFD5F7__INCLUDED_)
#define AFX_LISTSUMMARYVIEW_H__B2A66D8C_50B2_4533_A960_50C0FECFD5F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListSummaryView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CListSummaryView view
#include "FrameContext.h"
#include "ListSummaryCache.h"

#include <afxcview.h>

class CListSummaryView : public CListView,public CFrameContextListener
{
protected:
	CListSummaryView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CListSummaryView)

// Attributes
public:
	CWnd* m_pTabRing;
// Operations
public:
	void ContextChange( CFrameContext::Signal s );

// No need for method anymore
//	CString* GetLineData(int lineNo);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListSummaryView)
	public:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CListSummaryView();

private:

	BACnetPIInfo		m_summary;		// so it doesn't have to be allocated and deallocated
	CListSummaryCache	m_cache;

	void SetSelectedLine(int currentLineNo);
	void AddLine(int lineNo);
	char * FillColumnData( int nColumn, char * pszFill, VTSPacket * ppkt );
	void CacheItem(int nItem, LVCachedItem * pcacheditem );

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CListSummaryView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnItemchanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTSUMMARYVIEW_H__B2A66D8C_50B2_4533_A960_50C0FECFD5F7__INCLUDED_)
