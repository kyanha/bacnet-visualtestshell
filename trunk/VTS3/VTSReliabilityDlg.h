#if !defined(AFX_VTSRELIABILITYDLG_H__F1302C48_8255_4E6D_A8A7_79E256B31C2D__INCLUDED_)
#define AFX_VTSRELIABILITYDLG_H__F1302C48_8255_4E6D_A8A7_79E256B31C2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VTSReliabilityDlg.h : header file
//
#include "VTSCtrl.h"
/////////////////////////////////////////////////////////////////////////////
// VTSReliabilityDlg dialog

class VTSReliabilityDlg : public CDialog
{
// Construction
public:
	VTSReliabilityDlg(CWnd* pParent = NULL);   // standard constructor
	void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );
	void Decode( BACnetAPDUDecoder& dec );
	VTSEnumeratedCtrl	m_enumcombo;
// Dialog Data
	//{{AFX_DATA(VTSReliabilityDlg)
	enum { IDD = IDD_RELIABILITY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSReliabilityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VTSReliabilityDlg)
	afx_msg void OnSelchangeEnumratecombo();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTSRELIABILITYDLG_H__F1302C48_8255_4E6D_A8A7_79E256B31C2D__INCLUDED_)
