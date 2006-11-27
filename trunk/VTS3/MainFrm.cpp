// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSDoc.h"

#include "MainFrm.h"
#include "ScriptFrame.h"
#include "ScriptExecutor.h"
#include "ChildFrm.h"
#include "SendPage.h"
#include <atlbase.h>

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
	ON_WM_SIZE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//Modifieded by Zhu Zhennhua, 2004-11-27, move the functions to CChildFrame
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
//	Added by Yajun Zhou, 2002-4-22
// madanner, 5/03 moved to frame of edit
//	ID_LNCOLINDEX,
/////////////////////////
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

//Added by Zhenhua Zhu, 2003-5-26
//CallBack Func To catch the keyUp, to support the hotkey for Toolbar
//Modified by Zhu Zhenhua, 2004-11-27,for #508589 request 

///////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
	SaveReg();
}

//Modifed by Zhu Zhenhua, 2004-11-27,for #508589 request, 
//remove the keyboard hook function,
//because i think it only need accelerator for CChildFrame,no need for ScriptFrame
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
	
	int COMBOBOX_WIDTH = 180; //the width of the combo box
    
    //First get the index of the placeholder's position in the toolbar
    int index = 0;
    while(m_wndToolBar.GetItemID(index)!= ID_COMBOBOX_SEPARATOR) 
		index++;

	CRect rect;
    //next convert that button to a seperator and get its position
    m_wndToolBar.SetButtonInfo(index, ID_COMBOBOX_SEPARATOR,
                               TBBS_SEPARATOR, COMBOBOX_WIDTH);
    m_wndToolBar.GetItemRect(index, &rect);

    //expand the rectangle to allow the combo box room to drop down
    
    rect.bottom += 200;

    // then .Create the combo box and show it
    if (!m_wndFileCombo.Create(WS_CHILD|WS_VISIBLE | CBS_AUTOHSCROLL | CBS_DROPDOWN |
		CBS_HASSTRINGS, rect, &m_wndToolBar, ID_COMBOBOX_SEPARATOR))
    {
       TRACE0("Failed to create combo-box\n");
       return FALSE;
    }

	ReadReg();
	m_wndFileCombo.SetCurSel(0);

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
	
/////////////////////////////////////////////////////
	DWORD dwExStyle = TBSTYLE_EX_DRAWDDARROWS;
	m_wndToolBar.GetToolBarCtrl().SendMessage(TB_SETEXTENDEDSTYLE, 0, (LPARAM)dwExStyle);

	DWORD dwStyle = m_wndToolBar.GetButtonStyle(m_wndToolBar.CommandToIndex(ID_SCRIPT_RUN1));
	dwStyle |= TBSTYLE_DROPDOWN;
	m_wndToolBar.SetButtonStyle(m_wndToolBar.CommandToIndex(ID_SCRIPT_RUN1), dwStyle);
	m_wndToolBar.GetToolBarCtrl().SendMessage(TB_SETEXTENDEDSTYLE, 0, (LPARAM)dwExStyle);
	dwStyle = m_wndToolBar.GetButtonStyle(m_wndToolBar.CommandToIndex(ID_SCRIPT_CHECK_SYNTAX1));
	dwStyle |= TBSTYLE_DROPDOWN;
	m_wndToolBar.SetButtonStyle(m_wndToolBar.CommandToIndex(ID_SCRIPT_CHECK_SYNTAX1), dwStyle);
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

//	int	portListLen = gMasterPortList.Length();
// MAD_DB
	VTSDoc * pdoc = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();
	VTSPorts * pports = pdoc == NULL ? NULL : pdoc->GetPorts();
	int portListLen = pports == NULL ? 0 : pports->GetSize();

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
	CString strPortMenuItem;

	for (int k = 0; k < portListLen; k++)
	{
//MAD_DB		pPopupMenu->AppendMenu( MF_STRING | (gSelectedPort == k ? MF_CHECKED : 0),	0x8100 + k,	gMasterPortList[k]->portDesc.portName );
		strPortMenuItem.Format("%s (%s)", (*pports)[k]->GetName(), (*pports)[k]->GetTypeDesc() );
		pPopupMenu->AppendMenu( MF_STRING | (gSelectedPort == k ? MF_CHECKED : 0) | ((*pports)[k]->IsEnabled() ? 0 : MF_GRAYED), 0x8100 + k,	strPortMenuItem );
	}

	// if there is no selected port, we're done
	if (gSelectedPort == -1)
		return;

	// add each group of packets as defined by the selected port
//	CSendGroupList	curGroupList = gMasterPortList[gSelectedPort]->portSendGroup;
	CSendGroupList	curGroupList = (*pports)[gSelectedPort]->portSendGroup;

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

// madanner, 5/03 moved to frame of edit
//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-4-22
//	Purpose:	Display the line index on the status bar
//	In:			CString str: The information that should 
//				be display. 
//	Out:		void
//******************************************************************
// 
//void CMainFrame::SetLnPaneText(CString str)
//{
//	int i = m_wndStatusBar.CommandToIndex(ID_LNCOLINDEX);
//	m_wndStatusBar.SetPaneText(i, str);
//


