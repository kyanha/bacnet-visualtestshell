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
extern CWinThread	*gBACnetWinTaskThread;

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

	// Move and add it from WinBACnetTaskManager::WinBACnetTaskManager ,Modified by xuyiping-hust
	gBACnetWinTaskThread->ResumeThread();

#if NT_DebugMemState
	_CrtSetAllocHook( VTSAllocHook );
#endif

#if NT_DebugScriptFilters
	Bind( &gDebug1, &gDebug2 );
#endif

	// no initial environment
	gCurrentEnv = 0;

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
//			Support environment parameters.  One open script can be selected as the 'environment'
//				and all other open scripts will inherit the defined parameters.
//			TRANSMIT and RECEIVE are now aliases to SEND and EXPECT.  The error messages and 
//				user visible indicators should also be renamed, but I don't expect the code to 
//				be renamed.
//
//          The following SourceForge items were addressed:
//			444144: Add line numbers to script window
//			
//			A control has been added in the left of the edit view to display line numbers.
//			A status area has been added in the status bar to display the current line number, 
//			and current line will be highlighted.
//			A menu item "Go to Line" has been added. 
//			The problem " two consecutive dashes look like one long dash" has been solved now.
//			
//			444147: Export session contents
//			
//			A menu item "Export" has been added for exporting session contents to an ASCII file, 
//			the contents include the summary and detail information.
//			
//			531519: RPM error parsing acked-transitions
//			
//			Parsing the "ack-required" bit string is also incorrect, and error is same as 
//			parsing the "acked_transitions" bit string. These bugs have been solved now.
//			
//			544513: Allow sniffer panes to be arranged
//			
//			In former VTS3, there were 3 panes each contains information of packet summary, 
//			packet details, and packet hex codes.	Now VTS3 will create 2 docking bar each 
//			contains a view in it, one displays the packet details, and the other one displays 
//			the packet hex codes. Either of these two docking bars can be dragged, resized and 
//			visible/invisible. So VTS3 allows all three windows view to be viewed full width - 
//			one over the other and select which combination of the three is displayed.
//			The docking views are implemented using a free library, CSizingControlBar library, 
//			which is a framework for creating resizable docking windows. 
//			This is a free code library whose functionality is to realize Microsoft DevStudio 
//			like docking bars. This code is free for personal and commercial use, providing 
//			the copyright notice remains intact in the source files and all eventual changes 
//			are clearly marked with comments. One must obtain the author's consent before he 
//			can include this code in a software library. For more information, 
//			please visit http://www.datamekanix.com/.
//			
//			544516: Sniffer statistics view
//			
//			Now VTS3 has the capability to provide statistics information in real time when 
//			the operator chooses to view the statistics. The statistics include:
//			1.	The overall traffic load;
//			2.	The packet number of one or several kinds of messages; for example, 
//			    how many Who-Is packets are received/send, How many Alarm&Events packets 
//			    are received/send etc;
//			3.	The packet number of certain size or of certain size range; for example, 
//			    how many packets whose size ranges from 100 to 1000 are received/send etc.
//			
//			444231: Associate script outline with editor
//			
//			Now when operator double click a section item in the tree/outline overview of a 
//			script (the lower left pane of a script window), VTS3 can auto-scroll to the 
//			relative position and highlight the first line of the section.
//			
//			562488: B/IP 'BDT Entry' UI needs easier format 
//			
//			Now BDT entry has been broken into three fields from single field. 
//			4 Octet B/IP address (IP Address:Port) field, 2Octet UDP port field and 4 Octet 
//			Broadcast Distribution Mask field.
//			
//			558884:Active_COV_Subscriptions not selectable
//			
//			Now property "Active_COV_Subscriptions" is selectable in the device object for 
//			read-property requests. In order to parse this property, some data structures and 
//			decoding methods have been added in VTS3, All these works were done according 
//			to Addendum C to ANSI/ASHRAE Standard 135-1995.
//			
//			561641:Cat,Sched 'profile' prop not selectable
//			
//			Now property "Profile_Name" is selectable in all standard objects for 
//			read-property requests. All these works were done according to Addendum E 
//			to ANSI/ASHRAE Standard 135-1995.
//			
//			In Addendum E, the sequence number of property "Profile_Name" is 168, in order to 
//			add this property, we must add it into data structure "BACnetPropertyIdentifier[]". 
//			So we have added other properties whose sequence numbers are in 152 to 168 into 
//			"BACnetPropertyIdentifier[]", but we haven't parsed them at present. These 
//			properties have been given in Addendum c and e, but we don't know if they have 
//			been included in ANSI/ASHRAE Standard 135-2001 as they have been described in 
//			Addendum C and E .
//
//	3.1.9	472392: ReadPropertyMultiple error (UI fix)
//			
//			Init new properties with Present_Value, now shows in list.  Increased the Property
//			column length as well as the property drop-down list.  Performed same mods to
//			ReadPropertyMultipleAck and WritePropertyMultiple dialog.  Previously the combos
//			in the Ack dialog were not large enough to drop-down.
//	
//			In relation to the list control problem in the ReadPropertyMultiple bug, a general 
//			clean up of all dialogs presenting list controls was performed.  The following lists were
//			affected:
//				ReadBDTAck
//								- Added controls for IP / Port creation instead of edit box
//								- Address defaults and appears in list, rather than empty space item
//								- List keeps selection, preventing GPF in removals of list items
//				ReadFDTAck
//								- Added controls for IP / Port creation instead of edit box
//								- Address defaults and appears in list, rather than empty space item
//								- List keeps selection, preventing GPF in removals of list items
//								- Changed tab order of dialog items
//				IAmRTN, RouterBusyToNetwork, RouterAvailToNetwork, InitRT, InitRTAck
//								- Default '1' for DNET address, port (where applicable) default to 0xBAC0
//								- Address defaults and appears in list, rather than empty space item
//								- List keeps selection, preventing GPF in removals of list items
//				UnconfCOVNotification, ConfCOVNotification, CreateObject
//								- Default 'Present_Value' for property
//								- Property value appears in list, rather than empty space item
//								- List keeps selection, preventing GPF in removals of list items
//				VTClose, VTCloseAck
//								- Default '1' for Session ID
//								- ID appears in list, rather than empty space item
//								- List keeps selection, preventing GPF in removals of list items
//				WriteFile, ReadFileAck
//								- Default '(record data)' as string for adding data
//								- Character data no longer needs to be placed in quotes in edit box (see below)
//								- Data appears in list instead of empty space
//								- List keeps selection, preventing GPF in removals of list items
//				ReadPropMult, ReadPropMultAck, WritePropMult
//								- Default object ID
//								- Defaulted data appears in list instead of blank space item
//								- Resized property drop-downs
//								- List keeps selection, preventing GPF in removals of list items
//				GetAlarmSummaryAck, GetEnrollmentSummaryAck
//								- Default object ID and state
//								- ID and state appear in list instead of empty space
//								- Enabled transition checks - checks were previously always disabled 
//								- List keeps selection, preventing GPF in removals of list items
//								- Changed 'property' label to 'Event State' for proper field
//				ComplexObjectType
//								- Default 'Present_Value' in list
//								- Property appears in list instead of empty space
//								- Disabled controls on dialog start when no items in list (formerly enabled)
//								- List keeps selection, preventing GPF in removals of list items
//				VTSAny
//								- List keeps selection, preventing GPF in removals of list items
//
//			511345, 511412:  Charstring handling
//
//			User was unable to see char string entered for passwords in DCC & Charstring was apparently
//			not sent to device in WP/WPM.  This problem was actually caused by the method for entering
//			char strings in edit boxes.  Single or double quotes were required within the edit box to
//			properly input the desired text for passwords, messages, names, etc..  Strings were using the
//			script parser to identify strings but this is not necessary in the UI.  Edit fields requiring 
//			strings throughout VTS have been altered accept what is entered.  If quotes are present, the quotes
//			are assumed to be part of the text and sent to the device in the relavent BACnet message.  This
//			change effects the following areas:
//
//				SendAckAlarm -> ack source
//				ConfirmedEventNotification -> message text
//				ConfirmedTextMessage -> character class, text message
//				DeviceCommunicationControl -> password (original bug report)
//				IHave -> object name
//				ReinitializeDevice -> password
//				Test? -> character string
//				UnconfirmedEventNotification -> message text
//				UnconfirmedTextMessage -> character class, text message
//				WhoHas -> object name
//				Any -> character string
//
//			Drop-downs for property values have been resized in various places to allow more than only a few
//			properties to be visible.
//
//			444134: Auto Segmentation Support
//
//			User can now select a device from the Send menu, or the port drop down menu.  There is a new 
//			Device page which is looking for a destination address (type, network, and address) and the 
//			outgoing APDU will be set to that address.  The device object will take care of segmentation,
//			retries, etc, as necessary.  Properly configuring a built-in device object and binding it to a 
//			port will be part of a separate document.
//
//			511406:  Status flags in a variable comp fails
//
//			Comparisons for status flags in RP/M, WP/M work properly now.  This problem was caused
//			by reading one byte passed the encoded PDU for the bitstring.  The comparison failed 
//			because the tag was being interpreted as part of the incoming bitstring.
//

const int kReleaseVersion = 9;

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