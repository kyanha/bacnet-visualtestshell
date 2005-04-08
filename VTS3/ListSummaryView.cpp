// ListSummaryView.cpp : implementation file
// author : Hu Meng 
// date   : 12-12-2002
// This view class is created to replace the old CSummaryView.
// The CSummaryView will not be used any more.

#include "stdafx.h"
#include "vts.h"
#include "VTSPreferences.h"
#include "VTSDoc.h"
#include "ListSummaryView.h"
#include "ChildFrm.h"
#include "ColorSettingDlg.h"
#include <atlbase.h>
#include "EditResentPktDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern COLORREF SUMMARY_PACKET_COLOR[];
extern int gSelectedPort;
/////////////////////////////////////////////////////////////////////////////

// CListSummaryView

IMPLEMENT_DYNCREATE(CListSummaryView, CListView)

CListSummaryView::CListSummaryView()
				: m_summary(true, false), m_cache(gVTSPreferences.Setting_GetCachePacketCount())
{
	m_bSortType = TRUE;
	m_sortColumn = 0;
}


CListSummaryView::~CListSummaryView()
{
	SaveReg();
}


//	ON_NOTIFY_REFLECT(LVN_ITEMCHANGING, OnItemchanging)

BEGIN_MESSAGE_MAP(CListSummaryView, CListView)
	//{{AFX_MSG_MAP(CListSummaryView)
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CHAR()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_COMMAND(ID_VIEW_COLOR, OnViewColor)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND_RANGE(ID_SW_NO, ID_SW_SERVICE, OnShowColumns)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SW_NO, ID_SW_SERVICE, OnUpdateColumns)
	ON_COMMAND(ID_EDIT_DELETESEL, OnEditDeleteSel)
	ON_COMMAND(ID_EDIT_AUTOSCROLL, OnEditAutoScroll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_AUTOSCROLL, OnUpdateEditAutoScroll)
	ON_COMMAND(ID_EDIT_RECEIVEPKT, OnEditReceivePkt)
	ON_UPDATE_COMMAND_UI(ID_EDIT_RECEIVEPKT, OnUpdateEditReceivePkt)
	ON_COMMAND(ID_EDIT_SAVESENTPKT, OnEditSaveSentPkt)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SAVESENTPKT, OnUpdateEditSaveSentPkt)
	ON_COMMAND(ID_EDIT_SENDPACKET, OnEditSendPacket)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SENDPACKET, OnUpdateEditSendPacket)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClk)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETESEL, OnUpdateEditDeleteSel)
	ON_COMMAND(ID_EDIT_COLUMNVALUE, OnEditColumnValue)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COLUMNVALUE, OnUpdateEditColumnValue)
	ON_COMMAND(ID_EDIT_SENDALLPACKET, OnEditSendAllPacket)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SENDALLPACKET, OnUpdateEditSendAllPacket)
	ON_COMMAND(ID_EDIT_EDITSENDALLPKT, OnEditEditSendAllPkt)
	ON_UPDATE_COMMAND_UI(ID_EDIT_EDITSENDALLPKT, OnUpdateEditEditSendAllPkt)
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomdrawList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListSummaryView drawing

void CListSummaryView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
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
				
				if ( m_FrameContext->m_PacketCount == 0 )
				{
					m_ElemList.DeleteAllItems();
					m_cache.InitCache();
				}
				else{
					int curCount=m_ElemList.GetItemCount();
					for(int i=curCount;i< m_FrameContext->m_PacketCount;i++)
						AddLine(i);

					//modified: 2004/12/07 author:Xiao Shiyuan	purpose:if auto scrolling is enabled					 
					//if last one selected,auto select the new coming one.
					if(curCount!=0 && m_FrameContext->m_CurrentPacket == curCount - 1
						&& gVTSPreferences.Setting_IsAutoScroll())
					{
						SetSelectedLine(m_FrameContext->m_PacketCount-1);
						m_FrameContext->SetCurrentPacket(m_FrameContext->m_PacketCount-1);
					}

				}
				break;
			}
		case CFrameContext::eNewCurrentPacket:
			{
				//modified: 2004/10/15 author:Xiao Shiyuan	purpose:For sorting items
				for(int index = 0; index < GetListCtrl().GetItemCount(); index++)
				{
					if((int)GetListCtrl().GetItemData(index) == m_FrameContext->m_CurrentPacket)
					{
						SetSelectedLine( index );
						break;
					}
				}				
			}			
			break;

		case CFrameContext::eUpdatePrefs:
			
			// we've been told to change the cache size... make sure that's true
			if ( gVTSPreferences.Setting_GetCachePacketCount() != m_cache.GetCacheSize() )
				m_cache.AllocCacheSlots(gVTSPreferences.Setting_GetCachePacketCount());
			break;
	}

}


/////////////////////////////////////////////////////////////////////////////
// CListSummaryView message handlers
BOOL CListSummaryView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;

	return CListView::PreCreateWindow(cs);
}

