// VTSPortEthernetDialog.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSPortEthernetDialog.h"

#include "WinPacket32.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

short UnicodeToAscii( char *ap, wchar_t *up, short max );

/////////////////////////////////////////////////////////////////////////////
// VTSPortEthernetDialog dialog

VTSPortEthernetDialog::VTSPortEthernetDialog(CString *cp, CWnd* pParent /*=NULL*/)
	: CDialog(VTSPortEthernetDialog::IDD, pParent)
	, m_Config( cp )
{
	//{{AFX_DATA_INIT(VTSPortEthernetDialog)
	//}}AFX_DATA_INIT
}

void VTSPortEthernetDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSPortEthernetDialog)
	DDX_Control(pDX, IDC_ADAPTERCOMBO, m_AdapterCombo);
	DDX_Control(pDX, IDC_PROMISCUOUS, m_Promiscuous);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(VTSPortEthernetDialog, CDialog)
	//{{AFX_MSG_MAP(VTSPortEthernetDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	VTSPortEthernetDialog::OnOK
//
//	When the user accepts the configuration information and clicks OK, 
//	this function is called which transfers the new configuration into 
//	the config data provided by the caller, VTSPort::Configuration.
//

void VTSPortEthernetDialog::OnOK()
{
	CString adapter
	;

	// sync the vars with the controls
	UpdateData();

	// control text has info (might not be a selection!)
	m_AdapterCombo.GetWindowText( adapter );

	// transfer
	*m_Config = adapter + "," + (m_Promiscuous.GetCheck() ? "1" : "0");

	// continue normal processing
	CDialog::OnOK();
}

//
//	VTSPortEthernetDialog::OnInitDialog
//

BOOL VTSPortEthernetDialog::OnInitDialog() 
{
	int		i, len
	;
	char	nameBuff[64]
	;

	// let the base class do its work
	CDialog::OnInitDialog();
	
	// make sure the adapter list is initialized
	if (!gAdapterList)
		InitAdapterList();

	// look for ethernet ports
	len = 0;
	for (i = 0; i < gAdapterListLen; i++)
		if (gAdapterList[i].AdapterType == ETH_802_3_ADAPTER) {
			// convert the name
			UnicodeToAscii( nameBuff, gAdapterList[i].MacDriverName, sizeof(nameBuff) );

			// add it to the combo
			m_AdapterCombo.AddString( nameBuff );
			m_AdapterCombo.SetItemData( len, i );

			// count it
			len++;
		}
	
	// if there's no adapters, toss up a message box and exit
	if (len == 0) {
		AfxMessageBox( "No Ethernet adapters available" );
		CDialog::EndDialog( IDCANCEL );
	}

	// If there is already a configuration, set the combo box text to
	// it, otherwise, set it to the first one.  It would be nice if I went 
	// through all of the ports and picked one that wasn't already picked.
	// That will make a nice enhancement.
	if (m_Config->GetLength() != 0) {
		char config[64], *src, *dst;

		// copy the name over
		src = m_Config->GetBuffer(0);
		dst = config;
		while (*src && (*src != ','))
			*dst++ = *src++;
		*dst++ = 0;

		// look for promiscuous mode
		if (*src == ',') {
			src += 1;
			if (*src == '1')
				m_Promiscuous.SetCheck( 1 );
		}

		// set the combo text
		m_AdapterCombo.SetWindowText( config );
	} else
		m_AdapterCombo.SetCurSel( 0 );

	return TRUE;
}

//
//	UnicodeToAscii
//

short UnicodeToAscii( char *ap, wchar_t *up, short max )
{
	short	n = 0
	;
	
	while ((*up != 0) && (n < max-1)) {
		*ap++ = (char)(*up++);
		n++;
	}
	*ap = 0;

	return n;
}
