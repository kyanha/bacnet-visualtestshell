// SendMSTP.cpp: implementation of the CSendMSTP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vts.h"
#include "Send.h"
#include "SendMSTP.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


unsigned char MSTPCalcHeaderCRC( const unsigned char * pba, int len );
unsigned short MSTPCalcDataCRC( const unsigned char * pba, int len );


static const unsigned char MSTPHeaderCRC_table[256] = 
{
  0000,  0xfe,  0xff,  0x01,  0xfd,  0x03,  0x02,  0xfc,
  0xf9,  0x07,  0x06,  0xf8,  0x04,  0xfa,  0xfb,  0x05,
  0xf1,  0x0f,  0x0e,  0xf0,  0x0c,  0xf2,  0xf3,  0x0d,
  0x08,  0xf6,  0xf7,  0x09,  0xf5,  0x0b,  0x0a,  0xf4,
  0xe1,  0x1f,  0x1e,  0xe0,  0x1c,  0xe2,  0xe3,  0x1d,
  0x18,  0xe6,  0xe7,  0x19,  0xe5,  0x1b,  0x1a,  0xe4,
  0x10,  0xee,  0xef,  0x11,  0xed,  0x13,  0x12,  0xec,
  0xe9,  0x17,  0x16,  0xe8,  0x14,  0xea,  0xeb,  0x15,
  0xc1,  0x3f,  0x3e,  0xc0,  0x3c,  0xc2,  0xc3,  0x3d,
  0x38,  0xc6,  0xc7,  0x39,  0xc5,  0x3b,  0x3a,  0xc4,
  0x30,  0xce,  0xcf,  0x31,  0xcd,  0x33,  0x32,  0xcc,
  0xc9,  0x37,  0x36,  0xc8,  0x34,  0xca,  0xcb,  0x35,
  0x20,  0xde,  0xdf,  0x21,  0xdd,  0x23,  0x22,  0xdc,
  0xd9,  0x27,  0x26,  0xd8,  0x24,  0xda,  0xdb,  0x25,
  0xd1,  0x2f,  0x2e,  0xd0,  0x2c,  0xd2,  0xd3,  0x2d,
  0x28,  0xd6,  0xd7,  0x29,  0xd5,  0x2b,  0x2a,  0xd4,
  0x81,  0x7f,  0x7e,  0x80,  0x7c,  0x82,  0x83,  0x7d,
  0x78,  0x86,  0x87,  0x79,  0x85,  0x7b,  0x7a,  0x84,
  0x70,  0x8e,  0x8f,  0x71,  0x8d,  0x73,  0x72,  0x8c,
  0x89,  0x77,  0x76,  0x88,  0x74,  0x8a,  0x8b,  0x75,
  0x60,  0x9e,  0x9f,  0x61,  0x9d,  0x63,  0x62,  0x9c,
  0x99,  0x67,  0x66,  0x98,  0x64,  0x9a,  0x9b,  0x65,
  0x91,  0x6f,  0x6e,  0x90,  0x6c,  0x92,  0x93,  0x6d,
  0x68,  0x96,  0x97,  0x69,  0x95,  0x6b,  0x6a,  0x94,
  0x40,  0xbe,  0xbf,  0x41,  0xbd,  0x43,  0x42,  0xbc,
  0xb9,  0x47,  0x46,  0xb8,  0x44,  0xba,  0xbb,  0x45,
  0xb1,  0x4f,  0x4e,  0xb0,  0x4c,  0xb2,  0xb3,  0x4d,
  0x48,  0xb6,  0xb7,  0x49,  0xb5,  0x4b,  0x4a,  0xb4,
  0xa1,  0x5f,  0x5e,  0xa0,  0x5c,  0xa2,  0xa3,  0x5d,
  0x58,  0xa6,  0xa7,  0x59,  0xa5,  0x5b,  0x5a,  0xa4,
  0x50,  0xae,  0xaf,  0x51,  0xad,  0x53,  0x52,  0xac,
  0xa9,  0x57,  0x56,  0xa8,  0x54,  0xaa,  0xab,  0x55
};

