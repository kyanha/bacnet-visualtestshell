// SendWritePropMult.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"

#include "Send.h"
#include "SendWritePropMult.h"

#include "VTSObjectIdentifierDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace NetworkSniffer {
	extern char *BACnetPropertyIdentifier[];
}

void EncoderToHex( const BACnetAPDUEncoder &enc, CString &str );

BACnetAPDUEncoder CSendWritePropMult::pageContents;

/////////////////////////////////////////////////////////////////////////////
// CSendWritePropMult dialog

IMPLEMENT_DYNCREATE( CSendWritePropMult, CPropertyPage )

CSendWritePropMult::CSendWritePropMult( void )
	: CSendPage( CSendWritePropMult::IDD )
	, m_PropListList( this )
{
	//{{AFX_DATA_INIT(CSendWritePropMult)
	//}}AFX_DATA_INIT
}

void CSendWritePropMult::DoDataExchange(CDataExchange* pDX)
{
	WritePropListPtr	wplp = m_PropListList.wpllCurElem
	;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendWritePropMult)
	DDX_Control(pDX, IDC_OBJLIST, m_ObjList);
	DDX_Control(pDX, IDC_PROPLIST, m_PropList);
	//}}AFX_DATA_MAP

	// if there is a selected object, allow the ObjID to update
	if (wplp)
		wplp->wplObjID.UpdateData( pDX->m_bSaveAndValidate );

	// if there is a selected property, allow the controls to update
	if (wplp && wplp->wplCurElem) {
		wplp->wplCurElem->wpePropCombo.UpdateData( pDX->m_bSaveAndValidate );
		wplp->wplCurElem->wpeArrayIndex.UpdateData( pDX->m_bSaveAndValidate );
		wplp->wplCurElem->wpePriority.UpdateData( pDX->m_bSaveAndValidate );
	}
}