int CListSummaryView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{	
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	SetContext( gNewFrameContext );

	CListCtrl& m_ElemList = GetListCtrl();
//	DWORD dwStyle = GetWindowLong(this->GetSafeHwnd(), GWL_STYLE); 
//	SetWindowLong(this->GetSafeHwnd(), GWL_STYLE, dwStyle | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
	m_ElemList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_iconList.Create(16, 16, ILC_MASK, 2, 2);
	m_iconList.Add(AfxGetApp()->LoadIcon(IDI_SENDPKT));
	m_iconList.Add(AfxGetApp()->LoadIcon(IDI_RECVPKT));
	m_ElemList.SetImageList(&m_iconList, LVSIL_SMALL);

	// column hiding not available yet...
	//m_ElemList.m_nFlags |= LVS_REPORT;
	m_ElemList.InsertColumn( 0, _T("No."), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(0) );
	m_ElemList.InsertColumn( 1, _T("TimeStamp"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(1) );
	m_ElemList.InsertColumn( 2, _T("Port"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(2) );
	//Xiao Shiyuan 2004-Sep-17
	m_ElemList.InsertColumn( 3, _T("Source"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(3) );	
	m_ElemList.InsertColumn( 4, _T("Destination"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(6) );
	m_ElemList.InsertColumn( 5, _T("SNET"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(4) );	
	m_ElemList.InsertColumn( 6, _T("SADDR"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(5) );
	m_ElemList.InsertColumn( 7, _T("DNET"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(4) );	
	m_ElemList.InsertColumn( 8, _T("DADDR"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(5) );
	m_ElemList.InsertColumn( 9, _T("Service Type"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(7) );
	//Xiao Shiyuan 2004-Sep-17

	m_tooltip.Create(this);
	m_tooltip.AddTool(this, "");
	m_tooltip.Activate(TRUE);

	// Start timer for auto-scroll mode... 5 seconds to start
	if ( gVTSPreferences.Setting_GetAutoscrollTimeout() != 0 )
		SetTimer(2, 1000 * gVTSPreferences.Setting_GetAutoscrollTimeout(), NULL);
	return 0;
}



void CListSummaryView::OnTimer(UINT nIDEvent) 
{
	// Timeout fired for inactivity (reset by selection)..
	// put this puppy into auto-scroll mode by simply selecting the last line in the list
	// The selection will restart the timer anyway.

	if ( nIDEvent == 2  &&  gVTSPreferences.Setting_GetAutoscrollTimeout() != 0 )
//		m_FrameContext->SetCurrentPacket( GetListCtrl().GetItemCount() - 1 );
		SetSelectedLine(GetListCtrl().GetItemCount()-1);

	CListView::OnTimer(nIDEvent);
}


void CListSummaryView::OnDestroy() 
{
	// View is being destroyed... save column positions...
	for (int i = 0; i < SUMMARY_VIEW_COLUMN_COUNT; i++ )
		gVTSPreferences.SView_SetColumnWidth(i, GetListCtrl().GetColumnWidth(i) );

	CListView::OnDestroy();
}


/*  madanner 6/03, used to be ItemChanging... this message isn't sent when ownerdata
    is set so change this to ItemChanged... which will work with non ownerdata stuff anyway...

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
*/


void CListSummaryView::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
 	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	CListCtrl& listCtrl = GetListCtrl();

	// forget messages that dont change the state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0)
	{
		int nPacket = (int)GetListCtrl().GetItemData(pNMListView->iItem);

		VTSPacketPtr ptr = m_FrameContext->m_pDoc->GetPacket(nPacket);

		BOOL flag = ptr->keyParameters.IsEmpty();

		m_FrameContext->SetCurrentPacket( nPacket );		
		
		if(flag)
		{
			listCtrl.SetItemText(pNMListView->iItem, 9, 
			listCtrl.GetItemText(pNMListView->iItem, 9) + ptr->keyParameters);						
		}

		// We've been selected...  Cancel Timer for auto-scroll mode and restart
		if ( gVTSPreferences.Setting_GetAutoscrollTimeout() != 0 )
		{
			KillTimer(2);
			SetTimer(2, 1000 * gVTSPreferences.Setting_GetAutoscrollTimeout(), NULL);
		}
 	}
	
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



BOOL CListSummaryView::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	if ( message == WM_NOTIFY )
    {
        switch( ((NMHDR *) lParam)->code )
        {
	        case LVN_ODCACHEHINT:			// list control is giving us cache hints...

				{
				NMLVCACHEHINT * pcachehint = (NMLVCACHEHINT*) lParam;
//				TRACE2("CACHE SUGGEST F:%d  T:%d\n", pcachehint->iFrom, pcachehint->iTo);

				// first pass on cache hint to document so if it's doing a virtual thing it can
				// load the proper guys

				((VTSDoc *) GetDocument())->CacheHint(pcachehint->iFrom, pcachehint->iTo);

				LVCachedItem	cacheditem;

				for ( int i = pcachehint->iFrom; i <= pcachehint->iTo; i++ )
					CacheItem(i, &cacheditem);
				}
				break;
        }
    }
	
	return CListView::OnChildNotify(message, wParam, lParam, pLResult);
}


void CListSummaryView::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO * pDispInfo = (LV_DISPINFO*) pNMHDR;
	LV_ITEM * pItem= &(pDispInfo)->item;

	// We only have to support handline of text and state messages...
	// Others are needed if we do images, columns, lParam data or anything else

	if (pItem->mask & LVIF_TEXT)
	{
		// control is requesting text...  fill in all of the columns when they are selected
		// This seems to always be called ... yuck.  Implement caching for efficiency

		//modified: 2004/10/14 author:Xiao Shiyuan	purpose: For sorting items		
        int nPacket = (int)GetListCtrl().GetItemData(pItem->iItem);		
		
		LVCachedItem	cacheditem;
		CacheItem(nPacket, &cacheditem);

		strcpy(pItem->pszText, cacheditem.pszColumnData[pItem->iSubItem]);
	}
	
	*pResult = 0;
}


void CListSummaryView::CacheItem(int nItem, LVCachedItem * pcacheditem )
{
	if ( !m_cache.GetCachedItem((DWORD) nItem, pcacheditem) )
	{
		VTSPacket * ppkt = m_FrameContext->m_pDoc->GetPacket(nItem);

		FillColumnData(0, pcacheditem->pszColumnData[0], (VTSPacket *) (nItem+1) );
		for ( int i = 1; i < SUMMARY_VIEW_COLUMN_COUNT; i++ )
			FillColumnData(i, pcacheditem->pszColumnData[i], ppkt );
	}
}



/////////////////////////////////////////////////////////////////////////////
// CListSummaryView frame context display methods
void CListSummaryView::AddLine(int lineNo)
{
	// Only need to insert an item into the list... the list will then ask for data to display
	CString columnText;
	columnText.Format("%d", lineNo);
	VTSPacketPtr ppkt = m_FrameContext->m_pDoc->GetPacket(lineNo);
	GetListCtrl().InsertItem( lineNo, columnText, ppkt->packetHdr.packetType);	
	GetListCtrl().SetItemData(lineNo, lineNo);
/*
	char sztemp[200];

	VTSPacketPtr ppkt = m_FrameContext->m_pDoc->GetPacket(lineNo);

	if ( ppkt == NULL )
		return;

	CListCtrl& m_ElemList=GetListCtrl();
	sprintf(temp,"%d",lineNo+1);
	m_ElemList.InsertItem( lineNo, (LPCTSTR)sztemp);

	m_ElemList.SetItemText(lineNo,1, ppkt->GetTimeStampString() );
	m_ElemList.SetItemText(lineNo, 2, ppkt->packetHdr.m_szPortName );

	if ( ppkt->packetHdr.packetType == msgData )
		m_ElemList.SetItemText(lineNo, 3, "VTS Message" );
	else
	{
		CString str = ppkt->GetAddressString(m_FrameContext->m_pDoc, ppkt->packetHdr.packetType);		// no length
		if ( ppkt->packetHdr.packetType == rxData )
			str = "  -> " + str;

		m_ElemList.SetItemText(lineNo, 3, str );
	}

	BACnetPIInfo	summary( true, false );
	summary.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen);

	m_ElemList.SetItemText(lineNo, 4, summary.summaryLine );
*/
}


// This method fills in the buffer requested by the list control... it is for ownerdata handling
// to allow DWORD elements in the list
// Also used to fill in our own cache for speed

char * CListSummaryView::FillColumnData( int nColumn, char * pszFill, VTSPacket * ppkt )
{
	if ( ppkt == NULL )
	{
		lstrcpy(pszFill, "OUT OF MEMORY");
		nColumn = -1;
	}

	switch(nColumn)
	{
		case 0:		// Packet Number:  cheating... just fill in the number
					sprintf(pszFill, "%d", (UINT) ppkt);
					break;

		case 1:		// Time Stamp
					lstrcpy(pszFill, ppkt->GetTimeStampString());
					break;

		case 2:		// Port Name
					lstrcpy(pszFill, ppkt->packetHdr.m_szPortName);
					break;

        //Xiao Shiyuan 2004-Sep-20 Separate source columns
		case 3:		// Source
					if ( ppkt->packetHdr.packetType == msgData )
						lstrcpy(pszFill, "VTS Message" );
					else
					{
						if( ppkt->packetHdr.packetType == rxData)
							lstrcpy(pszFill, ppkt->GetAddressString(m_FrameContext->m_pDoc));
					}
					break;					
		
		case 4:		
					//destination
					if( ppkt->packetHdr.packetType == txData)
						lstrcpy(pszFill, ppkt->GetAddressString(m_FrameContext->m_pDoc, FALSE));					
			        break;	

		case 5:		
					//SNET
					{
						unsigned short snet = 0;
						if(ppkt->GetSNET(snet))
						{
							CString str;
							str.Format("%d", snet);
							lstrcpy(pszFill, (LPCTSTR)str);
						}						
					}				
			        break;
					
		case 6:					
					//SADDR
					{						
						CString sadrStr = ppkt->GetSADRString(m_FrameContext->m_pDoc);
												
						lstrcpy(pszFill, (LPCTSTR)sadrStr);						
					}
			        break;

		case 7:		
					//DNET
					{	
						unsigned short dnet = 0;
						if(ppkt->GetDNET(dnet))
						{
							CString str;
							str.Format("%d", dnet);
							lstrcpy(pszFill, (LPCTSTR)str);
						}
					}
					break;

		case 8:
					//DADDR
					{
						CString dadrStr = ppkt->GetDADRString(m_FrameContext->m_pDoc);
							
						lstrcpy(pszFill, (LPCTSTR)dadrStr);						
					}
			        break;

		case 9:
					//Type of BACnet message
					m_summary.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen);
					lstrcpy(pszFill, m_summary.summaryLine );
			        break;
		//Xiao Shiyuan 2004-Sep-20
	}

	return pszFill;
}



/* Optimized madanner 5/03

	// read the packet, the frame context holds the information
//MAD_DB	m_FrameContext->m_pDoc->m_pDB->ReadPacket( lineNo, pkt );

	int			i, len = 12;
	char		nameBuff[32], addrBuff[32], *addr;
	const char	*name;

	// look up the source
//MAD_DB	if(pkt.packetHdr.packetType==rxData)
	if( ppkt->packetHdr.packetType==rxData)
	{
//MAD_DB		name = m_FrameContext->m_pDoc->m_Names.AddrToName( pkt.packetHdr.packetSource, pkt.packetHdr.packetPortID );
		name = m_FrameContext->m_pDoc->AddrToName( ppkt->packetHdr.packetSource, ppkt->packetHdr.m_szPortName );
		if (name)
			sprintf( nameBuff, "  -> %-*.*s", len+1, len, name );
		else {
			addrBuff[0] = 0;
			addr = addrBuff;
// MAD_DB
//			for (i = 0; i < pkt.packetHdr.packetSource.addrLen; i++) {
//				sprintf( addr, "%02X", pkt.packetHdr.packetSource.addrAddr[i] );
//				addr += 2;
//			}

			for (i = 0; i < ppkt->packetHdr.packetSource.addrLen; i++)
			{
				sprintf( addr, "%02X", ppkt->packetHdr.packetSource.addrAddr[i] );
				addr += 2;
			}

			sprintf( nameBuff, "  -> %-*.*s", len+1, len, addrBuff );
		}
	}

//MAD_DB	if(pkt.packetHdr.packetType==txData)
	if( ppkt->packetHdr.packetType==txData)
	{
//MAD_DB		name = m_FrameContext->m_pDoc->m_Names.AddrToName( pkt.packetHdr.packetDestination, pkt.packetHdr.packetPortID );
		name = m_FrameContext->m_pDoc->AddrToName( ppkt->packetHdr.packetDestination, ppkt->packetHdr.m_szPortName );
		if (name)
			sprintf( nameBuff, "%-*.*s", len+1, len, name );
		else {
			addrBuff[0] = 0;
			addr = addrBuff;

// MAD_DB
//			for (i = 0; i < pkt.packetHdr.packetDestination.addrLen; i++) {
//				sprintf( addr, "%02X", pkt.packetHdr.packetDestination.addrAddr[i] );
//				addr += 2;
//			}

			for (i = 0; i < ppkt->packetHdr.packetDestination.addrLen; i++)
			{
				sprintf( addr, "%02X", ppkt->packetHdr.packetDestination.addrAddr[i] );
				addr += 2;
			}

			sprintf( nameBuff, "%-*.*s", len+1, len, addrBuff );
		}
	}

//MAD_DB	if(pkt.packetHdr.packetType==msgData)
	if( ppkt->packetHdr.packetType==msgData)
	{
		sprintf( nameBuff, "VTS Message");
	}

	BACnetPIInfo	summary( true, false );

// MAD_DB
//	summary.Interpret( (BACnetPIInfo::ProtocolType)pkt.packetHdr.packetProtocolID
//		, (char *)pkt.packetData
//		, pkt.packetLen);
//
	summary.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen);

// added madanner 5/03
	m_ElemList.SetItemText(lineNo,1, ppkt->GetTimeStampString(ppkt) );
	m_ElemList.SetItemText(lineNo,2,nameBuff);
	m_ElemList.SetItemText(lineNo,3,summary.summaryLine);
}
*/



//This method is only used for export file
// Need for method is nill... see export
// madanner 5/03
/*

CString* CListSummaryView::GetLineData(int lineNo)
{
	CString*	pString = new CString();

//MAD_DB	VTSPacket	pkt;
	VTSPacketPtr ppkt = m_FrameContext->m_pDoc->GetPacket(lineNo);

	// format the packet number
	pString->Format( "%5d ", lineNo );

	// read the packet, the frame context holds the information
//MAD_DB	m_FrameContext->m_pDoc->m_pDB->ReadPacket( lineNo, pkt );


	// format the time
	FILETIME	locFileTime;
    SYSTEMTIME	st;

//MAD_DB	::FileTimeToLocalFileTime( &pkt.packetHdr.packetTime, &locFileTime );
	::FileTimeToLocalFileTime( &(ppkt->packetHdr.packetTime), &locFileTime );
	::FileTimeToSystemTime( &locFileTime, &st );

	sprintf( theTime, "%02d:%02d:%02d.%03d "
		, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
		);
	*pString += ppkt->GetTimeStampString();


#if 1
	int			i, len = 9
	;
	char		nameBuff[32], addrBuff[32], *addr
	;
	const char	*name
	;

	// look up the source
//MAD_DB	name = m_FrameContext->m_pDoc->m_Names.AddrToName( pkt.packetHdr.packetSource, pkt.packetHdr.packetPortID );
	name = m_FrameContext->m_pDoc->AddrToName( ppkt->packetHdr.packetSource, ppkt->packetHdr.m_szPortName );

	if (name)
		sprintf( nameBuff, "%-*.*s", len+1, len, name );
	else {
		addrBuff[0] = 0;
		addr = addrBuff;
//
//		for (i = 0; i < pkt.packetHdr.packetSource.addrLen; i++) {
//			sprintf( addr, "%02X", pkt.packetHdr.packetSource.addrAddr[i] );
//			addr += 2;
//		}

		for (i = 0; i < ppkt->packetHdr.packetSource.addrLen; i++)
		{
			sprintf( addr, "%02X", ppkt->packetHdr.packetSource.addrAddr[i] );
			addr += 2;
		}

		sprintf( nameBuff, "%-*.*s", len+1, len, addrBuff );
	}
	*pString += nameBuff;

	// look up the destination
//MAD_DB	name = m_FrameContext->m_pDoc->m_Names.AddrToName( pkt.packetHdr.packetDestination, pkt.packetHdr.packetPortID );
	name = m_FrameContext->m_pDoc->AddrToName( ppkt->packetHdr.packetDestination, ppkt->packetHdr.m_szPortName );

	if (name)
		sprintf( nameBuff, "%-*.*s", len+1, len, name );
	else {
		addrBuff[0] = 0;
		addr = addrBuff;

//		for (i = 0; i < pkt.packetHdr.packetDestination.addrLen; i++) {
//			sprintf( addr, "%02X", pkt.packetHdr.packetDestination.addrAddr[i] );
//			addr += 2;
//		}

		for (i = 0; i < ppkt->packetHdr.packetDestination.addrLen; i++)
		{
			sprintf( addr, "%02X", ppkt->packetHdr.packetDestination.addrAddr[i] );
			addr += 2;
		}

		sprintf( nameBuff, "%-*.*s", len+1, len, addrBuff );
	}
	*pString += nameBuff;
#endif

#if 1
	// format the summary line
	BACnetPIInfo	summary( true, false );

//	summary.Interpret( (BACnetPIInfo::ProtocolType)pkt.packetHdr.packetProtocolID
//		, (char *)pkt.packetData
//		, pkt.packetLen
//		);

	summary.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen );

	*pString += summary.summaryLine;
#endif

	return pString;
}
*/

// Xiao Shiyuan 10/1/2004. For list view with different color rows
void CListSummaryView::OnCustomdrawList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    // Take the default processing unless we set this to something else below.
    *pResult = 0;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.
    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        *pResult = CDRF_NOTIFYITEMDRAW;
        }
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        // This is the prepaint stage for an item. Here's where we set the
        // item's text color. Our return value will tell Windows to draw the
        // item itself, but it will use the new color we set here.
        // We'll cycle the colors through red, green, and light blue.
        COLORREF crText;
        CString serviceText;
		
		int nPacket = (int)GetListCtrl().GetItemData(pLVCD->nmcd.dwItemSpec);

		VTSPacketPtr ptr = m_FrameContext->m_pDoc->GetPacket(nPacket);

		serviceText = GetListCtrl().GetItemText(pLVCD->nmcd.dwItemSpec, 9);
		serviceText.MakeLower();
		
		if(ptr->bErrorDecode)
		{
			crText = SUMMARY_PACKET_COLOR[5];
		}
		else
		{
			if ( serviceText.Find("readproperty,") > -1)
				crText = SUMMARY_PACKET_COLOR[0];
			else if ( serviceText.Find("readproperty-ack") > -1)
				crText = SUMMARY_PACKET_COLOR[0];
			else if ( serviceText.Find("writeproperty,") > -1 )
				crText = SUMMARY_PACKET_COLOR[1];
			else if ( serviceText.Find("writeproperty-ack") > -1 )
				crText = SUMMARY_PACKET_COLOR[1];
			else if ( serviceText.Find("readpropertymultiple,") > -1)
				crText = SUMMARY_PACKET_COLOR[2];
			else if ( serviceText.Find("readpropertymultiple-ack") > -1)
				crText = SUMMARY_PACKET_COLOR[2];
			else if ( serviceText.Find("writepropertymultiple,") > -1)
				crText = SUMMARY_PACKET_COLOR[3];
			else if ( serviceText.Find("error,") > -1 )
				crText = SUMMARY_PACKET_COLOR[4];
			else
				crText = RGB(0,0,0);
		}

        // Store the color back in the NMLVCUSTOMDRAW struct.
        pLVCD->clrText = crText;

        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
        }
}

void CListSummaryView::OnViewColor() 
{
	// TODO: Add your command handler code here
	CColorSettingDlg dlg;
	if( dlg.DoModal() == IDOK )
		RedrawWindow();
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, 
   LPARAM lParamSort)
{	
	int result = 0;
	ListSortParam* pSortParam = (ListSortParam*)lParamSort;

	if(pSortParam->column > 0)
	{
		LVCachedItem	cacheditem;
		pSortParam->pt->CacheItem(lParam1, &cacheditem);
		CString text1 = cacheditem.pszColumnData[pSortParam->column];
		
		pSortParam->pt->CacheItem(lParam2, &cacheditem);
		CString text2 = cacheditem.pszColumnData[pSortParam->column];

		result = text1.CompareNoCase(text2);
	}
	else
	{
		result = lParam1 - lParam2;
	}

	if(result < 0)
		return pSortParam->type?-1:1;
	else	
		return pSortParam->type?1:-1;
    
	return 0;
}

void CListSummaryView::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	//pNMListView->iSubItem indicates which column is clicked
	if(pNMListView->iSubItem == m_sortColumn)
		m_bSortType = !m_bSortType;

	m_sortColumn = pNMListView->iSubItem;

	CListCtrl& listCtrl = GetListCtrl();	
	
	ListSortParam sortParam;
	sortParam.pt = this;
	sortParam.type = m_bSortType;
	sortParam.column = m_sortColumn;
	listCtrl.SortItems(CompareFunc, (DWORD)&sortParam);
    
	*pResult = 0;
}

void CListSummaryView::RelayEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
// This function will create a MSG structure, fill it in a pass it to
// the ToolTip control, m_ttip.  Note that we ensure the point is in window
// coordinates (relative to the control's window).
	if(NULL != m_tooltip.m_hWnd){
		MSG msg;
		msg.hwnd = m_hWnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.time = 0;
		msg.pt.x = LOWORD(lParam);
		msg.pt.y = HIWORD(lParam);

		m_tooltip.RelayEvent(&msg);
	}
}

void CListSummaryView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl& listCtrl = GetListCtrl();

	int nItem = listCtrl.HitTest(point, &nFlags);
	
	if(nItem == -1)	
		return;
	
	int nPacket = (int)listCtrl.GetItemData(nItem);	

	VTSPacketPtr ptr = m_FrameContext->m_pDoc->GetPacket(nPacket);

	if(ptr->bErrorDecode)
	{		
		m_tooltip.SetTipBkColor(RGB(255,0,0));
		m_tooltip.SetTipTextColor(RGB(255,255,0));
	}
	else
	{
		m_tooltip.SetTipBkColor(0xE1FFFF);
		m_tooltip.SetTipTextColor(RGB(0,0,0));
	}

	m_tooltip.Activate(TRUE);
	m_tooltip.UpdateTipText(listCtrl.GetItemText(nItem, 9), this);	
	//m_tooltip.UpdateTipText(listCtrl.GetItemText(nItem, 9) + "." + ptr->keyParameters, this);
	
	RelayEvent(WM_MOUSEMOVE,(WPARAM)nFlags,MAKELPARAM(LOWORD(point.x),LOWORD(point.y)));
	
	CListView::OnMouseMove(nFlags, point);
}

