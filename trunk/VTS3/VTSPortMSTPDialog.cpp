// VTSPortMSTPDialog.cpp: implementation of the VTSPortMSTPDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vts.h"
#include "VTSPortMSTPDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(VTSPortNullDialog, VTSPropertyPage)

VTSPortNullDialog::VTSPortNullDialog( VTSPageCallbackInterface * pparent )
                      :VTSPropertyPage(VTSPortNullDialog::IDD, pparent)
{
}


BEGIN_MESSAGE_MAP(VTSPortNullDialog, VTSPropertyPage)
	//{{AFX_MSG_MAP(VTSPortNullDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL VTSPortNullDialog::OnSetActive() 
{
	// need to call this to notify parent of selection change
	if ( RetrieveCurrentData() != NULL )
		NotifyOfDataChange();

	VTSPropertyPage::OnSetActive();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(VTSPortMSTPDialog, VTSPropertyPage)

VTSPortMSTPDialog::VTSPortMSTPDialog( VTSPageCallbackInterface * pparent )
                      :VTSPropertyPage(VTSPortMSTPDialog::IDD, pparent)
{
	//{{AFX_DATA_INIT(VTSPortMSTPDialog)
	//}}AFX_DATA_INIT

	m_pstrConfigParms = NULL;
}

VTSPortMSTPDialog::VTSPortMSTPDialog(void) : VTSPropertyPage()
{
	ASSERT(0);
}


VTSPortMSTPDialog::~VTSPortMSTPDialog()
{
}


void VTSPortMSTPDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSPortMSTPDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSPortMSTPDialog, VTSPropertyPage)
	//{{AFX_MSG_MAP(VTSPortMSTPDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSPortPTPDialog message handlers

BOOL VTSPortMSTPDialog::OnSetActive() 
{
	m_pstrConfigParms = (CString * ) RetrieveCurrentData();

	ObjToCtrl();

	// calls DDX
	VTSPropertyPage::OnSetActive();

	// if we return FALSE here then this page will not be activated.  It will move on to the next. 
	// That's good because it's extremely painful to disable all of the stuff.

	return FALSE; // change to this when implemented... m_pstrConfigParms != NULL;
}
	


BOOL VTSPortMSTPDialog::OnKillActive() 
{
	// calls DDX
	VTSPropertyPage::OnKillActive();
	CtrlToObj();

	return TRUE;
}



void VTSPortMSTPDialog::ObjToCtrl()
{
	if ( m_pstrConfigParms == NULL )
		return;

	// insert code here to push decoded object data into dialog variables
}


void VTSPortMSTPDialog::CtrlToObj()
{
	if ( m_pstrConfigParms == NULL )
		return;

	// insert code here to encode dialog data variables into string
}
