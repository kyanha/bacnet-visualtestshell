// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "VTS.h"

#include "VTSPreferences.h"
#include "ChildFrm.h"
#include "MainFrm.h"

#include "SummaryView.h"
#include "ListSummaryView.h"

#include "DetailView.h"
#include "HexView.h"
#include "VTSStatisticsCollector.h"

// Added by Liangping Xu,2002-11
//madanner 6/03, moved out of ScriptFrame
#include "CheckEPICSCons.h"
#include "ScriptLoadResults.h"

#include "VTSPreferences.h"

///////////////////////////////
namespace PICS {
#include "vtsapi.h"
#include "dudapi.h"
}

extern PICS::PICSdb *gPICSdb;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern VTSPreferences gVTSPreferences;

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)


BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_CANCELMODE()
	ON_UPDATE_COMMAND_UI(ID_FILE_WKS_NEW, OnUpdateFileWksNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_WKS_SWITCH, OnUpdateFileWksSwitch)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REFRESH, OnUpdateEditRefresh)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PORTS, OnUpdateEditPorts)
	ON_UPDATE_COMMAND_UI(ID_EDIT_NAMES, OnUpdateEditNames)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DEVICES, OnUpdateEditDevices)
	ON_UPDATE_COMMAND_UI(ID_EDIT_LOGFILE, OnUpdateEditLogfile)
//	ON_UPDATE_COMMAND_UI(ID_FILE_WKS_SAVEAS, OnUpdateFileWksSaveAs)
//	ON_UPDATE_COMMAND_UI(AFX_IDS_FILESAVEAS, OnUpdateFileWksSaveAs)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIRSTFRAME, OnUpdateViewFirstFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LASTFRAME, OnUpdateViewLastFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTFRAME, OnUpdateViewNextFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVFRAME, OnUpdateViewPrevFrame)
	ON_UPDATE_COMMAND_UI(ID_EPICS_LOAD, OnUpdateEPICSLoad)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_REFRESH, OnEditRefresh)
	ON_COMMAND(ID_EDIT_PORTS, OnEditPorts)
	ON_COMMAND(ID_EDIT_NAMES, OnEditNames)
	ON_COMMAND(ID_EDIT_DEVICES, OnEditDevices)
	ON_COMMAND(ID_EDIT_LOGFILE, OnEditLogfile)
	ON_COMMAND(ID_EDIT_PREFERENCES, OnEditPreferences)
	ON_COMMAND_RANGE( 0x8100, 0x81FF, OnSendSelectPort)
	ON_COMMAND_RANGE( 0x8200, 0x82FF, OnSendSelectPacket)
	ON_COMMAND(ID_VIEW_FIRSTFRAME, OnViewFirstFrame)
	ON_COMMAND(ID_VIEW_PREVFRAME, OnViewPrevFrame)
	ON_COMMAND(ID_VIEW_NEXTFRAME, OnViewNextFrame)
	ON_COMMAND(ID_VIEW_LASTFRAME, OnViewLastFrame)
	ON_COMMAND(ID_SEND_NEWPACKET, OnSendNewPacket)
	ON_COMMAND(ID_VIEW_DETAIL, OnViewDetail)
	ON_COMMAND(ID_VIEW_HEX, OnViewHex)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DETAIL, OnUpdateViewDetail)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HEX, OnUpdateViewHex)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_EPICS_LOAD, OnEPICSLoad)
	ON_COMMAND(ID_EPICS_LOADAUTO, OnEPICSLoadAuto)
	ON_WM_DESTROY()
	//Added by Zhenhua ZHu, 2003-6-2
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_SETUP,OnFilePrintSetup)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT,OnUpdateFilePrint)
	////////////////////////////////////////////////
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
	m_bPrintSetup = false;
	m_bInPrinting = false;
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

	//MAD_DB
	// Get rid of the overlapping min, restore, max buttons... this should 
	// always be maximized.

	cs.style = (cs.style & ~WS_OVERLAPPEDWINDOW) | WS_OVERLAPPED | WS_CAPTION;

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
	
	if (!m_pwndDetailViewBar->Create(_T("Detail View"), this, 124))
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
	DockControlBar(m_pwndHexViewBar, AFX_IDW_DOCKBAR_RIGHT);
	
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
	m_pwndDetailViewBar->LoadState(_T("Detail Bar Status"));
	m_pwndHexViewBar->LoadState(_T("Hex Bar Status"));
	LoadBarState("Bar Status");
	
	return TRUE;
}