BEGIN_MESSAGE_MAP(CSendWritePropMult, CPropertyPage)
	//{{AFX_MSG_MAP(CSendWritePropMult)
	ON_BN_CLICKED(IDC_ADDOBJ, OnAddObj)
	ON_BN_CLICKED(IDC_REMOVEOBJ, OnRemoveObj)
	ON_EN_CHANGE(IDC_OBJID, OnChangeObjID)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_OBJLIST, OnItemchangingObjList)
	ON_BN_CLICKED(IDC_ADDPROP, OnAddProp)
	ON_BN_CLICKED(IDC_REMOVEPROP, OnRemoveProp)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_PROPLIST, OnItemchangingPropList)
	ON_CBN_SELCHANGE(IDC_PROPCOMBO, OnSelchangePropCombo)
	ON_EN_CHANGE(IDC_ARRAYINDEX, OnChangeArrayIndex)
	ON_BN_CLICKED(IDC_VALUE, OnValue)
	ON_EN_CHANGE(IDC_PRIORITYX, OnChangePriority)
	ON_BN_CLICKED(IDC_OBJECTIDBTN, OnObjectIDButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	CSendWritePropMult::InitPage
//

void CSendWritePropMult::InitPage( void )
{
	TRACE0( "CSendWritePropMult::InitPage()\n" );
}

//
//	CSendWritePropMult::EncodePage
//

void CSendWritePropMult::EncodePage( CByteArray* contents )
{
	CByteArray			header
	;
	BACnetAPDUEncoder	enc
	;

	// encode the service choice
	header.Add( 16 );

	// encode the contents
	m_PropListList.Encode( enc );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		header.Add( enc.pktBuffer[i] );

	// stuff the header on the front
	contents->InsertAt( 0, &header );
}

//
//	CSendWritePropMult::SavePage
//

void CSendWritePropMult::SavePage( void )
{
	TRACE0( "CSendWritePropMult::SavePage\n" );

	pageContents.Flush();

//	m_PropListList.SaveList( pageContents );
}

//
//	CSendWritePropMult::RestorePage
//

void CSendWritePropMult::RestorePage( void )
{
	BACnetAPDUDecoder	dec( pageContents )
	;

	TRACE0( "CSendWritePropMult::RestorePage\n" );

	if (dec.pktLength == 0)
		return;

//	m_PropListList.RestoreList( dec );
}

//
//	CSendWritePropMult::OnInitDialog
//

BOOL CSendWritePropMult::OnInitDialog() 
{
	TRACE0( "CSendWritePropMult::OnInitDialog()\n" );

	CDialog::OnInitDialog();
	
	// only allow one selection at a time, no sorting
	m_ObjList.m_nFlags |= LVS_SINGLESEL;
	m_ObjList.m_nFlags &= ~LBS_SORT;

	// initialize the object list
	m_ObjList.InsertColumn( 0, "Object ID", LVCFMT_LEFT, 128 );

	// only allow one selection at a time, no sorting
	m_PropList.m_nFlags |= LVS_SINGLESEL;
	m_PropList.m_nFlags &= ~LBS_SORT;

	// set up the property list columns
	m_PropList.InsertColumn( 0, "Property", LVCFMT_LEFT, 96 );
	m_PropList.InsertColumn( 1, "Index", LVCFMT_RIGHT, 48 );
	m_PropList.InsertColumn( 2, "Value", LVCFMT_LEFT, 96 );
	m_PropList.InsertColumn( 3, "Priority", LVCFMT_RIGHT, 48 );

	// disable the controls, they'll be enabled when an object is selected
	GetDlgItem( IDC_OBJID )->EnableWindow( false );
	GetDlgItem( IDC_OBJECTIDBTN )->EnableWindow( false );
	GetDlgItem( IDC_ADDPROP )->EnableWindow( false );
	GetDlgItem( IDC_REMOVEPROP )->EnableWindow( false );
	GetDlgItem( IDC_PROPCOMBO )->EnableWindow( false );
	GetDlgItem( IDC_ARRAYINDEX )->EnableWindow( false );
	GetDlgItem( IDC_VALUE )->EnableWindow( false );
	GetDlgItem( IDC_PRIORITYX )->EnableWindow( false );
	
	// load the enumeration table
	CComboBox	*cbp = (CComboBox *)GetDlgItem( IDC_PROPCOMBO );
	for (int i = 0; i < MAX_PROP_ID; i++)
		cbp->AddString( NetworkSniffer::BACnetPropertyIdentifier[i] );

	return TRUE;
}

void CSendWritePropMult::OnAddObj() 
{
	m_PropListList.AddButtonClick();
}

void CSendWritePropMult::OnRemoveObj() 
{
	m_PropListList.RemoveButtonClick();
}

void CSendWritePropMult::OnChangeObjID() 
{
	m_PropListList.OnChangeObjID();
}

void CSendWritePropMult::OnObjectIDButton() 
{
	m_PropListList.OnObjectIDButton();
}

void CSendWritePropMult::OnItemchangingObjList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_PropListList.OnItemChanging( pNMHDR, pResult );
}

void CSendWritePropMult::OnAddProp() 
{
	if (m_PropListList.wpllCurElem)
		m_PropListList.wpllCurElem->AddButtonClick();
}

void CSendWritePropMult::OnRemoveProp() 
{
	if (m_PropListList.wpllCurElem)
		m_PropListList.wpllCurElem->RemoveButtonClick();
}

void CSendWritePropMult::OnItemchangingPropList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (m_PropListList.wpllCurElem)
		m_PropListList.wpllCurElem->OnItemChanging( pNMHDR, pResult );
}

void CSendWritePropMult::OnSelchangePropCombo()
{
	if (m_PropListList.wpllCurElem)
		m_PropListList.wpllCurElem->OnSelchangePropCombo();
}

void CSendWritePropMult::OnChangeArrayIndex()
{
	if (m_PropListList.wpllCurElem)
		m_PropListList.wpllCurElem->OnChangeArrayIndex();
}

void CSendWritePropMult::OnValue() 
{
	if (m_PropListList.wpllCurElem)
		m_PropListList.wpllCurElem->OnValue();
}

void CSendWritePropMult::OnChangePriority()
{
	if (m_PropListList.wpllCurElem)
		m_PropListList.wpllCurElem->OnChangePriority();
}

//
//	WritePropElem::WritePropElem
//

