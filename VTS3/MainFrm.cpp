// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSDoc.h"

#include "MainFrm.h"

#include "SendPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int gSelectedPort = -1;		// which port selected for Send menu
int gSelectedGroup = -1;
int gSelectedItem = -1;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_NEW, OnWindowNew)
	ON_WM_INITMENUPOPUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );
	EnableDocking( CBRS_ALIGN_ANY );
	DockControlBar( &m_wndToolBar );

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnWindowNew() 
{
	//
	//	###
	//
	//	Somewhere in this glop of code should be a few hints on how to build 
	//	an MDI template that uses a different frame for the same document. 
	//	There should be two versions of creating a new window: one that shares 
	//	the current frame with an existing window and one that maintains its 
	//	own context.  Double clicking on a frame in a summary view should open 
	//	a detail (only) view.
	//
	//	Someday it would be nice to have summary/detail/hex as options in the 
	//	window menu that shows/hides that view from the current window.
	//
	CMDIFrameWnd::OnWindowNew();
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	static int		gCount = 0
	;

	CMDIFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	// skip all but the Send menu
	if (pPopupMenu->GetMenuItemID(0) != ID_SEND_NEWPACKET)
		return;

	// delete existing dynamic content
	while (pPopupMenu->DeleteMenu( 2, MF_BYPOSITION ))
		;

	int	portListLen = gMasterPortList.Length()
	;

	// if there are no ports, we're done
	if ((portListLen == 0) || (gSelectedPort >= portListLen)) {
		gSelectedPort = -1;
		return;
	}

	// if there is only only port, select it
	if (portListLen == 1)
		gSelectedPort = 0;

	// add a separator
	pPopupMenu->AppendMenu( MF_SEPARATOR );

	// add the ports
	for (int k = 0; k < portListLen; k++)
		pPopupMenu->AppendMenu( MF_STRING | (gSelectedPort == k ? MF_CHECKED : 0)
			,	0x8100 + k
			,	gMasterPortList[k]->portDesc.portName
			);

	// if there is no selected port, we're done
	if (gSelectedPort == -1)
		return;

	// add each group of packets as defined by the selected port
	CSendGroupList	curGroupList = gMasterPortList[gSelectedPort]->portSendGroup
	;

	if (!curGroupList)
		return;

	// add a separator
	pPopupMenu->AppendMenu( MF_SEPARATOR );

	CMenu	menuPopup
	;

	// establish each group in the tree
	for (int i = 0; curGroupList[i]; i++) {
		CSendGroupPtr	curGrp = curGroupList[i];

		// create a popup menu
		menuPopup.CreatePopupMenu();

		// if this has items then install them
		if (curGrp->groupItemList)
			for (int j = 0; curGrp->groupItemList[j].itemName; j++)
				menuPopup.AppendMenu( MF_STRING
					, 0x8200 + (i << 4) + j
					, curGrp->groupItemList[j].itemName
					);

		// use the group title as the popup name
		pPopupMenu->AppendMenu( MF_POPUP, (UINT)menuPopup.Detach(), curGrp->groupName );
	}
}
