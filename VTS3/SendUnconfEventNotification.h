#if !defined(AFX_SENDUNCONFEVENTNOTIFICATION_H__74A41751_89FC_11D4_BEDD_00A0C95A9812__INCLUDED_)
#define AFX_SENDUNCONFEVENTNOTIFICATION_H__74A41751_89FC_11D4_BEDD_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendUnconfEventNotification.h : header file
//

#include "SendPage.h"
#include "VTSCtrl.h"
#include "VTSTimeStamp.h"

#include "VTSNotificationParameters.h"

/////////////////////////////////////////////////////////////////////////////
// CSendUnconfEventNotification dialog

class CSendUnconfEventNotification : public CSendPage
{
	DECLARE_DYNCREATE(CSendUnconfEventNotification)

// Construction
public:
	CSendUnconfEventNotification( void );   // non-standard constructor

	VTSUnsignedCtrl				m_ProcessID;
	VTSObjectIdentifierCtrl		m_DeviceID;
	VTSObjectIdentifierCtrl		m_ObjectID;
	VTSTimeStamp				m_TimeStamp;
	VTSUnsignedCtrl				m_NotifyClass;
	VTSUnsignedCtrl				m_Priority;
	VTSCharacterStringCtrl		m_MessageText;
	VTSEnumeratedCtrl			m_NotifyTypeCombo;
	VTSBooleanCtrl				m_AckRequired;
	VTSEnumeratedCtrl			m_FromStateCombo;
	VTSEnumeratedCtrl			m_ToStateCombo;
	VTSNotificationParameters	m_EventValues;

	void InitPage( void );						// give it a chance to init
	void EncodePage( CByteArray* contents );	// encode the page

	static BACnetAPDUEncoder	pageContents;

	void SavePage( void );						// save contents
	void RestorePage( void );					// restore contents to last saved values

// Dialog Data
	//{{AFX_DATA(CSendUnconfEventNotification)
	enum { IDD = IDD_SENDUNCONFEVENTNOTIFICATION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendUnconfEventNotification)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSendUnconfEventNotification)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeProcessID();
	afx_msg void OnChangeDeviceID();
	afx_msg void OnChangeObjectID();
	afx_msg void OnTimeStamp();
	afx_msg void OnChangeNotifyClass();
	afx_msg void OnChangePriority();
	afx_msg void OnChangeMessageText();
	afx_msg void OnSelchangeNotifyTypeCombo();
	afx_msg void OnAckRequired();
	afx_msg void OnSelchangeFromStateCombo();
	afx_msg void OnSelchangeToStateCombo();
	afx_msg void OnEventValues();
	afx_msg void OnDeviceIDButton();
	afx_msg void OnObjectIDButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDUNCONFEVENTNOTIFICATION_H__74A41751_89FC_11D4_BEDD_00A0C95A9812__INCLUDED_)
