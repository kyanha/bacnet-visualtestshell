// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "VTS.h"

#include "ChildFrm.h"

#include "SummaryView.h"
#include "ListSummaryView.h"

#include "DetailView.h"
#include "HexView.h"
#include "VTSStatisticsCollector.h"



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
	ON_UPDATE_COMMAND_UI(ID_EDIT_DEVICES, OnUpdateEditDevices)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIRSTFRAME, OnUpdateViewFirstFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LASTFRAME, OnUpdateViewLastFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTFRAME, OnUpdateViewNextFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVFRAME, OnUpdateViewPrevFrame)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_PORTS, OnEditPorts)
	ON_COMMAND(ID_EDIT_NAMES, OnEditNames)
	ON_COMMAND(ID_EDIT_DEVICES, OnEditDevices)
	ON_COMMAND(ID_EDIT_PREFERENCES, OnEditPreferences)
	ON_COMMAND(ID_VIEW_FIRSTFRAME, OnViewFirstFrame)
	ON_COMMAND(ID_VIEW_PREVFRAME, OnViewPrevFrame)
	ON_COMMAND(ID_VIEW_NEXTFRAME, OnViewNextFrame)
	ON_COMMAND(ID_VIEW_LASTFRAME, OnViewLastFrame)
	ON_COMMAND(ID_SEND_NEWPACKET, OnSendNewPacket)
	ON_COMMAND_RANGE( 0x8100, 0x81FF, OnSendSelectPort)
	ON_COMMAND_RANGE( 0x8200, 0x82FF, OnSendSelectPacket)
	ON_COMMAND(ID_VIEW_DETAIL, OnViewDetail)
	ON_COMMAND(ID_VIEW_HEX, OnViewHex)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DETAIL, OnUpdateViewDetail)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HEX, OnUpdateViewHex)
	// Added by Yajun Zhou, 2002-7-24
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	///////////////////////////////////////////
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
	delete m_pwndDetailViewBar;
	delete m_pwndHexViewBar;
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


	// it all worked, we now have two splitter windows which contain
	//  three different views
	m_pwndHexViewBar=new CDockingHexViewBar(pContext);
	m_pwndDetailViewBar=new CDockingDetailViewBar(pContext);

	if (!m_pwndHexViewBar->Create(_T("Hex View"), this, 123))
	{
    TRACE0("Failed to create mybar\n");
    return -1;
    // fail to create
	}
	m_pwndHexViewBar->SetBarStyle(m_pwndHexViewBar->GetBarStyle() |
    CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_pwndHexViewBar->SetSCBStyle(SCBS_SIZECHILD);
	m_pwndHexViewBar->EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	#ifdef _SCB_REPLACE_MINIFRAME
    m_pFloatingFrameClass = RUNTIME_CLASS(CSCBMiniDockFrameWnd);
	#endif //_SCB_REPLACE_MINIFRAME
	
	
	if (!m_pwndDetailViewBar->Create(_T("Detail View"), this, 123))
	{
    TRACE0("Failed to create mybar\n");
    return -1;
    // fail to create
	}
	m_pwndDetailViewBar->SetBarStyle(m_pwndDetailViewBar->GetBarStyle() |
    CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_pwndDetailViewBar->SetSCBStyle(SCBS_SIZECHILD);

	m_pwndDetailViewBar->EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);
	#ifdef _SCB_REPLACE_MINIFRAME
    m_pFloatingFrameClass = RUNTIME_CLASS(CSCBMiniDockFrameWnd);
	#endif //_SCB_REPLACE_MINIFRAME
	
	DockControlBar(m_pwndDetailViewBar,AFX_IDW_DOCKBAR_RIGHT);
	DockControlBar(m_pwndHexViewBar, AFX_IDW_DOCKBAR_BOTTOM);
//	DockControlBar(m_pwndHexViewBar, AFX_IDW_DOCKBAR_RIGHT);
	CMDIChildWnd::OnCreateClient(lpcs,pContext);
	

	m_pDetailView = m_pwndDetailViewBar->m_pList;
	m_pHexView = m_pwndHexViewBar->m_pHexView;
	m_pSummaryView=(CListSummaryView *)GetDlgItem(AFX_IDW_PANE_FIRST);
	

	// set up the tab ring
	m_pSummaryView->m_pTabRing = m_pDetailView;
	m_pDetailView->m_pTabRing = m_pHexView;
	m_pHexView->m_pTabRing = m_pSummaryView;
	
	// make sure this isn't used
	gNewFrameContext = NULL;


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

void CChildFrame::OnUpdateEditDevices(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateEditDevices()\n" );
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
	delete m_frameContext->m_pDoc->m_pStatisticsCollector;
	m_frameContext->m_pDoc->m_pStatisticsCollector=new VTSStatisticsCollector();
}

void CChildFrame::OnEditPorts() 
{
	m_frameContext->m_pDoc->DoPortsDialog();
}

void CChildFrame::OnEditNames() 
{
	m_frameContext->m_pDoc->DoNamesDialog();
}

void CChildFrame::OnEditDevices() 
{
	m_frameContext->m_pDoc->DoDevicesDialog();
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

void CChildFrame::OnViewDetail() 
{
	ShowControlBar(m_pwndDetailViewBar, !m_pwndDetailViewBar->IsVisible(), FALSE);
}

void CChildFrame::OnViewHex() 
{	
	ShowControlBar(m_pwndHexViewBar, !m_pwndHexViewBar->IsVisible(), FALSE);
}

void CChildFrame::OnUpdateViewDetail(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_pwndDetailViewBar->IsVisible());
}

void CChildFrame::OnUpdateViewHex(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_pwndHexViewBar->IsVisible());
}

