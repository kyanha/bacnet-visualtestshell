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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////

// CListSummaryView

IMPLEMENT_DYNCREATE(CListSummaryView, CListView)

CListSummaryView::CListSummaryView()
				: m_summary(true, false), m_cache(gVTSPreferences.Setting_GetCachePacketCount())
{
}


CListSummaryView::~CListSummaryView()
{
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
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	//}}AFX_MSG_MAP
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

		case CFrameContext::eUpdatePrefs:
			
			// we've been told to change the cache size... make sure that's true
			if ( gVTSPreferences.Setting_GetCachePacketCount() != m_cache.GetCacheSize() )
				m_cache.AllocCacheSlots(gVTSPreferences.Setting_GetCachePacketCount());
			break;
	}

}


/////////////////////////////////////////////////////////////////////////////
// CListSummaryView message handlers

int CListSummaryView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->style |= LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_OWNERDATA;
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	SetContext( gNewFrameContext );

	CListCtrl& m_ElemList=GetListCtrl();
//	DWORD dwStyle = GetWindowLong(this->GetSafeHwnd(), GWL_STYLE); 
//	SetWindowLong(this->GetSafeHwnd(), GWL_STYLE, dwStyle | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
	m_ElemList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// column hiding not available yet...
	m_ElemList.m_nFlags |= LVS_REPORT;
	m_ElemList.InsertColumn( 0, _T("No"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(0) );
	m_ElemList.InsertColumn( 1, _T("TimeStamp"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(1) );
	m_ElemList.InsertColumn( 2, _T("Port"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(2) );
	m_ElemList.InsertColumn( 3, _T("Destination/Source"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(3) );
	m_ElemList.InsertColumn( 4, _T("Service Type"), LVCFMT_LEFT, gVTSPreferences.SView_GetColumnWidth(4) );

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

	// forget messages that dont change the state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0)
	{
		m_FrameContext->SetCurrentPacket( pNMListView->iItem );

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

		LVCachedItem	cacheditem;
		CacheItem(pItem->iItem, &cacheditem);

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
	GetListCtrl().InsertItem( lineNo, "" );
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

		case 3:		// Address
					if ( ppkt->packetHdr.packetType == msgData )
						lstrcpy(pszFill, "VTS Message" );
					else
					{
						lstrcpy(pszFill, ppkt->packetHdr.packetType == rxData ? "  -> " : "");
						lstrcat(pszFill, ppkt->GetAddressString(m_FrameContext->m_pDoc, ppkt->packetHdr.packetType));
					}
					break;

		case 4:		//Type of BACnet message
					m_summary.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen);
					lstrcpy(pszFill, m_summary.summaryLine );
					break;
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

