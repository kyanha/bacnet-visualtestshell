// VTSLifeSafetyModeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "VTSLifeSafetyModeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
namespace NetworkSniffer {
	extern char *BACnetLifeSafetyMode[];
}
/////////////////////////////////////////////////////////////////////////////
// VTSLifeSafetyModeDlg dialog


VTSLifeSafetyModeDlg::VTSLifeSafetyModeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(VTSLifeSafetyModeDlg::IDD, pParent)
	, m_enumcombo( this, IDC_ENUMRATECOMBO, NetworkSniffer::BACnetLifeSafetyMode, 15, true )	
{
	//{{AFX_DATA_INIT(VTSLifeSafetyModeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void VTSLifeSafetyModeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSLifeSafetyModeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSLifeSafetyModeDlg, CDialog)
	//{{AFX_MSG_MAP(VTSLifeSafetyModeDlg)
	ON_CBN_SELCHANGE(IDC_ENUMRATECOMBO, OnSelchangeEnumratecombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSLifeSafetyModeDlg message handlers

void VTSLifeSafetyModeDlg::OnSelchangeEnumratecombo() 
{	
	m_enumcombo.ctrlNull = false;
	m_enumcombo.CtrlToObj();
		
}
void VTSLifeSafetyModeDlg::Encode(BACnetAPDUEncoder& enc, int context)
{
	m_enumcombo.Encode(enc, context);
}
void VTSLifeSafetyModeDlg::Decode(BACnetAPDUDecoder& dec )
{	
	if(dec.pktLength!=0)
	m_enumcombo.Decode(dec);
}
BOOL VTSLifeSafetyModeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_enumcombo.LoadCombo();
	m_enumcombo.ObjToCtrl();
	
	return TRUE;  
}