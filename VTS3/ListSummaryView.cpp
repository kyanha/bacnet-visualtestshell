// ListSummaryView.cpp : implementation file
// author : Hu Meng 
// date   : 12-12-2002
// This view class is created to replace the old CSummaryView.
// The CSummaryView will not be used any more.

#include "stdafx.h"
#include "vts.h"
#include "VTSDoc.h"
#include "ListSummaryView.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListSummaryView

IMPLEMENT_DYNCREATE(CListSummaryView, CListView)

CListSummaryView::CListSummaryView()
{
}

CListSummaryView::~CListSummaryView()
{
}


BEGIN_MESSAGE_MAP(CListSummaryView, CListView)
	//{{AFX_MSG_MAP(CListSummaryView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGING, OnItemchanging)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListSummaryView drawing

void CListSummaryView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CListSummaryView diagnostics

#ifdef _DEBUG
void CListSummaryView::AssertValid() const
{
	CListView::AssertValid();
}

void CListSummaryView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CListSummaryView frame context listner methods
void CListSummaryView::ContextChange( CFrameContext::Signal s )
{
	switch (s) {
		case CFrameContext::eNewPacketCount:
			{
				CListCtrl& m_ElemList=GetListCtrl();
				
				if(m_FrameContext->m_PacketCount==0)
					m_ElemList.DeleteAllItems();
				else{
					int curCount=m_ElemList.GetItemCount();
					for(int i=curCount;i< m_FrameContext->m_PacketCount;i++)
						AddLine(i);
					//if last one selected,auto select the new coming one.
					if(curCount!=0&&m_FrameContext->m_CurrentPacket==curCount-1)
					{
						SetSelectedLine(m_FrameContext->m_PacketCount-1);
						m_FrameContext->SetCurrentPacket(m_FrameContext->m_PacketCount-1);
					}

				}
				break;
			}
		case CFrameContext::eNewCurrentPacket:
			SetSelectedLine( m_FrameContext->m_CurrentPacket );
			break;
	}

}


/////////////////////////////////////////////////////////////////////////////
// CListSummaryView message handlers

int CListSummaryView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	SetContext( gNewFrameContext );

	CListCtrl& m_ElemList=GetListCtrl();
	DWORD dwStyle = GetWindowLong(this->GetSafeHwnd(), GWL_STYLE); 
	SetWindowLong(this->GetSafeHwnd(),
		GWL_STYLE,
		dwStyle | LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS);
	m_ElemList.SetExtendedStyle(LVS_EX_FULLROWSELECT);


	m_ElemList.m_nFlags |= LVS_REPORT;
	m_ElemList.InsertColumn( 0, _T("No"), LVCFMT_LEFT, 30 );
	m_ElemList.InsertColumn( 1, _T("Source/Destination"), LVCFMT_LEFT, 120 );
	m_ElemList.InsertColumn( 2, _T("Service Type"), LVCFMT_LEFT, 150 );
	
	return 0;
}

void CListSummaryView::OnItemchanging(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// forget messages that dont change the state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0)
		m_FrameContext->SetCurrentPacket( pNMListView->iItem );
	
	*pResult = 0;
}


void CListSummaryView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd*	parent = GetParentFrame();

	//notify other view to display proper packet's info
	if(! ((CChildFrame*)parent)->m_pwndDetailViewBar->IsVisible())
			((CChildFrame*)parent)->ShowControlBar( ((CChildFrame*)parent)->m_pwndDetailViewBar, TRUE, FALSE);

	CListView::OnLButtonDblClk(nFlags, point);
}

