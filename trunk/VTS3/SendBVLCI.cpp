// SendBVLCI.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"

#include "Send.h"
#include "SendBVLCI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BACnetAPDUEncoder CSendBVLCI::pageContents;

/////////////////////////////////////////////////////////////////////////////
// CSendBVLCI dialog

IMPLEMENT_DYNCREATE( CSendBVLCI, CPropertyPage )

CSendBVLCI::CSendBVLCI( void )
	: CSendPage( CSendBVLCI::IDD )
	, m_HeaderType(0)
	, m_OADR( this, IDC_OADRCOMBO, IDC_OADR )
{
	//{{AFX_DATA_INIT(CSendBVLCI)
	//}}AFX_DATA_INIT
}

void CSendBVLCI::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendBVLCI)
	DDX_Radio(pDX, IDC_NONE, m_HeaderType);
	//}}AFX_DATA_MAP

	m_OADR.UpdateData( pDX->m_bSaveAndValidate );
}

BEGIN_MESSAGE_MAP(CSendBVLCI, CPropertyPage)
	//{{AFX_MSG_MAP(CSendBVLCI)
	ON_BN_CLICKED(IDC_NONE, OnNone)
	ON_BN_CLICKED(IDC_DBTN, OnDistributeBroadcastToNetwork)
	ON_BN_CLICKED(IDC_UNICAST, OnOriginalUnicast)
	ON_BN_CLICKED(IDC_BROADCAST, OnOriginalBroadcast)
	ON_EN_CHANGE(IDC_OADR, OnChangeOADR)
	ON_BN_CLICKED(IDC_FORWARD, OnForwardedNPDU)
	ON_CBN_SELCHANGE(IDC_OADRCOMBO, OnSelchangeOADRCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	CSendBVLCI::OnInitDialog
//

BOOL CSendBVLCI::OnInitDialog() 
{
	TRACE0( "CSendBVLCI::OnInitDialog\n" );

	CPropertyPage::OnInitDialog();
	
	// load the combo
	m_OADR.LoadCombo( &pageParent->m_pPort->portDoc->m_Names, pageParent->m_pPort->portDescID );
	
	return TRUE;
}

//
//	CSendBVLCI::InitPage
//

void CSendBVLCI::InitPage( void )
{
	TRACE0( "CSendBVLCI::InitPage()\n" );

	m_OADR.ctrlNull = true;
}

//
//	CSendBVLCI::EncodePage
//

void CSendBVLCI::EncodePage( CByteArray* contents )
{
	int			len = contents->GetSize() + 4
	;
	CByteArray	header
	;

	TRACE0( "CSendBVLCI::EncodePage()\n" );

	// don't bother if there's no header
	if (m_HeaderType <= 0)
		return;

	// encode the message header
	header.Add( 0x81 );
	switch (m_HeaderType) {
		case 1:						// Distribute-Broadcast-To-Network
			header.Add( 0x09 );
			header.Add( len >> 8 );
			header.Add( len & 0xFF );
			break;
		case 2:						// Original-Unicast-NPDU
			header.Add( 0x0A );
			header.Add( len >> 8 );
			header.Add( len & 0xFF );
			break;
		case 3:						// Original-Broadcast-NPDU
			header.Add( 0x0B );
			header.Add( len >> 8 );
			header.Add( len & 0xFF );
			break;
		case 4:						// Forwarded-NPDU
			// validate
			if (m_OADR.ctrlNull || (m_OADR.addrLen != 6))
				throw "Originating device address required";

			// this is a little longer
			len += 6;

			// encode the header
			header.Add( 0x04 );
			header.Add( len >> 8 );
			header.Add( len & 0xFF );
			for (int i = 0; i < m_OADR.addrLen; i++)
				header.Add( m_OADR.addrAddr[i] );
			break;
	}

	// stuff the header on the front
	contents->InsertAt( 0, &header );
}

//
//	CSendBVLCI::SavePage
//

void CSendBVLCI::SavePage( void )
{
	TRACE0( "CSendBVLCI::SavePage\n" );

	pageContents.Flush();

	m_OADR.SaveCtrl( pageContents );
	BACnetInteger( m_HeaderType ).Encode( pageContents );
}

//
//	CSendBVLCI::RestorePage
//

void CSendBVLCI::RestorePage( void )
{
	BACnetAPDUDecoder	dec( pageContents )
	;
	BACnetInteger		hdrType
	;

	TRACE0( "CSendBVLCI::RestorePage\n" );

	if (dec.pktLength == 0)
		return;

	m_OADR.RestoreCtrl( dec );
	hdrType.Decode( dec );
	m_HeaderType = hdrType.intValue;
}

void CSendBVLCI::OnNone() 
{
	UpdateData();
	m_OADR.Disable();

	SavePage();
	UpdateEncoded();
}

void CSendBVLCI::OnDistributeBroadcastToNetwork() 
{
	UpdateData();
	m_OADR.Disable();

	SavePage();
	UpdateEncoded();
}

void CSendBVLCI::OnOriginalUnicast() 
{
	UpdateData();
	m_OADR.Disable();

	SavePage();
	UpdateEncoded();
}

void CSendBVLCI::OnOriginalBroadcast() 
{
	UpdateData();
	m_OADR.Disable();

	SavePage();
	UpdateEncoded();
}

void CSendBVLCI::OnForwardedNPDU() 
{
	UpdateData();
	m_OADR.Enable();

	SavePage();
	UpdateEncoded();
}

void CSendBVLCI::OnChangeOADR() 
{
	m_OADR.UpdateData();
	SavePage();
	UpdateEncoded();
}

void CSendBVLCI::OnSelchangeOADRCombo() 
{
	m_OADR.Selchange();
	SavePage();
	UpdateEncoded();
}
