// VTSNamesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSNamesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VTSNamesDlg dialog

VTSNamesDlg::VTSNamesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(VTSNamesDlg::IDD, pParent)
	, m_pNameList( 0 ), m_pPortList( 0 )
	, m_iSelectedName( -1 )
{
	//{{AFX_DATA_INIT(VTSNamesDlg)
	m_Name = _T("");
	m_Address = _T("");
	m_Network = _T("");
	//}}AFX_DATA_INIT
}

VTSNamesDlg::VTSNamesDlg( VTSNameListPtr nlp, VTSPortListPtr plp )
	: CDialog( VTSNamesDlg::IDD, NULL )
	, m_pNameList( nlp ), m_pPortList( plp )
	, m_iSelectedName( -1 )
{
	// duplicate the member initalization that the class 'wizard' adds
	m_Name = _T("");
	m_Address = _T("");
	m_Network = _T("");
}

void VTSNamesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSNamesDlg)
	DDX_Control(pDX, IDC_NAMELIST, m_NameList);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Control(pDX, IDC_PORTCOMBO, m_PortCombo);
	DDX_Text(pDX, IDC_ADDRESS, m_Address);
	DDX_Text(pDX, IDC_NETWORK, m_Network);
	DDX_Radio(pDX, IDC_NULLADDR, m_AddrType);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(VTSNamesDlg, CDialog)
	//{{AFX_MSG_MAP(VTSNamesDlg)
	ON_BN_CLICKED(ID_NEWNAME, OnNew)
	ON_BN_CLICKED(ID_DELETENAME, OnDelete)
	ON_BN_CLICKED(ID_IMPORTNAMES, OnImportNames)
	ON_BN_CLICKED(ID_EXPORTNAMES, OnExportNames)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_NAMELIST, OnItemchangingNamelist)
	ON_EN_CHANGE(IDC_NAME, SaveChanges)
	ON_BN_CLICKED(IDC_NULLADDR, SaveChanges)
	ON_BN_CLICKED(IDC_LOCALBROADCAST, SaveChanges)
	ON_BN_CLICKED(IDC_LOCALSTATION, SaveChanges)
	ON_BN_CLICKED(IDC_REMOTEBROADCAST, SaveChanges)
	ON_BN_CLICKED(IDC_REMOTESTATION, SaveChanges)
	ON_BN_CLICKED(IDC_GLOBALBROADCAST, SaveChanges)
	ON_CBN_SELCHANGE(IDC_PORTCOMBO, SaveChanges)
	ON_EN_CHANGE(IDC_NETWORK, SaveChanges)
	ON_EN_CHANGE(IDC_ADDRESS, SaveChanges)
	ON_NOTIFY(NM_DBLCLK, IDC_NAMELIST, OnDblclkNamelist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSNamesDlg message handlers

BOOL VTSNamesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// disable import and export for now
	GetDlgItem( ID_IMPORTNAMES )->EnableWindow( false );
	GetDlgItem( ID_EXPORTNAMES )->EnableWindow( false );

	InitPortList();					// get the ports from the database
	InitNameList();					// fill in the table from the database

	ResetSelection();				// nothing selected by default
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void VTSNamesDlg::InitPortList()
{
	VTSPortPtr	curPort
	;

	// Add the ports
	m_PortCombo.AddString( "(any port)" );

	// add items and subitems
	for (int i = 0; i < m_pPortList->Length(); i++) {
		// get a pointer to the ith port
		curPort = (*m_pPortList)[i];
		m_PortCombo.AddString( curPort->portDesc.portName );
	}

	// set the first item as the default
	m_PortCombo.SetCurSel( 0 );
}

void VTSNamesDlg::InitNameList()
{
	VTSNameDesc		name
	;
	int				len = m_pNameList->Length()
	;

	// only allow one selected name at a time
	m_NameList.m_nFlags |= LVS_SINGLESEL;

	// do not sort the list
	m_NameList.m_nFlags &= ~LBS_SORT;

	// add columns.
	m_NameList.InsertColumn (0, _T("Name"), LVCFMT_LEFT, 96 );
	m_NameList.InsertColumn (1, _T("Port"), LVCFMT_LEFT, 64 );
	m_NameList.InsertColumn (2, _T("Network"), LVCFMT_LEFT, 64 );
	m_NameList.InsertColumn (3, _T("Address"), LVCFMT_LEFT, 128 );

	// add names
	for (int i = 0; i < len; i++) {
		// make a placeholder for the item
		m_NameList.InsertItem( i, _T("") );

		// read the name from the database
		m_pNameList->ReadName( i, &name );

		// transfer the record contents to the list
		NameToList( name, i );
	}
}

void VTSNamesDlg::ResetSelection()
{
	// no name selected
	m_iSelectedName = -1;

	// clear out the contents of the local variables
	m_AddrType = -1;
	m_Name = _T("");
	m_PortCombo.SetCurSel( 0 );
	m_Address = _T("");
	m_Network = _T("");

	// make sure the control values reflect the empty settings
	UpdateData( false );

	// make sure all the controls are properly disabled
	SynchronizeControls();
}

void VTSNamesDlg::SynchronizeControls()
{
	// the name is enabled when there is something selected
	GetDlgItem( IDC_NAME )->EnableWindow( m_iSelectedName != -1 );

	// so are the radio buttons for the address type
	GetDlgItem( IDC_NULLADDR )->EnableWindow( m_iSelectedName != -1 );
	GetDlgItem( IDC_LOCALBROADCAST )->EnableWindow( m_iSelectedName != -1 );
	GetDlgItem( IDC_LOCALSTATION )->EnableWindow( m_iSelectedName != -1 );
	GetDlgItem( IDC_REMOTEBROADCAST )->EnableWindow( m_iSelectedName != -1 );
	GetDlgItem( IDC_REMOTESTATION )->EnableWindow( m_iSelectedName != -1 );
	GetDlgItem( IDC_GLOBALBROADCAST )->EnableWindow( m_iSelectedName != -1 );
	
	// so is the port selection
	GetDlgItem( IDC_PORTCOMBO )->EnableWindow( m_iSelectedName != -1 );

	// network and address controls are a little harder
	switch (m_AddrType) {
		case -1:	// no selection
		case 0:		// null address
		case 1:		// local broadcast
		case 5:		// global broadcast
			GetDlgItem( IDC_NETWORK )->EnableWindow( false );
			GetDlgItem( IDC_ADDRESS )->EnableWindow( false );
			break;

		case 2:		// local station
			GetDlgItem( IDC_NETWORK )->EnableWindow( false );
			GetDlgItem( IDC_ADDRESS )->EnableWindow( true );
			break;

		case 3:		// remote broadcast
			GetDlgItem( IDC_NETWORK )->EnableWindow( true );
			GetDlgItem( IDC_ADDRESS )->EnableWindow( false );
			break;

		case 4:		// remote station
			GetDlgItem( IDC_NETWORK )->EnableWindow( true );
			GetDlgItem( IDC_ADDRESS )->EnableWindow( true );
			break;
	}
}

//
//	VTSNamesDlg::NameToList
//

void VTSNamesDlg::NameToList( const VTSNameDesc &name, int elem )
{
	CString		netStr, addrStr
	;

	// update the display list
	m_NameList.SetItemText( elem, 0, name.nameName );

	// check the port
	if (name.namePort == 0)
		m_NameList.SetItemText( elem, 1, _T("*") );
	else {
		VTSPortPtr	curPort
		;

		// look for the matching ID
		for (int i = 0; i < m_pPortList->Length(); i++) {
			// get a pointer to the ith port
			curPort = (*m_pPortList)[i];
			if (name.namePort == curPort->portDescID) {
				m_NameList.SetItemText( elem, 1, curPort->portDesc.portName );
				break;
			}
		}
	}

	// check for some remote broadcast or remote station
	if ((name.nameAddr.addrType == remoteBroadcastAddr) || (name.nameAddr.addrType == remoteStationAddr))
		netStr.Format( "%d", name.nameAddr.addrNet );
	else
		netStr = _T("");
	m_NameList.SetItemText( elem, 2, netStr );

	// check for local or remote station
	if ((name.nameAddr.addrType == localStationAddr) || (name.nameAddr.addrType == remoteStationAddr)) {
		static const char	hex[] = "0123456789ABCDEF"
		;
		char	buff[kMaxAddressLen * 3], *s = buff
		;

		// clear the buffer
		buff[0] = 0;

		// encode the address
		for (int i = 0; i < name.nameAddr.addrLen; i++) {
			if (i) *s++ = '-';
			*s++ = hex[ name.nameAddr.addrAddr[i] >> 4 ];
			*s++ = hex[ name.nameAddr.addrAddr[i] & 0x0F ];
		}
		*s = 0;

		addrStr = buff;
	} else
		addrStr = _T("");
	m_NameList.SetItemText( elem, 3, addrStr );
}

//
//	VTSNamesDlg::NameToCtrl
//

void VTSNamesDlg::NameToCtrl( const VTSNameDesc &name )
{
	// set the contents of the member vars from the record
	m_Name = name.nameName;
	m_AddrType = name.nameAddr.addrType;

	m_Address = _T("");

	if ((name.nameAddr.addrType == remoteBroadcastAddr) || (name.nameAddr.addrType == remoteStationAddr))
		m_Network.Format( "%d", name.nameAddr.addrNet );
	else
		m_Network = _T("");

	if ((name.nameAddr.addrType == localStationAddr) || (name.nameAddr.addrType == remoteStationAddr)) {
		static const char	hex[] = "0123456789ABCDEF"
		;
		char	buff[kMaxAddressLen * 3], *s = buff
		;

		// clear the buffer
		buff[0] = 0;

		// encode the address
		for (int i = 0; i < name.nameAddr.addrLen; i++) {
			if (i) *s++ = '-';
			*s++ = hex[ name.nameAddr.addrAddr[i] >> 4 ];
			*s++ = hex[ name.nameAddr.addrAddr[i] & 0x0F ];
		}
		*s = 0;

		m_Address = buff;
	} else
		m_Address = _T("");

	// sync the controls
	SynchronizeControls();

	// sync the dialog controls with the member variables
	UpdateData( false );
}

//
//	VTSNamesDlg::CtrlToName
//

void VTSNamesDlg::CtrlToName( VTSNameDesc &name )
{
	int			valu = 0
	;
	LPCSTR		s
	;
	CComboBox	*cbp = (CComboBox *)GetDlgItem( IDC_PORTCOMBO )
	;

	// sync the member variables with the dialog controls
	UpdateData( true );

	// make sure the correct UI elements are enable/disabled
	SynchronizeControls();

	// copy the vars into the name
	strcpy( name.nameName, m_Name );

	// do the address type
	name.nameAddr.addrType = (BACnetAddressType)m_AddrType;

	// check the port
	valu = cbp->GetCurSel();
	name.namePort = (valu == 0 ? 0 : (*m_pPortList)[valu - 1]->portDescID);

	// check for some remote broadcast or remote station
	if ((name.nameAddr.addrType == remoteBroadcastAddr) || (name.nameAddr.addrType == remoteStationAddr)) {
		s = m_Network;

		if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) {
			s += 2;
			for (valu = 0; isxdigit(*s); s++)
				valu = (valu << 4) + (isdigit(*s) ? (*s - '0') : (toupper(*s) - 'A' + 10));
		} else {
			for (valu = 0; isdigit(*s); s++)
				valu = (valu *10) + (*s - '0');
		}

		name.nameAddr.addrNet = valu;
	}

	// check for local or remote station
	if ((name.nameAddr.addrType == localStationAddr) || (name.nameAddr.addrType == remoteStationAddr)) {
		int			upperNibble, lowerNibble
		;
		char		c
		;

		s = m_Address;

		// remove contents
		name.nameAddr.addrLen = 0;

		// translate the text into octets
		for (;;) {
			// look for a hex digit
			while ((c = toupper(*s++)) && !isxdigit(c))
				;
			if (!c) break;
			upperNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

			// look for another hex digit
			while ((c = toupper(*s++)) && !isxdigit(c))
				;
			if (!c) break;
			lowerNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

			// add the byte
			if (name.nameAddr.addrLen < kMaxAddressLen)
				name.nameAddr.addrAddr[ name.nameAddr.addrLen++ ] = (upperNibble << 4) + lowerNibble;
		}
	}
}

