#if !defined(AFX_EPICSVIEWINFOPANEL_H__BC3848AD_A732_4CFC_8AB8_7EA337121943__INCLUDED_)
#define AFX_EPICSVIEWINFOPANEL_H__BC3848AD_A732_4CFC_8AB8_7EA337121943__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EPICSViewInfoPanel.h : header file
//

#include <afxrich.h>

/////////////////////////////////////////////////////////////////////////////
// CEPICSViewInfoPanel window

class CEPICSViewInfoPanel : public CRichEditView
{
// Construction
public:
	CEPICSViewInfoPanel();
	DECLARE_DYNCREATE(CEPICSViewInfoPanel)

// Attributes
public:


// Operations
public:

	void ResetPanel();
	void AddText(LPCSTR lpsz);
	void AddLine(LPCSTR lpsz);
	void SetParagraphStyle(WORD wNumbering, LONG lIndent, LONG lOffset, WORD wAlignment);
	void SetCharStyle(int nPoints, COLORREF clr = 0xFFFFFFFF );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEPICSViewInfoPanel)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEPICSViewInfoPanel();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEPICSViewInfoPanel)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EPICSVIEWINFOPANEL_H__BC3848AD_A732_4CFC_8AB8_7EA337121943__INCLUDED_)
