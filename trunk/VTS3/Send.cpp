// Send.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"

#include "Send.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	IDC_PORT		0x1001
#define IDC_PACKETTREE	0x1002
#define IDC_PACKETDATA	0x1003
#define IDC_SEND		0x1004

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gIPItemList[] =
	{ { "Raw Data", (CSendPageMPtr)&CSend::RawPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gIPGroup = { "IP", gIPItemList, (CSendPageMPtr)&CSend::IPPage, (CSendPageMPtr)&CSend::IPPage };

CSendGroupPtr gIPGroupList[] =
	{ &gIPGroup, &gBVLLGroup
	, &gNetworkGroup
	, &gAlarmEventAccessGroup
	, &gFileAccessGroup
	, &gObjectAccessGroup
	, &gRemoteDevMgmtGroup
	, &gVirtualTerminalGroup
	, &gSimpleACKGroup
	, &gErrorGroup
	, 0
	};

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gBVLLItemList[] =
	{ { "BVLC-Result", (CSendPageMPtr)&CSend::BVLCResultPage, 0 }
	, { "Write-BDT", (CSendPageMPtr)&CSend::WriteBDTPage, 0 }
	, { "Read-BDT", (CSendPageMPtr)&CSend::ReadBDTPage, 0 }
	, { "Read-BDT-Ack", (CSendPageMPtr)&CSend::ReadBDTAckPage, 0 }
	, { "Register-FD", (CSendPageMPtr)&CSend::RegisterFDPage, 0 }
	, { "Read-FDT", (CSendPageMPtr)&CSend::ReadFDTPage, 0 }
	, { "Read-FDT-Ack", (CSendPageMPtr)&CSend::ReadFDTAckPage, 0 }
	, { "Delete-FDT-Entry", (CSendPageMPtr)&CSend::DeleteFDTEntryPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gBVLLGroup = { "BVLL", gBVLLItemList, 0, (CSendPageMPtr)&CSend::BVLCIPage };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gEthernetItemList[] =
	{ { "Raw Data", (CSendPageMPtr)&CSend::RawPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gEthernetGroup = { "Ethernet", gEthernetItemList, (CSendPageMPtr)&CSend::EnetPage, (CSendPageMPtr)&CSend::EnetPage };

CSendGroupPtr gEthernetGroupList[] =
	{ &gEthernetGroup
	, &gNetworkGroup
	, &gAlarmEventAccessGroup
	, &gFileAccessGroup
	, &gObjectAccessGroup
	, &gRemoteDevMgmtGroup
	, &gVirtualTerminalGroup
	, &gSimpleACKGroup
	, &gErrorGroup
	, 0
	};

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gARCNETItemList[] =
	{ { "Raw Data", (CSendPageMPtr)&CSend::RawPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gARCNETGroup = { "ARCNET", gARCNETItemList, (CSendPageMPtr)&CSend::ARCNETPage, 0 };

CSendGroupPtr gARCNETGroupList[] = { &gARCNETGroup, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gMSTPItemList[] =
	{ { "Raw Data", (CSendPageMPtr)&CSend::RawPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gMSTPGroup = { "MS/TP", gMSTPItemList, (CSendPageMPtr)&CSend::MSTPPage, 0 };

CSendGroupPtr gMSTPGroupList[] = { &gMSTPGroup, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gPTPItemList[] =
	{ { "Raw Data", (CSendPageMPtr)&CSend::RawPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gPTPGroup = { "PTP", gPTPItemList, (CSendPageMPtr)&CSend::PTPPage, 0 };

CSendGroupPtr gPTPGroupList[] = { &gPTPGroup, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gNetworkItemList[] =
	{ { "Raw Data", (CSendPageMPtr)&CSend::RawPage, 0 }
	, { "Vendor NPDU", (CSendPageMPtr)&CSend::VendorNPDUPage, 0 }
	, { "Who-Is-Router-To-Network", (CSendPageMPtr)&CSend::WhoIsRTNPage, 0 }
	, { "I-Am-Router-To-Network", (CSendPageMPtr)&CSend::IAmRTNPage, 0 }
	, { "I-Could-Be-Router-To-Network", (CSendPageMPtr)&CSend::ICouldBeRTNPage, 0 }
	, { "Reject-Message-To-Network", (CSendPageMPtr)&CSend::RejectMTNPage, 0 }
	, { "Router-Busy-To-Network", (CSendPageMPtr)&CSend::RouterBusyPage, 0 }
	, { "Router-Available-To-Network", (CSendPageMPtr)&CSend::RouterAvailablePage, 0 }
	, { "Initialize-Routing-Table", (CSendPageMPtr)&CSend::InitRTPage, 0 }
	, { "Initialize-Routing-Table-ACK", (CSendPageMPtr)&CSend::InitRTAckPage, 0 }
	, { "Establish-Connection-To-Network", (CSendPageMPtr)&CSend::EstablishCTNPage, 0 }
	, { "Disconnect-Connection-To-Network", (CSendPageMPtr)&CSend::DisconnectCTNPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gNetworkGroup = { "Network", gNetworkItemList, (CSendPageMPtr)&CSend::NPCIPage, (CSendPageMPtr)&CSend::NPCIPage };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gAlarmEventItemList[] =
	{ { "AcknowledgeAlarm", (CSendPageMPtr)&CSend::AckAlarmPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "ConfirmedCOVNotification", (CSendPageMPtr)&CSend::ConfCOVNotificationPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "UnconfirmedCOVNotification", (CSendPageMPtr)&CSend::UnconfCOVNotificationPage, 0 }
	, { "GetAlarmSummary", (CSendPageMPtr)&CSend::GetAlarmSummaryPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "GetAlarmSummary-ACK", (CSendPageMPtr)&CSend::GetAlarmSummaryACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { "ConfirmedEventNotification", (CSendPageMPtr)&CSend::ConfEventNotificationPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "UnconfirmedEventNotification", (CSendPageMPtr)&CSend::UnconfEventNotificationPage, 0 }
	, { "GetEnrollmentSummary", (CSendPageMPtr)&CSend::GetEnrollmentSummaryPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "GetEnrollmentSummary-ACK", (CSendPageMPtr)&CSend::GetEnrollmentSummaryACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { "SubscribeCOV", (CSendPageMPtr)&CSend::SubscribeCOVPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { 0, 0, 0 }
	};

CSendGroup gAlarmEventAccessGroup = { "Alarm and Event", gAlarmEventItemList, 0, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gFileAccessItemList[] =
	{ { "AtomicReadFile", (CSendPageMPtr)&CSend::ReadFilePage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "AtomicReadFile-ACK", (CSendPageMPtr)&CSend::ReadFileACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { "AtomicWriteFile", (CSendPageMPtr)&CSend::WriteFilePage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "AtomicWriteFile-ACK", (CSendPageMPtr)&CSend::WriteFileACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { 0, 0, 0 }
	};

CSendGroup gFileAccessGroup = { "File Access", gFileAccessItemList, 0, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gObjectAccessItemList[] =
	{ { "AddListElement", (CSendPageMPtr)&CSend::AddListElementPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "ChangeList-Error", (CSendPageMPtr)&CSend::ChangeListErrorPage, 0 }
	, { "CreateObject", (CSendPageMPtr)&CSend::CreateObjectPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "CreateObject-ACK", (CSendPageMPtr)&CSend::CreateObjectACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { "CreateObject-Error", (CSendPageMPtr)&CSend::CreateObjectErrorPage, 0 }
	, { "DeleteObject", (CSendPageMPtr)&CSend::DeleteObjectPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "ReadProperty", (CSendPageMPtr)&CSend::ReadPropPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "ReadProperty-ACK", (CSendPageMPtr)&CSend::ReadPropACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { "ReadPropertyMultiple", (CSendPageMPtr)&CSend::ReadPropMultPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "ReadPropertyMultiple-ACK", (CSendPageMPtr)&CSend::ReadPropMultACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { "ReadRange", (CSendPageMPtr)&CSend::ReadRangePage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "RemoveListElement", (CSendPageMPtr)&CSend::RemoveListElementPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "WriteProperty", (CSendPageMPtr)&CSend::WritePropPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "WritePropertyMultiple", (CSendPageMPtr)&CSend::WritePropMultPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "WritePropertyMultiple-Error", (CSendPageMPtr)&CSend::WritePropMultErrorPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gObjectAccessGroup = { "Object Access", gObjectAccessItemList, 0, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gRemoteDevMgmtItemList[] =
	{ { "DeviceCommunicationControl", (CSendPageMPtr)&CSend::DeviceCommCtrlPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
//	, { "ConfirmedPrivateTransfer", (CSendPageMPtr)0, 0 }
//	, { "ConfirmedPrivateTransfer-Ack", (CSendPageMPtr)0, 0 }
	, { "ConfirmedTextMessage", (CSendPageMPtr)&CSend::ConfTextMsgPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "I-Am", (CSendPageMPtr)&CSend::IAmPage, 0 }
	, { "I-Have", (CSendPageMPtr)&CSend::IHavePage, 0 }
	, { "ReinitializeDevice", (CSendPageMPtr)&CSend::ReinitDevicePage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
//	, { "UnconfirmedPrivateTransfer", (CSendPageMPtr)0, 0 }
	, { "UnconfirmedTextMessage", (CSendPageMPtr)&CSend::UnconfTextMsgPage, 0 }
	, { "TimeSynchronization", (CSendPageMPtr)&CSend::TimeSyncPage, 0 }
	, { "Who-Has", (CSendPageMPtr)&CSend::WhoHasPage, 0 }
	, { "Who-Is", (CSendPageMPtr)&CSend::WhoIsPage, 0 }
	, { "UTCTimeSynchronization", (CSendPageMPtr)&CSend::UTCTimeSyncPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gRemoteDevMgmtGroup = { "Remote Device Management", gRemoteDevMgmtItemList, 0, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gVirtualTerminalItemList[] =
	{ { "VT-Open", (CSendPageMPtr)&CSend::VTOpenPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "VT-Open-ACK", (CSendPageMPtr)&CSend::VTOpenACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { "VT-Close", (CSendPageMPtr)&CSend::VTClosePage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "VT-Close-Error", (CSendPageMPtr)&CSend::VTCloseErrorPage, 0 }
	, { "VT-Data", (CSendPageMPtr)&CSend::VTDataPage, (CSendPageMPtr)&CSend::ConfirmedRequestPage }
	, { "VT-Data-ACK", (CSendPageMPtr)&CSend::VTDataACKPage, (CSendPageMPtr)&CSend::ComplexACKPage }
	, { 0, 0, 0 }
	};

CSendGroup gVirtualTerminalGroup = { "Virtual Terminal", gVirtualTerminalItemList, 0, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gSimpleACKItemList[] =
	{ { "SimpleACK", (CSendPageMPtr)&CSend::SimpleACKPage, 0 }
	, { "SegmentACK", (CSendPageMPtr)&CSend::SegmentACKPage, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gSimpleACKGroup = { "Simple/Segment ACK", gSimpleACKItemList, 0, 0 };

/////////////////////////////////////////////////////////////////////////////

CSendGroupItem gErrorItemList[] =
	{ { "Error", (CSendPageMPtr)&CSend::ErrorPage, 0 }
	, { "Reject", (CSendPageMPtr)&CSend::RejectPage, 0 }
	, { "Abort", (CSendPageMPtr)&CSend::AbortPage, 0 }
//	, { "ConfirmedPrivateTransfer-Error", (CSendPageMPtr)0, 0 }
	, { 0, 0, 0 }
	};

CSendGroup gErrorGroup = { "Errors", gErrorItemList, 0, 0 };

/////////////////////////////////////////////////////////////////////////////
// CSend

IMPLEMENT_DYNAMIC(CSend, CPropertySheet)

CSend::CSend(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	InitPages();
}

CSend::CSend(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	InitPages();
}

CSend::~CSend()
{
}

void CSend::InitPages( void )
{
	// no item selected
	m_currentItem = 0;

	// initialize the pages
	NullPage.pageParent = this;
	RawPage.pageParent = this;
	TestPage.pageParent = this;

	IPPage.pageParent = this;

	BVLCIPage.pageParent = this;
	BVLCResultPage.pageParent = this;
	WriteBDTPage.pageParent = this;
	ReadBDTPage.pageParent = this;
	ReadBDTAckPage.pageParent = this;
	RegisterFDPage.pageParent = this;
	ReadFDTPage.pageParent = this;
	ReadFDTAckPage.pageParent = this;
	DeleteFDTEntryPage.pageParent = this;

	EnetPage.pageParent = this;

	NPCIPage.pageParent = this;
	VendorNPDUPage.pageParent = this;
	WhoIsRTNPage.pageParent = this;
	IAmRTNPage.pageParent = this;
	ICouldBeRTNPage.pageParent = this;
	RejectMTNPage.pageParent = this;
	RouterBusyPage.pageParent = this;
	RouterAvailablePage.pageParent = this;
	InitRTPage.pageParent = this;
	InitRTAckPage.pageParent = this;
	EstablishCTNPage.pageParent = this;
	DisconnectCTNPage.pageParent = this;

	ConfirmedRequestPage.pageParent = this;
	ComplexACKPage.pageParent = this;

	AckAlarmPage.pageParent = this;
	ConfCOVNotificationPage.pageParent = this;
	UnconfCOVNotificationPage.pageParent = this;
	GetAlarmSummaryPage.pageParent = this;
	GetAlarmSummaryACKPage.pageParent = this;
	ConfEventNotificationPage.pageParent = this;
	UnconfEventNotificationPage.pageParent = this;
	GetEnrollmentSummaryPage.pageParent = this;
	GetEnrollmentSummaryACKPage.pageParent = this;
	SubscribeCOVPage.pageParent = this;

	ReadFilePage.pageParent = this;
	ReadFileACKPage.pageParent = this;
	WriteFilePage.pageParent = this;
	WriteFileACKPage.pageParent = this;

	AddListElementPage.pageParent = this;
	CreateObjectPage.pageParent = this;
	CreateObjectACKPage.pageParent = this;
	CreateObjectErrorPage.pageParent = this;
	DeleteObjectPage.pageParent = this;
	ReadPropPage.pageParent = this;
	ReadPropACKPage.pageParent = this;
	ReadPropMultPage.pageParent = this;
	ReadPropMultACKPage.pageParent = this;
	ReadRangePage.pageParent = this;
	RemoveListElementPage.pageParent = this;
	WritePropPage.pageParent = this;
	WritePropMultPage.pageParent = this;
	WritePropMultErrorPage.pageParent = this;

	DeviceCommCtrlPage.pageParent = this;
	ConfTextMsgPage.pageParent = this;
	IAmPage.pageParent = this;
	IHavePage.pageParent = this;
	ReinitDevicePage.pageParent = this;
	UnconfTextMsgPage.pageParent = this;
	TimeSyncPage.pageParent = this;
	UTCTimeSyncPage.pageParent = this;
	WhoHasPage.pageParent = this;
	WhoIsPage.pageParent = this;

	VTOpenPage.pageParent = this;
	VTOpenACKPage.pageParent = this;
	VTClosePage.pageParent = this;
	VTCloseErrorPage.pageParent = this;
	VTDataPage.pageParent = this;
	VTDataACKPage.pageParent = this;

	SimpleACKPage.pageParent = this;
	SegmentACKPage.pageParent = this;

	ErrorPage.pageParent = this;
	RejectPage.pageParent = this;
	AbortPage.pageParent = this;
	ChangeListErrorPage.pageParent = this;

	// install the null page
	AddPage( &NullPage );
	m_pages[0] = &NullPage;
	m_pages[1] = 0;

#if 1
	// install the test page
	AddPage( &TestPage );
	m_pages[1] = &TestPage;
	m_pages[2] = 0;
#endif
}

//
//	If anybody could tell me why I seem to need both A and W versions of the messages
//	it would be appreciated.
//

BEGIN_MESSAGE_MAP(CSend, CPropertySheet)
	//{{AFX_MSG_MAP(CSend)
	ON_CBN_SELCHANGE( IDC_PORT, OnSelchangePort )
	ON_NOTIFY( TVN_SELCHANGEDA, IDC_PACKETTREE, OnSelchangePacketTree )
	ON_NOTIFY( TVN_SELCHANGEDW, IDC_PACKETTREE, OnSelchangePacketTree )
	ON_NOTIFY( TVN_ITEMEXPANDEDA, IDC_PACKETTREE, OnItemExpandedPacketTree)
	ON_NOTIFY( TVN_ITEMEXPANDEDW, IDC_PACKETTREE, OnItemExpandedPacketTree)
	ON_BN_CLICKED( IDC_SEND, OnSend )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSend encoding functions

//
//	CSend::UpdateEncoded
//

void CSend::UpdateEncoded( void )
{
	static const char	hex[] = "0123456789ABCDEF"
	;
	int					lastPage, i, x
	;
	CString				enc
	;
	CByteArray			contents
	;

	TRACE0( "CSend::UpdateEncoded()\n" );

	// find the last page
	for (lastPage = -1; m_pages[lastPage+1]; lastPage++)
		;

	// encode the pages
	try {
		for (i = lastPage; i >= 0; i--)
			m_pages[i]->EncodePage( &contents );
		m_send.EnableWindow( true );
	}
	catch (char *errTxt) {
		m_packetData.SetWindowText( errTxt );
		m_send.EnableWindow( false );
		return;
	}

	// translate contents into hex stream
	for (i = 0; i < contents.GetSize(); i++ ) {
		if ((i > 0) && ((i % 4) == 0))
			enc += ' ';
		if ((i > 0) && ((i % 16) == 0)) {
			enc += 0x0D;
			enc += 0x0A;
		}
		x = contents.GetAt( i );
		enc += hex[ x >> 4 ];
		enc += hex[ x & 0x0F ];
	}

	// display hex content in window
	m_packetData.SetWindowText( enc );
}

//
//	CSend::OnInitDialog
//

const int kTreeWidth = 225;

extern int gSelectedPort, gSelectedGroup, gSelectedItem;

BOOL CSend::OnInitDialog() 
{
	BOOL	bResult = CPropertySheet::OnInitDialog()
	;
	CRect	rectWnd
	;
	
	// resize the window to make space for the tree
	GetWindowRect( rectWnd );
	SetWindowPos( NULL
		, 0, 0, rectWnd.Width() + kTreeWidth, rectWnd.Height() /* + 30 */
		, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
		);
	
	// create the port combo box
	GetWindowRect( rectWnd );
	ScreenToClient( &rectWnd );
	CRect rectPort( rectWnd.right - kTreeWidth, 6, rectWnd.right - 10, rectWnd.bottom - 9 );
	m_port.Create( CBS_DROPDOWNLIST
		| WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | WS_VSCROLL
		, rectPort, this, IDC_PORT
		);
	
	// define the ports
	m_port.AddString( "(select a port)" );
	for (int i = 0; i < gMasterPortList.Length(); i++)
		m_port.AddString( gMasterPortList[i]->portDesc.portName );
	
	// select the null port, note that the null page has already been added 
	// in the constructor, you can't add it here! (gag)
	m_port.SetCurSel( 0 );
	
	// create the tree object
	GetWindowRect( rectWnd );
	ScreenToClient( &rectWnd );
	CRect rectTree( rectWnd.right - kTreeWidth, 37, rectWnd.right - 10, rectWnd.bottom - 9 );
	m_packetTree.Create( TVS_HASLINES
		| WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP
		, rectTree, this, IDC_PACKETTREE
		);
	
	// initialize the image list.
    m_packetTreeImageList.Create( IDB_PACKETTREE, 16, 1, RGB(255,0,255) );
    m_packetTree.SetImageList( &m_packetTreeImageList, TVSIL_NORMAL );

	// resize the window to make space for the edit text
	GetWindowRect( rectWnd );
	SetWindowPos( NULL
		, 0, 0, rectWnd.Width(), rectWnd.Height() + 60
		, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
		);
	
	// create the edit text object
	GetWindowRect( rectWnd );
	ScreenToClient( &rectWnd );
	CRect rectData( rectWnd.left + 10, rectWnd.bottom - 60
		, rectWnd.right - (kTreeWidth + 10), rectWnd.bottom - 9
		);
	m_packetData.Create( ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL
		| ES_WANTRETURN | ES_LEFT
		| WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP
		, rectData, this, IDC_PACKETDATA
		);
	
	// this should be empty at some point
	m_packetData.SetWindowText( "" );
	
	// create the send button
	GetWindowRect( rectWnd );
	ScreenToClient( &rectWnd );
	int hmiddle = rectWnd.right - (kTreeWidth / 2);
	CRect rectSend( hmiddle - 60, rectWnd.bottom - 50
		, hmiddle + 60, rectWnd.bottom - 19
		);
	m_send.Create( "Send", BS_PUSHBUTTON
		| WS_CHILD | WS_VISIBLE | WS_TABSTOP
		, rectSend, this, IDC_SEND
		);

	// disable it until a port is selected
	m_send.EnableWindow( false );

	// put it in the middle
	CenterWindow();
	
	// update the encoding
	UpdateEncoded();

	// make sure the selected port is clipped
	if (gSelectedPort >= gMasterPortList.Length())
		gSelectedPort = -1;

	// change to the selected port if there is one
	if (gSelectedPort != -1)
		ChangePort( gSelectedPort + 1 );
	if ((gSelectedGroup != -1) && (gSelectedItem != -1))
		ChangePacketTree( gSelectedGroup, gSelectedItem );

	return bResult;
}

//
//	CSend::PostNcDestroy
//
//	This function is called when the window goes away.  Normally property sheets are treated 
//	like dialog boxes, so the object is allocated on the stack and goes away when the function 
//	exits.  CSend is modeless, so it needs to be told that the object is going away.
//

void CSend::PostNcDestroy() 
{
	CPropertySheet::PostNcDestroy();
	delete this;
}

//
//	CSend::OnSelchangePort
//

void CSend::OnSelchangePort()
{
	ChangePort( m_port.GetCurSel() );
}

//
//	CSend::ChangePort
//

void CSend::ChangePort( int indx )
{
	CSendGroupPtr	curGrp
	;
	HTREEITEM		curParent
	;

	TRACE1( "Change port to %d\n", indx );

	// make sure the popup list has the right value
	m_port.SetCurSel( indx );

	// enable the send button if a port is selected
	m_send.EnableWindow( indx != 0 );

	// return to a null page
	SetNullPageList();

	// if no port selected, return to null page and empty tree
	if (indx == 0) {
		m_groupList = 0;
		m_packetTree.DeleteAllItems();
		return;
	}

	// get a pointer to the port
	m_pPort = gMasterPortList[indx - 1];

	// if the port didn't specify a group, return to null page and empty tree
	if (!m_pPort->portSendGroup) {
		m_groupList = 0;
		m_packetTree.DeleteAllItems();
		return;
	}

	// if the new one is the same group we've already got, return
	if (m_pPort->portSendGroup == m_groupList)
		return;

	// set the new group
	m_groupList = m_pPort->portSendGroup;

	// flush the old tree
	m_packetTree.DeleteAllItems();

	// establish each group in the tree
	for (int i = 0; m_groupList[i]; i++) {
		curGrp = m_groupList[i];

		// insert the item, set it to our special unused value
		curParent = m_packetTree.InsertItem( curGrp->groupName, 0, 0 );
		m_packetTree.SetItemData( curParent, kCSendPageUnused );

		// if this has items then install them
		if (curGrp->groupItemList)
			for (int j = 0; curGrp->groupItemList[j].itemName; j++)
				m_packetTree.SetItemData
					( m_packetTree.InsertItem( curGrp->groupItemList[j].itemName, 2, 2, curParent )
					, (i << 4) + j
					);
	}
}

//
//	CSend::OnSelchangePacketTree
//
//	Packet tree items are groups or items.  Groups have the kCSendUnused value in the 
//	item data, items refer to the index in the current group list and index into the 
//	groups item list.
//

void CSend::OnSelchangePacketTree( NMHDR* pNotifyStruct, LRESULT* result )
{
	int				iGroup, iItem
	;
	HTREEITEM		curItem = m_packetTree.GetSelectedItem()
	;
	DWORD			curData = m_packetTree.GetItemData( curItem )
	;

	// skip it if this is a group
	if (curData == kCSendPageUnused)
		return;

	// get the index of the group
	iGroup = (curData >> 4);
	iItem = (curData & 0x0000000F);

	TRACE2( "Selected (%d,%d)\n", iGroup, iItem );

	ChangePacketTree( iGroup, iItem );
}

//
//	CSend::ChangePacketTree
//
//	When a new item is selected build up a new collection of pages.  The first set 
//	will be followup pages from previous groups (if any), followed by the base page 
//	of the select item, followed by the item page.
//

void CSend::ChangePacketTree( int iGroup, int iItem )
{
	int				newLen = 0
	;
	HTREEITEM		curItem = 0
	;
	
	// shuffle along until we find the handle to the group
	curItem = m_packetTree.GetNextItem( 0, TVGN_CHILD );
	for (int x = 0; x < iGroup; x++) {
		curItem = m_packetTree.GetNextItem( curItem, TVGN_NEXT );
		ASSERT( curItem != 0 );
	}

	// make sure it is expanded
	m_packetTree.SetItemState( curItem, TVIS_EXPANDED, TVIS_EXPANDED );

	// dive into the children until we find the handle to the item
	curItem = m_packetTree.GetChildItem( curItem );
	for (int y = 0; y < iItem; y++) {
		curItem = m_packetTree.GetNextItem( curItem, TVGN_NEXT );
		ASSERT( curItem != 0 );
	}

	// make sure it is selected
	m_packetTree.SelectItem( curItem );

	// set the icon of the previous to 'normal'
	if (m_currentItem)
		m_packetTree.SetItemImage( m_currentItem, 2, 2 );

	// set the icon of this new one to 'selected'
	m_currentItem = curItem;
	m_packetTree.SetItemImage( m_currentItem, 3, 3 );

	CSendPageMPtr	newPages[kCSendMaxPages]
	;

	// add all of the followup pages
	for (int i = 0; i < iGroup; i++)
		if (m_groupList[i]->groupFollowupPage)
			newPages[newLen++] = m_groupList[i]->groupFollowupPage;

	// add the base page
	if (m_groupList[i]->groupBasePage)
		newPages[newLen++] = m_groupList[i]->groupBasePage;

	// see if the item specified its own base page
	if (m_groupList[i]->groupItemList[iItem].itemBase)
		newPages[newLen++] = m_groupList[i]->groupItemList[ iItem ].itemBase;

	// add the item page, null terminate the list
	newPages[newLen++] = m_groupList[i]->groupItemList[ iItem ].itemPage;
	newPages[newLen] = 0;

	// install the pages
	SetPageList( newPages );
}

//
//	CSend::OnItemExpandedPacketTree
//

void CSend::OnItemExpandedPacketTree( NMHDR* pNotifyStruct, LRESULT* result )
{
	NMTREEVIEW		*pView = (NMTREEVIEW*)pNotifyStruct
	;

	if ((pView->itemNew.state & TVIS_EXPANDED) != 0)
		m_packetTree.SetItemImage( pView->itemNew.hItem, 1, 1 );
	else
		m_packetTree.SetItemImage( pView->itemNew.hItem, 0, 0 );
}

//
//	CSend::SetPageList
//
//	This member function takes a pointer to a null terminated list of page pointers 
//	(actually member pointers) and installs the pages.  It first compares the list 
//	to what is already installed keeping the pages that are similar, adding the new 
//	ones, and then removing the old ones.  The new ones must be added before the 
//	old ones are removed because there must be at least one page installed.
//

void CSend::SetPageList( CSendPageMList lp )
{
	int				i, j, k
	,				installLen = 0
	,				removeLen = 0
	;
	CSendPagePtr	installList[kCSendMaxPages]
	,				removeList[kCSendMaxPages]
	;

	// translate the member pointers to real page pointers (wierd syntax)
	for (i = 0; lp[i]; i++)
		installList[i] = &(this->*lp[i]);
	installList[i] = 0;

	// scan for pages already installed
	i = 0;
	while (m_pages[i] && (m_pages[i] == installList[i]))
		i += 1;

	// copy the pages not found to the remove list
	for (j = i; m_pages[j]; j++)
		removeList[removeLen++] = m_pages[j];

	// install the new pages
	for ( ; lp[i]; i++ ) {
		m_pages[i] = installList[i];
		AddPage( installList[i] );
	}
	m_pages[i] = 0;

	// remove the pages that shouldn't be there
	for (k = 0; k < removeLen; k++)
		RemovePage( removeList[k] );

#if 0
	// tell all the pages to reset themselves
	for (k = 0; m_pages[k]; k++)
		m_pages[k]->InitPage();

	// tell all the pages to restore to the last saved contents
	for (k = 0; m_pages[k]; k++)
		m_pages[k]->RestorePage();

	// set the first page as the active page
	SetActivePage( 0 );
#endif
#if 1
	TRACE0( "---------- Init Pages\n" );
	// tell all the pages to reset themselves
	for (k = 0; m_pages[k]; k++)
		m_pages[k]->InitPage();

	TRACE0( "---------- Restore Pages\n" );
	// tell all the pages to restore to the last saved contents
	for (k = 0; m_pages[k]; k++)
		m_pages[k]->RestorePage();

	TRACE0( "---------- Set Pages Active\n" );
	// set all the pages as active at least once
	for (int p = 0; m_pages[p]; p++ ) {
		SetActivePage( p );
	}

	TRACE0( "---------- Away we go...\n" );
#endif

	// update the encoding
	UpdateEncoded();
}

//
//	CSend::SetNullPageList
//
//	This simple function is an easy way to install just the null page.
//

void CSend::SetNullPageList( void )
{
	CSendPageMPtr	nullList[] = { (CSendPageMPtr)&CSend::NullPage, 0 }
	;

	// install this null list
	SetPageList( nullList );
}

//
//	CSend::OnSend
//

void CSend::OnSend()
{
	BACnetOctet			upperNibble, lowerNibble
	;
	BACnetOctetString	msg
	;
	char				c
	;
	CString				str
	;
	LPCTSTR				s
	;

	// get the text from the control
	m_packetData.GetWindowText( str );
	s = str.operator LPCTSTR();

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
		msg.Append( (upperNibble << 4) + lowerNibble );
	}

	// forward it along to the port
	m_pPort->SendData( msg.strBuff, msg.strLen );
}