// Added by Yajun Zhou, 2002-7-24
void CChildFrame::OnFileExport()
{
	static char BASED_CODE pchFilter[]="TXT Files(*.txt)|*.txt||";
	CFileDialog exportFileDlg(FALSE,NULL,NULL,NULL,pchFilter,NULL);
	
	if(exportFileDlg.DoModal() == IDOK)
	{
		CString strPathName = exportFileDlg.GetPathName();
		CString strFileFullName;
		if(exportFileDlg.GetFileExt() != "txt")
			strFileFullName = strPathName + ".txt";
		else
			strFileFullName = strPathName;
		
		int nNameLength = strFileFullName.GetLength();

		int nPacketCount = m_frameContext->m_PacketCount;
		int nCurrentPacket = m_frameContext->m_CurrentPacket;
		
		CFile exportFile;
		try
		{
			exportFile.Open(strFileFullName.GetBuffer(nNameLength), 
				CFile::modeCreate|CFile::modeWrite);//|CFile::typeText
		
			CString* pszLine;
			char cLineEnd[2];
			cLineEnd[0] = 0x0d;
			cLineEnd[1] = 0x0a;

			CString strStartSeparator;
			CString strEndSeparator;

			CString strSummarySeparator	= "[ Summary Information ]";
			CString strDetailSeparator	= "[ Detail Information ]";

			CString strTagTime			= "Time:        ";
			CString strTagSource		= "Source:      ";
			CString strTagDestination	= "Destination: ";
			CString strTagSummary		= "Summary:     ";
			
			CString strTime;
			CString strSource;
			CString strDestination;
			CString strSummary;

			int i, j, nDetailCount;

			for(i = 0; i < nPacketCount; i++)
			{
				strStartSeparator.Format("%s%d%s",
					"*************************** [ Packet ",
					i,
					" Start ] ***************************");
				
				exportFile.Write(strStartSeparator.GetBuffer(1), strStartSeparator.GetLength());
				exportFile.Write(cLineEnd, 2);
				exportFile.Write(cLineEnd, 2);

				exportFile.Write(strSummarySeparator.GetBuffer(1), strSummarySeparator.GetLength());
				exportFile.Write(cLineEnd, 2);
				exportFile.Write(cLineEnd, 2);

				pszLine = m_pSummaryView->GetLineData(i);
				{
					pszLine->Delete(0,6);
					
					exportFile.Write(strTagTime.GetBuffer(1), strTagTime.GetLength());
					strTime = pszLine->Left(12);
					exportFile.Write(strTime.GetBuffer(1), strTime.GetLength());
					exportFile.Write(cLineEnd, 2);
					pszLine->Delete(0,13);

					exportFile.Write(strTagSource.GetBuffer(1), strTagSource.GetLength());
					strSource = pszLine->Left(9);
					strSource.TrimLeft();
					exportFile.Write(strSource.GetBuffer(1), strSource.GetLength());
					exportFile.Write(cLineEnd, 2);
					pszLine->Delete(0,10);

					exportFile.Write(strTagDestination.GetBuffer(1), strTagDestination.GetLength());
					strDestination = pszLine->Left(9);
					strDestination.TrimLeft();
					exportFile.Write(strDestination.GetBuffer(1), strDestination.GetLength());
					exportFile.Write(cLineEnd, 2);
					pszLine->Delete(0,10);
					
					exportFile.Write(strTagSummary.GetBuffer(1), strTagSummary.GetLength());
					exportFile.Write(pszLine->GetBuffer(1), pszLine->GetLength());
					exportFile.Write(cLineEnd, 2);
				}
				
				exportFile.Write(cLineEnd, 2);
				exportFile.Write(strDetailSeparator.GetBuffer(1), strDetailSeparator.GetLength());
				exportFile.Write(cLineEnd, 2);
				exportFile.Write(cLineEnd, 2);

				m_frameContext->SetCurrentPacket(i);

				nDetailCount = m_frameContext->m_PacketInfo.detailCount;
				for(j = 0; j < nDetailCount; j++)
				{
					pszLine->Format(m_frameContext->m_PacketInfo.detailLine[j]->piLine);
						//m_pDetailView->GetLineData(j);
					exportFile.Write(pszLine->GetBuffer(1), pszLine->GetLength());
					exportFile.Write(cLineEnd, 2);
				}

				strEndSeparator.Format("%s%d%s",
					"*************************** [ Packet ",
					i,
					" End ] ***************************");
				
				exportFile.Write(strEndSeparator.GetBuffer(1), strEndSeparator.GetLength());
				exportFile.Write(cLineEnd, 2);

				exportFile.Write(cLineEnd, 2);
				exportFile.Write(cLineEnd, 2);
			}
			exportFile.Close();

		}
		catch(CFileException e)
		{
			CString str;
			e.GetErrorMessage(str.GetBuffer(1), 1);
			AfxMessageBox(str);
			return;
		}

		if(nCurrentPacket != -1)
			m_frameContext->SetCurrentPacket(nCurrentPacket);
	}

	
}
///////////////////////////////////////////////////////////