//
//	VTSNamesDlg::OnNew
//

void VTSNamesDlg::OnNew() 
{
	int		listLen = m_pNameList->Length()
	;
	VTSNameDesc		name
	;

	// deselect if something was selected
	POSITION	selPos = m_NameList.GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int	 nItem = m_NameList.GetNextSelectedItem( selPos );
		m_NameList.SetItemState( nItem, 0, LVIS_SELECTED );
	}
	
	// create a new name
	m_pNameList->Add();

	// read the name from the database
	m_pNameList->ReadName( listLen, &name );

	// make a placeholder for the item
	m_NameList.InsertItem( listLen, _T("") );

	// copy the contents to the list
	NameToList( name, listLen );

	// make sure it is visible and selected, this also transfers the info to the ctrls
	m_NameList.EnsureVisible( listLen, false );
	m_NameList.SetItemState( listLen, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	// set the focus to the name for editing, select all of the text
	GetDlgItem( IDC_NAME )->SetFocus();
	((CEdit*)GetDlgItem( IDC_NAME ))->SetSel( 0, -1 );
}

//
//	VTSNamesDlg::OnDelete
//

void VTSNamesDlg::OnDelete() 
{
	POSITION	selPos = m_NameList.GetFirstSelectedItemPosition()
	;

	// make sure something was selected
	if (selPos == NULL)
		return;

	// figure out which item it is
	int	 nItem = m_NameList.GetNextSelectedItem( selPos );

	// now deselect it, clearing out the controls in the process
	m_NameList.SetItemState( nItem, 0, LVIS_SELECTED | LVIS_FOCUSED );

	// delete it from the list
	m_NameList.DeleteItem( nItem );

	// delete the name from the database
	m_pNameList->Remove( nItem );
}