WritePropElem::WritePropElem( CSendPagePtr wp )
	: wpePropCombo( wp, IDC_PROPCOMBO, NetworkSniffer::BACnetPropertyIdentifier, MAX_PROP_ID, true )
	, wpeArrayIndex( wp, IDC_ARRAYINDEX )
	, wpePriority( wp, IDC_PRIORITYX )
{
	// controls start out disabled
	wpePropCombo.ctrlEnabled = false;
	wpeArrayIndex.ctrlEnabled = false;
	wpePriority.ctrlEnabled = false;
}

//
//	WritePropElem::Bind
//

void WritePropElem::Bind( void )
{
	// set the control value to this element values
	wpePropCombo.ObjToCtrl();
	wpePropCombo.Enable();
	wpeArrayIndex.ObjToCtrl();
	wpeArrayIndex.Enable();

	wpePropCombo.ctrlWindow->GetDlgItem( IDC_VALUE )->EnableWindow( true );

	wpePriority.ObjToCtrl();
	wpePriority.Enable();
}

//
//	WritePropElem::Unbind
//

void WritePropElem::Unbind( void )
{
	// clear out the contents of the controls
	wpePropCombo.ctrlWindow->GetDlgItem( IDC_PROPCOMBO )->SetWindowText( "" );
	wpePropCombo.Disable();
	wpeArrayIndex.ctrlWindow->GetDlgItem( IDC_ARRAYINDEX )->SetWindowText( "" );
	wpeArrayIndex.Disable();

	wpePropCombo.ctrlWindow->GetDlgItem( IDC_VALUE )->EnableWindow( false );

	wpePriority.ctrlWindow->GetDlgItem( IDC_PRIORITYX )->SetWindowText( "" );
	wpePriority.Disable();
}

//
//	WritePropElem::Encode
//

void WritePropElem::Encode( BACnetAPDUEncoder& enc )
{
	// encode the property
	if (wpePropCombo.ctrlNull)
		throw "Property ID required";
	wpePropCombo.Encode( enc, 0 );

	// encode the (optional) array index
	if (!wpeArrayIndex.ctrlNull)
		wpeArrayIndex.Encode( enc, 1 );

	// do the value
	if (wpeValue.m_anyList.Length() < 1)
		throw "Value required";
	wpeValue.Encode( enc, 2 );

	// encode the (optional) priority
	if (!wpePriority.ctrlNull) {
		if ((wpePriority.uintValue < 1) || (wpePriority.uintValue > 16))
			throw "Priority out of range 1..16";
		wpePriority.Encode( enc, 3 );
	}
}

//
//	WritePropList::WritePropList
//

WritePropList::WritePropList( CSendWritePropMultPtr pp )
	: wplPagePtr( pp )
	, wplCurElem(0), wplCurElemIndx(0)
	, wplObjID( pp, IDC_OBJID )
{
	// give the object ID a default value
	wplObjID.ctrlEnabled = false;
	wplObjID.ctrlNull = false;
	wplObjID.objID = 0;
}

//
//	WritePropList::~WritePropList
//
//	If there have been any property value objects added to the list they need to 
//	be removed here.
//

WritePropList::~WritePropList( void )
{
	for (POSITION pos = GetHeadPosition(); pos != NULL; )
		delete GetNext( pos );
}

//
//	WritePropList::Bind
//

