// ReadAllPropSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "ReadAllPropSettingsDlg.h"

#include "VTSDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReadAllPropSettingsDlg dialog


CReadAllPropSettingsDlg::CReadAllPropSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReadAllPropSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReadAllPropSettingsDlg)
	m_strIUTIP = _T("");
	m_strNetwork = _T("");
	//}}AFX_DATA_INIT
}


void CReadAllPropSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReadAllPropSettingsDlg)
	DDX_Text(pDX, IDC_EDIT_IUTIP, m_strIUTIP);
	DDX_CBString(pDX, IDC_COMBO_NETWORK, m_strNetwork);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReadAllPropSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CReadAllPropSettingsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadAllPropSettingsDlg message handlers
//MAD_DB extern VTSPortList gMasterPortList;

BOOL CReadAllPropSettingsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CComboBox* pComboBox =(CComboBox*)GetDlgItem(IDC_COMBO_NETWORK);

//	VTSPortPtr	curPort;
	VTSDoc * pdoc = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();
	VTSPorts * pports = pdoc == NULL ? NULL : pdoc->GetPorts();

//MAD_DB	for (int i = 0; i < gMasterPortList.Length(); i++)
	for (int i = 0; pports != NULL && i < pports->GetSize(); i++)
	{
		// get a pointer to the ith port
//		curPort = gMasterPortList[i];
//		pComboBox->AddString( curPort->portDesc.portName );
		pComboBox->AddString( (*pports)[i]->GetName() );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CReadAllPropSettingsDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData();

	if(m_strNetwork == "")
	{
		AfxMessageBox("You must choose a VTSPort!");
		return;
	}
	if(m_strIUTIP == "")
	{
		AfxMessageBox("You must specified the IUTIP!");
		return;
	}
	
	CDialog::OnOK();
}
