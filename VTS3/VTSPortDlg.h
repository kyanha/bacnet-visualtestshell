#if !defined(AFX_VTSPORTDLG_H__DF2DB570_3513_11D4_BEA5_00A0C95A9812__INCLUDED_)
#define AFX_VTSPORTDLG_H__DF2DB570_3513_11D4_BEA5_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VTSPortDlg.h : header file
//

#include "VTSDoc.h"

/////////////////////////////////////////////////////////////////////////////
// VTSPortDlg dialog

class VTSPortDlg : public CDialog
{
// Construction
public:
	VTSPortDlg(CWnd* pParent = NULL);   // standard constructor (not used)
	VTSPortDlg( VTSPortListPtr plp, VTSDeviceListPtr dlp );		// ctor

	CString		m_Config;				// transfer config string

	void PortStatusChange( void );						// status of some port has changed

// Dialog Data
	//{{AFX_DATA(VTSPortDlg)
	enum { IDD = IDD_PORT };
	CListCtrl	m_PortList;
	CString		m_Name;
	BOOL		m_Enabled;
	int			m_Type;
	CString		m_Network;
	CComboBox	m_DeviceCombo;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSPortDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	VTSPortListPtr		m_pPortList;					// pointer to list of ports
	VTSDeviceListPtr	m_pDeviceList;					// pointer to list of devices
	int					m_iSelectedPort;				// index of selected port
	CImageList			m_ilStatus;						// status images

	void SetSelection( int indx );
	void ResetSelection( void );
	void SynchronizeControls( void );

	afx_msg void SaveChanges();							// save changes in port desc

	// Generated message map functions
	//{{AFX_MSG(VTSPortDlg)
	afx_msg void OnNewPort();
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangingPortList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkPortList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnConfig();
	afx_msg void OnColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTSPORTDLG_H__DF2DB570_3513_11D4_BEA5_00A0C95A9812__INCLUDED_)
