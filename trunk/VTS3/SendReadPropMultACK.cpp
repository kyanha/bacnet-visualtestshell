// SendReadPropMultACK.cpp : implementation file
//


#include "stdafx.h"
#include "VTS.h"


#include "Send.h"
#include "SendReadPropMultACK.h"

#include "VTSObjectIdentifierDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace NetworkSniffer {
	extern char *BACnetPropertyIdentifier[];
	extern char *BACnetErrorClass[];
	extern char *BACnetErrorCode[];
}

void EncoderToHex( const BACnetAPDUEncoder &enc, CString &str );

BACnetAPDUEncoder CSendReadPropMultACK::pageContents;

/////////////////////////////////////////////////////////////////////////////
// CSendReadPropMultACK dialog

IMPLEMENT_DYNCREATE( CSendReadPropMultACK, CPropertyPage )

#pragma warning( disable : 4355 )
CSendReadPropMultACK::CSendReadPropMultACK( void )
	: CSendPage( CSendReadPropMultACK::IDD )
	, m_PropListList( this )
{
	//{{AFX_DATA_INIT(CSendReadPropMultACK)
	//}}AFX_DATA_INIT
}
#pragma warning( default : 4355 )

void CSendReadPropMultACK::DoDataExchange(CDataExchange* pDX)
{
	ReadPropACKListPtr	rpalp = m_PropListList.rpallCurElem
	;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendReadPropMultACK)
	DDX_Control(pDX, IDC_OBJLIST, m_ObjList);
	DDX_Control(pDX, IDC_PROPLIST, m_PropList);
	//}}AFX_DATA_MAP

	// if there is a selected object, allow the ObjID to update
	if (rpalp)
		rpalp->rpalObjID.UpdateData( pDX->m_bSaveAndValidate );

	// if there is a selected property, allow the controls to update
	if (rpalp && rpalp->rpalCurElem) {
		rpalp->rpalCurElem->rpaePropCombo.UpdateData( pDX->m_bSaveAndValidate );
		rpalp->rpalCurElem->rpaeArrayIndex.UpdateData( pDX->m_bSaveAndValidate );
		rpalp->rpalCurElem->rpaeClassCombo.UpdateData( pDX->m_bSaveAndValidate );
		rpalp->rpalCurElem->rpaeCodeCombo.UpdateData( pDX->m_bSaveAndValidate );
	}
}