//
//	VTSNamesDlg::OnImportNames
//

void VTSNamesDlg::OnImportNames() 
{
}

//
//	VTSNamesDlg::OnExportNames
//

void VTSNamesDlg::OnExportNames() 
{
}

//
//	VTSNamesDlg::OnItemchangingNamelist
//

void VTSNamesDlg::OnItemchangingNamelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	VTSNameDesc		name
	;
	NM_LISTVIEW*	pNMListView = (NM_LISTVIEW*)pNMHDR
	;
	
#if 0
	TRACE3( "Item %d from %d to %d\n"
		, pNMListView->iItem
		, pNMListView->uOldState
		, pNMListView->uNewState
		);
#endif

	// forget messages that don't change anything
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0) {
		m_iSelectedName = pNMListView->iItem;

		// read the name from the database
		m_pNameList->ReadName( m_iSelectedName, &name );

		// transfer the values to the controls
		NameToCtrl( name );
	} else {
		if (pNMListView->iItem == m_iSelectedName)
			ResetSelection();
	}

	*pResult = 0;
}

//
//	VTSNamesDlg::OnDblclkNamelist
//

void VTSNamesDlg::OnDblclkNamelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// set the focus to the name for editing, select all of the text
	GetDlgItem( IDC_NAME )->SetFocus();
	((CEdit*)GetDlgItem( IDC_NAME ))->SetSel( 0, -1 );

	*pResult = 0;
}

//
//	VTSNamesDlg::SaveChanges
//

void VTSNamesDlg::SaveChanges()
{
	VTSNameDesc		name
	;

	// read the name from the database
	m_pNameList->ReadName( m_iSelectedName, &name );

	// transfer the contents of the controls to the record
	CtrlToName( name );

	// save the updated record
	m_pNameList->WriteName( m_iSelectedName, &name );

	// bring the list up-to-date
	NameToList( name, m_iSelectedName );
}
