#if !defined(AFX_SENDIHAVE_H__009107C1_57B6_11D4_BEC1_00A0C95A9812__INCLUDED_)
#define AFX_SENDIHAVE_H__009107C1_57B6_11D4_BEC1_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendIHave.h : header file
//

#include "SendPage.h"
#include "VTSCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CSendIHave dialog

class CSendIHave : public CSendPage
{
	DECLARE_DYNCREATE(CSendIHave)

// Construction
public:
	CSendIHave( void );   // non-standard constructor

	VTSObjectIdentifierCtrl		m_DeviceID;
	VTSObjectIdentifierCtrl		m_ObjectID;
	VTSCharacterStringCtrl		m_ObjectName;

	void InitPage( void );						// give it a chance to init
	void EncodePage( CByteArray* contents );	// encode the page

	static BACnetAPDUEncoder	pageContents;

	void SavePage( void );						// save contents
	void RestorePage( void );					// restore contents to last saved values

// Dialog Data
	//{{AFX_DATA(CSendIHave)
	enum { IDD = IDD_SENDIHAVE };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSendIHave)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSendIHave)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeDeviceID();
	afx_msg void OnChangeObjectID();
	afx_msg void OnChangeObjectName();
	afx_msg void OnDeviceIDButton();
	afx_msg void OnObjectIDButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDIHAVE_H__009107C1_57B6_11D4_BEC1_00A0C95A9812__INCLUDED_)