const unsigned short MSTPDataCRC_table[256] = 
{
  000000,  0x1189,  0x2312,  0x329b,  0x4624,  0x57ad,  0x6536,  0x74bf,
  0x8c48,  0x9dc1,  0xaf5a,  0xbed3,  0xca6c,  0xdbe5,  0xe97e,  0xf8f7,
  0x1081,  0x0108,  0x3393,  0x221a,  0x56a5,  0x472c,  0x75b7,  0x643e,
  0x9cc9,  0x8d40,  0xbfdb,  0xae52,  0xdaed,  0xcb64,  0xf9ff,  0xe876,
  0x2102,  0x308b,  0x0210,  0x1399,  0x6726,  0x76af,  0x4434,  0x55bd,
  0xad4a,  0xbcc3,  0x8e58,  0x9fd1,  0xeb6e,  0xfae7,  0xc87c,  0xd9f5,
  0x3183,  0x200a,  0x1291,  0x0318,  0x77a7,  0x662e,  0x54b5,  0x453c,
  0xbdcb,  0xac42,  0x9ed9,  0x8f50,  0xfbef,  0xea66,  0xd8fd,  0xc974,
  0x4204,  0x538d,  0x6116,  0x709f,  0x0420,  0x15a9,  0x2732,  0x36bb,
  0xce4c,  0xdfc5,  0xed5e,  0xfcd7,  0x8868,  0x99e1,  0xab7a,  0xbaf3,
  0x5285,  0x430c,  0x7197,  0x601e,  0x14a1,  0x0528,  0x37b3,  0x263a,
  0xdecd,  0xcf44,  0xfddf,  0xec56,  0x98e9,  0x8960,  0xbbfb,  0xaa72,
  0x6306,  0x728f,  0x4014,  0x519d,  0x2522,  0x34ab,  0x0630,  0x17b9,
  0xef4e,  0xfec7,  0xcc5c,  0xddd5,  0xa96a,  0xb8e3,  0x8a78,  0x9bf1,
  0x7387,  0x620e,  0x5095,  0x411c,  0x35a3,  0x242a,  0x16b1,  0x0738,
  0xffcf,  0xee46,  0xdcdd,  0xcd54,  0xb9eb,  0xa862,  0x9af9,  0x8b70,
  0x8408,  0x9581,  0xa71a,  0xb693,  0xc22c,  0xd3a5,  0xe13e,  0xf0b7,
  0x0840,  0x19c9,  0x2b52,  0x3adb,  0x4e64,  0x5fed,  0x6d76,  0x7cff,
  0x9489,  0x8500,  0xb79b,  0xa612,  0xd2ad,  0xc324,  0xf1bf,  0xe036,
  0x18c1,  0x0948,  0x3bd3,  0x2a5a,  0x5ee5,  0x4f6c,  0x7df7,  0x6c7e,
  0xa50a,  0xb483,  0x8618,  0x9791,  0xe32e,  0xf2a7,  0xc03c,  0xd1b5,
  0x2942,  0x38cb,  0x0a50,  0x1bd9,  0x6f66,  0x7eef,  0x4c74,  0x5dfd,
  0xb58b,  0xa402,  0x9699,  0x8710,  0xf3af,  0xe226,  0xd0bd,  0xc134,
  0x39c3,  0x284a,  0x1ad1,  0x0b58,  0x7fe7,  0x6e6e,  0x5cf5,  0x4d7c,
  0xc60c,  0xd785,  0xe51e,  0xf497,  0x8028,  0x91a1,  0xa33a,  0xb2b3,
  0x4a44,  0x5bcd,  0x6956,  0x78df,  0x0c60,  0x1de9,  0x2f72,  0x3efb,
  0xd68d,  0xc704,  0xf59f,  0xe416,  0x90a9,  0x8120,  0xb3bb,  0xa232,
  0x5ac5,  0x4b4c,  0x79d7,  0x685e,  0x1ce1,  0x0d68,  0x3ff3,  0x2e7a,
  0xe70e,  0xf687,  0xc41c,  0xd595,  0xa12a,  0xb0a3,  0x8238,  0x93b1,
  0x6b46,  0x7acf,  0x4854,  0x59dd,  0x2d62,  0x3ceb,  0x0e70,  0x1ff9,
  0xf78f,  0xe606,  0xd49d,  0xc514,  0xb1ab,  0xa022,  0x92b9,  0x8330,
  0x7bc7,  0x6a4e,  0x58d5,  0x495c,  0x3de3,  0x2c6a,  0x1ef1,  0x0f78
};



