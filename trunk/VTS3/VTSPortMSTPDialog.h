// VTSPortMSTPDialog.h: interface for the VTSPortMSTPDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VTSPORTMSTPDIALOG_H__6339FBE7_7A94_47B2_9497_FC2389E80DB2__INCLUDED_)
#define AFX_VTSPORTMSTPDIALOG_H__6339FBE7_7A94_47B2_9497_FC2389E80DB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VTSPropertyPage.h"


class VTSPortNullDialog : public VTSPropertyPage
{
	DECLARE_DYNCREATE(VTSPortNullDialog)

public:
	VTSPortNullDialog( VTSPageCallbackInterface * pparent );
	VTSPortNullDialog() {};
	
// Dialog Data
	//{{AFX_DATA(VTSPortNullDialog)
	enum { IDD = IDD_PORTPAGE_NULL };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSPortNullDialog)
	public:
	virtual BOOL OnSetActive();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VTSPortNullDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



class VTSPortMSTPDialog : public VTSPropertyPage
{
	DECLARE_DYNCREATE(VTSPortMSTPDialog)

	CString		* m_pstrConfigParms;

public:
	VTSPortMSTPDialog( VTSPageCallbackInterface * pparent );
	VTSPortMSTPDialog();
	~VTSPortMSTPDialog();
	
// Dialog Data
	//{{AFX_DATA(VTSPortMSTPDialog)
	enum { IDD = IDD_PORTPAGE_MSTP };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSPortMSTPDialog)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VTSPortMSTPDialog)
	//}}AFX_MSG

	void ObjToCtrl(void);
	void CtrlToObj(void);

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_VTSPORTMSTPDIALOG_H__6339FBE7_7A94_47B2_9497_FC2389E80DB2__INCLUDED_)
