// VTSPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSPortDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VTSPortDlg dialog


VTSPortDlg::VTSPortDlg(CWnd* pParent /*=NULL*/)
	: CDialog(VTSPortDlg::IDD, pParent)
	, m_pPortList( 0 )
{
	//{{AFX_DATA_INIT(VTSPortDlg)
	m_Name = _T("");
	m_Enabled = FALSE;
	m_Type = -1;
	//}}AFX_DATA_INIT
}

VTSPortDlg::VTSPortDlg( VTSPortListPtr plp )
	: CDialog( VTSPortDlg::IDD, NULL )
	, m_pPortList( plp )
{
	// duplicate the member initalization that the class 'wizard' adds
	m_Name = _T("");
	m_Type = -1;
	m_Enabled = FALSE;

	// configuration no longer a bound variable
	m_Config = _T("");
}

void VTSPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSPortDlg)
	DDX_Control(pDX, IDC_PORTLIST, m_PortList);
	DDX_Text(pDX, IDC_PORTNAME, m_Name);
	DDX_Check(pDX, IDC_ENABLEPORT, m_Enabled);
	DDX_Radio(pDX, IDC_NULLPORT, m_Type);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSPortDlg, CDialog)
	//{{AFX_MSG_MAP(VTSPortDlg)
	ON_BN_CLICKED(IDC_NEWPORT, OnNewPort)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_PORTLIST, OnItemchangingPortList)
	ON_NOTIFY(NM_DBLCLK, IDC_PORTLIST, OnDblclkPortList)
	ON_BN_CLICKED(IDC_CONFIG, OnConfig)
	ON_EN_UPDATE(IDC_PORTNAME, SaveChanges)
	ON_BN_CLICKED(IDC_NULLPORT, SaveChanges)
	ON_BN_CLICKED(IDC_IPPORT, SaveChanges)
	ON_BN_CLICKED(IDC_ETHERNETPORT, SaveChanges)
	ON_BN_CLICKED(IDC_ARCNETPORT, SaveChanges)
	ON_BN_CLICKED(IDC_MSTPPORT, SaveChanges)
	ON_BN_CLICKED(IDC_PTPPORT, SaveChanges)
	ON_BN_CLICKED(IDC_ENABLEPORT, SaveChanges)
	ON_BN_CLICKED(IDC_COLOR, OnColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSPortDlg message handlers

//
//	VTSPortDlg::OnInitDialog
//

BOOL VTSPortDlg::OnInitDialog() 
{
	VTSPortPtr	curPort
	;

	CDialog::OnInitDialog();
	
	// initialize the port list
	m_PortList.m_nFlags |= LVS_SINGLESEL;
	m_PortList.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 96 );
	m_PortList.InsertColumn( 1, _T("Type"), LVCFMT_LEFT, 64 );
	m_PortList.InsertColumn( 2, _T("Config"), LVCFMT_LEFT, 96 );
	m_PortList.InsertColumn( 3, _T("Status"), LVCFMT_LEFT, 256 );

	// initialize the status image list
	m_ilStatus.Create( IDB_PORTSTATUS, 16, 1, RGB(255,0,255) );
	m_PortList.SetImageList( &m_ilStatus, LVSIL_SMALL );

	// add items and subitems
	for (int i = 0; i < m_pPortList->Length(); i++) {
		// get a pointer to the ith port
		curPort = (*m_pPortList)[i];

		// copy the descriptions into the list
		m_PortList.InsertItem( i, (LPCTSTR)curPort->portDesc.portName, curPort->portStatus );
		m_PortList.SetItemText( i, 1, (LPCTSTR)gVTSPortTypes[curPort->portDesc.portType] );
		m_PortList.SetItemText( i, 2, (LPCTSTR)curPort->portDesc.portConfig );
		m_PortList.SetItemText( i, 3
			, (LPCTSTR)(curPort->portStatusDesc ? curPort->portStatusDesc : "")
			);
	}

	// make sure nothing is selected
	ResetSelection();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//
//	VTSPortDlg::SetSelection
//

void VTSPortDlg::SetSelection( int indx )
{
	VTSPortPtr	curPort
	;

	TRACE1( "SetSelection( %d )\n", indx );

	// if something is selected and its not this one, reset
	if ((m_iSelectedPort >= 0) && (indx != m_iSelectedPort))
		ResetSelection();

	// get a pointer to the port
	curPort = (*m_pPortList)[indx];

	// set the local variables to the port information
	m_iSelectedPort = indx;
	m_Name = curPort->portDesc.portName;
	m_Type = curPort->portDesc.portType;
	m_Enabled = curPort->portDesc.portEnabled;

	// this is no longer a dialog bound variable, but the config button needs the data
	m_Config = curPort->portDesc.portConfig;

	// make sure the controls are properly enabled
	SynchronizeControls();

	// let the CDialog sync the controls with the local vars
	UpdateData( false );
}

//
//	VTSPortDlg::ResetSelection
//

void VTSPortDlg::ResetSelection( void )
{
	TRACE0( "ResetSelection()\n" );

	// nothing selected, just return
	if (m_iSelectedPort < 0)
		return;

#if 0
	// deselect from the list
	POSITION selPos = m_PortList.GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int nItem = m_PortList.GetNextSelectedItem( selPos );
		m_PortList.SetItemState( nItem, 0, LVIS_SELECTED );
	}
#endif

	// clear out the contents of the local vars
	m_iSelectedPort = -1;
	m_Name = _T("");
	m_Type = -1;
	m_Enabled = 0;
	m_Config = _T("");

	// let the CDialog sync the controls with the local vars
	UpdateData( false );

	// make sure the controls are properly disabled
	SynchronizeControls();
}

