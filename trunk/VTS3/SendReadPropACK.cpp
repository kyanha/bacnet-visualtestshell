// SendReadPropACK.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"

#include "Send.h"
#include "SendReadPropACK.h"

#include "VTSObjectIdentifierDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace NetworkSniffer {
	extern char *BACnetPropertyIdentifier[];
}

BACnetAPDUEncoder CSendReadPropACK::pageContents;

/////////////////////////////////////////////////////////////////////////////
// CSendReadPropACK dialog

IMPLEMENT_DYNCREATE( CSendReadPropACK, CPropertyPage )

CSendReadPropACK::CSendReadPropACK( void )
	: CSendPage( CSendReadPropACK::IDD )
	, m_ObjectID( this, IDC_OBJECTID )
	, m_PropCombo( this, IDC_PROPCOMBO, NetworkSniffer::BACnetPropertyIdentifier, MAX_PROP_ID, true )
	, m_ArrayIndex( this, IDC_ARRAYINDEX )
{
	//{{AFX_DATA_INIT(CSendReadPropACK)
	//}}AFX_DATA_INIT
}

void CSendReadPropACK::DoDataExchange(CDataExchange* pDX)
{
	TRACE1( "CSendReadPropACK::DoDataExchange(%d)\n", pDX->m_bSaveAndValidate );

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendReadPropACK)
	//}}AFX_DATA_MAP

	m_ObjectID.UpdateData( pDX->m_bSaveAndValidate );
	m_PropCombo.UpdateData( pDX->m_bSaveAndValidate );
	m_ArrayIndex.UpdateData( pDX->m_bSaveAndValidate );
}

BEGIN_MESSAGE_MAP(CSendReadPropACK, CPropertyPage)
	//{{AFX_MSG_MAP(CSendReadPropACK)
	ON_EN_CHANGE(IDC_OBJECTID, OnChangeObjectID)
	ON_CBN_SELCHANGE(IDC_PROPCOMBO, OnSelchangePropCombo)
	ON_EN_CHANGE(IDC_ARRAYINDEX, OnChangeArrayIndex)
	ON_BN_CLICKED(IDC_VALUE, OnValue)
	ON_BN_CLICKED(IDC_OBJECTIDBTN, OnObjectIDButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	CSendReadPropACK::OnInitDialog
//

BOOL CSendReadPropACK::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// load the enumeration table
	m_PropCombo.LoadCombo();
	
	return TRUE;
}

//
//	CSendReadPropACK::InitPage
//

void CSendReadPropACK::InitPage( void )
{
	// flush the data
	m_ObjectID.ctrlNull = true;
	m_PropCombo.ctrlNull = true;
	m_ArrayIndex.ctrlNull = true;
}

//
//	CSendReadPropACK::EncodePage
//

void CSendReadPropACK::EncodePage( CByteArray* contents )
{
	CByteArray			header
	;
	BACnetAPDUEncoder	enc
	;

	// encode the service choice
	header.Add( 12 );

	// encode the object ID
	if (m_ObjectID.ctrlNull)
		throw "Object ID required";
	m_ObjectID.Encode( enc, 0 );

	// encode the property
	if (m_PropCombo.ctrlNull)
		throw "Property ID required";
	m_PropCombo.Encode( enc, 1 );

	// encode the (optional) array index
	if (!m_ArrayIndex.ctrlNull)
		m_ArrayIndex.Encode( enc, 2 );

	// do the value
	if (m_Value.m_anyList.Length() < 1)
		throw "Value required";
	m_Value.Encode( enc, 3 );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		header.Add( enc.pktBuffer[i] );

	// stuff the header on the front
	contents->InsertAt( 0, &header );
}

//
//	CSendReadPropACK::SavePage
//

void CSendReadPropACK::SavePage( void )
{
	TRACE0( "CSendReadPropACK::SavePage\n" );

	pageContents.Flush();

	m_ObjectID.SaveCtrl( pageContents );
	m_PropCombo.SaveCtrl( pageContents );
	m_ArrayIndex.SaveCtrl( pageContents );
	m_Value.SaveCtrl( pageContents );
}

//
//	CSendReadPropACK::RestorePage
//

void CSendReadPropACK::RestorePage( void )
{
	BACnetAPDUDecoder	dec( pageContents )
	;

	TRACE0( "CSendReadPropACK::RestorePage\n" );

	if (dec.pktLength == 0)
		return;

	m_ObjectID.RestoreCtrl( dec );
	m_PropCombo.RestoreCtrl( dec );
	m_ArrayIndex.RestoreCtrl( dec );
	m_Value.RestoreCtrl( dec );
}

//
//	CSendReadPropACK::OnChangeX
//
//	The following set of functions are called when one of the interface elements
//	has changed.
//

void CSendReadPropACK::OnChangeObjectID()
{
	m_ObjectID.UpdateData();
	SavePage();
	UpdateEncoded();
}

void CSendReadPropACK::OnObjectIDButton() 
{
	VTSObjectIdentifierDialog	dlg
	;

	dlg.objID = m_ObjectID.objID;
	if (dlg.DoModal() && dlg.validObjID) {
		m_ObjectID.ctrlNull = false;
		m_ObjectID.objID = dlg.objID;
		m_ObjectID.ObjToCtrl();

		SavePage();
		UpdateEncoded();
	}
}

void CSendReadPropACK::OnSelchangePropCombo()
{
	m_PropCombo.UpdateData();
	SavePage();
	UpdateEncoded();
}

void CSendReadPropACK::OnChangeArrayIndex()
{
	m_ArrayIndex.UpdateData();
	SavePage();
	UpdateEncoded();
}

void CSendReadPropACK::OnValue() 
{
	m_Value.DoModal();
	SavePage();
	UpdateEncoded();
}