BOOL CChildFrame::DestroyWindow() 
{
	SaveBarStates();
	return CMDIChildWnd::DestroyWindow();
}


void CChildFrame::SaveBarStates()
{
	if(!m_pwndDetailViewBar->IsFloating()&&!m_pwndHexViewBar->IsFloating())
		SaveBarState("Bar Status");
	if(!m_pwndDetailViewBar->IsFloating())
		m_pwndDetailViewBar->SaveState(_T("Detail Bar Status"));
	if(!m_pwndHexViewBar->IsFloating())
		m_pwndHexViewBar->SaveState(_T("Hex Bar Status"));
}

void CChildFrame::OnDestroy() 
{
	SaveBarStates();
	CMDIChildWnd::OnDestroy();
}


void CChildFrame::OnCancelMode() 
{
	CMDIChildWnd::OnCancelMode();
	
	TRACE0( "CChildFrame::OnCancelMode() ?\n" );
}

void CChildFrame::OnUpdateFileWksNew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(true);
}

void CChildFrame::OnUpdateFileWksSwitch(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(true);
}

void CChildFrame::OnUpdateFileWksSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(true);
}

void CChildFrame::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
//	TRACE0( "VTSDoc::OnUpdateEditDelete()\n" );
	pCmdUI->Enable( (m_frameContext->m_PacketCount > 0) );
}

