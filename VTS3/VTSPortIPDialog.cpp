// VTSPortIPDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSDoc.h"

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
	m_HostAddr = _T("");
	m_TTL = _T("");
	m_PortType = -1;
	//}}AFX_DATA_INIT

	int		argc = 0
	;
	char	config[kVTSPortConfigLength], *src
	,		*argv[16]
	;

	// copy the configuration, split it up
	if (**cp == 0)
		strcpy( config, "0xBAC0" );
	else
		strcpy( config, *cp );
	for (src = config; *src; ) {
		argv[argc++] = src;
		while (*src && (*src != ','))
			src++;
		if (*src == ',')
			*src++ = 0;
	}
	for (int i = argc; i < 16; i++)
		argv[i] = src;

	// set the socket
	m_Socket = argv[0];

	// if that's the only thing, that's OK
	if (argc == 1)
		m_PortType = 0;
	else {
		m_PortType = atoi( argv[1] );
		switch (m_PortType) {
			case 0:				// raw
			case 1:				// BTR
			case 2:				// BBMD
			case 3:				// BIP simple
				break;
			case 4:
				m_HostAddr = argv[2];
				m_TTL = argv[3];
				break;
		}
	}
}

void VTSPortIPDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSPortIPDialog)
	DDX_Text(pDX, IDC_SOCKET, m_Socket);
	DDX_Text(pDX, IDC_HOSTADDR, m_HostAddr);
	DDX_Text(pDX, IDC_TTL, m_TTL);
	DDX_Radio(pDX, IDC_RAW, m_PortType);
	//}}AFX_DATA_MAP

	GetDlgItem( IDC_HOSTADDR )->EnableWindow( m_PortType == 4 );
	GetDlgItem( IDC_TTL )->EnableWindow( m_PortType == 4 );
}

BEGIN_MESSAGE_MAP(VTSPortIPDialog, CDialog)
	//{{AFX_MSG_MAP(VTSPortIPDialog)
	ON_BN_CLICKED(IDC_RAW, OnPortType)
	ON_BN_CLICKED(IDC_BTR, OnPortType)
	ON_BN_CLICKED(IDC_BBMD, OnPortType)
	ON_BN_CLICKED(IDC_BIP, OnPortType)
	ON_BN_CLICKED(IDC_FOREIGN, OnPortType)
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
	CString		rslt, form
	;

	// sync the vars with the controls
	UpdateData();

	// format the result string
	rslt = m_Socket;
	switch (m_PortType) {
		case 0:				// Raw
			break;

		case 1:				// BTR [, ipaddr ]*
			rslt += ",1";
			break;

		case 2:				// BBMD [, ipaddr/host:socket ]*
			rslt += ",2";
			break;

		case 3:				// BIP
			rslt += ",3";
			break;

		case 4:				// FOREIGN, ipaddr:socket, TTL
			rslt += ",4," + m_HostAddr + "," + m_TTL;
			break;
	}

	// transfer
	*m_Config = rslt;

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

//
//	VTSPortIPDialog::OnPortType
//

void VTSPortIPDialog::OnPortType() 
{
	// sync the member variables with the dialog controls
	UpdateData( true );

	GetDlgItem( IDC_HOSTADDR )->EnableWindow( m_PortType == 4 );
	GetDlgItem( IDC_TTL )->EnableWindow( m_PortType == 4 );
}
