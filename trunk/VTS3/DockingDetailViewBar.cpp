// DockingDetailViewBar.cpp: implementation of the CDockingDetailViewBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DockingDetailViewBar.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDockingDetailViewBar::CDockingDetailViewBar(CCreateContext* pContext)
{
	m_pContext = pContext;

	CRuntimeClass* pFactory = RUNTIME_CLASS(CDetailView);
    m_pDetailView = (CDetailView *)(pFactory->CreateObject() );
}

CDockingDetailViewBar::~CDockingDetailViewBar()
{

}


BEGIN_MESSAGE_MAP(CDockingDetailViewBar, baseCMyBar)
	//{{AFX_MSG_MAP(CDockingDetailViewBar)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CDockingDetailViewBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (baseCMyBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	CRect r;
	GetParentFrame()->GetClientRect(&r);
	//ClientToScreen(&r);
	// TODO: Add your specialized creation code here
	if (m_pDetailView )
   {
       m_pDetailView->CreateView(this,m_pContext,r);
   }

	return 0;
}
