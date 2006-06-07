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



//VTSNamesDlg::VTSNamesDlg( VTSNameListPtr nlp, VTSPortListPtr plp )
// MAD_DB
VTSNamesDlg::VTSNamesDlg(  VTSNames * pnames, VTSPorts * pports, CWnd* pParent /* = NULL */ )
	: CDialog( VTSNamesDlg::IDD, pParent )
//MAD_DB	, m_pNameList( nlp ), m_pPortList( plp )
//MAD_DB	, m_pPortList( plp )
//MAD_DB	, m_iSelectedName( -1 )
{
	// duplicate the member initalization that the class 'wizard' adds
	m_Name = _T("");
	m_Address = _T("");
	m_Network = _T("");

	m_names.DeepCopy(pnames);
	m_pnames = pnames;
	m_pports = pports;
	m_fWarnedAlready = false;
	m_iSelectedName = -1;
}

void VTSNamesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSNamesDlg)
	DDX_Control(pDX, IDC_NAMELIST, m_NameList);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Control(pDX, IDC_PORTCOMBO, m_PortCombo);
	DDX_Control(pDX, IDC_ADDRESS, m_AddressCtrl);	
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
	ON_NOTIFY(NM_DBLCLK, IDC_NAMELIST, OnDblclkNamelist)
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
	ON_EN_KILLFOCUS(IDC_ADDRESS, OnKillfocusAddress)
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
	// Add the ports
	m_PortCombo.AddString( "(any port)" );

	// add items and subitems
	for (int i = 0; i < m_pports->GetSize(); i++)
		m_PortCombo.AddString( (*m_pports)[i]->GetName() );

	// set the first item as the default
	m_PortCombo.SetCurSel( 0 );
}