BEGIN_MESSAGE_MAP(CSendReadPropMultACK, CPropertyPage)
	//{{AFX_MSG_MAP(CSendReadPropMultACK)
	ON_BN_CLICKED(IDC_ADDOBJ, OnAddObj)
	ON_BN_CLICKED(IDC_REMOVEOBJ, OnRemoveObj)
	ON_BN_CLICKED(IDC_OBJUP, OnObjUp)
	ON_BN_CLICKED(IDC_OBJDOWN, OnObjDown)
	ON_EN_CHANGE(IDC_OBJID, OnChangeObjID)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_OBJLIST, OnItemchangingObjList)
	ON_BN_CLICKED(IDC_ADDPROP, OnAddProp)
	ON_BN_CLICKED(IDC_REMOVEPROP, OnRemoveProp)
	ON_BN_CLICKED(IDC_PROPUP, OnPropUp)
	ON_BN_CLICKED(IDC_PROPDOWN, OnPropDown)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_PROPLIST, OnItemchangingPropList)
	ON_CBN_SELCHANGE(IDC_PROPCOMBO, OnSelchangePropCombo)
	ON_EN_CHANGE(IDC_ARRAYINDEX, OnChangeArrayIndex)
	ON_BN_CLICKED(IDC_VALUE, OnValue)
	ON_CBN_SELCHANGE(IDC_ERRORCLASSCOMBO, OnSelchangeClassCombo)
	ON_CBN_SELCHANGE(IDC_ERRORCODECOMBO, OnSelchangeCodeCombo)
	ON_BN_CLICKED(IDC_OBJECTIDBTN, OnObjectIDButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	CSendReadPropMultACK::InitPage
//

void CSendReadPropMultACK::InitPage( void )
{
	TRACE0( "CSendReadPropMultACK::InitPage()\n" );
}

//
//	CSendReadPropMultACK::EncodePage
//

void CSendReadPropMultACK::EncodePage( CByteArray* contents )
{
	CByteArray			header
	;
	BACnetAPDUEncoder	enc
	;

	// encode the service choice
	header.Add( 14 );

	// encode the contents
	m_PropListList.Encode( enc );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		header.Add( enc.pktBuffer[i] );

	// stuff the header on the front
	contents->InsertAt( 0, &header );
}

//
//	CSendReadPropMultACK::SavePage
//

void CSendReadPropMultACK::SavePage( void )
{
	TRACE0( "CSendReadPropMultACK::SavePage\n" );

	pageContents.Flush();

	// ### save the list
}

//
//	CSendReadPropMultACK::RestorePage
//

void CSendReadPropMultACK::RestorePage( void )
{
	BACnetAPDUDecoder	dec( pageContents )
	;

	TRACE0( "CSendReadPropMultACK::RestorePage\n" );

	if (dec.pktLength == 0)
		return;

	// ### restore the list
}

//
//	CSendReadPropMultACK::OnInitDialog
//

BOOL CSendReadPropMultACK::OnInitDialog() 
{
	CComboBox	*cbp
	;

	TRACE0( "CSendReadPropMultACK::OnInitDialog()\n" );

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
	m_PropList.InsertColumn( 3, "Class", LVCFMT_RIGHT, 48 );
	m_PropList.InsertColumn( 4, "Code", LVCFMT_RIGHT, 48 );

	// disable the controls, they'll be enabled when an object is selected
	GetDlgItem( IDC_OBJID )->EnableWindow( false );
	GetDlgItem( IDC_OBJECTIDBTN )->EnableWindow( false );
	GetDlgItem( IDC_ADDPROP )->EnableWindow( false );
	GetDlgItem( IDC_REMOVEPROP )->EnableWindow( false );
	GetDlgItem( IDC_PROPCOMBO )->EnableWindow( false );
	GetDlgItem( IDC_ARRAYINDEX )->EnableWindow( false );
	GetDlgItem( IDC_VALUE )->EnableWindow( false );
	GetDlgItem( IDC_ERRORCLASSCOMBO )->EnableWindow( false );
	GetDlgItem( IDC_ERRORCODECOMBO )->EnableWindow( false );
	
	// load the property enumeration table
	cbp = (CComboBox *)GetDlgItem( IDC_PROPCOMBO );
	for (int i = 0; i < MAX_PROP_ID; i++)		  //MAX_PROP_ID is located in 
		cbp->AddString( NetworkSniffer::BACnetPropertyIdentifier[i] );

	// load the error class enumeration table
	cbp = (CComboBox *)GetDlgItem( IDC_ERRORCLASSCOMBO );
	for (int j = 0; j < 7; j++)
		cbp->AddString( NetworkSniffer::BACnetErrorClass[j] );

	// load the error code enumeration table
	cbp = (CComboBox *)GetDlgItem( IDC_ERRORCODECOMBO );
	for (int k = 0; k < 43; k++)
		cbp->AddString( NetworkSniffer::BACnetErrorCode[k] );

	return TRUE;
}

void CSendReadPropMultACK::OnAddObj() 
{
	m_PropListList.AddButtonClick();
}

void CSendReadPropMultACK::OnRemoveObj() 
{
	m_PropListList.RemoveButtonClick();
}

void CSendReadPropMultACK::OnObjUp() 
{
	m_PropListList.UpButtonClick();
}

void CSendReadPropMultACK::OnObjDown() 
{
	m_PropListList.DownButtonClick();
}

void CSendReadPropMultACK::OnChangeObjID() 
{
	m_PropListList.OnChangeObjID();
}

void CSendReadPropMultACK::OnObjectIDButton() 
{
	m_PropListList.OnObjectIDButton();
}

void CSendReadPropMultACK::OnItemchangingObjList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_PropListList.OnItemChanging( pNMHDR, pResult );
}

void CSendReadPropMultACK::OnAddProp() 
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->AddButtonClick();
}

void CSendReadPropMultACK::OnRemoveProp() 
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->RemoveButtonClick();
}

void CSendReadPropMultACK::OnPropUp() 
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->UpButtonClick();
}

void CSendReadPropMultACK::OnPropDown() 
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->DownButtonClick();
}

void CSendReadPropMultACK::OnItemchangingPropList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->OnItemChanging( pNMHDR, pResult );
}

