// VTSDevicesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "VTSDevicesDlg.h"
#include "VTSObjPropDialog.h"

#include "VTSValue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VTSDevicesDlg dialog


VTSDevicesDlg::VTSDevicesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(VTSDevicesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(VTSDevicesDlg)
	m_Segmentation = -1;
	//}}AFX_DATA_INIT
}

VTSDevicesDlg::VTSDevicesDlg( VTSDeviceListPtr dlp )
	: CDialog( VTSDevicesDlg::IDD, NULL )
	, m_pDeviceList( dlp )
	, m_iSelectedDevice( -1 )
{
	// duplicate the member initalization that the class 'wizard' adds
	m_Name = _T("");

	m_Instance = _T("");
	m_SegSize = _T("");
	m_WindowSize = _T("");
	m_NextInvokeID = _T("");
	m_MaxAPDUSize = _T("");
	m_APDUTimeout = _T("");
	m_APDUSegTimeout = _T("");
	m_APDURetries = _T("");
	m_VendorID = _T("");

	m_Segmentation = -1;
}

void VTSDevicesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSDevicesDlg)
	DDX_Control(pDX, IDC_DEVICELIST, m_DeviceList);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Text(pDX, IDC_INSTANCE, m_Instance);
	DDX_Text(pDX, IDC_SEGSIZE, m_SegSize);
	DDX_Text(pDX, IDC_WINDOWSIZE, m_WindowSize);
	DDX_Text(pDX, IDC_NEXTINVOKEID, m_NextInvokeID);
	DDX_Text(pDX, IDC_MAXAPDUSIZE, m_MaxAPDUSize);
	DDX_Text(pDX, IDC_APDUTIMEOUT, m_APDUTimeout);
	DDX_Text(pDX, IDC_APDUSEGTIMEOUT, m_APDUSegTimeout);
	DDX_Text(pDX, IDC_APDURETRIES, m_APDURetries);
	DDX_Text(pDX, IDC_VENDORID, m_VendorID);
	DDX_Radio(pDX, IDC_SEGBOTH, m_Segmentation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSDevicesDlg, CDialog)
	//{{AFX_MSG_MAP(VTSDevicesDlg)
	ON_BN_CLICKED(IDC_NEWDEVICE, OnNew)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_DEVICELIST, OnItemchangingDevicelist)
	ON_NOTIFY(NM_DBLCLK, IDC_DEVICELIST, OnDblclkDevicelist)
	ON_BN_CLICKED(IDC_IAM, OnIAm)
	ON_EN_CHANGE(IDC_NAME, SaveChanges)
	ON_EN_CHANGE(IDC_INSTANCE, SaveChanges)
	ON_EN_CHANGE(IDC_SEGSIZE, SaveChanges)
	ON_EN_CHANGE(IDC_WINDOWSIZE, SaveChanges)
	ON_EN_CHANGE(IDC_NEXTINVOKEID, SaveChanges)
	ON_EN_CHANGE(IDC_MAXAPDUSIZE, SaveChanges)
	ON_EN_CHANGE(IDC_APDUTIMEOUT, SaveChanges)
	ON_EN_CHANGE(IDC_APDUSEGTIMEOUT, SaveChanges)
	ON_EN_CHANGE(IDC_APDURETRIES, SaveChanges)
	ON_EN_CHANGE(IDC_VENDORID, SaveChanges)
	ON_BN_CLICKED(IDC_SEGBOTH, SaveChanges)
	ON_BN_CLICKED(IDC_SEGTRANSMIT, SaveChanges)
	ON_BN_CLICKED(IDC_SEGRECEIVE, SaveChanges)
	ON_BN_CLICKED(IDC_SEGNONE, SaveChanges)
	ON_BN_CLICKED(IDC_OBJPROP, OnObjProp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSDevicesDlg message handlers

BOOL VTSDevicesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	InitDeviceList();				// get device definitions from the database

	ResetSelection();				// nothing selected by default
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void VTSDevicesDlg::InitDeviceList()
{
	VTSDevicePtr	curDevice
	;
	int				len = m_pDeviceList->Length()
	;

	// only allow one selected name at a time
	m_DeviceList.m_nFlags |= LVS_SINGLESEL;

	// do not sort the list
	m_DeviceList.m_nFlags &= ~LBS_SORT;

	// add columns.
	m_DeviceList.InsertColumn (0, _T("Name"), LVCFMT_LEFT, 198 );
	m_DeviceList.InsertColumn (1, _T("Instance"), LVCFMT_LEFT, 96 );

	// add items and subitems
	for (int i = 0; i < m_pDeviceList->Length(); i++) {
		// get a pointer to the ith port
		curDevice = (*m_pDeviceList)[i];

		// make a placeholder for the item
		m_DeviceList.InsertItem( i, _T("") );

		// transfer the record contents to the list
		DeviceToList( curDevice, i );
	}
}

//
//	VTSDevicesDlg::SetSelection
//

void VTSDevicesDlg::SetSelection( int indx )
{
	VTSDevicePtr	curDevice
	;

	TRACE1( "SetSelection( %d )\n", indx );

	// if something is selected and its not this one, reset
	if ((m_iSelectedDevice >= 0) && (indx != m_iSelectedDevice))
		ResetSelection();

	// get a pointer to the port
	curDevice = (*m_pDeviceList)[indx];

	// set controls to content of record
	m_iSelectedDevice = indx;
	DeviceToCtrl( curDevice );

	// make sure the controls are properly enabled
	SynchronizeControls();

	// make sure the control values reflect the empty settings
	UpdateData( false );
}

void VTSDevicesDlg::ResetSelection()
{
	// no name selected
	m_iSelectedDevice = -1;

	// clear out the contents of the local variables
	m_Name = _T("");
	m_Instance = _T("");
	m_SegSize = _T("");
	m_WindowSize = _T("");
	m_NextInvokeID = _T("");
	m_MaxAPDUSize = _T("");
	m_APDUTimeout = _T("");
	m_APDUSegTimeout = _T("");
	m_APDURetries = _T("");
	m_VendorID = _T("");

	m_Segmentation = -1;

	// make sure the control values reflect the empty settings
	UpdateData( false );

	// make sure all the controls are properly disabled
	SynchronizeControls();
}

void VTSDevicesDlg::SynchronizeControls()
{
	// the name is enabled when there is something selected
	GetDlgItem( IDC_NAME )->EnableWindow( m_iSelectedDevice != -1 );

	GetDlgItem( IDC_INSTANCE )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_SEGSIZE )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_WINDOWSIZE )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_NEXTINVOKEID )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_MAXAPDUSIZE )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_APDUTIMEOUT )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_APDUSEGTIMEOUT )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_APDURETRIES )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_VENDORID )->EnableWindow( m_iSelectedDevice != -1 );

	GetDlgItem( IDC_SEGBOTH )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_SEGTRANSMIT )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_SEGRECEIVE )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_SEGNONE )->EnableWindow( m_iSelectedDevice != -1 );

	GetDlgItem( IDC_IAM )->EnableWindow( m_iSelectedDevice != -1 );
	GetDlgItem( IDC_OBJPROP )->EnableWindow( m_iSelectedDevice != -1 );
}

