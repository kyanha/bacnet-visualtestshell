#if !defined(AFX_VTSPORTETHERNETDIALOG_H__9CB63CA2_8D00_11D4_BEDD_00A0C95A9812__INCLUDED_)
#define AFX_VTSPORTETHERNETDIALOG_H__9CB63CA2_8D00_11D4_BEDD_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VTSPortEthernetDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VTSPortEthernetDialog dialog

class VTSPortEthernetDialog : public CDialog
{
// Construction
public:
	VTSPortEthernetDialog(CString *cp, CWnd* pParent = NULL);   // standard constructor

	CString		*m_Config;

	virtual void OnOK();

// Dialog Data
	//{{AFX_DATA(VTSPortEthernetDialog)
	enum { IDD = IDD_PORT_ENET };
	CComboBox	m_AdapterCombo;
	CButton		m_Promiscuous;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSPortEthernetDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VTSPortEthernetDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTSPORTETHERNETDIALOG_H__9CB63CA2_8D00_11D4_BEDD_00A0C95A9812__INCLUDED_)