void CSendReadPropMultACK::OnSelchangePropCombo()
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->OnSelchangePropCombo();
}

void CSendReadPropMultACK::OnChangeArrayIndex()
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->OnChangeArrayIndex();
}

void CSendReadPropMultACK::OnValue() 
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->OnValue();
}

void CSendReadPropMultACK::OnSelchangeClassCombo()
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->OnSelchangeClassCombo();
}

void CSendReadPropMultACK::OnSelchangeCodeCombo()
{
	if (m_PropListList.rpallCurElem)
		m_PropListList.rpallCurElem->OnSelchangeCodeCombo();
}

//
//	ReadPropACKElem::ReadPropACKElem
//

ReadPropACKElem::ReadPropACKElem( CSendPagePtr wp )
	: rpaePropCombo( wp, IDC_PROPCOMBO, NetworkSniffer::BACnetPropertyIdentifier, MAX_PROP_ID, true )
	, rpaeArrayIndex( wp, IDC_ARRAYINDEX )
	, rpaeClassCombo( wp, IDC_ERRORCLASSCOMBO, NetworkSniffer::BACnetErrorClass, 7, true )
	, rpaeCodeCombo( wp, IDC_ERRORCODECOMBO, NetworkSniffer::BACnetErrorCode, 43, true )
{
	// controls start out disabled
	rpaePropCombo.ctrlEnabled = false;
	rpaeArrayIndex.ctrlEnabled = false;
	rpaeClassCombo.ctrlEnabled = false;
	rpaeCodeCombo.ctrlEnabled = false;
}

//
//	ReadPropACKElem::Bind
//

void ReadPropACKElem::Bind( void )
{
	// set the control value to this element values
	rpaePropCombo.ObjToCtrl();
	rpaePropCombo.Enable();
	rpaeArrayIndex.ObjToCtrl();
	rpaeArrayIndex.Enable();

	rpaePropCombo.ctrlWindow->GetDlgItem( IDC_VALUE )->EnableWindow( true );

	rpaeClassCombo.ObjToCtrl();
	rpaeClassCombo.Enable();
	rpaeCodeCombo.ObjToCtrl();
	rpaeCodeCombo.Enable();
}

//
//	ReadPropACKElem::Unbind
//

void ReadPropACKElem::Unbind( void )
{
	// clear out the contents of the controls
	rpaePropCombo.ctrlWindow->GetDlgItem( IDC_PROPCOMBO )->SetWindowText( "" );
	rpaePropCombo.Disable();
	rpaeArrayIndex.ctrlWindow->GetDlgItem( IDC_ARRAYINDEX )->SetWindowText( "" );
	rpaeArrayIndex.Disable();

	rpaePropCombo.ctrlWindow->GetDlgItem( IDC_VALUE )->EnableWindow( false );

	rpaeClassCombo.ctrlWindow->GetDlgItem( IDC_ERRORCLASSCOMBO )->SetWindowText( "" );
	rpaeClassCombo.Disable();
	rpaeCodeCombo.ctrlWindow->GetDlgItem( IDC_ERRORCODECOMBO )->SetWindowText( "" );
	rpaeCodeCombo.Disable();
}

//
//	ReadPropACKElem::Encode
//

void ReadPropACKElem::Encode( BACnetAPDUEncoder& enc )
{
	// encode the property
	if (rpaePropCombo.ctrlNull)
		throw "Property ID required";
	rpaePropCombo.Encode( enc, 2 );

	// encode the (optional) array index
	if (!rpaeArrayIndex.ctrlNull)
		rpaeArrayIndex.Encode( enc, 3 );

	// do the value
	if (rpaeValue.m_anyList.Length() > 0) {
		if (!rpaeClassCombo.ctrlNull)
			throw "Omit error class with value";
		if (!rpaeCodeCombo.ctrlNull)
			throw "Omit error code with value";

		rpaeValue.Encode( enc, 4 );
	} else {
		if (rpaeClassCombo.ctrlNull)
			throw "Error class required with no value";
		if (rpaeCodeCombo.ctrlNull)
			throw "Error code required with no value";

		BACnetOpeningTag().Encode( enc, 5 );
		rpaeClassCombo.Encode( enc );
		rpaeCodeCombo.Encode( enc );
		BACnetClosingTag().Encode( enc, 5 );
	}
}