void CListSummaryView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	CListCtrl& listCtrl = GetListCtrl();	
	listCtrl.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );	
	
	ReadReg();	
}

void CListSummaryView::ReadReg()
{
	CRegKey regKey;
	long lRet = regKey.Open(HKEY_LOCAL_MACHINE,
		"Software\\vts3\\Columns");
	if(lRet != ERROR_SUCCESS)
	{
		regKey.Create(HKEY_LOCAL_MACHINE,
		"Software\\vts3\\Columns");
	}
	else
	{
		CString keyName;
		DWORD keyValue;
		for(int index = 0; index < 10; index++)
		{
			keyName.Format("%d", index);
			regKey.QueryValue(keyValue, keyName);
			m_bColumn[index] = keyValue > 0;

			keyName.Format("width%d", index);
			regKey.QueryValue(keyValue, keyName);
			m_columnWidth[index] = keyValue;
		}		
	}	
	regKey.Close();
}

void CListSummaryView::SaveReg()
{
	CRegKey regKey;
	CString keyName;
	DWORD keyValue;

	// msdanner 4/8/2005:  added protection against trying to save
	// keys that do not already exist.  This can happen on the initial upgrade
	// to the first version of VTS that preserves column settings. 
	if (regKey.Open(HKEY_LOCAL_MACHINE,	"Software\\vts3\\Columns") == ERROR_SUCCESS)
	{
		for(int index = 0; index < 10; index++)
		{
			keyName.Format("%d", index);
			if(m_bColumn[index])
				keyValue = 1;
			else
				keyValue = 0;

			regKey.SetValue(keyValue, keyName);		

			keyName.Format("width%d", index);
			keyValue = m_columnWidth[index];
			regKey.SetValue(keyValue, keyName);		
		}	
		regKey.Close();
	}
}