void CListSummaryView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if ((nChar == 0x09) && (m_pTabRing))
		m_pTabRing->SetFocus();
	CListView::OnChar(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// CListSummaryView frame context display methods
void CListSummaryView::AddLine(int lineNo)
{
	char temp[20];

	VTSPacket	pkt;
	
	CListCtrl& m_ElemList=GetListCtrl();
	sprintf(temp,"%d",lineNo+1);
	m_ElemList.InsertItem( lineNo, (LPCTSTR)temp);
	
	// read the packet, the frame context holds the information
	m_FrameContext->m_pDoc->m_pDB->ReadPacket( lineNo, pkt );

	int			i, len = 12;
	char		nameBuff[32], addrBuff[32], *addr;
	const char	*name;

	// look up the source
	if(pkt.packetHdr.packetType==rxData)
	{
		name = m_FrameContext->m_pDoc->m_Names.AddrToName( pkt.packetHdr.packetSource, pkt.packetHdr.packetPortID );
		if (name)
			sprintf( nameBuff, "%-*.*s", len+1, len, name );
		else {
			addrBuff[0] = 0;
			addr = addrBuff;
			for (i = 0; i < pkt.packetHdr.packetSource.addrLen; i++) {
				sprintf( addr, "%02X", pkt.packetHdr.packetSource.addrAddr[i] );
				addr += 2;
			}
			sprintf( nameBuff, "%-*.*s", len+1, len, addrBuff );
		}
	}
	if(pkt.packetHdr.packetType==txData)
	{
		name = m_FrameContext->m_pDoc->m_Names.AddrToName( pkt.packetHdr.packetDestination, pkt.packetHdr.packetPortID );
		if (name)
			sprintf( nameBuff, "%-*.*s", len+1, len, name );
		else {
			addrBuff[0] = 0;
			addr = addrBuff;
			for (i = 0; i < pkt.packetHdr.packetDestination.addrLen; i++) {
				sprintf( addr, "%02X", pkt.packetHdr.packetDestination.addrAddr[i] );
				addr += 2;
			}
			sprintf( nameBuff, "%-*.*s", len+1, len, addrBuff );
		}
	}
	if(pkt.packetHdr.packetType==msgData)
	{
		sprintf( nameBuff, "VTS Message");
	}

	BACnetPIInfo	summary( true, false );
	summary.Interpret( (BACnetPIInfo::ProtocolType)pkt.packetHdr.packetProtocolID
		, (char *)pkt.packetData
		, pkt.packetLen);

	m_ElemList.SetItemText(lineNo,1,nameBuff);
	m_ElemList.SetItemText(lineNo,2,summary.summaryLine);
}

void CListSummaryView::SetSelectedLine(int currentLineNo)
{
	CListCtrl& m_ElemList=GetListCtrl();
	m_ElemList.SetItemState(currentLineNo,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

	//scroll the content
	RECT area;
	m_ElemList.GetItemRect(0,&area,LVIR_BOUNDS );
	int visible=m_ElemList.GetCountPerPage();
	int top=m_ElemList.GetTopIndex();
	if (currentLineNo>top+visible)
		m_ElemList.Scroll(CSize(0,2*(area.bottom-area.top)*(currentLineNo-top-visible)));
    if (currentLineNo<top)
		m_ElemList.Scroll(CSize(0,2*(area.top-area.bottom)*(top-currentLineNo)));
}

//This method is only used for export file
CString* CListSummaryView::GetLineData(int lineNo)
{
	CString*	pString = new CString();
	VTSPacket	pkt;
	char		theTime[16];

	// format the packet number
	pString->Format( "%5d ", lineNo );

	// read the packet, the frame context holds the information
	m_FrameContext->m_pDoc->m_pDB->ReadPacket( lineNo, pkt );


	// format the time
	FILETIME	locFileTime;
    SYSTEMTIME	st;

	::FileTimeToLocalFileTime( &pkt.packetHdr.packetTime, &locFileTime );
	::FileTimeToSystemTime( &locFileTime, &st );

	sprintf( theTime, "%02d:%02d:%02d.%03d "
		, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
		);
	*pString += theTime;


#if 1
	int			i, len = 9
	;
	char		nameBuff[32], addrBuff[32], *addr
	;
	const char	*name
	;

	// look up the source
	name = m_FrameContext->m_pDoc->m_Names.AddrToName( pkt.packetHdr.packetSource, pkt.packetHdr.packetPortID );
	if (name)
		sprintf( nameBuff, "%-*.*s", len+1, len, name );
	else {
		addrBuff[0] = 0;
		addr = addrBuff;
		for (i = 0; i < pkt.packetHdr.packetSource.addrLen; i++) {
			sprintf( addr, "%02X", pkt.packetHdr.packetSource.addrAddr[i] );
			addr += 2;
		}
		sprintf( nameBuff, "%-*.*s", len+1, len, addrBuff );
	}
	*pString += nameBuff;

	// look up the destination
	name = m_FrameContext->m_pDoc->m_Names.AddrToName( pkt.packetHdr.packetDestination, pkt.packetHdr.packetPortID );
	if (name)
		sprintf( nameBuff, "%-*.*s", len+1, len, name );
	else {
		addrBuff[0] = 0;
		addr = addrBuff;
		for (i = 0; i < pkt.packetHdr.packetDestination.addrLen; i++) {
			sprintf( addr, "%02X", pkt.packetHdr.packetDestination.addrAddr[i] );
			addr += 2;
		}
		sprintf( nameBuff, "%-*.*s", len+1, len, addrBuff );
	}
	*pString += nameBuff;
#endif

#if 1
	// format the summary line
	BACnetPIInfo	summary( true, false );
	summary.Interpret( (BACnetPIInfo::ProtocolType)pkt.packetHdr.packetProtocolID
		, (char *)pkt.packetData
		, pkt.packetLen
		);

	*pString += summary.summaryLine;
#endif

	return pString;
}