//
//	ReadPropACKList::ReadPropACKList
//

ReadPropACKList::ReadPropACKList( CSendReadPropMultACKPtr pp )
	: rpalPagePtr( pp )
	, rpalCurElem(0), rpalCurElemIndx(0)
	, rpalObjID( pp, IDC_OBJID )
{
	// give the object ID a default value
	rpalObjID.ctrlEnabled = false;
	rpalObjID.ctrlNull = false;
	rpalObjID.objID = 0;
}

//
//	ReadPropACKList::~ReadPropACKList
//
//	If there have been any property value objects added to the list they need to 
//	be removed here.
//

ReadPropACKList::~ReadPropACKList( void )
{
	for (POSITION pos = GetHeadPosition(); pos != NULL; )
		delete GetNext( pos );
}

//
//	ReadPropACKList::Bind
//

void ReadPropACKList::Bind( void )
{
	int			i
	;
	CString		someText
	;

	// set the control value to this object id
	rpalObjID.ObjToCtrl();
	rpalObjID.Enable();
	rpalPagePtr->GetDlgItem( IDC_OBJECTIDBTN )->EnableWindow( true );

	// clear out the current table contents
	rpalPagePtr->m_PropList.DeleteAllItems();

	// fill out the table with the current list of elements
	i = 0;
	for (POSITION pos = GetHeadPosition(); pos != NULL; i++ ) {
		ReadPropACKElemPtr	rpaep = GetNext( pos )
		;

		rpalPagePtr->m_PropList.InsertItem( i
			, NetworkSniffer::BACnetPropertyIdentifier[ rpaep->rpaePropCombo.enumValue ]
			);
		if (rpaep->rpaeArrayIndex.ctrlNull)
			rpalPagePtr->m_PropList.SetItemText( i, 1, "" );
		else {
			someText.Format( "%d", rpaep->rpaeArrayIndex.uintValue );
			rpalPagePtr->m_PropList.SetItemText( i, 1, someText );
		}

		BACnetAPDUEncoder enc;
		rpaep->rpaeValue.Encode( enc );
		EncoderToHex( enc, someText );
		rpalPagePtr->m_PropList.SetItemText( i, 2, someText );

		// ###
	}

	// enable the other controls
	rpalPagePtr->GetDlgItem( IDC_ADDPROP )->EnableWindow( true );
	rpalPagePtr->GetDlgItem( IDC_REMOVEPROP )->EnableWindow( true );
	rpalPagePtr->GetDlgItem( IDC_PROPUP )->EnableWindow( true );
	rpalPagePtr->GetDlgItem( IDC_PROPDOWN )->EnableWindow( true );
}

//
//	ReadPropACKList::Unbind
//

void ReadPropACKList::Unbind( void )
{
	// clear out the contents of the object id control
	rpalObjID.ctrlWindow->GetDlgItem( IDC_OBJID )->SetWindowText( "" );
	rpalObjID.Disable();
	rpalPagePtr->GetDlgItem( IDC_OBJECTIDBTN )->EnableWindow( false );

	// wipe out the contents of the table
	rpalPagePtr->m_PropList.DeleteAllItems();

	// disable the other controls
	rpalPagePtr->GetDlgItem( IDC_ADDPROP )->EnableWindow( false );
	rpalPagePtr->GetDlgItem( IDC_REMOVEPROP )->EnableWindow( false );
	rpalPagePtr->GetDlgItem( IDC_PROPUP )->EnableWindow( false );
	rpalPagePtr->GetDlgItem( IDC_PROPDOWN )->EnableWindow( false );
}

//
//	ReadPropACKList::AddButtonClick
//