void CListSummaryView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl& listCtrl = GetListCtrl();	
	
	CMenu menu;	
	menu.LoadMenu(IDR_SUMMARYWIN);	

	//show popup menu	
	CMenu* pPopup = menu.GetSubMenu(0);
	CWnd* pWndPopupOwner = listCtrl.GetSafeOwner();
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();
	ClientToScreen(&point);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);	
	menu.DestroyMenu();	
}


void CListSummaryView::OnShowColumns(UINT nID) 
{
	// TODO: Add your command handler code here
	int index = nID - ID_SW_NO;
	m_bColumn[index] = !m_bColumn[index];

	CListCtrl& listCtrl = GetListCtrl();

	if(m_bColumn[index])
	{
		listCtrl.SetColumnWidth(index, m_columnWidth[index]);		
	}
	else
	{
		m_columnWidth[index] = listCtrl.GetColumnWidth(index);
		listCtrl.SetColumnWidth(index, 0);
	}		
}

void CListSummaryView::OnUpdateColumns(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	int index = pCmdUI->m_nID - ID_SW_NO;

	pCmdUI->ContinueRouting();
	pCmdUI->SetCheck(m_bColumn[index]);	
}

//prevent changing the width of the columns which are hiden
BOOL CListSummaryView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	HD_NOTIFY *pHDN = (HD_NOTIFY*)lParam;

	//prevent changing the width of the columns which are hiden 
	if( (pHDN->hdr.code == HDN_BEGINTRACKW || pHDN->hdr.code == HDN_BEGINTRACKA)
		&& !m_bColumn[pHDN->iItem]) 
	{		
		*pResult = TRUE; 
		return TRUE; 
	} 

	return CListView::OnNotify(wParam, lParam, pResult);
}

