#if !defined(AFX_READALLPROPSETTINGSDLG_H__0E2523E1_24D9_4C5D_9C83_293E1FD40574__INCLUDED_)
#define AFX_READALLPROPSETTINGSDLG_H__0E2523E1_24D9_4C5D_9C83_293E1FD40574__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReadAllPropSettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReadAllPropSettingsDlg dialog

class CReadAllPropSettingsDlg : public CDialog
{
// Construction
public:
	CReadAllPropSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CReadAllPropSettingsDlg)
	enum { IDD = IDD_READALLPROPSETTING };
	CString	m_strIUTIP;
	CString	m_strNetwork;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReadAllPropSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReadAllPropSettingsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_READALLPROPSETTINGSDLG_H__0E2523E1_24D9_4C5D_9C83_293E1FD40574__INCLUDED_)