CProgressCtrl * CMainFrame::InitializeProgress( void )
{
	RECT rect;

	if ( !::IsWindow(m_progress.m_hWnd) )
	{
		m_wndStatusBar.GetItemRect(m_wndStatusBar.CommandToIndex(ID_SEPARATOR), &rect);  
		m_progress.Create(WS_VISIBLE | WS_CHILD | WS_BORDER | PBS_SMOOTH, rect, &m_wndStatusBar, 1); 
	}

	return &m_progress;
}


void CMainFrame::ReleaseProgress( void )
{
	if ( ::IsWindow(m_progress.m_hWnd) )
		m_progress.DestroyWindow();
}


void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIFrameWnd::OnSize(nType, cx, cy);
	
	if ( ::IsWindow(m_wndStatusBar.m_hWnd) &&  ::IsWindow( m_progress.m_hWnd) )
    {
		RECT	rect;
		m_wndStatusBar.GetItemRect(m_wndStatusBar.CommandToIndex(ID_SEPARATOR), &rect);  

		// Reposition the progress control correctly!
		m_progress.SetWindowPos(&wndTop, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
    }
}

void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CMDIFrameWnd::OnClose();
}

	//Added by Zhu Zhenhua, 2004-11-27, to control mainframe toobar's show up while in Childframe
void CMainFrame::ShowOwnToolBar()
{
  ShowControlBar(&m_wndToolBar, !m_wndToolBar.IsVisible(), FALSE);	
}
bool CMainFrame::GetToolBarStatus()
{
	return (m_wndToolBar.IsVisible()? true: false);
}

//Added by Zhenhua Zhu, 2003-5-26
// To get the Special MDIChildWnd of CMainFrame(Not opened return NULL)
CMDIChildWnd* CMainFrame::GetChildFrame(CRuntimeClass *pClass, int nNum)
{	CView* pView;
	CMDIChildWnd* pFrame;
	int i = 1;
	bool bNumadd = false;
	POSITION pos1 = AfxGetApp()->GetFirstDocTemplatePosition();
	while(pos1 != NULL)
	{	
		CDocTemplate* pDocTemplate = AfxGetApp()->GetNextDocTemplate(pos1);
		POSITION pos2 = pDocTemplate->GetFirstDocPosition();
		while(pos2 != NULL)
		{	
			CDocument* pDoc = pDocTemplate->GetNextDoc(pos2);
			POSITION pos3 = pDoc->GetFirstViewPosition();
			while (pos3 != NULL)
			{	
				pView = pDoc->GetNextView(pos3);
				pFrame = (CMDIChildWnd *)pView->GetParentFrame();
				if(pFrame != NULL && pFrame->IsKindOf(pClass))
				{	if(i == nNum)
					return pFrame;
					bNumadd = true;
				}
			}
			if(bNumadd)
				i++;
			
		}
	}
	return NULL;
}

//read register table and initialize log file combobox
void CMainFrame::ReadReg()
{
	CRegKey regKey;
	long lRet = regKey.Open(HKEY_LOCAL_MACHINE,
		"Software\\vts3\\logfiles");
	if(lRet != ERROR_SUCCESS)
	{
		regKey.Create(HKEY_LOCAL_MACHINE,
		"Software\\vts3\\logfiles");
	}
	else
	{
		CString keyName;	
		CString filename;		
		for(int index = 0; index < HISTORY_LOGFILE_COUNT; index++)
		{
			TCHAR keyValue[MAX_PATH] = _T("");
			DWORD bufferLen = MAX_PATH;
			keyName.Format("%d", index);
			regKey.QueryValue(keyValue, (LPCTSTR)keyName, &bufferLen);
			filename = keyValue;
			if( !filename.IsEmpty() )
			{
				m_historyLogList.AddTail(filename);
				m_wndFileCombo.AddString(filename);
			}			
		}		
	}	
	regKey.Close();
}

void CMainFrame::SaveReg()
{
	CRegKey regKey;
	CString keyName;
	
	long lRet = regKey.Open(HKEY_LOCAL_MACHINE,
		"Software\\vts3\\logfiles");

	ASSERT(m_historyLogList.GetCount() <= HISTORY_LOGFILE_COUNT);

	for(int index = 0; index < m_historyLogList.GetCount(); index++)
	{
		keyName.Format("%d", index);
		regKey.SetValue((LPCTSTR)m_historyLogList.GetAt(m_historyLogList.FindIndex(index)), 
			(LPCTSTR)keyName);		
	}	
}

void CMainFrame::SaveLogFileHistory(LPCTSTR filepath)
{
	POSITION pos = m_historyLogList.Find(CString(filepath));
	
	if(pos)
	{
		m_historyLogList.RemoveAt(pos);
		m_historyLogList.AddHead(CString(filepath));
	}
	else
	{
		if(m_historyLogList.GetCount() == HISTORY_LOGFILE_COUNT)
			m_historyLogList.RemoveTail();
		m_historyLogList.AddHead(CString(filepath));
	}
	
	while( m_wndFileCombo.GetCount() )
		m_wndFileCombo.DeleteString(0);
	
	for(int index = 0; index < m_historyLogList.GetCount(); index++)
	{
		m_wndFileCombo.AddString(m_historyLogList.GetAt(m_historyLogList.FindIndex(index)));
	}

	if(m_wndFileCombo.GetCount())
		m_wndFileCombo.SetCurSel(0);
}