// 2/25/05 Shiyuan Xiao. Delete selected packet
void CListSummaryView::OnEditDeleteSel() 
{
	// TODO: Add your command handler code here
	CListCtrl& listCtrl = GetListCtrl();

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	int nItem = -1;
	if( pos )
		nItem = listCtrl.GetNextSelectedItem(pos);
	else
		return;

	int nPacket = (int)listCtrl.GetItemData(nItem);		

	m_FrameContext->m_pDoc->DeleteSelPacket(nPacket);

	//update packet index
	for(int i = 0; i < listCtrl.GetItemCount(); i++)
	{
		int tempPktIndex = (int)listCtrl.GetItemData(i);	
		if(tempPktIndex > nPacket)
		{
			tempPktIndex--;
			listCtrl.SetItemData(i, tempPktIndex);
		}
	}

	m_cache.DeleteCachedItem(nPacket);

	listCtrl.DeleteItem(nItem);	
}

void CListSummaryView::OnUpdateEditDeleteSel(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CListCtrl& listCtrl = GetListCtrl();

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	int nItem = -1;
	
	pCmdUI->Enable(pos != NULL);	
}

void CListSummaryView::OnEditAutoScroll() 
{
	// TODO: Add your command handler code here
	gVTSPreferences.Setting_SetAutoScroll(!gVTSPreferences.Setting_IsAutoScroll());
}

