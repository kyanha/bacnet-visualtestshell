#if !defined(AFX_SENDUTCTIMESYNC_H__2CE2CDC0_81EA_11D6_86F1_0050DA21507F__INCLUDED_)
#define AFX_SENDUTCTIMESYNC_H__2CE2CDC0_81EA_11D6_86F1_0050DA21507F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SendPage.h" 
#include "VTSCtrl.h"


/////////////////////////////////////////////////////////////////////////////
// CSendUTCTimeSync dialog
class CSendUTCTimeSync : public CSendPage
//class CSendUTCTimeSync : public CPropertyPage
{
	DECLARE_DYNCREATE(CSendUTCTimeSync)

// Construction
public:
	CSendUTCTimeSync(void);
	VTSDateCtrl				m_Date;
	VTSTimeCtrl				m_Time;
	VTSIntegerCtrl			m_UTC_offset;

	void SynchronizeControls( void );

	void InitPage( void );						// get read for input
	void EncodePage( CByteArray* contents );	// encode the page into contents

	static BACnetAPDUEncoder	pageContents;

	void SavePage( void );						// save contents
	void RestorePage( int index = 0 );					// restore contents to last saved values
// Dialog Data
	//{{AFX_DATA(CSendUTCTimeSync)
	enum { IDD = IDD_SENDUTCTIMESYNC };
	BOOL	m_AutoSync;
	BOOL	m_DaylightSavingsStatus;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSendUTCTimeSync)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSendUTCTimeSync)
	afx_msg void OnAutosynctime();
	afx_msg void OnDaylightsavings();
	afx_msg void OnChangeUtcdate();
	afx_msg void OnChangeUtcoffset();
	afx_msg void OnChangeUtctime();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDUTCTIMESYNC_H__2CE2CDC0_81EA_11D6_86F1_0050DA21507F__INCLUDED_)
