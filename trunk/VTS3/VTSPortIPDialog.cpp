// VTSPortIPDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSPortIPDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VTSPortIPDialog dialog

VTSPortIPDialog::VTSPortIPDialog(CString *cp, CWnd* pParent /*=NULL*/)
	: CDialog(VTSPortIPDialog::IDD, pParent)
	, m_Config( cp )
{
	//{{AFX_DATA_INIT(VTSPortIPDialog)
	if (**cp == 0)
		m_Socket = "0xBAC0";
	else
		m_Socket = *cp;
	//}}AFX_DATA_INIT
}

void VTSPortIPDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSPortIPDialog)
	DDX_Text(pDX, IDC_SOCKET, m_Socket);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(VTSPortIPDialog, CDialog)
	//{{AFX_MSG_MAP(VTSPortIPDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	VTSPortIPDialog::OnOK
//
//	When the user accepts the configuration information and clicks OK, 
//	this function is called which transfers the new configuration into 
//	the config data provided by the caller, VTSPort::Configuration.
//

void VTSPortIPDialog::OnOK()
{
	// sync the vars with the controls
	UpdateData();

	// transfer
	*m_Config = m_Socket;

	// continue normal processing
	CDialog::OnOK();
}

//
//	VTSPortIPDialog::OnInitDialog
//

BOOL VTSPortIPDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	return TRUE;
}