//
//	VTSPortDlg::SynchronizeControls
//

void VTSPortDlg::SynchronizeControls( void )
{
	// disable if nothing selected
	GetDlgItem( IDC_PORTNAME )->EnableWindow( m_iSelectedPort >= 0 );

	GetDlgItem( IDC_ENABLEPORT )->EnableWindow( m_iSelectedPort >= 0 );
	GetDlgItem( IDC_CONFIG )->EnableWindow( m_iSelectedPort >= 0 );

	GetDlgItem( IDC_NULLPORT )->EnableWindow( m_iSelectedPort >= 0 );
	GetDlgItem( IDC_IPPORT )->EnableWindow( m_iSelectedPort >= 0 );
	GetDlgItem( IDC_ETHERNETPORT )->EnableWindow( m_iSelectedPort >= 0 );
	GetDlgItem( IDC_ARCNETPORT )->EnableWindow( m_iSelectedPort >= 0 );
	GetDlgItem( IDC_MSTPPORT )->EnableWindow( m_iSelectedPort >= 0 );
	GetDlgItem( IDC_PTPPORT )->EnableWindow( m_iSelectedPort >= 0 );
}

//
//	VTSPortDlg::PortStatusChange
//

void VTSPortDlg::PortStatusChange( void )
{
	VTSPortPtr	curPort
	;

	// add items and subitems
	for (int i = 0; i < m_pPortList->Length(); i++) {
		// get a pointer to the ith port
		curPort = (*m_pPortList)[i];

		// set the image for the list to match the status of the port
		m_PortList.SetItem( i, 0, TVIF_IMAGE, 0, curPort->portStatus, 0, 0, 0 );

		// copy the status description
		m_PortList.SetItemText( i, 3
			, (LPCTSTR)(curPort->portStatusDesc ? curPort->portStatusDesc : "")
			);
	}
}

//
//	VTSPortDlg::OnNewPort
//

void VTSPortDlg::OnNewPort() 
{
	int		indx
	;
	VTSPortPtr	curPort
	;

	// tell the list to add a new port
	m_pPortList->Add();

	// get its index, it will always be the last one
	indx = m_pPortList->Length() - 1;

	// get a pointer to the new port
	curPort = (*m_pPortList)[indx];

	// copy the descriptions into the list
	m_PortList.InsertItem( indx, (LPCTSTR)curPort->portDesc.portName );
	m_PortList.SetItemText( indx, 1, (LPCTSTR)gVTSPortTypes[curPort->portDesc.portType] );
	m_PortList.SetItemText( indx, 2, (LPCTSTR)curPort->portDesc.portConfig );
	m_PortList.SetItemText( indx, 3, (LPCTSTR)"" );

	// make sure the record is visible and selected
	m_PortList.EnsureVisible( indx, false );
	m_PortList.SetItemState( indx, LVIS_SELECTED, LVIS_SELECTED );
}

//
//	VTSPortDlg::OnItemchangingPortList
//

