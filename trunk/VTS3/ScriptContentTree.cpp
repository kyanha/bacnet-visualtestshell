// ScriptContentTree.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"

#include "ScriptContentTree.h"
#include "ScriptDocument.h"
// Added by Yajun Zhou, 2002-6-20
#include "ScriptFrame.h"

#include "ScriptEditIncludeDlg.h"

//////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ScriptContentTree

IMPLEMENT_DYNCREATE(ScriptContentTree, CTreeView)

ScriptContentTree::ScriptContentTree()
	: m_pDoc(0)
	, m_pScriptContent(0)
{
	m_pframe = NULL;
}

ScriptContentTree::~ScriptContentTree()
{
	if (m_pScriptContent)
		delete m_pScriptContent;
}

BEGIN_MESSAGE_MAP(ScriptContentTree, CTreeView)
	//{{AFX_MSG_MAP(ScriptContentTree)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScriptContentTree drawing

void ScriptContentTree::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// ScriptContentTree diagnostics

#ifdef _DEBUG
void ScriptContentTree::AssertValid() const
{
	CTreeView::AssertValid();
}

void ScriptContentTree::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//
//	ScriptContentTree::PreCreateWindow
//

BOOL ScriptContentTree::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CTreeView::PreCreateWindow (cs))
		return FALSE;

	cs.style |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	return TRUE;
}

//
//	ScriptContentTree::OnCreate
//

int ScriptContentTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// get a copy of the control
	m_pTreeCtrl = &GetTreeCtrl();

	// initialize the status image list
	m_ilStatus.Create( IDB_CONTENTSTATUS, 16, 1, RGB(255,0,255) );
	m_pTreeCtrl->SetImageList( &m_ilStatus, TVSIL_NORMAL );

	return 0;
}

//
//	ScriptContentTree::Bind
//

void ScriptContentTree::Bind( ScriptBasePtr sbp )
{
	// delete the existing content
	if (m_pScriptContent)
		delete m_pScriptContent;

	// set to the new content
	m_pScriptContent = sbp;

	// make sure the document doesn't have a test selected
	m_pDoc->m_pSelectedTest = 0;
	m_pDoc->m_pSelectedSection = 0; // Added by Zhu Zhenhua, 2003-12-18, to run select section
	// delete the items from the tree
	m_pTreeCtrl->DeleteAllItems();

	// if there is no new content, return
	if (!sbp)
		return;

	// add the content
	Load( TVI_ROOT, sbp );
}

//
//	ScriptContentTree::Load
//

void ScriptContentTree::Load( HTREEITEM parent, ScriptBasePtr sbp )
{
	HTREEITEM	itm
	;

	for (int i = 0; i < sbp->Length(); i++) {
		// get the child
		ScriptBasePtr child = sbp->Child( i );

		// insert the item
		itm = m_pTreeCtrl->InsertItem( child->baseLabel
			, child->baseImage + child->baseStatus
			, child->baseImage + child->baseStatus
			, parent
			);

		// let the child know its handle
		child->baseItem = itm;

		// tell the list item to point back to the child
		m_pTreeCtrl->SetItemData( itm, (DWORD)child );

		// do all of its children
		Load( itm, child );
	}
}

//
//	ScriptContentTree::OnSelchanged
//

void ScriptContentTree::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM		hNewItem = m_pTreeCtrl->GetSelectedItem();
	ScriptBasePtr	pElem = (ScriptBasePtr)m_pTreeCtrl->GetItemData( hNewItem );

	m_pDoc->m_pSelectedTest = 0;
	m_pDoc->m_pSelectedSection = 0; // Added by Zhu Zhenhua, 2003-12-18, to run select section
	*pResult = 0;

	// Sometimes this method is called and it contains bogus data... IsKindOf didn't work (when I 
	// declared/implemented DYNAMIC, so let's just trap the problem before we go further...
	// This case covers NULL and something pointed to that is not a ScriptBase

	try {
		pElem->m_strFile.IsEmpty();
	} catch(...) {
		return;
	}

	//madanner 6/03, change status text for include filename if selected...
	if ( m_pframe != NULL )
	{
		CString strStatusText;
		if ( !pElem->m_strFile.IsEmpty() )
			strStatusText.Format("INCLUDE: [%s]", pElem->m_strFile );
		m_pframe->SetMessageText(strStatusText);
	}

	// if a test selected
	if (pElem->baseType == ScriptBase::scriptTest)
		m_pDoc->m_pSelectedTest = (ScriptTestPtr)pElem;

// Added by Zhu Zhenhua, 2003-12-18, to run select section
	if (pElem->baseType == ScriptBase::scriptSection)
		m_pDoc->m_pSelectedSection = (ScriptSectionPtr)pElem;
}

//
//	ScriptContentTree::OnDblclk
//

void ScriptContentTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TRACE0( "Double click\n" );

#if 0
	// test flipping the status
	if (m_pSelectedElement->baseType == scriptTest)
		m_pDoc->SetTestStatus( (ScriptTestPtr)m_pSelectedElement
			, (m_pSelectedElement->baseStatus + 1) % 4
			);
#endif

	*pResult = 0;
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-7-12
//	Purpose:	Auto-scroll to the interrelated text in the edit pane
//				when double-clicking on a test item.  
//******************************************************************
void ScriptContentTree::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	UINT uFlags;
	HTREEITEM htItem = m_pTreeCtrl->HitTest(point, &uFlags);
	
	if ((htItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		ScriptBasePtr	pElem = (ScriptBasePtr)m_pTreeCtrl->GetItemData( htItem );
		
		ScriptFrame* pCFrm = (ScriptFrame*)GetParentFrame();

		// madanner 6/03, add support for editing include files...
		if ( pElem->m_strFile.IsEmpty() )
		{
			pCFrm->SetActiveView(m_pEditView);
			m_pEditView->GotoLine(pElem->baseLineStart+1);
		}
		else
		{
			CScriptEditIncludeDlg	dlgInclude(pElem->m_strFile, NULL, NULL, pElem->baseLineStart+1, pCFrm);
			dlgInclude.DoModal();
		}
	}
	
	CTreeView::OnLButtonDblClk(nFlags, point);
}


//madanner 6/03, added to support different colors in tree for noded found in include file

BOOL ScriptContentTree::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	if (message == WM_NOTIFY)
	{
		NMHDR *pnmh = (LPNMHDR) lParam; 

		// Is the code set to custom drawing?
		if (pnmh->code == NM_CUSTOMDRAW)
		{
			LPNMTVCUSTOMDRAW pTVCustomDraw = (LPNMTVCUSTOMDRAW) lParam;

			if (pTVCustomDraw)
			{
				switch (pTVCustomDraw->nmcd.dwDrawStage)
				{
					case CDDS_PREPAINT:             *pLResult = CDRF_NOTIFYITEMDRAW;
													return TRUE;

					case CDDS_ITEMPREPAINT:
						{
							//HTREEITEM hItem = HTREEITEM(pTVCustomDraw->nmcd.dwItemSpec);
							ScriptBasePtr pbase = (ScriptBasePtr) pTVCustomDraw->nmcd.lItemlParam;

							try {
							if ( pbase != NULL  && !pbase->m_strFile.IsEmpty() )
								pTVCustomDraw->clrText = RGB(0,0,255);
							} catch (...) {}
						}
				}
			}
		}
	}

	return CTreeView::OnChildNotify(message, wParam, lParam, pLResult);
}