void ReadPropACKList::AddButtonClick( void )
{
	int		listLen = GetCount()
	;

	// deselect if something was selected
	POSITION selPos = rpalPagePtr->m_PropList.GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int nItem = rpalPagePtr->m_PropList.GetNextSelectedItem( selPos );
		rpalPagePtr->m_PropList.SetItemState( nItem, 0, LVIS_SELECTED );
	}

	// create a new list item
	rpalAddInProgress = true;
	rpalPagePtr->m_PropList.InsertItem( listLen, "" );

	// create a new item, add to the end of the list
	rpalCurElem = new ReadPropACKElem( rpalPagePtr );
	rpalCurElemIndx = listLen;

	// madanner, 8/26/02.  Sourceforge bug #472392
	// Init property with 'Present_Value' from NetworkSniffer::BACnetPropertyIdentifier
	// Can't find mnemonic for Present Value... something like:  PRESENT_VALUE ??   So hard coding 85 will blow
	// if list is altered.

	rpalCurElem->rpaePropCombo.enumValue = 85;

	AddTail( rpalCurElem );

	// bind the element to the controls
	rpalCurElem->Bind();

	// update the encoding
	rpalAddInProgress = false;

	// madanner, 8/26/02.  Sourceforge bug #472392
	OnSelchangePropCombo();				// Insert new list text for Present_Value
//	rpalPagePtr->UpdateEncoded();		Commented because prior call to selchange updates.
}

//
//	ReadPropACKList::RemoveButtonClick
//

void ReadPropACKList::RemoveButtonClick( void )
{
	int					curRow = rpalCurElemIndx
	;
	ReadPropACKElemPtr	curElem = rpalCurElem
	;

	// must have a selected row
	if (curRow < 0)
		return;

	// deselect the row, this will cause an unbind
	rpalPagePtr->m_PropList.SetItemState( curRow, 0, LVIS_SELECTED );

	// delete the row from the list
	rpalPagePtr->m_PropList.DeleteItem( curRow );

	// delete the element
	POSITION pos = FindIndex( curRow );
	delete GetAt( pos );
	RemoveAt( pos );

	// update the encoding
	rpalPagePtr->UpdateEncoded();
}

//
//	ReadPropACKList::UpButtonClick
//

void ReadPropACKList::UpButtonClick( void )
{
	int					curRow = rpalCurElemIndx
	,					prevRow = curRow - 1
	;
	ReadPropACKElemPtr	curElem = rpalCurElem
	;

	// must have a selected row and a previous row
	if (curRow < 1)
		return;

	// move the row in the list
	rpalPagePtr->m_PropList.SetItemState( curRow, 0, LVIS_SELECTED );
	rpalPagePtr->m_PropList.DeleteItem( curRow );
	rpalPagePtr->m_PropList.InsertItem( prevRow, "?placeholder?" );

	// delete the element
	POSITION curPos = FindIndex( curRow );
	RemoveAt( curPos );
	POSITION prevPos = FindIndex( prevRow );
	InsertBefore( prevPos, curElem );

	// current element moved up
	rpalCurElemIndx -= 1;

	// select the row in its new position
	rpalPagePtr->m_PropList.SetItemState( prevRow, LVIS_SELECTED, LVIS_SELECTED );

	// make believe we have update events (dont ask why we dont!)
	OnSelchangePropCombo();
	OnChangeArrayIndex();
	if (rpalCurElem) {
		CString				someText
		;
		BACnetAPDUEncoder	enc
		;

		rpalCurElem->rpaeValue.Encode( enc );
		EncoderToHex( enc, someText );
		rpalPagePtr->m_PropList.SetItemText( rpalCurElemIndx, 2, someText );
	}
	OnSelchangeClassCombo();
	OnSelchangeCodeCombo();

	// update the encoding
	rpalPagePtr->UpdateEncoded();
}

//
//	ReadPropACKList::DownButtonClick
//

void ReadPropACKList::DownButtonClick( void )
{
	int					curRow = rpalCurElemIndx
	,					nextRow = curRow + 1
	;
	ReadPropACKElemPtr	curElem = rpalCurElem
	;

	// must have a selected row and a following row
	if ((curRow < 0) || (curRow >= GetCount()-1))
		return;

	// move the row in the list
	rpalPagePtr->m_PropList.SetItemState( curRow, 0, LVIS_SELECTED );
	rpalPagePtr->m_PropList.DeleteItem( curRow );
	rpalPagePtr->m_PropList.InsertItem( nextRow, "?placeholder?" );

	// delete the element
	POSITION curPos = FindIndex( curRow );
	RemoveAt( curPos );
	POSITION nextPos = FindIndex( curRow );
	InsertAfter( nextPos, curElem );

	// current element moved down
	rpalCurElemIndx += 1;

	// select the row in its new position
	rpalPagePtr->m_PropList.SetItemState( nextRow, LVIS_SELECTED, LVIS_SELECTED );

	// make believe we have update events (dont ask why we dont!)
	OnSelchangePropCombo();
	OnChangeArrayIndex();
	if (rpalCurElem) {
		CString				someText
		;
		BACnetAPDUEncoder	enc
		;

		rpalCurElem->rpaeValue.Encode( enc );
		EncoderToHex( enc, someText );
		rpalPagePtr->m_PropList.SetItemText( rpalCurElemIndx, 2, someText );
	}
	OnSelchangeClassCombo();
	OnSelchangeCodeCombo();

	// update the encoding
	rpalPagePtr->UpdateEncoded();
}