void WritePropList::Bind( void )
{
	int			i
	;
	CString		someText
	;

	// set the control value to this object id
	wplObjID.ObjToCtrl();
	wplObjID.Enable();
	wplPagePtr->GetDlgItem( IDC_OBJECTIDBTN )->EnableWindow( true );

	// fill out the table with the current list of elements
	i = 0;
	for (POSITION pos = GetHeadPosition(); pos != NULL; i++ ) {
		WritePropElemPtr	wpep = GetNext( pos )
		;

		wplPagePtr->m_PropList.InsertItem( i
			, NetworkSniffer::BACnetPropertyIdentifier[ wpep->wpePropCombo.enumValue ]
			);
		if (wpep->wpeArrayIndex.ctrlNull)
			wplPagePtr->m_PropList.SetItemText( i, 1, "" );
		else {
			someText.Format( "%d", wpep->wpeArrayIndex.uintValue );
			wplPagePtr->m_PropList.SetItemText( i, 1, someText );
		}

		BACnetAPDUEncoder enc;
		wpep->wpeValue.Encode( enc );
		EncoderToHex( enc, someText );
		wplPagePtr->m_PropList.SetItemText( i, 2, someText );

		if (wpep->wpePriority.ctrlNull)
			wplPagePtr->m_PropList.SetItemText( i, 3, "" );
		else {
			someText.Format( "%d", wpep->wpePriority.uintValue );
			wplPagePtr->m_PropList.SetItemText( i, 3, someText );
		}
	}

	// enable the other controls
	wplPagePtr->GetDlgItem( IDC_ADDPROP )->EnableWindow( true );
	wplPagePtr->GetDlgItem( IDC_REMOVEPROP )->EnableWindow( true );
}

//
//	WritePropList::Unbind
//

void WritePropList::Unbind( void )
{
	// clear out the contents of the object id control
	wplObjID.ctrlWindow->GetDlgItem( IDC_OBJID )->SetWindowText( "" );
	wplObjID.Disable();
	wplPagePtr->GetDlgItem( IDC_OBJECTIDBTN )->EnableWindow( false );

	// wipe out the contents of the table
	wplPagePtr->m_PropList.DeleteAllItems();

	// disable the other controls
	wplPagePtr->GetDlgItem( IDC_ADDPROP )->EnableWindow( false );
	wplPagePtr->GetDlgItem( IDC_REMOVEPROP )->EnableWindow( false );
}

//
//	WritePropList::AddButtonClick
//

void WritePropList::AddButtonClick( void )
{
	int		listLen = GetCount()
	;

	// deselect if something was selected
	POSITION selPos = wplPagePtr->m_PropList.GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int nItem = wplPagePtr->m_PropList.GetNextSelectedItem( selPos );
		wplPagePtr->m_PropList.SetItemState( nItem, 0, LVIS_SELECTED );
	}

	// create a new list item
	wplAddInProgress = true;
	wplPagePtr->m_PropList.InsertItem( listLen, "" );

	// create a new item, add to the end of the list
	wplCurElem = new WritePropElem( wplPagePtr );
	wplCurElemIndx = listLen;
	AddTail( wplCurElem );

	// bind the element to the controls
	wplCurElem->Bind();

	// update the encoding
	wplAddInProgress = false;
	wplPagePtr->UpdateEncoded();
}

//
//	WritePropList::RemoveButtonClick
//

void WritePropList::RemoveButtonClick( void )
{
	int					curRow = wplCurElemIndx
	;
	WritePropElemPtr	curElem = wplCurElem
	;

	// must have a selected row
	if (curRow < 0)
		return;

	// deselect the row, this will cause an unbind
	wplPagePtr->m_PropList.SetItemState( curRow, 0, LVIS_SELECTED );

	// delete the row from the list
	wplPagePtr->m_PropList.DeleteItem( curRow );

	// delete the element
	POSITION pos = FindIndex( curRow );
	delete GetAt( pos );
	RemoveAt( pos );

	// update the encoding
	wplPagePtr->UpdateEncoded();
}

//
//	WritePropList::OnSelchangePropCombo
//

void WritePropList::OnSelchangePropCombo( void )
{
	if (wplCurElem) {
		wplCurElem->wpePropCombo.UpdateData();
		wplPagePtr->UpdateEncoded();

		wplPagePtr->m_PropList.SetItemText( wplCurElemIndx, 0
			, NetworkSniffer::BACnetPropertyIdentifier[ wplCurElem->wpePropCombo.enumValue ]
			);
	}
}

//
//	WritePropList::OnChangeArrayIndex
//

void WritePropList::OnChangeArrayIndex( void )
{
	if (wplCurElem) {
		CString		someText
		;

		wplCurElem->wpeArrayIndex.UpdateData();
		wplPagePtr->UpdateEncoded();

		if (wplCurElem->wpeArrayIndex.ctrlNull)
			wplPagePtr->m_PropList.SetItemText( wplCurElemIndx, 1, "" );
		else {
			someText.Format( "%d", wplCurElem->wpeArrayIndex.uintValue );
			wplPagePtr->m_PropList.SetItemText( wplCurElemIndx, 1, someText );
		}
	}
}

