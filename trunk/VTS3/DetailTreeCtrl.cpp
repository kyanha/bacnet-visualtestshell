// DetailTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "DetailTreeCtrl.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDetailTreeCtrl

CDetailTreeCtrl::CDetailTreeCtrl()
{
}

CDetailTreeCtrl::~CDetailTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CDetailTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CDetailTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDetailTreeCtrl message handlers
void CDetailTreeCtrl::ContextChange( CFrameContext::Signal s )
{
	switch (s) {
		case CFrameContext::eNewCurrentPacket:
			m_FrameContext->SetCurrentDetail( -1 );
			if (m_FrameContext->m_CurrentPacket == -1)
			{
				m_FrameContext->SetDetailCount( 0 );
				SetRedraw(FALSE);
				DeleteAllItems();
				SetRedraw(TRUE);
			}
			else
			{
				m_FrameContext->SetDetailCount( m_FrameContext->m_PacketInfo.detailCount );
				ShowDetail();
			}
			break;
			
		case CFrameContext::eNewDetailCount:
			break;
			
		case CFrameContext::eNewCurrentDetail:
			break;
	}
}

void CDetailTreeCtrl::ShowDetail()
{
	SetRedraw(FALSE);
	DeleteAllItems();
	
	int currentToken=0;
	bool inParsing=false;

	int lastOffset=0,lastLen=0;
	TVINSERTSTRUCT tvInsert;
	HTREEITEM hLastRoot=NULL;
	HTREEITEM hLastALRoot=NULL;

	int index=0;
	HTREEITEM hDLItem,hALItem;
	for(int i=0;i<m_FrameContext->m_PacketInfo.detailCount;i++)
	{
		CString temp(m_FrameContext->m_PacketInfo.detailLine[i]->piLine);
	
		tvInsert.hParent = NULL;
		tvInsert.hInsertAfter = NULL;
		tvInsert.item.mask = TVIF_TEXT;
		tvInsert.item.pszText = _T(temp.GetBuffer(0));

		if(temp=="----- IP Frame Detail -----")
		{
			hLastRoot =InsertItem(&tvInsert);
			SetItemData(hLastRoot,index++);
			hDLItem=hLastRoot; 
			continue;
		}

		if(temp=="----- Ethernet Frame Detail -----")
		{
			hLastRoot =InsertItem(&tvInsert);
			SetItemData(hLastRoot,index++);
			hDLItem=hLastRoot; 
			continue;
		}

		if(temp=="----- ARCNET Frame Detail -----")
		{
			hLastRoot =InsertItem(&tvInsert);
			SetItemData(hLastRoot,index++);
			hDLItem=hLastRoot; 
			continue;
		}

		if(temp=="----- BACnet MS/TP Frame Detail -----")
		{
			hLastRoot =InsertItem(&tvInsert);
			SetItemData(hLastRoot,index++);
			hDLItem=hLastRoot; 
			continue;
		}

		if(temp=="----- BACnet PTP Frame Detail -----")
		{
			hLastRoot =InsertItem(&tvInsert);
			SetItemData(hLastRoot,index++);
			hDLItem=hLastRoot; 
			continue;
		}
	
		if(temp=="----- BACnet Virtual Link Layer Detail -----")
		{
			hLastRoot =InsertItem(&tvInsert);
			SetItemData(hLastRoot,index++);
			continue;
		}
		
		if(temp=="----- BACnet Network Layer Detail -----")
		{
			hLastRoot =InsertItem(&tvInsert);
			SetItemData(hLastRoot,index++);
			inParsing=true;
			continue;
		}
				
		if(temp=="----- BACnet Application Layer Detail -----")
		{
			hLastRoot =InsertItem(&tvInsert);
			SetItemData(hLastRoot,index++);
			inParsing=true;
			hALItem=hLastRoot;
			continue;
		}

		if(hLastRoot!=NULL)
		{
			if(inParsing)
			{
				if(m_FrameContext->m_PacketInfo.detailLine[i]->piOffset==lastOffset)
				{
					tvInsert.hParent=hLastALRoot;
					HTREEITEM temp=InsertItem(&tvInsert);
					SetItemData(temp,index++);
					lastOffset=m_FrameContext->m_PacketInfo.detailLine[i]->piOffset;
				}
				else 
					if(m_FrameContext->m_PacketInfo.detailLine[i]->piLen!=0)
					{
						tvInsert.hParent=hLastRoot;
						hLastALRoot=InsertItem(&tvInsert);
						SetItemData(hLastALRoot,index++);
						lastOffset=m_FrameContext->m_PacketInfo.detailLine[i]->piOffset;
					}	
					else{
						tvInsert.hParent=hLastRoot;
						HTREEITEM temp=InsertItem(&tvInsert);
						SetItemData(temp,index++);
					}			
			}
			else
			{
				tvInsert.hParent=hLastRoot;
				HTREEITEM temp=InsertItem(&tvInsert);
				SetItemData(temp,index++);
			}
		}
		else
		{
			HTREEITEM temp1=InsertItem(&tvInsert);
			SetItemData(temp1,index++);
		}
	}
	
	Expand(hDLItem,TVE_EXPAND);
	Expand(hALItem,TVE_EXPAND);
	SetRedraw(TRUE);
}

void CDetailTreeCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	HTREEITEM sel=GetSelectedItem();
	
	//notify hex view to show selected field		
	m_FrameContext->SetCurrentDetail(GetItemData(sel));

	*pResult = 0;
}