//
//	ReadPropACKList::OnSelchangePropCombo
//

void ReadPropACKList::OnSelchangePropCombo( void )
{
	if (rpalCurElem) {
		rpalCurElem->rpaePropCombo.UpdateData();
		rpalPagePtr->UpdateEncoded();

		rpalPagePtr->m_PropList.SetItemText( rpalCurElemIndx, 0
			, NetworkSniffer::BACnetPropertyIdentifier[ rpalCurElem->rpaePropCombo.enumValue ]
			);
	}
}

//
//	ReadPropACKList::OnChangeArrayIndex
//

void ReadPropACKList::OnChangeArrayIndex( void )
{
	if (rpalCurElem) {
		CString		someText
		;

		rpalCurElem->rpaeArrayIndex.UpdateData();
		rpalPagePtr->UpdateEncoded();

		if (rpalCurElem->rpaeArrayIndex.ctrlNull)
			rpalPagePtr->m_PropList.SetItemText( rpalCurElemIndx, 1, "" );
		else {
			someText.Format( "%d", rpalCurElem->rpaeArrayIndex.uintValue );
			rpalPagePtr->m_PropList.SetItemText( rpalCurElemIndx, 1, someText );
		}
	}
}

//
//	ReadPropACKList::OnChangeValue
//

void ReadPropACKList::OnValue( void )
{
	if (rpalCurElem) {
		CString				someText
		;
		BACnetAPDUEncoder	enc
		;

		rpalCurElem->rpaeValue.DoModal();
		rpalPagePtr->UpdateEncoded();

		rpalCurElem->rpaeValue.Encode( enc );
		EncoderToHex( enc, someText );
		rpalPagePtr->m_PropList.SetItemText( rpalCurElemIndx, 2, someText );
	}
}

//
//	ReadPropACKList::OnSelchangeClassCombo
//

void ReadPropACKList::OnSelchangeClassCombo( void )
{
	if (rpalCurElem) {
		rpalCurElem->rpaeClassCombo.UpdateData();
		rpalPagePtr->UpdateEncoded();

		rpalPagePtr->m_PropList.SetItemText( rpalCurElemIndx, 3
			, NetworkSniffer::BACnetErrorClass[ rpalCurElem->rpaeClassCombo.enumValue ]
			);
	}
}

//
//	ReadPropACKList::OnSelchangeCodeCombo
//

void ReadPropACKList::OnSelchangeCodeCombo( void )
{
	if (rpalCurElem) {
		rpalCurElem->rpaeCodeCombo.UpdateData();
		rpalPagePtr->UpdateEncoded();

		rpalPagePtr->m_PropList.SetItemText( rpalCurElemIndx, 4
			, NetworkSniffer::BACnetErrorCode[ rpalCurElem->rpaeCodeCombo.enumValue ]
			);
	}
}

//
//	ReadPropACKList::OnItemChanging
//

void ReadPropACKList::OnItemChanging( NMHDR *pNMHDR, LRESULT *pResult )
{
	int					curRow = rpalCurElemIndx
	;
	ReadPropACKElemPtr	curElem = rpalCurElem
	;
	NM_LISTVIEW*		pNMListView = (NM_LISTVIEW*)pNMHDR
	;

	// forget messages that don't change the selection state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	// skip messages during new item creation
	if (rpalAddInProgress)
		return;

	if ((pNMListView->uNewState * LVIS_SELECTED) != 0) {
		// item becoming selected
		rpalCurElemIndx = pNMListView->iItem;
		rpalCurElem = GetAt( FindIndex( rpalCurElemIndx ) );

		// bind the new current element
		rpalCurElem->Bind();
	} else {
		// item no longer selected
		if (pNMListView->iItem == rpalCurElemIndx) {
			// set nothing selected
			rpalCurElem = 0;
			rpalCurElemIndx = -1;

			// unbind from the controls
			curElem->Unbind();
		}
	}
}

