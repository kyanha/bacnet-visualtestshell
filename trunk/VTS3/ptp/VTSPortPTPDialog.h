#if !defined(AFX_VTSPORTPTPDIALOG_H__BF8F3289_760B_44F0_879F_1FCB264AB6E8__INCLUDED_)
#define AFX_VTSPORTPTPDIALOG_H__BF8F3289_760B_44F0_879F_1FCB264AB6E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VTSPortPTPDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VTSPortPTPDialog dialog
class VTSPortPTPDialog : public CDialog
{
// Construction
public:
	VTSPortPTPDialog(CString *cp, CWnd* pParent = NULL);   // standard constructor

	CString		*m_Config;
	
	CString m_portStr; 
	CString m_baudrateStr; 

// Dialog Data
	//{{AFX_DATA(VTSPortPTPDialog)
	enum { IDD = IDD_PORT_PTP };
	CComboBox	m_port;
	CComboBox	m_baudrate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSPortPTPDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VTSPortPTPDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTSPORTPTPDIALOG_H__BF8F3289_760B_44F0_879F_1FCB264AB6E8__INCLUDED_)