//
//	VTSDevicesDlg::DeviceToList
//

void VTSDevicesDlg::DeviceToList( VTSDevicePtr dp, int elem )
{
	CString		devInstTxt
	;

	// update the display list
	m_DeviceList.SetItemText( elem, 0, dp->devDesc.deviceName );

	devInstTxt.Format( "%d", dp->devDesc.deviceInstance );
	m_DeviceList.SetItemText( elem, 1, devInstTxt );
}

//
//	VTSDevicesDlg::DeviceToCtrl
//

void VTSDevicesDlg::DeviceToCtrl( VTSDevicePtr dp )
{
	// set the contents of the member vars from the record
	m_Name = dp->devDesc.deviceName;

	m_Instance.Format( "%d", dp->devDesc.deviceInstance );
	m_SegSize.Format( "%d", dp->devDesc.deviceSegmentSize );
	m_WindowSize.Format( "%d", dp->devDesc.deviceWindowSize );
	m_NextInvokeID.Format( "%d", dp->devDesc.deviceNextInvokeID );
	m_MaxAPDUSize.Format( "%d", dp->devDesc.deviceMaxAPDUSize );
	m_APDUTimeout.Format( "%d", dp->devDesc.deviceAPDUTimeout );
	m_APDUSegTimeout.Format( "%d", dp->devDesc.deviceAPDUSegmentTimeout );
	m_APDURetries.Format( "%d", dp->devDesc.deviceAPDURetries );
	m_VendorID.Format( "%d", dp->devDesc.deviceVendorID );

	m_Segmentation = dp->devDesc.deviceSegmentation;

	// sync the controls
	SynchronizeControls();

	// sync the dialog controls with the member variables
	UpdateData( false );
}

//
//	VTSDevicesDlg::CtrlToDevice
//

