#if !defined(AFX_VTSPORTIPDIALOG_H__9CB63CA1_8D00_11D4_BEDD_00A0C95A9812__INCLUDED_)
#define AFX_VTSPORTIPDIALOG_H__9CB63CA1_8D00_11D4_BEDD_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VTSPortIPDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VTSPortIPDialog dialog

class VTSPortIPDialog : public CDialog
{
// Construction
public:
	VTSPortIPDialog(CString *cp, CWnd* pParent = NULL);   // non-standard constructor

	CString		*m_Config;

	virtual void OnOK();

// Dialog Data
	//{{AFX_DATA(VTSPortIPDialog)
	enum { IDD = IDD_PORT_IP };
	CString	m_Socket;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSPortIPDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VTSPortIPDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTSPORTIPDIALOG_H__9CB63CA1_8D00_11D4_BEDD_00A0C95A9812__INCLUDED_)
