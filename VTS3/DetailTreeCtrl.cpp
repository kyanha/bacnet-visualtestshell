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

/*
	Modified by Lei Chengxin   2003-7-21
	  to hide the tagging information and display more decoded information
*/
void CDetailTreeCtrl::ShowDetail()
{
	SetRedraw(FALSE);
	DeleteAllItems();
	
	int currentToken=0;
	bool inParsing=false;

	bool ALdetail = false;		// added by Lei Chengxin, true if it has AL detail

	int lastOffset=0,lastLen=0,endOffset=0;
	TVINSERTSTRUCT tvInsert;
	HTREEITEM hLastRoot=NULL;
	HTREEITEM hLastALRoot=NULL;

	HTREEITEM hLastParentRoot=NULL;
	HTREEITEM hLastChildRoot=NULL;
	HTREEITEM hLastNode=NULL;

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
			ALdetail = true;
			hLastNode = hLastRoot;
			continue;
		}

		if(hLastRoot!=NULL)
		{
			if(inParsing)
			{
				if(ALdetail)
				{
					if(m_FrameContext->m_PacketInfo.detailLine[i]->piNodeType == 1)
					{
						tvInsert.hParent = hLastRoot;
						hLastParentRoot = InsertItem(&tvInsert);
						SetItemData( hLastParentRoot , index++ );
						hLastNode = hLastParentRoot;
						endOffset = m_FrameContext->m_PacketInfo.detailLine[i]->piOffset
							+m_FrameContext->m_PacketInfo.detailLine[i]->piLen;
					}
					else if(m_FrameContext->m_PacketInfo.detailLine[i]->piNodeType == 2)
					{
						tvInsert.hParent = hLastParentRoot;
						hLastChildRoot = InsertItem(&tvInsert);
						SetItemData( hLastChildRoot , index++ );
						hLastNode = hLastChildRoot;
						endOffset = m_FrameContext->m_PacketInfo.detailLine[i]->piOffset
							+m_FrameContext->m_PacketInfo.detailLine[i]->piLen;
					}
					else
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
								if(m_FrameContext->m_PacketInfo.detailLine[i]->piOffset < endOffset)
									tvInsert.hParent=hLastNode;
								else
									tvInsert.hParent=hLastRoot;
								hLastALRoot=InsertItem(&tvInsert);
								SetItemData(hLastALRoot,index++);
								lastOffset=m_FrameContext->m_PacketInfo.detailLine[i]->piOffset;
							}	
							else{
								if(m_FrameContext->m_PacketInfo.detailLine[i]->piNodeType == 3)
									tvInsert.hParent=hLastRoot;
								else
									tvInsert.hParent=hLastNode;
								HTREEITEM temp=InsertItem(&tvInsert);
								SetItemData(temp,index++);
							}
					}
				}
				else
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
	
	if(!ALdetail){
		Expand(hLastRoot,TVE_EXPAND);
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

