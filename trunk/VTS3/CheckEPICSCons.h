#if !defined(AFX_CHECKEPICSCONS_H__383E775E_16D6_4CAB_8B9F_9BD86A2959C7__INCLUDED_)
#define AFX_CHECKEPICSCONS_H__383E775E_16D6_4CAB_8B9F_9BD86A2959C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CheckEPICSCons.h : header file
////Added by Liangping Xu,2002-11

/////////////////////////////////////////////////////////////////////////////
// CCheckEPICSCons dialog

class CCheckEPICSCons : public CDialog
{
// Construction
public:
	CCheckEPICSCons(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCheckEPICSCons)
	enum { IDD = IDD_LOADEPICSCONSCHK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckEPICSCons)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCheckEPICSCons)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHECKEPICSCONS_H__383E775E_16D6_4CAB_8B9F_9BD86A2959C7__INCLUDED_)
