// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "VTS.h"

#include "ChildFrm.h"

#include "SummaryView.h"
#include "DetailView.h"
#include "HexView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_CANCELMODE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PORTS, OnUpdateEditPorts)
	ON_UPDATE_COMMAND_UI(ID_EDIT_NAMES, OnUpdateEditNames)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIRSTFRAME, OnUpdateViewFirstFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LASTFRAME, OnUpdateViewLastFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTFRAME, OnUpdateViewNextFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVFRAME, OnUpdateViewPrevFrame)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_PORTS, OnEditPorts)
	ON_COMMAND(ID_EDIT_NAMES, OnEditNames)
	ON_COMMAND(ID_EDIT_PREFERENCES, OnEditPreferences)
	ON_COMMAND(ID_VIEW_FIRSTFRAME, OnViewFirstFrame)
	ON_COMMAND(ID_VIEW_PREVFRAME, OnViewPrevFrame)
	ON_COMMAND(ID_VIEW_NEXTFRAME, OnViewNextFrame)
	ON_COMMAND(ID_VIEW_LASTFRAME, OnViewLastFrame)
	ON_COMMAND(ID_SEND_NEWPACKET, OnSendNewPacket)
	ON_COMMAND_RANGE( 0x8100, 0x81FF, OnSendSelectPort)
	ON_COMMAND_RANGE( 0x8200, 0x82FF, OnSendSelectPacket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	//
	//	NOTE: the document and frame count for context is initialize in 
	//	OnCreateClient().
	//
	m_frameContext = new CFrameContext();
}

CChildFrame::~CChildFrame()
{
	delete m_frameContext;
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// Let the clients know which context to use
	gNewFrameContext = m_frameContext;

	// Tell the frame context which document, and get frame count.  Be careful
	// because the frame count in the document might not be fully initialized 
	// if this is a new frame for a new document, VTSDoc::OnNewDocument() has
	// not been called yet.
	((VTSDoc*)pContext->m_pCurrentDoc)->BindFrameContext( m_frameContext );
	
	// create a splitter with 1 row, 2 columns
	if (!m_wndSplit1.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	// add the first splitter pane - the summary view in column 0
	if (!m_wndSplit1.CreateView(0, 0,
		RUNTIME_CLASS(CSummaryView), CSize(200, 50), pContext))
	{
		TRACE0("Failed to create summary pane\n");
		return FALSE;
	}
	
	// nice to have around
	m_pSummaryView = (CSummaryView*)m_wndSplit1.GetPane( 0, 0 );

	// add the second splitter pane - which is a nested splitter with 2 rows
	if (!m_wndSplit2.CreateStatic(
		&m_wndSplit1,     // our parent window is the first splitter
		2, 1,               // the new splitter is 2 rows, 1 column
		WS_CHILD | WS_VISIBLE | WS_BORDER,  // style, WS_BORDER is needed
		m_wndSplit1.IdFromRowCol(0, 1)
			// new splitter is in the first row, 2nd column of first splitter
	   ))
	{
		TRACE0("Failed to create nested splitter\n");
		return FALSE;
	}
	
	// now create the two views inside the nested splitter
	int cyHalf = max(lpcs->cy / 2, 20);
	
	// detail view gets top half
	if (!m_wndSplit2.CreateView(0, 0,
		RUNTIME_CLASS(CDetailView), CSize(0, cyHalf), pContext))
	{
		TRACE0("Failed to create detail pane\n");
		return FALSE;
	}
	
	// nice to have around
	m_pDetailView = (CDetailView*)m_wndSplit2.GetPane( 0, 0 );

	// hex view gets bottom half
	if (!m_wndSplit2.CreateView(1, 0,
		RUNTIME_CLASS(CHexView), CSize(0, 0), pContext))
	{
		TRACE0("Failed to create hex pane\n");
		return FALSE;
	}

	// nice to have around
	m_pHexView = (CHexView*)m_wndSplit2.GetPane( 1, 0 );
	
	// make sure this isn't used
	gNewFrameContext = NULL;

	// set up the tab ring
	m_pSummaryView->m_pTabRing = m_pDetailView;
	m_pDetailView->m_pTabRing = m_pHexView;
	m_pHexView->m_pTabRing = m_pSummaryView;

	// it all worked, we now have two splitter windows which contain
	//  three different views
	return TRUE;
}

void CChildFrame::OnCancelMode() 
{
	CMDIChildWnd::OnCancelMode();
	
	TRACE0( "CChildFrame::OnCancelMode() ?\n" );
}

void CChildFrame::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateEditDelete()\n" );
	pCmdUI->Enable( (m_frameContext->m_PacketCount > 0) );
}

