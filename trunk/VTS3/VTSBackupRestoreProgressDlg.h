#include "afxcmn.h"
#if !defined(AFX_VTSBACKUPRESTOREPROGRESSDLG_H__912FB78F_CA73_47CC_8157_3F9E32F26F6D__INCLUDED_)
#define AFX_VTSBACKUPRESTOREPROGRESSDLG_H__912FB78F_CA73_47CC_8157_3F9E32F26F6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VTSBackupRestoreProgressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VTSBackupRestoreProgressDlg dialog

class VTSBackupRestoreProgressDlg : public CDialog
{
// Construction
public:
	VTSBackupRestoreProgressDlg(CWnd* pParent = NULL);   // standard constructor
    
	void OutMessage(const CString& msg, BOOL bnewLine = TRUE);
	void BeginTestProcess(void);
	void EndTestProcess(void);
// Dialog Data
	//{{AFX_DATA(VTSBackupRestoreProgressDlg)
	enum { IDD = IDD_BACKUP_RESTORE_PROGRESS };
	CButton	m_okCtrl;
	CButton	m_killCtrl;
	CRichEditCtrl	m_editOutput;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSBackupRestoreProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL
	virtual void OnOK();
	virtual void OnCancel();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VTSBackupRestoreProgressDlg)
	afx_msg void OnBackupRestoreKill();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTSBACKUPRESTOREPROGRESSDLG_H__912FB78F_CA73_47CC_8157_3F9E32F26F6D__INCLUDED_)