void VTSPortDlg::OnItemchangingPortList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// forget messages that dont change anything
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0)
		SetSelection( pNMListView->iItem );
	else
	if (pNMListView->iItem == this->m_iSelectedPort)
		ResetSelection();

	*pResult = 0;
}

//
//	VTSPortDlg::OnDblclkPortList
//

void VTSPortDlg::OnDblclkPortList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// set the focus to the name for editing, preselect all of the text
	GetDlgItem( IDC_PORTNAME )->SetFocus();
	((CEdit*)GetDlgItem( IDC_PORTNAME ))->SetSel( 0, -1 );

	*pResult = 0;
}

//
//	VTSPortDlg::SaveChanges
//
//	This function is called when something changes in one of the dialog box configuration 
//	fields.  It copies the dialog box information into the port description record and 
//	tells the port to refresh, which opens the port if possible.
//

void VTSPortDlg::SaveChanges() 
{
	bool		doRefresh
	;
	VTSPortPtr	curPort
	;

	ASSERT( m_iSelectedPort >= 0 );

	// get a pointer to the new port
	curPort = (*m_pPortList)[m_iSelectedPort];

	// sync the member variables with the dialog controls
	UpdateData( true );

	// assume nothing has changed
	doRefresh = false;

	// copy the data back to the current selected port
	strcpy( curPort->portDesc.portName, m_Name );

	if (curPort->portDesc.portEnabled != m_Enabled) {
		doRefresh = true;
		curPort->portDesc.portEnabled = m_Enabled;
	}
	if (strcmp(curPort->portDesc.portConfig, m_Config) != 0) {
		doRefresh = true;
		strcpy( curPort->portDesc.portConfig, m_Config );
	}
	if (curPort->portDesc.portType != (VTSPortType)m_Type) {
		doRefresh = true;
		curPort->portDesc.portType = (VTSPortType)m_Type;

		// flush the data
		curPort->portDesc.portEnabled = false;
		curPort->portDesc.portConfig[0] = 0;

		// make sure the dialog box knows
		m_Config.Empty();
		m_Enabled = false;
		UpdateData( false );
	}

	// tell the port to save changes
	curPort->WriteDesc();

	// if something other than the name changed, tell it to refresh
	if (doRefresh)
		curPort->Refresh();

	// set the image for the list to match the status of the port
	m_PortList.SetItem( m_iSelectedPort, 0, TVIF_IMAGE, 0, curPort->portStatus, 0, 0, 0 );

	// update the list
	m_PortList.SetItemText( m_iSelectedPort, 0, (LPCTSTR)curPort->portDesc.portName );
	m_PortList.SetItemText( m_iSelectedPort, 1, (LPCTSTR)gVTSPortTypes[curPort->portDesc.portType] );
	m_PortList.SetItemText( m_iSelectedPort, 2, (LPCTSTR)curPort->portDesc.portConfig );
	m_PortList.SetItemText( m_iSelectedPort, 3
		, (LPCTSTR)(curPort->portStatusDesc ? curPort->portStatusDesc : "")
		);
}

void VTSPortDlg::OnConfig() 
{
	VTSPortPtr	curPort
	;

	ASSERT( m_iSelectedPort >= 0 );

	// get a pointer to the port
	curPort = (*m_pPortList)[m_iSelectedPort];

	// tell the port to update this string with the configuration (can cancel!)
	curPort->Configure( &m_Config );

	// if nothing has changed, bail
	if (strcmp(curPort->portDesc.portConfig, m_Config) == 0)
		return;

	// save the data into the port
	strcpy( curPort->portDesc.portConfig, m_Config );

	// tell the port to save changes
	curPort->WriteDesc();

	// and now refresh
	curPort->Refresh();

	// set the image for the list to match the status of the port
	m_PortList.SetItem( m_iSelectedPort, 0, TVIF_IMAGE, 0, curPort->portStatus, 0, 0, 0 );

	// update the list
	m_PortList.SetItemText( m_iSelectedPort, 2, (LPCTSTR)curPort->portDesc.portConfig );
	m_PortList.SetItemText( m_iSelectedPort, 3
		, (LPCTSTR)(curPort->portStatusDesc ? curPort->portStatusDesc : "")
		);
}

void VTSPortDlg::OnColor() 
{
	CColorDialog	clr
	;

	if (clr.DoModal() == IDOK) {
		TRACE0( "New color?\n" );
	}
}
