// SendReinitDevice.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "SendReinitDevice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BACnetAPDUEncoder CSendReinitDevice::pageContents;

/////////////////////////////////////////////////////////////////////////////
// CSendReinitDevice property page

IMPLEMENT_DYNCREATE( CSendReinitDevice, CPropertyPage )

CSendReinitDevice::CSendReinitDevice( void )
	: CSendPage( CSendReinitDevice::IDD )
	, m_Password( this, IDC_PASSWORD )
{
	//{{AFX_DATA_INIT(CSendReinitDevice)
	m_State = 0;
	//}}AFX_DATA_INIT
}

void CSendReinitDevice::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendReinitDevice)
	DDX_Radio(pDX, IDC_COLDSTART, m_State);
	//}}AFX_DATA_MAP

	m_Password.UpdateData( pDX->m_bSaveAndValidate );
}

BEGIN_MESSAGE_MAP(CSendReinitDevice, CPropertyPage)
	//{{AFX_MSG_MAP(CSendReinitDevice)
	ON_BN_CLICKED(IDC_COLDSTART, OnColdStart)
	ON_BN_CLICKED(IDC_WARMSTART, OnWarmStart)
	ON_EN_CHANGE(IDC_PASSWORD, OnChangePassword)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	CSendReinitDevice::OnInitDialog
//

BOOL CSendReinitDevice::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	return TRUE;
}

//
//	CSendReinitDevice::InitPage
//

void CSendReinitDevice::InitPage( void )
{
	// flush the data
	m_Password.ctrlNull = true;
}

//
//	CSendReinitDevice::EncodePage
//

void CSendReinitDevice::EncodePage( CByteArray* contents )
{
	CByteArray			header
	;
	BACnetAPDUEncoder	enc
	;

	// encode the service choice
	header.Add( 20 );

	// encode the cold/warm start
	BACnetEnumerated( m_State ).Encode( enc, 0 );

	// encode the message
	if (!m_Password.ctrlNull)
		m_Password.Encode( enc, 1 );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		header.Add( enc.pktBuffer[i] );

	// stuff the header on the front
	contents->InsertAt( 0, &header );
}

//
//	CSendReinitDevice::SavePage
//

void CSendReinitDevice::SavePage( void )
{
	TRACE0( "CSendReinitDevice::SavePage\n" );

	pageContents.Flush();

	BACnetInteger( m_State ).Encode( pageContents );
	m_Password.SaveCtrl( pageContents );
}

//
//	CSendReinitDevice::RestorePage
//

void CSendReinitDevice::RestorePage( void )
{
	BACnetAPDUDecoder	dec( pageContents )
	;
	BACnetInteger		intValue
	;

	TRACE0( "CSendReinitDevice::RestorePage\n" );

	if (dec.pktLength == 0)
		return;

	intValue.Decode( dec );
	m_State = intValue.intValue;
	m_Password.RestoreCtrl( dec );
}

//
//	CSendReinitDevice::OnChangeX
//
//	The following set of functions are called when one of the interface elements
//	has changed.
//

void CSendReinitDevice::OnWarmStart() 
{
	UpdateData();
	SavePage();
	UpdateEncoded();
}

void CSendReinitDevice::OnColdStart() 
{
	UpdateData();
	SavePage();
	UpdateEncoded();
}

void CSendReinitDevice::OnChangePassword()
{
	m_Password.UpdateData();
	SavePage();
	UpdateEncoded();
}
