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

#include "file_ver.h"

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

	// check BACMACNT.SYS
	CheckBACMACNTVersion();

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

	sendInvokeID = pApp->GetProfileInt( "Send", "InvokeID", 0);
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

	pApp->WriteProfileInt( "Send", "InvokeID", sendInvokeID );
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
//          By ALC:
//          Enhancements to begin adding support for BACnet Addendum 135b
//          1) Added support for the Event_Time_Stamps property in each event-generating object. 
//          2) Added support for the TrendLog object, except for the Log_Buffer property. 
//          3) Added support for the Multi-state Value Object 
//          4) Added support for the Averaging Object 
//          By "added support", I mean that these objects and their properties can now be 
//          parsed from the EPICS correctly, and they are decoded properly in the 
//          interactive sniffer.   
//          Added the following macros to replace hard-coded maximums.
//          These are defined in VTS.H
//             MAX_DEFINED_OBJ 
//             MAX_PROP_ID
//             MAX_SERVS_SUPP
//
//
//	3.1.5
//			Built in support for BACnet/IP endpoints to be managed automatically (the peer list 
//			for BTR's and BBMD's have not been finished yet).  Added a new set of functions 
//			to support scripting at the device object level.  If a script does not specify a 
//			network, or BVLCI or network layer message then the SEND and EXPECT packet are 
//			assumed to operate from the context of the device object.  Only one device object 
//			may be defined, until there is a new keyword that describes which device object 
//			should be used.
//
//	3.1.6
//			Increased the size of the source and destination in the summary pane, and if the 
//			name lookup fails, show the address in hex.
//
//  3.1.7   Added SendUTCTimeSync to the send window.
//          Fixed minor problems in the Send ReadRange dialog.
//
//	3.1.8	Session data is now cleared, so it doesn't grow continuously, but is not compressed
//				That will be a future enhancement.
//			The Send windows have been sized up to make room for more buttons and clearer indicators.
//			The IDD_SENDREADPROPMULT, IDD_SENDREADPROPMULTACK and IDD_SENDWRITEPROPMULT dialog
//				boxes now have shuffle-up and shuffle-down buttons.  With all the focus and 
//				selection change messages flying around I'm sure the interface doesn't quite do 
//				what is expected.
//			All of the Send dialog boxes that have lists now have 'Show selection always' turned
//				on, which doesn't always seem to do what it's supposed to.
//			There is now an automatic invoke ID for the send window.  It is incremented when the
//				Send button is clicked.  The value is saved in the preferences (registry), so it 
//				comes back for the next VTS launch.  Note that this is NOT the same invoke ID that 
//				will be used by a built-in device object, and I think that is a good thing.
//			Now checks the version of BACMACNT.SYS it can find.  Only version 3.0 works, but I
//				don't know why.
//       Added decoding of UTCOffset and DayLightSavingStatus to the UTCTimeSynch service.
//       Changed the UTCOffset range from +/- 720 to +/-780. 
//

const int kReleaseVersion = 8;

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

//
//	VTSApp::CheckBACMACNTVersion
//
//	Thanks to Thomas Weller and www.codeguru.com, this function checks to see if 
//	BACMACNT.SYS is installed and checks the version.  As written this will not work 
//	if the driver is installed someplace else, like perhaps C: isn't the boot 
//	volume.
//
//	There is no warning if the file can't be found, so the user of strictly IP traffic
//	isn't bothered by an annoying message.
//

void VTSApp::CheckBACMACNTVersion( void )
{
	CFileVersionInfo	m_info
	;

	// get the information
	m_info.ReadVersionInfo( "C:\\WINNT\\system32\\drivers\\bacmacnt.sys" );

	if (m_info.IsValid())
	{
		CString version = m_info.GetFileVersionString()
		;
		int		ver = atoi(version)
		;

		if (ver < 3)
			AfxMessageBox( _T("WARNING: Installed version of BACMACNT.SYS must be 3.0 or later.") );
	}
}

//
//	VTSAllocHook
//

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