void CChildFrame::OnUpdateEditPorts(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateEditPorts()\n" );
	pCmdUI->Enable( true );
}

void CChildFrame::OnUpdateEditNames(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateEditNames()\n" );
	pCmdUI->Enable( true );
}

void CChildFrame::OnUpdateViewFirstFrame(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateViewFirstFrame()\n" );
	pCmdUI->Enable( (m_frameContext->m_PacketCount > 0) && (m_frameContext->m_CurrentPacket != 0) );
}

void CChildFrame::OnUpdateViewPrevFrame(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateViewPrevFrame()\n" );
	pCmdUI->Enable( (m_frameContext->m_PacketCount > 0) && (m_frameContext->m_CurrentPacket > 0) );
}

void CChildFrame::OnUpdateViewNextFrame(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateViewNextFrame()\n" );
	pCmdUI->Enable( (m_frameContext->m_PacketCount > 0) && (m_frameContext->m_CurrentPacket >= 0) && (m_frameContext->m_CurrentPacket < (m_frameContext->m_PacketCount - 1)) );
}

void CChildFrame::OnUpdateViewLastFrame(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateViewLastFrame()\n" );
	pCmdUI->Enable( (m_frameContext->m_PacketCount > 0) && (m_frameContext->m_CurrentPacket != (m_frameContext->m_PacketCount - 1)) );
}

void CChildFrame::OnUpdateViewSend(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateViewSend()\n" );
	pCmdUI->Enable( true );
}

void CChildFrame::OnEditDelete() 
{
	m_frameContext->m_pDoc->DeletePackets();
}

void CChildFrame::OnEditPorts() 
{
	m_frameContext->m_pDoc->DoPortsDialog();
}

void CChildFrame::OnEditNames() 
{
	m_frameContext->m_pDoc->DoNamesDialog();
}

void CChildFrame::OnEditPreferences()
{
	m_frameContext->m_pDoc->DoPreferencesDialog();
}

void CChildFrame::OnViewFirstFrame() 
{
	m_frameContext->SetCurrentPacket( 0 );
}

void CChildFrame::OnViewPrevFrame() 
{
	m_frameContext->SetCurrentPacket( m_frameContext->m_CurrentPacket - 1 );
}

void CChildFrame::OnViewNextFrame() 
{
	m_frameContext->SetCurrentPacket( m_frameContext->m_CurrentPacket + 1 );
}

void CChildFrame::OnViewLastFrame() 
{
	m_frameContext->SetCurrentPacket( m_frameContext->m_PacketCount - 1 );
}

void CChildFrame::OnSendNewPacket() 
{
	m_frameContext->m_pDoc->DoSendWindow( -1, -1 );
}

//
//	CChildFrame::OnSendSelectPort
//
//	Selecting a port from the Send menu will change the content below the packet list
//	to reflect the types of packets available according to that port type.  Because the 
//	menu is built every time the menu is clicked on (or any other time that 
//	CMainFrame::OnInitMenuPopup is called) the only thing to do is set the global.
//
//	Note that because the port list is global, you can select a port that is not in the 
//	foreground session.
//

extern int gSelectedPort;

void CChildFrame::OnSendSelectPort( UINT uID )
{
	TRACE1( "Select port 0x%04X\n", uID );
	gSelectedPort = (uID & 0x00FF);
}

//
//	CChildFrame::OnSendSelectPacket
//

void CChildFrame::OnSendSelectPacket( UINT uID )
{
	TRACE1( "Select packet 0x%04X\n", uID );
	m_frameContext->m_pDoc->DoSendWindow( (uID >> 4) & 0x0F, uID & 0x0F );
}

//
//	CChildFrame::ActivateFrame
//
//	This interesting little chunk of code sucked right out of the tech notes
//	opens a child window maximized if there are no other child windows open.
//

void CChildFrame::ActivateFrame(int nCmdShow) 
{
	// if another window is open, use default
	if(GetMDIFrame()->MDIGetActive())
		CMDIChildWnd::ActivateFrame(nCmdShow); 
	else
		CMDIChildWnd::ActivateFrame(SW_SHOWMAXIMIZED); // else open maximized
}
