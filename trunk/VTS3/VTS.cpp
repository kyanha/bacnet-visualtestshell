// VTS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VTS.h"

#include "MainFrm.h"
#include "ChildFrm.h"

#include "VTSDoc.h"
#include "VTSView.h"

#include "SummaryView.h"
#include "DetailView.h"
#include "HexView.h"

#include "ScriptDocument.h"
#include "ScriptFrame.h"
#include "ScriptExecutor.h"

#include "WinPacket32.hpp"
#include "WinBACnetTaskManager.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#define NT_DebugMemState		0
#define	NT_DebugScriptFilters	1

int __cdecl VTSAllocHook(
	int		nAllocType,
	void	*pvData,
	size_t	nSize,
	int		nBlockUse,
	long	lRequest,
	const	unsigned char *szFileName,
	int		nLine
	);

#endif

#if NT_DebugScriptFilters
ScriptDebugNetFilter	gDebug1( "_1" );
ScriptDebugNetFilter	gDebug2( "_2" );
#endif

//
//	VTSApp
//

VTSApp theApp;

BEGIN_MESSAGE_MAP(VTSApp, CWinApp)
	//{{AFX_MSG_MAP(VTSApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

//
//	VTSApp::VTSApp
//

VTSApp::VTSApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

//
//	VTSApp::InitInstance
//

BOOL VTSApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Visual Test Shell 3"));

	LoadStdProfileSettings(5);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	AddDocTemplate(new CMultiDocTemplate(
		IDR_VDBTYPE,
		RUNTIME_CLASS(VTSDoc),
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CSummaryView))
		);

	AddDocTemplate(new CMultiDocTemplate(
		IDR_VTSTYPE,
		RUNTIME_CLASS(ScriptDocument),
		RUNTIME_CLASS(ScriptFrame),
		RUNTIME_CLASS(CEditView))
		);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// disable the creation of a new session, allow others
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	
	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	// initialize the random number generator
//	srand( (unsigned)time( NULL ) );
	srand( 1 );

	// make sure the adapter list is intialized
//	InitAdapterList();

	// initialize the BACnet task manager
	gTaskManager = new WinBACnetTaskManager();

#if NT_DebugMemState
	_CrtSetAllocHook( VTSAllocHook );
#endif

#if NT_DebugScriptFilters
	Bind( &gDebug1, &gDebug2 );
#endif

	// load the preferences
	gVTSPreferences.Load();

	return TRUE;
}

//
//	VTSApp::ExitInstance
//

int VTSApp::ExitInstance()
{
	// save the preferences
	gVTSPreferences.Save();

	// delete the task manager
	delete gTaskManager;

	// clean up the adapter list
	if (gAdapterList)
		delete[] gAdapterList;

	// continue with normal egress
	return CWinApp::ExitInstance();
}

//
//	VTSPreferences
//

VTSPreferences	gVTSPreferences;

//
//	VTSPreferences::Load
//

void VTSPreferences::Load( void )
{
	CWinApp* pApp = AfxGetApp();

	summaryFields = pApp->GetProfileInt( "Summary", "Fields", 0);
	summaryNameWidth = pApp->GetProfileInt( "Summary", "NameWidth", 0);
	summaryTimeFormat = pApp->GetProfileInt( "Summary", "TimeFormat", 0);
}

//
//	VTSPreferences::Save
//

void VTSPreferences::Save( void )
{
	CWinApp* pApp = AfxGetApp();

	pApp->WriteProfileInt( "Summary", "Fields", summaryFields );
	pApp->WriteProfileInt( "Summary", "NameWidth", summaryNameWidth );
	pApp->WriteProfileInt( "Summary", "TimeFormat", summaryTimeFormat );
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

//
//	VTS Release Version
//
//	Every released version of VTS gets this number bumped up by one.  If the major 
//	or minor database version changes, this can be reset back down to one.
//
//	Release History
//
//	3.1.2
//
//			New release versioning mechanism installed.
//
//	3.1.3
//			Changed the configuration character string field in the port descriptor
//			from 64 to 512 bytes to make room for BTR and BBMD configuration info.
//			The old value of 64 bytes is way to short for a list of peer IP addresses.
//
//	3.1.4
//			Massive changes to APDU structure, encoding, and decoding.  Changes to device 
//			object to allow it to limit itself to looking like it is a local object on 
//			one network.
//

const int kReleaseVersion = 4;

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_Version;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_Version = _T("");
	//}}AFX_DATA_INIT

	m_Version.Format( "Visual Test Shell\nVersion %d.%d.%d"
		, kVTSDBMajorVersion
		, kVTSDBMinorVersion
		, kReleaseVersion		// defined above
		);
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_VERSION, m_Version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void VTSApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

#if NT_Platform
int __cdecl VTSAllocHook(
	int		nAllocType,
	void	*pvData,
	size_t	nSize,
	int		nBlockUse,
	long	lRequest,
	const	unsigned char *szFileName,
	int		nLine
	)
{
	char *operation[] = { "", "allocating", "re-allocating", "freeing" };
	char *blockType[] = { "Free", "Normal", "CRT", "Ignore", "Client" };
	if ( nBlockUse == _CRT_BLOCK )   // Ignore internal C runtime library allocations
		return( TRUE );

	_ASSERT( ( nAllocType > 0 ) && ( nAllocType < 4 ) );
	_ASSERT( ( nBlockUse >= 0 ) && ( nBlockUse < 5 ) );

	return( TRUE );         // Allow the memory operation to proceed
}
#endif

//
//	VTSApp::PreTranslateMessage
//
//	This function is called just before Windows does its usual message processing.  All 
//	of the normal messages are passed through.  Application messages (those begining 
//	with WM_APP and up) are reserved for VTS for communicating between IO threads and 
//	main windows.
//

BOOL VTSApp::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_APP) {
//		TRACE3( "***** User Msg %d, %d, 0x%08X\n"
//			, pMsg->message
//			, pMsg->wParam, pMsg->lParam
//			);
		
		// ### we really need a mechanism to make sure that the lParam
		// is a VTSDocPtr without dereferencing it.

		switch (pMsg->message) {
			case WM_VTS_RCOUNT:
				((VTSDocPtr)(pMsg->lParam))->NewPacketCount();
				break;

			case WM_VTS_PORTSTATUS:
				((VTSDocPtr)(pMsg->lParam))->PortStatusChange();
				break;

			case WM_VTS_EXECMSG:
				ScriptExecMsgPtr msg;
				while ((msg = gExecutor.ReadMsg()) != 0) {
					// pass along to document
					msg->msgDoc->SetImageStatus( msg->msgBase, msg->msgStatus );

					// all done with it
					delete msg;
				}
				break;
		}

		// we completely processed this message
		return true;
	}

	// pass along to regular processing
	return CWinApp::PreTranslateMessage(pMsg);
}