//
//	WritePropList::OnChangeValue
//

void WritePropList::OnValue( void )
{
	if (wplCurElem) {
		CString				someText
		;
		BACnetAPDUEncoder	enc
		;

		wplCurElem->wpeValue.DoModal();
		wplPagePtr->UpdateEncoded();

		wplCurElem->wpeValue.Encode( enc );
		EncoderToHex( enc, someText );
		wplPagePtr->m_PropList.SetItemText( wplCurElemIndx, 2, someText );
	}
}

//
//	WritePropList::OnChangePriority
//

void WritePropList::OnChangePriority( void )
{
	if (wplCurElem) {
		CString				someText
		;

		wplCurElem->wpePriority.UpdateData();
		wplPagePtr->UpdateEncoded();

		if (wplCurElem->wpePriority.ctrlNull)
			wplPagePtr->m_PropList.SetItemText( wplCurElemIndx, 3, "" );
		else {
			someText.Format( "%d", wplCurElem->wpePriority.uintValue );
			wplPagePtr->m_PropList.SetItemText( wplCurElemIndx, 3, someText );
		}
	}
}

//
//	WritePropList::OnItemChanging
//

void WritePropList::OnItemChanging( NMHDR *pNMHDR, LRESULT *pResult )
{
	int					curRow = wplCurElemIndx
	;
	WritePropElemPtr	curElem = wplCurElem
	;
	NM_LISTVIEW*		pNMListView = (NM_LISTVIEW*)pNMHDR
	;

	// forget messages that don't change the selection state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	// skip messages during new item creation
	if (wplAddInProgress)
		return;

	if ((pNMListView->uNewState * LVIS_SELECTED) != 0) {
		// item becoming selected
		wplCurElemIndx = pNMListView->iItem;
		wplCurElem = GetAt( FindIndex( wplCurElemIndx ) );

		// bind the new current element
		wplCurElem->Bind();
	} else {
		// item no longer selected
		if (pNMListView->iItem == wplCurElemIndx) {
			// set nothing selected
			wplCurElem = 0;
			wplCurElemIndx = -1;

			// unbind from the controls
			curElem->Unbind();
		}
	}
}

//
//	WritePropList::Encode
//

void WritePropList::Encode( BACnetAPDUEncoder& enc )
{
	// encode the object ID
	if (wplObjID.ctrlNull)
		throw "Object ID required";
	wplObjID.Encode( enc, 0 );

	// encode each of the bound properties
	BACnetOpeningTag().Encode( enc, 1 );
	for (POSITION pos = GetHeadPosition(); pos != NULL; )
		GetNext( pos )->Encode( enc );
	BACnetClosingTag().Encode( enc, 1 );
}

//
//	WritePropListList::WritePropListList
//

WritePropListList::WritePropListList( CSendWritePropMultPtr pp )
	: wpllPagePtr( pp )
	, wpllCurElem(0), wpllCurElemIndx(-1)
{
}

//
//	WritePropListList::~WritePropListList
//
//	If there are any objects that have been added to the list they need to be 
//	deleted here.  The CList class will delete the stuff that it knows how to 
//	handle, but that doesn't include the WritePropList objects.
//

WritePropListList::~WritePropListList( void )
{
	for (POSITION pos = GetHeadPosition(); pos != NULL; )
		delete GetNext( pos );
}

//
//	WritePropListList::AddButtonClick
//
//	This procedure is called when the user wants to add an additional object to
//	the list.
//