//
//	ReadPropACKList::Encode
//

void ReadPropACKList::Encode( BACnetAPDUEncoder& enc )
{
	// encode the object ID
	if (rpalObjID.ctrlNull)
		throw "Object ID required";
	rpalObjID.Encode( enc, 0 );

	// encode each of the bound properties
	BACnetOpeningTag().Encode( enc, 1 );
	for (POSITION pos = GetHeadPosition(); pos != NULL; )
		GetNext( pos )->Encode( enc );
	BACnetClosingTag().Encode( enc, 1 );
}

//
//	ReadPropACKListList::ReadPropACKListList
//

ReadPropACKListList::ReadPropACKListList( CSendReadPropMultACKPtr pp )
	: rpallPagePtr( pp )
	, rpallCurElem(0), rpallCurElemIndx(-1)
{
}

//
//	ReadPropACKListList::~ReadPropACKListList
//
//	If there are any objects that have been added to the list they need to be 
//	deleted here.  The CList class will delete the stuff that it knows how to 
//	handle, but that doesn't include the ReadPropACKList objects.
//

ReadPropACKListList::~ReadPropACKListList( void )
{
	for (POSITION pos = GetHeadPosition(); pos != NULL; )
		delete GetNext( pos );
}

//
//	ReadPropACKListList::AddButtonClick
//
//	This procedure is called when the user wants to add an additional object to
//	the list.
//

void ReadPropACKListList::AddButtonClick( void )
{
	int		listLen = GetCount()
	;

	// deselect if something was selected
	POSITION selPos = rpallPagePtr->m_ObjList.GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int nItem = rpallPagePtr->m_ObjList.GetNextSelectedItem( selPos );
		rpallPagePtr->m_ObjList.SetItemState( nItem, 0, LVIS_SELECTED );
	}

	// create a new list item
	rpallAddInProgress = true;
	rpallPagePtr->m_ObjList.InsertItem( listLen, "" );

	// create a new item, add to the end of the list
	rpallCurElem = new ReadPropACKList( rpallPagePtr );
	rpallCurElemIndx = listLen;
	AddTail( rpallCurElem );

	// bind the element to the controls
	rpallCurElem->Bind();

	// update the encoding
	rpallAddInProgress = false;
	rpallPagePtr->UpdateEncoded();
}

//
//	ReadPropACKListList::RemoveButtonClick
//

void ReadPropACKListList::RemoveButtonClick( void )
{
	int					curRow = rpallCurElemIndx
	;
	ReadPropACKListPtr	curElem = rpallCurElem
	;

	// must have a selected row
	if (curRow < 0)
		return;

	// deselect the row, this will cause an unbind
	rpallPagePtr->m_ObjList.SetItemState( curRow, 0, LVIS_SELECTED );

	// delete the row from the list
	rpallPagePtr->m_ObjList.DeleteItem( curRow );

	// delete the element
	POSITION pos = FindIndex( curRow );
	delete GetAt( pos );
	RemoveAt( pos );

	// update the encoding
	rpallPagePtr->UpdateEncoded();
}

//
//	ReadPropACKListList::UpButtonClick
//

void ReadPropACKListList::UpButtonClick( void )
{
	int					curRow = rpallCurElemIndx
	,					prevRow = curRow - 1
	;
	ReadPropACKListPtr	curElem = rpallCurElem
	;

	// must have a selected row and a previous row
	if (curRow < 1)
		return;

	// move the row in the list
	rpallPagePtr->m_ObjList.SetItemState( curRow, 0, LVIS_SELECTED );
	rpallPagePtr->m_ObjList.DeleteItem( curRow );
	rpallPagePtr->m_ObjList.InsertItem( prevRow, "?placeholder?" );

	// delete the element
	POSITION curPos = FindIndex( curRow );
	RemoveAt( curPos );
	POSITION prevPos = FindIndex( prevRow );
	InsertBefore( prevPos, curElem );

	// current element moved up
	rpallCurElemIndx -= 1;

	// select the row in its new position
	rpallPagePtr->m_ObjList.SetItemState( prevRow, LVIS_SELECTED, LVIS_SELECTED );

	// make believe we have update events
	OnChangeObjID();

	// update the encoding
	rpallPagePtr->UpdateEncoded();
}