#define BACnetLSAP		0x82

BACnetAPDUEncoder CSendMSTP::pageContents;


/////////////////////////////////////////////////////////////////////////////
// CSendMSTP dialog

IMPLEMENT_DYNCREATE( CSendMSTP, CPropertyPage )

#pragma warning( disable : 4355 )
CSendMSTP::CSendMSTP( void )
	: CSendPage( CSendMSTP::IDD )
	, m_SADR( this, IDC_SADRCOMBO, IDC_SADR )
	, m_DADR( this, IDC_DADRCOMBO, IDC_DADR )
{
	//{{AFX_DATA_INIT(CSendMSTP)
	//}}AFX_DATA_INIT
}
#pragma warning( default : 4355 )

void CSendMSTP::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendMSTP)
	//}}AFX_DATA_MAP

	m_SADR.UpdateData( pDX->m_bSaveAndValidate );
	m_DADR.UpdateData( pDX->m_bSaveAndValidate );
}

BEGIN_MESSAGE_MAP(CSendMSTP, CPropertyPage)
	//{{AFX_MSG_MAP(CSendMSTP)
	ON_EN_CHANGE(IDC_SADR, OnChangeSADR)
	ON_EN_CHANGE(IDC_DADR, OnChangeDADR)
	ON_CBN_SELCHANGE(IDC_DADRCOMBO, OnSelchangeDADRCombo)
	ON_CBN_SELCHANGE(IDC_SADRCOMBO, OnSelchangeSADRCombo)
	ON_CBN_SELCHANGE(IDC_MSTPFRTYPECOMBO, OnSelchangeMstpfrtypecombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	CSendMSTP::OnInitDialog
//

BOOL CSendMSTP::OnInitDialog() 
{
	TRACE0( "CSendMSTP::OnInitDialog()\n" );

	CPropertyPage::OnInitDialog();
	
	VTSDoc * pdoc = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();

	// load the combos
	if ( pdoc )
		m_SADR.LoadCombo( pdoc->GetNames(), pageParent->m_pPort );
	m_SADR.FindName( "TD" );

	if ( pdoc )
		m_DADR.LoadCombo( pdoc->GetNames(), pageParent->m_pPort );
	m_DADR.FindName( "IUT" );

	CComboBox * pcomboFrameType = (CComboBox *) GetDlgItem(IDC_MSTPFRTYPECOMBO);
	pcomboFrameType->AddString("Token");
	pcomboFrameType->AddString("Poll For Master");
	pcomboFrameType->AddString("Reply To Poll For Master");
	pcomboFrameType->AddString("Test Request");
	pcomboFrameType->AddString("Test Response");
	pcomboFrameType->AddString("Data Expecting Reply");
	pcomboFrameType->AddString("Data Not Expecting Reply");
	pcomboFrameType->AddString("Reply Postponed");

	pcomboFrameType->SetCurSel(5);
	return TRUE;
}

//
//	CSendMSTP::InitPage
//
//	This function will be called when the page is installed (see SetPageList()).
//

void CSendMSTP::InitPage( void )
{
	TRACE0( "CSendMSTP::InitPage()\n" );
}

//
//	CSendMSTP::EncodePage
//

void CSendMSTP::EncodePage( CByteArray* contents )
{
	unsigned short 	len;
	CByteArray	header;

	TRACE0( "CSendMSTP::EncodePage()\n" );

	// validate
	if (m_SADR.ctrlNull)
		throw "Source address required";
	if (m_SADR.addrLen != 1)
		throw "Invalid source address length for MS/TP (0-254)";

	if (m_DADR.ctrlNull)
		throw "Destination address required";
	if (m_DADR.addrLen != 1)
		throw "Invalid destination address length for MS/TP (0-254, 255=broadcast)";

	// get the length of the stuff already encoded, add SAP overhead
	len = (unsigned short) contents->GetSize();

	// stuff the octets on the front
	header.Add(0x55);		// Add preamble
	header.Add(0xFF);

	header.Add((unsigned char) ((CComboBox *) GetDlgItem(IDC_MSTPFRTYPECOMBO))->GetCurSel());
//	header.Add(((*contents)[1] & 0x04) ? 0x05 : 0x06);		// expecting reply?

	header.Add( m_DADR.addrAddr[0] );
	header.Add( m_SADR.addrAddr[0] );

	// encode the length
	header.Add( (len >> 8) & 0xFF );		// MS len
	header.Add( (len & 0xFF) );				// LS len

	// encode header CRC
	header.Add( ~MSTPCalcHeaderCRC((const unsigned char *) header.GetData() + 2, 5) );

	// use len to store data CRC
	len = ~MSTPCalcDataCRC((const unsigned char *) contents->GetData(), contents->GetSize());

	// stuff the header on the front
	contents->InsertAt( 0, &header );

	// Add data CRC, ls first
	contents->Add(len & 0xFF);
	contents->Add((len >> 8) & 0xff);
}




//
//	CSendMSTP::SavePage
//

void CSendMSTP::SavePage( void )
{
	TRACE0( "CSendMSTP::SavePage\n" );

	pageContents.Flush();

	m_SADR.SaveCtrl( pageContents );
	m_DADR.SaveCtrl( pageContents );
}

//
//	CSendMSTP::RestorePage
//

void CSendMSTP::RestorePage( int index )
{
	BACnetAPDUDecoder	dec( pageContents )
	;

	TRACE0( "CSendMSTP::RestorePage\n" );

	if (dec.pktLength == 0)
		return;

	m_SADR.RestoreCtrl( dec );
	m_DADR.RestoreCtrl( dec );
}

/////////////////////////////////////////////////////////////////////////////
// CSendMSTP message handlers


void CSendMSTP::OnChangeSADR() 
{
	m_SADR.UpdateData();
	SavePage();
	UpdateEncoded();
}

void CSendMSTP::OnChangeDADR() 
{
	m_DADR.UpdateData();
	SavePage();
	UpdateEncoded();
}

void CSendMSTP::OnSelchangeDADRCombo() 
{
	m_DADR.Selchange();
	SavePage();
	UpdateEncoded();
}

void CSendMSTP::OnSelchangeSADRCombo() 
{
	m_SADR.Selchange();
	SavePage();
	UpdateEncoded();
}


void CSendMSTP::OnSelchangeMstpfrtypecombo() 
{
	SavePage();
	UpdateEncoded();
}



unsigned char MSTPCalcHeaderCRC( const unsigned char * pba, int len )
{
	unsigned char headerCRC = 0xff;

	for ( int i = 0; i < len; i++ )
      headerCRC = MSTPHeaderCRC_table[headerCRC ^ pba[i]];

   return headerCRC;  
}



unsigned short MSTPCalcDataCRC( const unsigned char * pba, int len )
{
	unsigned short wCRC = 0xffff;

	for ( int i = 0; i < len; i++ )
      wCRC = (wCRC >> 8) ^ MSTPDataCRC_table[(wCRC & 0x00FF) ^ pba[i]];

   return wCRC;
}


