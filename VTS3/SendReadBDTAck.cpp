// SendReadBDTAck.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "SendReadBDTAck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BACnetAPDUEncoder CSendReadBDTAck::pageContents;

/////////////////////////////////////////////////////////////////////////////
// CSendReadBDTAck property page

IMPLEMENT_DYNCREATE(CSendReadBDTAck, CPropertyPage)

CSendReadBDTAck::CSendReadBDTAck( void )
	: CSendPage( CSendReadBDTAck::IDD )
{
	//{{AFX_DATA_INIT(CSendReadBDTAck)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CSendReadBDTAck::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendReadBDTAck)
	DDX_Control(pDX, IDC_BDTLIST, m_BDTList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSendReadBDTAck, CPropertyPage)
	//{{AFX_MSG_MAP(CSendReadBDTAck)
	ON_BN_CLICKED(IDC_ADDBDT, OnAddBDT)
	ON_BN_CLICKED(IDC_REMOVEBDT, OnRemoveBDT)
	ON_EN_CHANGE(IDC_BDTENTRY, OnChangeBDTEntry)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_BDTLIST, OnItemchangingBDTList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	CSendReadBDTAck::InitPage
//

void CSendReadBDTAck::InitPage( void )
{
	TRACE0( "CSendReadBDTAck::InitPage()\n" );
}

//
//	CSendReadBDTAck::EncodePage
//

void CSendReadBDTAck::EncodePage( CByteArray* contents )
{
	int				len = m_BDTCtrl.GetItemCount()
	,				msgLen = len * 10 + 4
	;
	unsigned long	host, mask
	;
	unsigned short	port
	;
	CByteArray	header
	;

	TRACE0( "CSendReadBDTAck::EncodePage()\n" );

	// encode the message type and function
	header.Add( 0x81 );
	header.Add( 0x03 );

	// encode the length
	header.Add( msgLen >> 8 );
	header.Add( msgLen & 0xFF );

	// validate and encode the BDT entries
	for (int i = 0; i < len; i++) {
		const char *txt = m_BDTCtrl.GetItemText( i, 0 );

		// make sure something was provided
		if (!txt || !*txt)
			throw "Invalid BDT entry";

		// convert to host, port and network mask
		BACnetIPAddr::StringToHostPort( txt, &host, &mask, &port );

		// encode the host
		for (int j = 3; j >= 0; j--)
			header.Add( (unsigned char)((host >> (j * 8)) & 0xFF) );

		// encode the port
		header.Add( port >> 8 );
		header.Add( port & 0xFF );

		// encode the mask
		for (int k = 3; k >= 0; k--)
			header.Add( (unsigned char)((mask >> (k * 8)) & 0xFF) );
	}

	// stuff the header on the front
	contents->InsertAt( 0, &header );
}

//
//	CSendReadBDTAck::SavePage
//

void CSendReadBDTAck::SavePage( void )
{
	TRACE0( "CSendReadBDTAck::SavePage\n" );

	pageContents.Flush();

	// ### save list contents
}

//
//	CSendReadBDTAck::RestorePage
//

void CSendReadBDTAck::RestorePage( void )
{
	BACnetAPDUDecoder	dec( pageContents )
	;

	TRACE0( "CSendReadBDTAck::RestorePage\n" );

	if (dec.pktLength == 0)
		return;

	// ### restore list contents
}

//
//	CSendReadBDTAck::OnInitDialog
//

BOOL CSendReadBDTAck::OnInitDialog() 
{
	static VTSListColDesc colDesc[] =
		{ { "BDT Entry", LVCFMT_RIGHT, 128, IDC_BDTENTRY }
		, { 0, 0, 0, 0 }
		};

	TRACE0( "CSendReadBDTAck::OnInitDialog()\n" );

	CDialog::OnInitDialog();
	
	// only allow one selection at a time, no sorting
	m_BDTList.m_nFlags |= LVS_SINGLESEL;
	m_BDTList.m_nFlags &= ~LBS_SORT;

	// initialize the list
	m_BDTCtrl.Init( this, &m_BDTList, colDesc );

	return TRUE;
}

void CSendReadBDTAck::OnAddBDT() 
{
	m_BDTCtrl.AddButtonClick();
}

void CSendReadBDTAck::OnRemoveBDT() 
{
	m_BDTCtrl.RemoveButtonClick();
}

void CSendReadBDTAck::OnChangeBDTEntry() 
{
	m_BDTCtrl.OnChangeItem( IDC_BDTENTRY );
}

void CSendReadBDTAck::OnItemchangingBDTList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_BDTCtrl.OnItemChanging( pNMHDR, pResult );
}