void VTSNamesDlg::InitNameList()
{
//MAD_DB
//	VTSNameDesc		name;
	//int				len = m_pNameList->Length();
	int				len = m_names.GetSize();

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
	for (int i = 0; i < len; i++)
	{
		// make a placeholder for the item
		m_NameList.InsertItem( i, _T("") );

		// read the name from the database
//MAD_DB		m_pNameList->ReadName( i, &name );

		// transfer the record contents to the list
//		NameToList( name, i );
		VTSName * p = (VTSName *) m_names[i];
		NameToList( (VTSName *) m_names[i], i );
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

//MAD_DB void VTSNamesDlg::NameToList( const VTSNameDesc &name, int elem )
void VTSNamesDlg::NameToList( const VTSName * pname, int elem )
{
	CString		netStr, addrStr;

	ASSERT(pname != NULL);

	// update the display list
//MAD_DB	m_NameList.SetItemText( elem, 0, name.nameName );
	m_NameList.SetItemText( elem, 0, pname->m_strName );

	if ( pname->m_pportLink == NULL )
		m_NameList.SetItemText( elem, 1, _T("*") );
	else
		m_NameList.SetItemText( elem, 1, _T(pname->m_pportLink->GetName()) );

	// check the port

/* MAD_DB
	if (name.namePort == 0)
		m_NameList.SetItemText( elem, 1, _T("*") );
	else
	{
		VTSPortPtr	curPort;

		// look for the matching ID
		for (int i = 0; i < m_pPortList->Length(); i++)
		{
			// get a pointer to the ith port
			curPort = (*m_pPortList)[i];

			if ( name.namePort == curPort->portDescID) {
				m_NameList.SetItemText( elem, 1, curPort->portDesc.portName );
				break;
			}
		}
	}
*/

	// check for some remote broadcast or remote station
//MAD_DB
//	if ((name.nameAddr.addrType == remoteBroadcastAddr) || (name.nameAddr.addrType == remoteStationAddr))
//		netStr.Format( "%d", name.nameAddr.addrNet );
	if ((pname->m_bacnetaddr.addrType == remoteBroadcastAddr) || (pname->m_bacnetaddr.addrType == remoteStationAddr))
		netStr.Format( "%d", pname->m_bacnetaddr.addrNet );
	else
		netStr = _T("");
	m_NameList.SetItemText( elem, 2, netStr );

	// check for local or remote station
//MAD_DB	if ((name.nameAddr.addrType == localStationAddr) || (name.nameAddr.addrType == remoteStationAddr)) {
	if ((pname->m_bacnetaddr.addrType == localStationAddr) || (pname->m_bacnetaddr.addrType == remoteStationAddr)) {
		static const char	hex[] = "0123456789ABCDEF"
		;
		char	buff[kMaxAddressLen * 3], *s = buff
		;

		// clear the buffer
		buff[0] = 0;

		// encode the address
//MAD_DB		for (int i = 0; i < name.nameAddr.addrLen; i++) {
		for (int i = 0; i < pname->m_bacnetaddr.addrLen; i++) {
			if (i) *s++ = '-';
//			*s++ = hex[ name.nameAddr.addrAddr[i] >> 4 ];
//			*s++ = hex[ name.nameAddr.addrAddr[i] & 0x0F ];
			*s++ = hex[ pname->m_bacnetaddr.addrAddr[i] >> 4 ];
			*s++ = hex[ pname->m_bacnetaddr.addrAddr[i] & 0x0F ];
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

//MAD_DB void VTSNamesDlg::NameToCtrl( const VTSNameDesc &name )
void VTSNamesDlg::NameToCtrl( const VTSName * pname )
{
	// set the contents of the member vars from the record
	m_Name = pname->m_strName;
	m_AddrType = pname->m_bacnetaddr.addrType;

	m_Address = _T("");

	if ((pname->m_bacnetaddr.addrType == remoteBroadcastAddr) || (pname->m_bacnetaddr.addrType == remoteStationAddr))
		m_Network.Format( "%d", pname->m_bacnetaddr.addrNet );
	else
		m_Network = _T("");

	if ((pname->m_bacnetaddr.addrType == localStationAddr) || (pname->m_bacnetaddr.addrType == remoteStationAddr)) {
		static const char	hex[] = "0123456789ABCDEF"
		;
		char	buff[kMaxAddressLen * 3], *s = buff
		;

		// clear the buffer
		buff[0] = 0;

		// encode the address
		for (int i = 0; i < pname->m_bacnetaddr.addrLen; i++) {
			if (i) *s++ = '-';
			*s++ = hex[ pname->m_bacnetaddr.addrAddr[i] >> 4 ];
			*s++ = hex[ pname->m_bacnetaddr.addrAddr[i] & 0x0F ];
		}
		*s = 0;

		m_Address = buff;
	} else
		m_Address = _T("");


	m_PortCombo.SelectString(0, pname->m_pportLink == NULL ? "(any port)" : pname->m_pportLink->GetName());

/*
MAD_DB

	//fixed bug 598977, by xuyiping, 2002-9-24
	if (name.namePort == 0)
		m_PortCombo.SetCurSel(0);
	else
	{
		VTSPortPtr	curPort	;

		// look for the matching ID
		for (int i = 0; i < m_pPortList->Length(); i++) {
			// get a pointer to the ith port
			curPort = (*m_pPortList)[i];
			if (name.namePort == curPort->portDescID) {
				m_PortCombo.SetCurSel(i+1);
				break;
			}
		}
	}
	//End fixed
*/

	// sync the controls
	SynchronizeControls();

	// sync the dialog controls with the member variables
	UpdateData( false );
}

//
//	VTSNamesDlg::CtrlToName
//

//MAD_DB void VTSNamesDlg::CtrlToName( VTSNameDesc &name )
void VTSNamesDlg::CtrlToName( VTSName * pname )
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
//	strcpy( name.nameName, m_Name );
	pname->m_strName = m_Name;

	// do the address type
//	name.nameAddr.addrType = (BACnetAddressType)m_AddrType;
	pname->m_bacnetaddr.addrType = (BACnetAddressType)m_AddrType;

	// check the port
//	valu = cbp->GetCurSel();
//	name.namePort = (valu == 0 ? 0 : (*m_pPortList)[valu - 1]->portDescID);

	// must do more than assing pointer link... we've got to setup the name as well
	// so if the ports are changed this NAME object can relink when ports dialog is used.

	if ( cbp->GetCurSel() == 0 )
	{
		pname->m_pportLink = NULL;
		pname->m_strPortNameTemp.Empty();
	}
	else
	{
		pname->m_pportLink = (*m_pports)[cbp->GetCurSel() - 1];
		pname->m_strPortNameTemp = pname->m_pportLink->GetName();
	}


	// check for some remote broadcast or remote station
//	if ((name.nameAddr.addrType == remoteBroadcastAddr) || (name.nameAddr.addrType == remoteStationAddr)) {
	if ((pname->m_bacnetaddr.addrType == remoteBroadcastAddr) || (pname->m_bacnetaddr.addrType == remoteStationAddr)) {
		s = m_Network;

		if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) {
			s += 2;
			for (valu = 0; isxdigit(*s); s++)
				valu = (valu << 4) + (isdigit(*s) ? (*s - '0') : (toupper(*s) - 'A' + 10));
		} else {
			for (valu = 0; isdigit(*s); s++)
				valu = (valu *10) + (*s - '0');
		}

//		name.nameAddr.addrNet = valu;
		pname->m_bacnetaddr.addrNet = valu;
	}

	// check for local or remote station
//	if ((name.nameAddr.addrType == localStationAddr) || (name.nameAddr.addrType == remoteStationAddr)) {
	if ((pname->m_bacnetaddr.addrType == localStationAddr) || (pname->m_bacnetaddr.addrType == remoteStationAddr)) {
		int			upperNibble, lowerNibble
		;
		char		c
		;

		s = m_Address;

		// remove contents
//		name.nameAddr.addrLen = 0;
		pname->m_bacnetaddr.addrLen = 0;

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
//			if (name.nameAddr.addrLen < kMaxAddressLen)
//				name.nameAddr.addrAddr[ name.nameAddr.addrLen++ ] = (upperNibble << 4) + lowerNibble;
			if (pname->m_bacnetaddr.addrLen < kMaxAddressLen)
				pname->m_bacnetaddr.addrAddr[ pname->m_bacnetaddr.addrLen++ ] = (upperNibble << 4) + lowerNibble;
		}
	}
}

//
//	VTSNamesDlg::OnNew
//

void VTSNamesDlg::OnOK() 
{
	// First we have to validate the data...  so far, the only thing can could go wrong is
	// when we let them off of the address edit control having entered an invalid address
	// for specific types...  loop through all names and check 'em.

	for ( int n = 0; n < m_names.GetSize(); n++ )
	{
		VTSName * pname = (VTSName *) m_names[n];

		if ( (pname->m_bacnetaddr.addrType == localStationAddr  || pname->m_bacnetaddr.addrType == remoteStationAddr)
			 &&  pname->m_bacnetaddr.addrLen != 1 && pname->m_bacnetaddr.addrLen != 6 )
		{
			// select the item in the list...  then change focus to address edit control

			m_NameList.SetItemState( n, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
			m_AddressCtrl.SetFocus();

			AfxMessageBox(IDS_NAME_ADDRERROR, MB_ICONEXCLAMATION | MB_OK);
			return;  // don't allow OK
		}
	}

	// copy names array...   then copy array.  Elements remain allocated
	m_pnames->KillContents();

	// copy only the element references... changes ownership of element memory
	for ( int i = 0; i < m_names.GetSize(); i++ )
	{
		VTSName * p = (VTSName *) m_names[i];
		m_pnames->Add(m_names[i]);
	}

	// empty list but don't remove elements...  this will avoid destructor
	// killing the memory... which we've already transferred ownership to document's member
	// names list

	m_names.RemoveAll();

	CDialog::OnOK(); // This will close the dialog and DoModal will return.
}


void VTSNamesDlg::OnCancel() 
{
	// remove allocated temp elements...  destructor will kill array
	m_names.KillContents();

	CDialog::OnCancel(); // This will close the dialog and DoModal will return.
}



void VTSNamesDlg::OnNew() 
{
//	int		listLen = m_pNameList->Length();
//	VTSNameDesc		name;

	int listLen = m_names.GetSize();
	VTSName * pname = new VTSName();

	// deselect if something was selected
	POSITION	selPos = m_NameList.GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int	 nItem = m_NameList.GetNextSelectedItem( selPos );
		m_NameList.SetItemState( nItem, 0, LVIS_SELECTED );
	}
	
	// create a new name
//	m_pNameList->Add();
	m_names.Add(pname);

	// read the name from the database
//	m_pNameList->ReadName( listLen, &name );

	// make a placeholder for the item
//	m_NameList.InsertItem( listLen, _T("") );
	m_NameList.InsertItem( listLen, _T(pname->m_strName) );

	// copy the contents to the list
//	NameToList( name, listLen );
	NameToList( pname, listLen );

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
	POSITION	selPos = m_NameList.GetFirstSelectedItemPosition();

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
//	m_pNameList->Remove( nItem );
	m_names.Remove( nItem );

	// now select the next item if there is one... select the previous item if not... until null
	if ( nItem >= m_NameList.GetItemCount() )
		nItem = m_NameList.GetItemCount() - 1;

	if ( nItem >= 0 )
		m_NameList.SetItemState( nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
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
//	VTSNameDesc		name	;
	NM_LISTVIEW*	pNMListView = (NM_LISTVIEW*)pNMHDR;
	
#if 1
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
//		m_pNameList->ReadName( m_iSelectedName, &name );

		// transfer the values to the controls
//		NameToCtrl( name );
		NameToCtrl( (VTSName *) m_names[m_iSelectedName]);

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
//	VTSNameDesc		name;

	// read the name from the database
//	m_pNameList->ReadName( m_iSelectedName, &name );

	// transfer the contents of the controls to the record
//	CtrlToName( name );
	CtrlToName( (VTSName *) m_names[m_iSelectedName] );

	// save the updated record
//	m_pNameList->WriteName( m_iSelectedName, &name );

	// bring the list up-to-date
//	NameToList( name, m_iSelectedName );
	NameToList( (VTSName *) m_names[m_iSelectedName], m_iSelectedName );
}

//Xiao Shiyuan 2002-12-25
void VTSNamesDlg::OnKillfocusAddress() 
{
//	VTSNameDesc		name;

	//Xiao Shiyuan 2005-1-18
	if ( m_iSelectedName == -1 || m_iSelectedName > m_names.GetSize() - 1)
		return;

	VTSName * pname = (VTSName *) m_names[m_iSelectedName];

	// MAG 06JUN06 fix lost focus crash bug
	CWnd *cwt;
	int nFocusID=0;

	cwt = GetFocus();
	if(cwt != NULL)
		nFocusID = cwt->GetDlgCtrlID();  // GetFocus() sometimes returns NULL, i.e. when new focus is not VTS
	// MAG 06JUN06 end modifications

	// read the name from the database
//	m_pNameList->ReadName( m_iSelectedName, &name );

//	if(name.nameAddr.addrLen != 1 && name.nameAddr.addrLen != 6)

	// Check for validity and warn if we haven't already warned and they didn't 
	// press the cancel or delete key...

	if(    pname->m_bacnetaddr.addrLen != 1 && pname->m_bacnetaddr.addrLen != 6
		&& nFocusID != GetDlgItem(IDCANCEL)->GetDlgCtrlID()  &&  nFocusID != GetDlgItem(ID_DELETENAME)->GetDlgCtrlID() )
	{
		if ( !m_fWarnedAlready )
		{
			AfxMessageBox(IDS_NAME_ADDRERROR, MB_ICONEXCLAMATION | MB_OK);

//		this->MessageBox("Address length should be 1 or 6 bytes!\nMSTP Address length is 1 byte, B/IP and Ethernet MAC Address length are both 6 bytes.", "Error", MB_ICONERROR);
			m_AddressCtrl.SetFocus();
			m_fWarnedAlready = true;
		}
	}
}