void VTSDevicesDlg::CtrlToDevice( VTSDevicePtr dp )
{
	// sync the member variables with the dialog controls
	UpdateData( true );

	// make sure the correct UI elements are enable/disabled
	SynchronizeControls();

	// copy the vars into the name
	strcpy( dp->devDesc.deviceName, m_Name );

	// do the rest
	sscanf( m_Instance, "%d", &dp->devDesc.deviceInstance );
	sscanf( m_SegSize, "%d", &dp->devDesc.deviceSegmentSize );
	sscanf( m_WindowSize, "%d", &dp->devDesc.deviceWindowSize );
	sscanf( m_NextInvokeID, "%d", &dp->devDesc.deviceNextInvokeID );
	sscanf( m_MaxAPDUSize, "%d", &dp->devDesc.deviceMaxAPDUSize );
	sscanf( m_APDUTimeout, "%d", &dp->devDesc.deviceAPDUTimeout );
	sscanf( m_APDUSegTimeout, "%d", &dp->devDesc.deviceAPDUSegmentTimeout );
	sscanf( m_APDURetries, "%d", &dp->devDesc.deviceAPDURetries );
	sscanf( m_VendorID, "%d", &dp->devDesc.deviceVendorID );

	dp->devDesc.deviceSegmentation = (BACnetSegmentation)m_Segmentation;
}

//
//	VTSDevicesDlg::OnNew
//

void VTSDevicesDlg::OnNew() 
{
	int				indx
	;
	VTSDevicePtr	curDevice
	;

	// deselect if something was selected
	POSITION	selPos = m_DeviceList.GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int	 nItem = m_DeviceList.GetNextSelectedItem( selPos );
		m_DeviceList.SetItemState( nItem, 0, LVIS_SELECTED );
	}
	
	// tell the list to add a new port
	m_pDeviceList->Add();

	// get its index, it will always be the last one
	indx = m_pDeviceList->Length() - 1;

	// get a pointer to the new port
	curDevice = (*m_pDeviceList)[indx];

	m_DeviceList.InsertItem( indx, _T("") );

	// copy the contents to the list
	DeviceToList( curDevice, indx );

	// make sure the record is visible and selected
	m_DeviceList.EnsureVisible( indx, false );
	m_DeviceList.SetItemState( indx, LVIS_SELECTED, LVIS_SELECTED );
}

//
//	VTSDevicesDlg::OnItemchangingDevicelist
//

void VTSDevicesDlg::OnItemchangingDevicelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW*	pNMListView = (NM_LISTVIEW*)pNMHDR
	;
	
	// forget messages that dont change anything
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0)
		SetSelection( pNMListView->iItem );
	else
	if (pNMListView->iItem == m_iSelectedDevice)
		ResetSelection();

	*pResult = 0;
}

//
//	VTSDevicesDlg::OnDblclkDevicelist
//

void VTSDevicesDlg::OnDblclkDevicelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// set the focus to the name for editing, select all of the text
	GetDlgItem( IDC_NAME )->SetFocus();
	((CEdit*)GetDlgItem( IDC_NAME ))->SetSel( 0, -1 );

	*pResult = 0;
}

//
//	VTSDevicesDlg::SaveChanges
//

void VTSDevicesDlg::SaveChanges()
{
	VTSDevicePtr	curDevice
	;

	// get a pointer to the device
	curDevice = (*m_pDeviceList)[m_iSelectedDevice];

	// transfer the contents of the controls to the record
	CtrlToDevice( curDevice );

	// save the updated record
	curDevice->WriteDesc();

	// bring the list up-to-date
	DeviceToList( curDevice, m_iSelectedDevice );
}

//
//	VTSDevicesDlg::OnIAm
//

void VTSDevicesDlg::OnIAm() 
{
	VTSDevicePtr	curDevice
	;

	// get a pointer to the device
	curDevice = (*m_pDeviceList)[m_iSelectedDevice];

	// tell it to send an I-Am
	curDevice->IAm();
}

//
//	VTSDevicesDlg::OnObjProp
//
//	Access to the built-in objects, properties and values.
//

void VTSDevicesDlg::OnObjProp() 
{
	VTSDevicePtr	curDevice
	;

	// get a pointer to the device
	curDevice = (*m_pDeviceList)[m_iSelectedDevice];

	// set the list busy while the user is editing
	curDevice->devObjPropValueList->SetBusy();

	// create a dialog for editing content
	VTSObjPropDialog	opd( curDevice->devObjPropValueList )
	;

	// let it run
	opd.DoModal();

	// all done with the list
	curDevice->devObjPropValueList->ResetBusy();
}