//
//	ReadPropACKListList::DownButtonClick
//

void ReadPropACKListList::DownButtonClick( void )
{
	int					curRow = rpallCurElemIndx
	,					nextRow = curRow + 1
	;
	ReadPropACKListPtr	curElem = rpallCurElem
	;

	// must have a selected row and a following row
	if ((curRow < 0) || (curRow >= GetCount()-1))
		return;

	// move the row in the list
	rpallPagePtr->m_ObjList.SetItemState( curRow, 0, LVIS_SELECTED );
	rpallPagePtr->m_ObjList.DeleteItem( curRow );
	rpallPagePtr->m_ObjList.InsertItem( nextRow, "?placeholder?" );

	// delete the element
	POSITION curPos = FindIndex( curRow );
	RemoveAt( curPos );
	POSITION nextPos = FindIndex( curRow );
	InsertAfter( nextPos, curElem );

	// current element moved down
	rpallCurElemIndx += 1;

	// select the row in its new position
	rpallPagePtr->m_ObjList.SetItemState( nextRow, LVIS_SELECTED, LVIS_SELECTED );

	// make believe we have update events (dont ask why we dont!)
	OnChangeObjID();

	// update the encoding
	rpallPagePtr->UpdateEncoded();
}

//
//	ReadPropACKListList::OnChangeObjID
//

void ReadPropACKListList::OnChangeObjID( void )
{
	CString			rStr
	;

	// make sure we have a current object
	if (!rpallCurElem)
		return;

	// pass the message to the current selected object
	rpallCurElem->rpalObjID.UpdateData();

	// get the control contents, update the list
	rpallPagePtr->GetDlgItem( IDC_OBJID )->GetWindowText( rStr );
	rpallPagePtr->m_ObjList.SetItemText( rpallCurElemIndx, 0, rStr );

	// update the encoding
	rpallPagePtr->UpdateEncoded();
}

//
//	ReadPropACKListList::OnObjectIDButton
//

void ReadPropACKListList::OnObjectIDButton( void )
{
	if (!rpallCurElem)
		return;

	VTSObjectIdentifierDialog	dlg
	;

	dlg.objID = rpallCurElem->rpalObjID.objID;
	if (dlg.DoModal() && dlg.validObjID) {
		rpallCurElem->rpalObjID.ctrlNull = false;
		rpallCurElem->rpalObjID.objID = dlg.objID;
		rpallCurElem->rpalObjID.ObjToCtrl();

		// copy the text from the control to the list
		CString text;
		rpallPagePtr->GetDlgItem( IDC_OBJID )->GetWindowText( text );
		rpallPagePtr->m_ObjList.SetItemText( rpallCurElemIndx, 0, text );

		rpallPagePtr->UpdateEncoded();
	}
}

//
//	ReadPropACKListList::OnItemChanging
//

void ReadPropACKListList::OnItemChanging( NMHDR *pNMHDR, LRESULT *pResult )
{
	int					curRow = rpallCurElemIndx
	;
	ReadPropACKListPtr	curElem = rpallCurElem
	;
	NM_LISTVIEW*		pNMListView = (NM_LISTVIEW*)pNMHDR
	;

	// forget messages that don't change the selection state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	// skip messages during new item creation
	if (rpallAddInProgress)
		return;

	if ((pNMListView->uNewState * LVIS_SELECTED) != 0) {
		// item becoming selected
		rpallCurElemIndx = pNMListView->iItem;
		rpallCurElem = GetAt( FindIndex( rpallCurElemIndx ) );

		// bind the new current element
		rpallCurElem->Bind();
	} else {
		// item no longer selected
		if (pNMListView->iItem == rpallCurElemIndx) {
			// set nothing selected
			rpallCurElem = 0;
			rpallCurElemIndx = -1;

			// unbind from the controls
			curElem->Unbind();
		}
	}
}

//
//	ReadPropACKListList::Encode
//
//	Each ReadPropACKList element understands how to encode itself, so this procedure 
//	just calls Encode() for each of the objects in its list.
//

void ReadPropACKListList::Encode( BACnetAPDUEncoder& enc )
{
	for (POSITION pos = GetHeadPosition(); pos != NULL; )
		GetNext( pos )->Encode( enc );
}