void CChildFrame::OnUpdateEditRefresh(CCmdUI* pCmdUI) 
{
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

void CChildFrame::OnUpdateEditLogfile(CCmdUI* pCmdUI) 
{
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
	if ( !gVTSPreferences.Setting_IsVerifyDelete() || AfxMessageBox(IDS_DELETEAREYOUSURE, MB_YESNO | MB_ICONQUESTION) == IDYES )
	{
		delete m_frameContext->m_pDoc->m_pStatisticsCollector;
		m_frameContext->m_pDoc->m_pStatisticsCollector=new VTSStatisticsCollector();
		m_frameContext->m_pDoc->DeletePackets();
	}
}

void CChildFrame::OnEditRefresh() 
{
	m_frameContext->m_pDoc->ReloadPacketStore();
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

void CChildFrame::OnEditLogfile() 
{
	m_frameContext->m_pDoc->DoPacketFileNameDialog();
}

void CChildFrame::OnEditPreferences()
{
	gVTSPreferences.DoPrefsDlg();
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

//	CChildFrame::OnUpdateEPICSLoad
//
//	It's kinda lame, but the menu item is checked when an EPICS has been 
//	successfully loaded.

afx_msg void CChildFrame::OnUpdateEPICSLoad(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( true );
	pCmdUI->SetCheck( gPICSdb != 0 );
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
/* Optimized madanner 5/03

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
		
			CString* pszLine = NULL;		// madanner, 5/03
			char cLineEnd[2];
			cLineEnd[0] = 0x0d;
			cLineEnd[1] = 0x0a;

			// optimized (slightly) by madanner, 5/03

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
					// What?  Why not just reassign this? madanner, 5/03
					//pszLine->Format(m_frameContext->m_PacketInfo.detailLine[j]->piLine);
					*pszLine = m_frameContext->m_PacketInfo.detailLine[j]->piLine;

						//m_pDetailView->GetLineData(j);
					// What?  GetBuffer without ReleaseBuffer... just use LPCSTR cast, madanner 5/03
					//exportFile.Write(pszLine->GetBuffer(1), pszLine->GetLength());
					exportFile.Write((LPCSTR) *pszLine, pszLine->GetLength());
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

				// madanner 5/03   Don't forget (already did) to delete memory returned by GetLineData
				if ( pszLine != NULL )
					delete pszLine;
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
*/


// modified madanner 5/03

void CChildFrame::OnFileExport()
{
	int nPacketCount = m_frameContext->m_pDoc->GetPacketCount();

	// pop up dialog asking for export filename
	CFileDialog exportFileDlg(FALSE,NULL,NULL,NULL,"TXT Files(*.txt)|*.txt||", NULL);
	if ( exportFileDlg.DoModal() != IDOK )
		return;

	CString strPathName = exportFileDlg.GetPathName();
	if( exportFileDlg.GetFileExt() != "txt")
		strPathName += ".txt";
		
	CString str;
	CStdioFile exportFile;

	CProgressCtrl * pprogress = ((CMainFrame *) AfxGetApp()->m_pMainWnd)->InitializeProgress();
	pprogress->SetRange(0, m_frameContext->m_pDoc->GetPacketCount());

	try
	{
		exportFile.Open(strPathName, CFile::modeCreate | CFile::modeWrite);
		
		// for some reason we have to have two info objects, one just for the summary and one for 
		// the detail lines...  If not, the detail lines are out of order.

		BACnetPIInfo	summary( true, false );

		for( int i = 0; i < m_frameContext->m_pDoc->GetPacketCount(); i++ )
		{
			pprogress->SetPos(i);

			VTSPacketPtr ppkt = m_frameContext->m_pDoc->GetPacket(i);
			if ( ppkt == NULL )
				break;

			str.Format("*************************** [ Packet %d Start ] ***************************\n\n" \
				       "[ Summary Information ]\n\n", i);
			exportFile.WriteString(str);

			str.Format("Time:        %s\n", ppkt->GetTimeStampString());
			exportFile.WriteString(str);

			str.Format("Source:      %s\n", ppkt->GetAddressString(m_frameContext->m_pDoc, rxData));
			exportFile.WriteString(str);

			str.Format("Destination: %s\n", ppkt->GetAddressString(m_frameContext->m_pDoc, txData));
			exportFile.WriteString(str);
			
			NetworkSniffer::SetLookupContext( ppkt->packetHdr.m_szPortName );
			summary.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen);

			str.Format("Summary:     %s\n\n" \
					   "[ Detail Information ]\n\n", summary.summaryLine );
			exportFile.WriteString(str);

			// have to call the interpreter twice because of the summery/detail breakdown problem..
			BACnetPIInfo	detail(false, true);
			detail.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen);
			
			for (int j = 0; j < detail.detailCount; j++ )
			{
				// Careful... BACnetPIInfo does not allocate space for the first element, which should be
				// used for the timestamp.  We already do that so everything's fine.  Just skip it.

				if ( detail.detailLine[j] != NULL )
				{
					exportFile.WriteString(detail.detailLine[j]->piLine);
					exportFile.WriteString("\n");
				}
			}

			str.Format("*************************** [ Packet %d End ] ***************************\n\n", i);
			exportFile.WriteString(str);
		}

		exportFile.Close();
	}
	catch(CFileException e)
	{
		e.GetErrorMessage(str.GetBuffer(100), 100);
		AfxMessageBox(str);
	}

	((CMainFrame *) AfxGetApp()->m_pMainWnd)->ReleaseProgress();
}

///////////////////////////////////////////////////////////
// Added by Zhenhua Zhu, 2003-6-2
//Modifyed by Zhenhua Zhu, 2003-6-11
void CChildFrame::OnFilePrint()
{	
	HPRIVATEFONT	hFont;
	if(!m_bPrintSetup)
	{
		if(m_printer.Dialog()!=-1)
			m_bPrintSetup= true;
		else return;
	}
	m_printer.StartPrint();
	m_bInPrinting = true;
	hFont = m_printer.AddFontToEnvironment("Arial");	
	m_printer.SetMargins(50,130,50,10);
	m_printer.SetDistance(2);
	m_printer.StartPage();	
	
	int nPacketCount = m_frameContext->m_pDoc->GetPacketCount();
	CString str;
	CProgressCtrl * pprogress = ((CMainFrame *) AfxGetApp()->m_pMainWnd)->InitializeProgress();
	pprogress->SetRange(0, m_frameContext->m_pDoc->GetPacketCount());	

		BACnetPIInfo	summary( true, false );
		for( int i = 0; i < m_frameContext->m_pDoc->GetPacketCount(); i++ )
		{
			pprogress->SetPos(i);
			VTSPacketPtr ppkt = m_frameContext->m_pDoc->GetPacket(i);
			if ( ppkt == NULL )
				break;
			str.Format("*************************** [ Packet %d Start ] ***************************", i);
			m_printer.Print(hFont, str, FORMAT_NORMAL);
			m_printer.Print(hFont, "", FORMAT_NORMAL);

			m_printer.Print(hFont, "[ Summary Information ]", FORMAT_NORMAL);
			m_printer.Print(hFont, "", FORMAT_NORMAL);

			str.Format("Time:        %s", ppkt->GetTimeStampString());
			m_printer.Print(hFont, str, FORMAT_NORMAL);

			str.Format("Source:      %s", ppkt->GetAddressString(m_frameContext->m_pDoc, rxData));
			m_printer.Print(hFont, str, FORMAT_NORMAL);

			str.Format("Destination: %s", ppkt->GetAddressString(m_frameContext->m_pDoc, txData));
			m_printer.Print(hFont, str, FORMAT_NORMAL);
			
			NetworkSniffer::SetLookupContext( ppkt->packetHdr.m_szPortName );
			summary.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen);

			str.Format("Summary:     %s", summary.summaryLine );
			m_printer.Print(hFont, str, FORMAT_NORMAL);
			m_printer.Print(hFont, "", FORMAT_NORMAL);
			m_printer.Print(hFont, "[ Detail Information ]", FORMAT_NORMAL);
			m_printer.Print(hFont, "", FORMAT_NORMAL);
			// have to call the interpreter twice because of the summery/detail breakdown problem..
			BACnetPIInfo	detail(false, true);
			detail.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen);
			
			for (int j = 0; j < detail.detailCount; j++ )
			{
				// Careful... BACnetPIInfo does not allocate space for the first element, which should be
				// used for the timestamp.  We already do that so everything's fine.  Just skip it.
				if ( detail.detailLine[j] != NULL )
				{
			m_printer.Print(hFont, detail.detailLine[j]->piLine, FORMAT_NORMAL);
				}
			}
			str.Format("*************************** [ Packet %d End ] ***************************", i);
			m_printer.Print(hFont, str, FORMAT_NORMAL);
			m_printer.Print(hFont, "", FORMAT_NORMAL);
		}
			m_printer.EndPage();
			m_printer.EndPrint();
	((CMainFrame *) AfxGetApp()->m_pMainWnd)->ReleaseProgress();
}

