// VTSVendorPropIDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "VTSVendorPropIDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////
//Added by Zhu Zhenhua 2003-7-22, Input Dlg for Vendor Property


/////////////////////////////////////////////////////////////////////////////
// VTSVendorPropIDDlg dialog


VTSVendorPropIDDlg::VTSVendorPropIDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(VTSVendorPropIDDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(VTSVendorPropIDDlg)
	m_nEditPropID = 512;
	//}}AFX_DATA_INIT
	m_PropID = -1;
}


void VTSVendorPropIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSVendorPropIDDlg)
	DDX_Text(pDX, IDC_VENDOR_PROPID, m_nEditPropID);
	DDV_MinMaxInt(pDX, m_nEditPropID, 512, 4194303);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSVendorPropIDDlg, CDialog)
	//{{AFX_MSG_MAP(VTSVendorPropIDDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSVendorPropIDDlg message handlers

void VTSVendorPropIDDlg::OnOK() 
{
	// TODO: Add extra validation here
	if(!UpdateData(TRUE))
		return;
	m_PropID = m_nEditPropID;
	CDialog::OnOK();
}

BOOL VTSVendorPropIDDlg::OnInitDialog() 
{	
	if(m_PropID != -1)
	m_nEditPropID = m_PropID;
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
