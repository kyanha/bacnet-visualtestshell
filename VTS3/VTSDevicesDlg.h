#if !defined(AFX_VTSDEVICESDLG_H__86B18060_B765_11D5_8886_00A0C9E370F1__INCLUDED_)
#define AFX_VTSDEVICESDLG_H__86B18060_B765_11D5_8886_00A0C9E370F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VTSDevicesDlg.h : header file
//

#include "VTSDoc.h"

/////////////////////////////////////////////////////////////////////////////
// VTSDevicesDlg dialog

class VTSDevicesDlg : public CDialog
{
// Construction
public:
	VTSDevicesDlg(CWnd* pParent = NULL);	// standard constructor (not used)
	VTSDevicesDlg( VTSDeviceListPtr dlp );	// ctor

	void InitDeviceList( void );		// init the device list

	void SetSelection( int indx );
	void ResetSelection( void );		// nothing selected
	void SynchronizeControls( void );	// sync list and controls

	void SaveChanges( void );			// new values to update selected name

	void DeviceToList( VTSDevicePtr dp, int elem );
	void DeviceToCtrl( VTSDevicePtr dp );
	void CtrlToDevice( VTSDevicePtr dp );

// Dialog Data
	//{{AFX_DATA(VTSDevicesDlg)
	enum { IDD = IDD_DEVICES };
	CListCtrl	m_DeviceList;
	CString		m_Name;
	CString		m_Instance;
	CString		m_SegSize;
	CString		m_WindowSize;
	CString		m_NextInvokeID;
	CString		m_MaxAPDUSize;
	CString		m_APDUTimeout;
	CString		m_APDUSegTimeout;
	CString		m_APDURetries;
	CString		m_VendorID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSDevicesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	VTSDeviceListPtr	m_pDeviceList;					// pointer to list of Devices
	int					m_iSelectedDevice;				// index of selected name

	// Generated message map functions
	//{{AFX_MSG(VTSDevicesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNew();
	afx_msg void OnItemchangingDevicelist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkDevicelist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnIAm();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTSDEVICESDLG_H__86B18060_B765_11D5_8886_00A0C9E370F1__INCLUDED_)
