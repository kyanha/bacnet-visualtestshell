// VTSPortPTPDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSPortPTPDialog.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char * gBaudrateList[7] = {
	"1200", "2400", "4800", "9600", "19200", "38400", "57600"
};

const char *gParityStr = "N"; 
const char *gDatabitsStr = "8";
const char *gStopbitsStr = "1";

/////////////////////////////////////////////////////////////////////////////
// VTSPortPTPDialog dialog


VTSPortPTPDialog::VTSPortPTPDialog(CString *cp, CWnd* pParent /*=NULL*/)
	: CDialog(VTSPortPTPDialog::IDD, pParent)
	, m_Config( cp )
{
	//{{AFX_DATA_INIT(VTSPortPTPDialog)
	//}}AFX_DATA_INIT

	CString temp;
	temp = *m_Config;

	int index;
	index = temp.Find(':');

	if(index == 4)
	{
		m_portStr = temp.Left(4);
	}
	else
		return;	

	DCB dcb;
	if( !BuildCommDCB(temp, &dcb) )
	{
		*m_Config = "";
		return;
	}
	 
    m_baudrateStr.Format("%d", dcb.BaudRate);
}


void VTSPortPTPDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSPortPTPDialog)
	DDX_Control(pDX, IDC_PORTCOMBO, m_port);
	DDX_Control(pDX, IDC_BAUDRATE, m_baudrate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSPortPTPDialog, CDialog)
	//{{AFX_MSG_MAP(VTSPortPTPDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSPortPTPDialog message handlers

BOOL VTSPortPTPDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here	

	if( m_portStr.Compare("COM1") == 0 )
		m_port.SetCurSel(0);
	else
	if( m_portStr.Compare("COM2") == 0 )
		m_port.SetCurSel(1);
	else
	if( m_portStr.Compare("COM3") == 0 )
		m_port.SetCurSel(2);
	else
	if( m_portStr.Compare("COM4") == 0 )
		m_port.SetCurSel(3);
	else
		m_port.SetCurSel(0);
	
	//init baudrate
	for(int index = 0; index < 7; index++)
	{
		m_baudrate.AddString(gBaudrateList[index]);		
	}	

	m_baudrate.SetCurSel(3); //"9600" is default selection
	for(index = 0; index < 7; index++)
	{
		if( m_baudrateStr.Compare(gBaudrateList[index]) == 0 )
			m_baudrate.SetCurSel(index);	
	}		
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void VTSPortPTPDialog::OnOK() 
{
	// TODO: Add extra validation here
	

	int index = m_port.GetCurSel();
	int port = (int)m_port.GetItemData(index);	

	m_port.GetWindowText(m_portStr);
	m_baudrate.GetWindowText(m_baudrateStr);	

	*m_Config = m_portStr + ":" + m_baudrateStr + "," + gParityStr + "," + gDatabitsStr + "," + gStopbitsStr;	

	CDialog::OnOK();
}
