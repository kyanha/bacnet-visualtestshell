#if !defined(AFX_SENDCHANGELISTERROR_H__468353B3_5606_11D4_BEBF_00A0C95A9812__INCLUDED_)
#define AFX_SENDCHANGELISTERROR_H__468353B3_5606_11D4_BEBF_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendChangeListError.h : header file
//

#include "SendPage.h"
#include "VTSCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CSendChangeListError dialog

class CSendChangeListError : public CSendPage
{
	DECLARE_DYNCREATE(CSendChangeListError)

// Construction
public:
	CSendChangeListError( void );   // non-standard constructor

	VTSIntegerCtrl			m_InvokeID;
	VTSEnumeratedCtrl		m_ServiceCombo;
	VTSEnumeratedCtrl		m_ErrorClassCombo;
	VTSEnumeratedCtrl		m_ErrorCodeCombo;
	VTSUnsignedCtrl			m_Element;

	void InitPage( void );						// give it a chance to init
	void EncodePage( CByteArray* contents );	// encode the page

	static BACnetAPDUEncoder	pageContents;

	void SavePage( void );						// save contents
	void RestorePage( void );					// restore contents to last saved values

// Dialog Data
	//{{AFX_DATA(CSendChangeListError)
	enum { IDD = IDD_SENDCHANGELISTERROR };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSendChangeListError)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSendChangeListError)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeInvokeID();
	afx_msg void OnSelchangeServiceCombo();
	afx_msg void OnSelchangeErrorClassCombo();
	afx_msg void OnSelchangeErrorCodeCombo();
	afx_msg void OnChangeElement();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDCHANGELISTERROR_H__468353B3_5606_11D4_BEBF_00A0C95A9812__INCLUDED_)