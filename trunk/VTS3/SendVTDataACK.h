#if !defined(AFX_SENDVTDATAACK_H__F3927661_77E9_11D4_BED7_00A0C95A9812__INCLUDED_)
#define AFX_SENDVTDATAACK_H__F3927661_77E9_11D4_BED7_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendVTDataACK.h : header file
//

#include "SendPage.h"
#include "VTSCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CSendVTDataACK dialog

class CSendVTDataACK : public CSendPage
{
	DECLARE_DYNCREATE(CSendVTDataACK)

// Construction
public:
	CSendVTDataACK( void );   // non-standard constructor

	VTSBooleanCtrl				m_AllDataFlag;
	VTSUnsignedCtrl				m_AcceptedCount;

	void InitPage( void );						// give it a chance to init
	void EncodePage( CByteArray* contents );	// encode the page

	static BACnetAPDUEncoder	pageContents;

	void SavePage( void );						// save contents
	void RestorePage( int index = 0 );					// restore contents to last saved values

// Dialog Data
	//{{AFX_DATA(CSendVTDataACK)
	enum { IDD = IDD_SENDVTDATAACK };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendVTDataACK)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSendVTDataACK)
	virtual BOOL OnInitDialog();
	afx_msg void OnAllDataFlag();
	afx_msg void OnChangeAcceptedCount();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDVTDATAACK_H__F3927661_77E9_11D4_BED7_00A0C95A9812__INCLUDED_)