///////////////////////////////////////////////////////////
// Added by Zhenhua Zhu, 2003-6-2
void CChildFrame::OnFilePrintSetup()
{
	if(m_printer.Dialog()!=-1)

		m_bPrintSetup = true;
}

///////////////////////////////////////////////////////////
// Added by Zhenhua Zhu, 2003-6-2
void CChildFrame::OnUpdateFilePrint(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_bInPrinting);
}


//madanner 6/03, moved from ScriptFrame
//	CChildFrame::OnEPICSLoad
//

void CChildFrame::OnEPICSLoad() 
{
	CFileDialog	fd( TRUE, "tpi", NULL, OFN_FILEMUSTEXIST, "EPICS (*.tpi)|*.tpi||" );
	
	if (fd.DoModal() == IDOK)
		EPICSLoad(fd.GetPathName());
}



void CChildFrame::EPICSLoad( LPCSTR lpszFileName )
{
	int				errc;
	int             errPICS;

	if ( lpszFileName == NULL || lstrlen(lpszFileName) == 0 )
		return;
	
	if (gPICSdb) {
		// delete the database
		PICS::MyDeletePICSObject( gPICSdb->Database );

		// toss the rest
		delete gPICSdb;
		gPICSdb = 0;
	}

	// make a new database
	gPICSdb = new PICS::PICSdb;

	// read in the EPICS
	// madanner 6/03: ReadTextPICS now returns false if canceled by user

	if ( PICS::ReadTextPICS( (char *) (LPCSTR) lpszFileName, gPICSdb, &errc,&errPICS ) )
	{
		TRACE1( "error count = %d\n", errc );
		//Added by Liangping Xu,2002-11
		CCheckEPICSCons dlgEPICSCons;
	    if(errPICS){
			errc+=errPICS;
			dlgEPICSCons.DoModal();
		}
	}

    /////////////////////////////////
	
	// display the results
	if (errc == 0)
	{
		// EPICS file was successfully loaded...  save it as the last
		gVTSPreferences.Setting_SetLastEPICS(lpszFileName);

		ScriptLoadResults	dlg;
		dlg.DoModal();
	} else {
		// delete the database
		PICS::MyDeletePICSObject( gPICSdb->Database );

		// toss the rest
		delete gPICSdb;
		gPICSdb = 0;
	}
}


void CChildFrame::OnEPICSLoadAuto() 
{
	EPICSLoad(gVTSPreferences.Setting_GetLastEPICS());
}