void WritePropListList::AddButtonClick( void )
{
	int		listLen = GetCount()
	;

	// deselect if something was selected
	POSITION selPos = wpllPagePtr->m_ObjList.GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int nItem = wpllPagePtr->m_ObjList.GetNextSelectedItem( selPos );
		wpllPagePtr->m_ObjList.SetItemState( nItem, 0, LVIS_SELECTED );
	}

	// create a new list item
	wpllAddInProgress = true;
	wpllPagePtr->m_ObjList.InsertItem( listLen, "" );

	// create a new item, add to the end of the list
	wpllCurElem = new WritePropList( wpllPagePtr );
	wpllCurElemIndx = listLen;
	AddTail( wpllCurElem );

	// bind the element to the controls
	wpllCurElem->Bind();

	// update the encoding
	wpllAddInProgress = false;
	wpllPagePtr->UpdateEncoded();
}

//
//	WritePropListList::RemoveButtonClick
//

void WritePropListList::RemoveButtonClick( void )
{
	int					curRow = wpllCurElemIndx
	;
	WritePropListPtr	curElem = wpllCurElem
	;

	// must have a selected row
	if (curRow < 0)
		return;

	// deselect the row, this will cause an unbind
	wpllPagePtr->m_ObjList.SetItemState( curRow, 0, LVIS_SELECTED );

	// delete the row from the list
	wpllPagePtr->m_ObjList.DeleteItem( curRow );

	// delete the element
	POSITION pos = FindIndex( curRow );
	delete GetAt( pos );
	RemoveAt( pos );

	// update the encoding
	wpllPagePtr->UpdateEncoded();
}

//
//	WritePropListList::OnChangeObjID
//

void WritePropListList::OnChangeObjID( void )
{
	CString			rStr
	;

	// make sure we have a current object
	if (!wpllCurElem)
		return;

	// pass the message to the current selected object
	wpllCurElem->wplObjID.UpdateData();

	// get the control contents, update the list
	wpllPagePtr->GetDlgItem( IDC_OBJID )->GetWindowText( rStr );
	wpllPagePtr->m_ObjList.SetItemText( wpllCurElemIndx, 0, rStr );

	// update the encoding
	wpllPagePtr->UpdateEncoded();
}

//
//	WritePropListList::OnObjectIDButton
//

void WritePropListList::OnObjectIDButton( void )
{
	if (!wpllCurElem)
		return;

	VTSObjectIdentifierDialog	dlg
	;

	dlg.objID = wpllCurElem->wplObjID.objID;
	if (dlg.DoModal() && dlg.validObjID) {
		wpllCurElem->wplObjID.ctrlNull = false;
		wpllCurElem->wplObjID.objID = dlg.objID;
		wpllCurElem->wplObjID.ObjToCtrl();

		// copy the text from the control to the list
		CString text;
		wpllPagePtr->GetDlgItem( IDC_OBJID )->GetWindowText( text );
		wpllPagePtr->m_ObjList.SetItemText( wpllCurElemIndx, 0, text );

		wpllPagePtr->UpdateEncoded();
	}
}

//
//	WritePropListList::OnItemChanging
//

void WritePropListList::OnItemChanging( NMHDR *pNMHDR, LRESULT *pResult )
{
	int					curRow = wpllCurElemIndx
	;
	WritePropListPtr	curElem = wpllCurElem
	;
	NM_LISTVIEW*		pNMListView = (NM_LISTVIEW*)pNMHDR
	;

	// forget messages that don't change the selection state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	// skip messages during new item creation
	if (wpllAddInProgress)
		return;

	if ((pNMListView->uNewState * LVIS_SELECTED) != 0) {
		// item becoming selected
		wpllCurElemIndx = pNMListView->iItem;
		wpllCurElem = GetAt( FindIndex( wpllCurElemIndx ) );

		// bind the new current element
		wpllCurElem->Bind();
	} else {
		// item no longer selected
		if (pNMListView->iItem == wpllCurElemIndx) {
			// set nothing selected
			wpllCurElem = 0;
			wpllCurElemIndx = -1;

			// unbind from the controls
			curElem->Unbind();
		}
	}
}

//
//	WritePropListList::Encode
//
//	Each WritePropList element understands how to encode itself, so this procedure 
//	just calls Encode() for each of the objects in its list.
//

void WritePropListList::Encode( BACnetAPDUEncoder& enc )
{
	for (POSITION pos = GetHeadPosition(); pos != NULL; )
		GetNext( pos )->Encode( enc );
}
