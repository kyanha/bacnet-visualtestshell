// EditResentPktDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "EditResentPktDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditResentPktDlg dialog
BOOL IF_HEX_FORMAT(CString valueStr)
{
	int length = valueStr.GetLength();
	if(length & 1) //The length of hex string is odd
		return FALSE;

	for(int index = 0; index < length; index++)
	{
		if(!isxdigit(valueStr.GetAt(index)))
			return FALSE;
	}

	return TRUE;
}

BOOL HEXSTR_TO_OCTETSTRING(CString valueStr, BACnetOctetString&str)
{
	int length = valueStr.GetLength();
	
	if(length & 1) //The length of hex str is odd
		return FALSE;	

	str.Flush();	
	
	for(int index = 0; index < length; index += 2)
	{
		BACnetOctet temp;
		char c1 = valueStr.GetAt(index);
		char c2 = valueStr.GetAt(index + 1);
		if(!isxdigit(c1) || !isxdigit(c2))
			return FALSE;

		temp = (isdigit(c1) ? (c1 - '0') : (toupper(c1) - 'A' + 10));
		temp = (temp << 4) + (isdigit(c2) ? (c2 - '0') : (toupper(c2) - 'A' + 10));
		str.Append(temp);
	}
	
	return TRUE;
}

CEditResentPktDlg::CEditResentPktDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditResentPktDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditResentPktDlg)
	m_dadr = _T("");
	m_dnet = 0;
	m_sadr = _T("");
	m_snet = 0;
	m_bSnet = FALSE;
	m_bDnet = FALSE;
	m_desIndex = -1;
	//}}AFX_DATA_INIT

	m_defaultDes = 0;
}


void CEditResentPktDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditResentPktDlg)
	DDX_Control(pDX, IDC_SADR, m_sadrCtrl);
	DDX_Control(pDX, IDC_SNET, m_snetCtrl);
	DDX_Control(pDX, IDC_DNET, m_dnetCtrl);
	DDX_Control(pDX, IDC_DADR, m_dadrCtrl);
	DDX_Control(pDX, IDC_DESCOMBO, m_desCombo);
	DDX_Text(pDX, IDC_DADR, m_dadr);
	DDX_Text(pDX, IDC_DNET, m_dnet);
	DDV_MinMaxUInt(pDX, m_dnet, 0, 65535);
	DDX_Text(pDX, IDC_SADR, m_sadr);
	DDX_Text(pDX, IDC_SNET, m_snet);
	DDV_MinMaxUInt(pDX, m_snet, 0, 65535);
	DDX_Check(pDX, IDC_SNETCHECK, m_bSnet);
	DDX_Check(pDX, IDC_DNETCHECK, m_bDnet);
	DDX_CBIndex(pDX, IDC_DESCOMBO, m_desIndex);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditResentPktDlg, CDialog)
	//{{AFX_MSG_MAP(CEditResentPktDlg)
	ON_BN_CLICKED(IDC_DNETCHECK, OnDnetCheck)
	ON_BN_CLICKED(IDC_SNETCHECK, OnSnetCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditResentPktDlg message handlers

BOOL CEditResentPktDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_dnetCtrl.EnableWindow(m_bDnet);
	m_dadrCtrl.EnableWindow(m_bDnet);

	m_snetCtrl.EnableWindow(m_bSnet);
	m_sadrCtrl.EnableWindow(m_bSnet);

	POSITION pos = m_desArray.GetHeadPosition();
	while( pos )
	{
		m_desCombo.AddString(m_desArray.GetNext(pos).m_name);
	}

	m_desCombo.SetCurSel(m_defaultDes);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditResentPktDlg::OnDnetCheck() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	m_dnetCtrl.EnableWindow(m_bDnet);
	m_dadrCtrl.EnableWindow(m_bDnet);
}

void CEditResentPktDlg::OnSnetCheck() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	m_snetCtrl.EnableWindow(m_bSnet);
	m_sadrCtrl.EnableWindow(m_bSnet);	
}

void CEditResentPktDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);

	if(m_bSnet)
	{
		if( !IF_HEX_FORMAT(m_sadr) )
		{
			MessageBox("Please input SADR in hex.", "Warning");
			return;
		}
	}

	if(m_bDnet)
	{
		if( !IF_HEX_FORMAT(m_dadr) )
		{
			MessageBox("Please input DADR in hex.", "Warning");
			return;
		}
	}

	CDialog::OnOK();
}