void CListSummaryView::OnUpdateEditAutoScroll(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(gVTSPreferences.Setting_IsAutoScroll())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CListSummaryView::OnEditReceivePkt() 
{
	// TODO: Add your command handler code here
	gVTSPreferences.Setting_SetRecvPkt(!gVTSPreferences.Setting_IsRecvPkt());
}

void CListSummaryView::OnUpdateEditReceivePkt(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(gVTSPreferences.Setting_IsRecvPkt())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CListSummaryView::OnEditSaveSentPkt() 
{
	// TODO: Add your command handler code here
	gVTSPreferences.Setting_SetSaveSentPkt(!gVTSPreferences.Setting_IsSaveSentPkt());
}

void CListSummaryView::OnUpdateEditSaveSentPkt(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(gVTSPreferences.Setting_IsSaveSentPkt())
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}


void CListSummaryView::OnEditSendPacket() 
{
	// TODO: Add your command handler code here
	CListCtrl& listCtrl = GetListCtrl();

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	int nItem = -1;
	if( pos )
		nItem = listCtrl.GetNextSelectedItem(pos);
	else
		return;
	
	int nPacket = (int)listCtrl.GetItemData(nItem);	

	VTSPacketPtr ptr = m_FrameContext->m_pDoc->GetPacket(nPacket);

	// get a pointer to the port
	VTSPorts * pports = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetPorts();
	if ( pports == NULL || gSelectedPort < 0 || gSelectedPort > pports->GetSize() )
	{
		return;
	}

	VTSPort *pPort = (*pports)[gSelectedPort];
	if( pPort->m_strName.CompareNoCase(ptr->packetHdr.m_szPortName) == 0 )
	{
		pPort->SendData(ptr->packetData, ptr->packetLen);
	}
	else
	{
		MessageBox("Please choose the same port to send this packet!");
	}	
}

void CListSummaryView::OnUpdateEditSendPacket(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CListCtrl& listCtrl = GetListCtrl();

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	int nItem = -1;
	if( pos )
		nItem = listCtrl.GetNextSelectedItem(pos);
	else
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	
	int nPacket = (int)listCtrl.GetItemData(nItem);	

	VTSPacketPtr ptr = m_FrameContext->m_pDoc->GetPacket(nPacket);
	
    //if there is selected port and packet
	VTSPorts * pports = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetPorts();

    pCmdUI->Enable(ptr->packetHdr.packetType == txData
		&& pports != NULL
		&& gSelectedPort >= 0
		&& gSelectedPort < pports->GetSize());			
}


void CListSummaryView::OnDblClk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int nItem = pNMListView->iItem;

	int subItem = pNMListView->iSubItem;

	CListCtrl& listCtrl = GetListCtrl();
	listCtrl.EditLabel(nItem);

	*pResult = 0;
}



void CListSummaryView::OnEditColumnValue() 
{
	// TODO: Add your command handler code here
	CListCtrl& listCtrl = GetListCtrl();

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	int nItem = -1;
	if( pos )
		nItem = listCtrl.GetNextSelectedItem(pos);
	else
		return;
	
	int nPacket = (int)listCtrl.GetItemData(nItem);	

	VTSPacketPtr ppkt = m_FrameContext->m_pDoc->GetPacket(nPacket);
	
	if(ppkt == NULL)
		return;

	CString packetDesName = 
		m_FrameContext->m_pDoc->AddrToName(ppkt->packetHdr.packetDestination,
		ppkt->packetHdr.m_szPortName);

	VTSNames * pnames = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetNames();
	if ( pnames == NULL )
	{
		return;
	}

	// get a pointer to the port
	VTSPorts * pports = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetPorts();
	if ( pports == NULL || gSelectedPort < 0 || gSelectedPort > pports->GetSize() )
	{
		MessageBox("Please choose the same port to send this packet!");
		return;
	}

	VTSPort *pPort = (*pports)[gSelectedPort];
	if( pPort->m_strName.CompareNoCase(ppkt->packetHdr.m_szPortName) != 0 )	
	{
		MessageBox("Please choose the same port to send this packet!");
		return;
	}	
	
	CEditResentPktDlg dlg;

	for(int index = 0; index < pnames->GetSize(); index++)
	{
		VTSName *pname = (*pnames)[index];

		if(pname->m_pportLink != NULL)
		{
			if( (pname->m_pportLink->m_nPortType == ipPort
				&& ppkt->packetHdr.packetType == BACnetPIInfo::ipProtocol) ||
				(pname->m_pportLink->m_nPortType == ethernetPort
				&& ppkt->packetHdr.packetType == BACnetPIInfo::ethernetProtocol) ||
				(pname->m_pportLink->m_nPortType == ptpPort
				&& ppkt->packetHdr.packetType == BACnetPIInfo::ptpProtocol) )
			{
				if( pname->m_strName.CompareNoCase(packetDesName) == 0)
				{
					dlg.m_defaultDes = dlg.m_desArray.GetCount();
				}

				DesName desname;
				desname.m_index = index;
				desname.m_name = pname->m_strName;
				dlg.m_desArray.AddTail(desname);				
			}
		}		
	}	

	unsigned short snet = 0;
	if( ppkt->GetSNET(snet) )
	{
		dlg.m_bSnet = TRUE;
		dlg.m_snet = snet;
		dlg.m_sadr = ppkt->GetSADRString(m_FrameContext->m_pDoc, FALSE);
	}
	else
	{
		dlg.m_bSnet = FALSE;
	}

	unsigned short dnet = 0;
	if( ppkt->GetDNET(dnet) )
	{
		dlg.m_bDnet = TRUE;
		dlg.m_dnet = dnet;
		dlg.m_dadr = ppkt->GetDADRString(m_FrameContext->m_pDoc, FALSE);
	}
	else
	{
		dlg.m_bDnet = FALSE;
	}

	if(dlg.DoModal() == IDOK)
	{
		VTSName *pname = (*pnames)[dlg.m_desArray.GetAt(dlg.m_desArray.FindIndex(dlg.m_desIndex)).m_index];

		VTSPacket packet = *ppkt;
		//change destination address
		packet.SetDesAddress(pname->m_bacnetaddr);

		if(dlg.m_bDnet)
		{
			packet.SetDNET(dlg.m_dnet);

			BACnetOctetString str;
			HEXSTR_TO_OCTETSTRING(dlg.m_dadr, str);			
			packet.SetDADR(str.strBuff, str.strLen);
		}
		else
		{
			packet.SetDNET(0, FALSE);
		}

		if(dlg.m_bSnet)
		{			
			packet.SetSNET(dlg.m_snet);
			BACnetOctetString str;
			HEXSTR_TO_OCTETSTRING(dlg.m_sadr, str);
			packet.SetSADR(str.strBuff, str.strLen);
		}
		else
		{
			packet.SetSNET(0, FALSE);
		}

		pPort->SendData(packet.packetData, packet.packetLen);
	}
	else
	{
		return;
	}
}

void CListSummaryView::OnUpdateEditColumnValue(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CListCtrl& listCtrl = GetListCtrl();

	POSITION pos = listCtrl.GetFirstSelectedItemPosition();
	int nItem = -1;
	if( pos )
		nItem = listCtrl.GetNextSelectedItem(pos);
	else	
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	int nPacket = (int)listCtrl.GetItemData(nItem);	

	VTSPacketPtr ptr = m_FrameContext->m_pDoc->GetPacket(nPacket);

	//if there is selected port and packet
	VTSPorts * pports = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetPorts();
	
    pCmdUI->Enable(ptr->packetHdr.packetType == txData
		&& pports != NULL
		&& gSelectedPort >= 0
		&& gSelectedPort < pports->GetSize());
}


void CListSummaryView::OnEditSendAllPacket() 
{
	// TODO: Add your command handler code here
	CListCtrl& listCtrl = GetListCtrl();

	VTSPorts * pports = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetPorts();
	if ( pports == NULL || gSelectedPort < 0 || gSelectedPort > pports->GetSize() )
	{
		return;
	}

	VTSPort *pPort = (*pports)[gSelectedPort];
	
	int count = listCtrl.GetItemCount();
	for(int nItem = 0; nItem < count; nItem++)
	{
		int nPacket = (int)listCtrl.GetItemData(nItem);

		VTSPacketPtr ptr = m_FrameContext->m_pDoc->GetPacket(nPacket);
		ASSERT(ptr != NULL);

		if( ptr->packetHdr.packetType != txData )
			continue;

		if( pPort->m_strName.CompareNoCase(ptr->packetHdr.m_szPortName) == 0 )
		{
			if( gVTSPreferences.Setting_GetResendInterval() > 0 )
			{
				Sleep(gVTSPreferences.Setting_GetResendInterval());
			}

			pPort->SendData(ptr->packetData, ptr->packetLen);
		}
	}
}

void CListSummaryView::OnUpdateEditSendAllPacket(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CListCtrl& listCtrl = GetListCtrl();
	
	VTSPorts * pports = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetPorts();

    pCmdUI->Enable( listCtrl.GetItemCount() > 0
		&& pports != NULL
		&& gSelectedPort >= 0
		&& gSelectedPort < pports->GetSize());
}


void CListSummaryView::OnEditEditSendAllPkt() 
{
	// TODO: Add your command handler code here
	CListCtrl& listCtrl = GetListCtrl();

	VTSNames * pnames = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetNames();
	if ( pnames == NULL )
	{
		return;
	}

	// get a pointer to the port
	VTSPorts * pports = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetPorts();
	if ( pports == NULL || gSelectedPort < 0 || gSelectedPort > pports->GetSize() )
	{
		MessageBox("Please choose the same port to send this packet!");
		return;
	}

	VTSPort *pPort = (*pports)[gSelectedPort];

	CEditResentPktDlg dlg;

	for(int index = 0; index < pnames->GetSize(); index++)
	{
		VTSName *pname = (*pnames)[index];

		if(pname->m_pportLink != NULL)
		{
			if( pname->m_pportLink->m_nPortType == pPort->m_nPortType )
			{			
				DesName desname;
				desname.m_index = index;
				desname.m_name = pname->m_strName;
				dlg.m_desArray.AddTail(desname);				
			}
		}		
	}	

	if(dlg.DoModal() == IDOK)
	{
		for(int nItem = 0; nItem < listCtrl.GetItemCount(); nItem++)
		{
			int nPacket = (int)listCtrl.GetItemData(nItem);	
			
			VTSPacketPtr ppkt = m_FrameContext->m_pDoc->GetPacket(nPacket);
			
			if(ppkt == NULL || ppkt->packetHdr.packetType != txData)
				continue;

			VTSName *pname = (*pnames)[dlg.m_desArray.GetAt(dlg.m_desArray.FindIndex(dlg.m_desIndex)).m_index];

			VTSPacket packet = *ppkt;
			//change destination address
			packet.SetDesAddress(pname->m_bacnetaddr);
			
			if(dlg.m_bDnet)
			{
				packet.SetDNET(dlg.m_dnet);
				
				BACnetOctetString str;
				HEXSTR_TO_OCTETSTRING(dlg.m_dadr, str);			
				packet.SetDADR(str.strBuff, str.strLen);
			}
			else
			{
				packet.SetDNET(0, FALSE);
			}
			
			if(dlg.m_bSnet)
			{			
				packet.SetSNET(dlg.m_snet);
				BACnetOctetString str;
				HEXSTR_TO_OCTETSTRING(dlg.m_sadr, str);
				packet.SetSADR(str.strBuff, str.strLen);
			}
			else
			{
				packet.SetSNET(0, FALSE);
			}

			if( gVTSPreferences.Setting_GetResendInterval() > 0 )
			{
				Sleep(gVTSPreferences.Setting_GetResendInterval());
			}
			
			pPort->SendData(packet.packetData, packet.packetLen);
		}
	}
	else
	{
		
	}
}

void CListSummaryView::OnUpdateEditEditSendAllPkt(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CListCtrl& listCtrl = GetListCtrl();
	
	VTSPorts * pports = ((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->GetPorts();

    pCmdUI->Enable( listCtrl.GetItemCount() > 0
		&& pports != NULL
		&& gSelectedPort >= 0
		&& gSelectedPort < pports->GetSize());
}
