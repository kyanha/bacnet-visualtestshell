// VTS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VTS.h"

#include "VTSPreferences.h"
#include "MainFrm.h"
#include "ChildFrm.h"

#include "VTSDoc.h"
#include "VTSView.h"

#include "SummaryView.h"
#include "DetailView.h"
#include "HexView.h"
#include "ListSummaryView.h"

#include "ScriptDocument.h"
#include "ScriptFrame.h"
#include "ScriptExecutor.h"

#include "WinWinPcap.hpp"
#include "WinBACnetTaskManager.hpp"

#include "ScriptMsgMake.h"
#include "ScriptMakeDlg.h"

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


// Defined in VTSPacketDB.h
extern const int kVTSDBMajorVersion = 3;			// current version
extern const int kVTSDBMinorVersion = 4;

extern CWinThread	*gBACnetWinTaskThread;


//
//	VTSApp
//

VTSApp theApp;

VTSPreferences	gVTSPreferences;


BEGIN_MESSAGE_MAP(VTSApp, CWinApp)
	//{{AFX_MSG_MAP(VTSApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_WKS1, OnUpdateRecentWorkspaceMenu)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_WKS1, ID_FILE_MRU_WKS15, OnOpenRecentWorkspace)
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_WKS_NEW, OnFileWksNew)
	ON_COMMAND(ID_FILE_WKS_SWITCH, OnFileWksSwitch)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

//
//	VTSApp::VTSApp
//

VTSApp::VTSApp(void)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


VTSApp::~VTSApp(void)
{
	if ( m_pdoctempConfig != NULL )		// kill saved workspace template
		delete m_pdoctempConfig;

	if (m_pRecentWorkspaceList != NULL)
		delete m_pRecentWorkspaceList;
}


void VTSApp::AddToRecentWorkspaceList(LPCTSTR lpszPathName)
{
	ASSERT_VALID(this);
	ASSERT(lpszPathName != NULL);
	ASSERT(AfxIsValidString(lpszPathName));

	if (m_pRecentWorkspaceList != NULL)
		m_pRecentWorkspaceList->Add(lpszPathName);
}


void VTSApp::LoadWorkspaceMRU(UINT nMaxMRU)
{
	ASSERT_VALID(this);
	ASSERT(m_pRecentWorkspaceList == NULL);

	if (nMaxMRU != 0)
	{
		// create file MRU since nMaxMRU not zero
		m_pRecentWorkspaceList = new CRecentFileList(0, _T("Recent Workspace List"), _T("File%d"),	nMaxMRU);
		m_pRecentWorkspaceList->ReadList();
	}

//	m_nNumPreviewPages = GetProfileInt(_afxPreviewSection, _afxPreviewEntry, 0);
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
//	SetRegistryKey(_T("Visual Test Shell 3"));
	SetRegistryKey(IDS_REGISTRYKEY);

	//First free the string allocated by MFC at CWinApp startup.
	//The string is allocated before InitInstance is called.
//	free((void*)m_pszProfileName);
	
	//Change the name of the .INI file.
	//The CWinApp destructor will free the memory.

// What in the world is this for?  madanner

//	CString temp;
//	GetCurrentDirectory(200,temp.GetBuffer(200));
//	temp.Format("%s%s",temp.GetBuffer(0),"\\vts.ini");
//	m_pszProfileName=_tcsdup(temp.GetBuffer(0));

	LoadStdProfileSettings(5);  // Load standard INI file options (including MRU)
	LoadWorkspaceMRU(5);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_pdoctempConfig = new CMultiDocTemplate(IDR_VDBTYPE, RUNTIME_CLASS(VTSDoc), RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CListSummaryView));
//	AddDocTemplate(pdoctempConfig);
	AddDocTemplate(new CMultiDocTemplate(IDR_VTSTYPE, RUNTIME_CLASS(ScriptDocument), RUNTIME_CLASS(ScriptFrame), RUNTIME_CLASS(CEditView)));

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

	// check WinPcap
	CheckWinPcapVersion();

	// Get the last opened workspace....  Will be empty if none.
	CString strLast = (*m_pRecentWorkspaceList)[0];

	// initialize the BACnet task manager
	gTaskManager = new WinBACnetTaskManager();

	// attempt to open the last document if they've had one... 
	if ( !strLast.IsEmpty() )
	{
		if ( !m_pdoctempConfig->OpenDocumentFile(strLast) )
		{
			// Open failed, ask if they want to open default config
			CString strErr;
			strErr.Format(IDS_ERR_FILEMOVEON, strLast, VTSDoc::m_pszDefaultFilename);
			switch ( AfxMessageBox(strErr, MB_ICONQUESTION | MB_YESNOCANCEL) )
			{
				case IDYES:		strLast = VTSDoc::m_pszDefaultFilename; break;
				case IDNO:		strLast.Empty();	break;
				default:		return FALSE;
			}
		}
		else
			strLast.Empty();
	}
	else
		strLast = VTSDoc::m_pszDefaultFilename;

	// when we get here strLast is empty if the file has already been opened...

	if ( !strLast.IsEmpty() )
		if ( !m_pdoctempConfig->OpenDocumentFile(VTSDoc::m_pszDefaultFilename) )
			m_pdoctempConfig->OpenDocumentFile(NULL);

	// move on from here... if we haven't opened a document file... we're just going to have to allow
	// the user to attempt one

	if ( gVTSPreferences.Setting_IsLoadEPICS() )
		pMainFrame->PostMessage(WM_COMMAND, ID_EPICS_LOADAUTO, NULL);

	// Move and add it from WinBACnetTaskManager::WinBACnetTaskManager ,Modified by xuyiping-hust
	gBACnetWinTaskThread->ResumeThread();
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

	if (m_pRecentWorkspaceList != NULL)
		m_pRecentWorkspaceList->WriteList();

	// continue with normal egress
	return CWinApp::ExitInstance();
}


void VTSApp::OnFileWksNew()
{
	CFileStatus	fs;
	LPCSTR lpszFile = NULL;

	// test to see if the default config file already exists... if it does it will get overwritten so
	// we need to make sure the user wants to do this...

	if ( CFile::GetStatus(VTSDoc::m_pszDefaultFilename, fs) )
	{
		// config file we're about to overwrite already exists...
		// Get the full path of the default file we're trying to create

		CString strmsg;
		TCHAR szFullPathNew[_MAX_PATH];

		LPTSTR lpszFilePart;
		GetFullPathName(VTSDoc::m_pszDefaultFilename, _MAX_PATH, szFullPathNew, &lpszFilePart);

		// Test to see if the full file is really the same as the one currently loaded
		CDocument * pdocCurrent = GetWorkspace();

		if ( pdocCurrent != NULL && pdocCurrent->GetPathName().CompareNoCase(szFullPathNew) == 0 )
		{
			// default file exists already...  and it happens to be the same one that is currently
			// loaded.  Going ahead will rewrite the currently loaded workspace with defaults
			// loosing what we've got.  Ask user if he wants to go ahead. 
			// If yes, just drop through and overwrite this file

			strmsg.Format(IDS_WKS_OVERWRITESAVE, VTSDoc::m_pszDefaultFilename);

			switch( AfxMessageBox(strmsg, MB_YESNOCANCEL | MB_ICONQUESTION) )
			{
				case IDNO:		// user wants to save currently workspace to different name... 
								// if he goes through with this, close down and drop through
								// to reopen file and recreate.
					
								if ( !GetWorkspace()->DoSave(NULL) )
									return;
								break;

				case IDCANCEL:	return;				// abort all
			}
		}
		else
		{
			// OK... file to be recreated is NOT the same as the one we've got loaded
			// so ask user if he wants to proceed.
			// If yes, just drop through and overwrite the new file... the current one is already saved

			strmsg.Format(IDS_WKS_OVERWRITE, VTSDoc::m_pszDefaultFilename);

			switch( AfxMessageBox(strmsg, MB_YESNOCANCEL | MB_ICONQUESTION) )
			{
				case IDNO:		// User wants to switch to default file and NOT overwrite...
								// so perform same as Workspace Switch...
								lpszFile = VTSDoc::m_pszDefaultFilename;	break;

				case IDCANCEL:	return;				// abort all
			}
		}
	}

	m_pdoctempConfig->CloseAllDocuments(TRUE);
	m_pdoctempConfig->OpenDocumentFile(lpszFile);
}


void VTSApp::OnFileWksSwitch()
{
	CString newName;
	if ( !DoPromptFileName(newName,IDS_WKS_SWITCH, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, m_pdoctempConfig))
		return;

	m_pdoctempConfig->CloseAllDocuments(TRUE);
	m_pdoctempConfig->OpenDocumentFile(newName);
}


CDocument * VTSApp::GetWorkspace(void)
{
	ASSERT(m_pdoctempConfig != NULL);
	if ( m_pdoctempConfig == NULL )
		return NULL;

	POSITION pos = m_pdoctempConfig->GetFirstDocPosition();
	return m_pdoctempConfig->GetNextDoc(pos);
}


/////////////////////////////////////////////////////////////////////////////
// MRU workspace list default implementation

void VTSApp::OnUpdateRecentWorkspaceMenu(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);

	if (m_pRecentWorkspaceList == NULL) // no MRU files
		pCmdUI->Enable(FALSE);
	else
		m_pRecentWorkspaceList->UpdateMenu(pCmdUI);
}


/////////////////////////////////////////////////////////////////////////////
// MRU workspace list default implementation

BOOL VTSApp::OnOpenRecentWorkspace(UINT nID)
{
	ASSERT(m_pRecentFileList != NULL);

	ASSERT(nID >= ID_FILE_MRU_WKS1);
	ASSERT(nID < ID_FILE_MRU_WKS1 + (UINT)m_pRecentWorkspaceList->GetSize());
	int nIndex = nID - ID_FILE_MRU_WKS1;
	ASSERT((*m_pRecentWorkspaceList)[nIndex].GetLength() != 0);

	m_pdoctempConfig->CloseAllDocuments(TRUE);
	if ( m_pdoctempConfig->OpenDocumentFile((*m_pRecentWorkspaceList)[nIndex]) == NULL )
	{
		CString strErr;
		strErr.Format(IDS_ERR_FILEOPENRCT, (*m_pRecentWorkspaceList)[nIndex] );
		AfxMessageBox(strErr, MB_OK);

		m_pRecentWorkspaceList->Remove(nIndex);
	}

	return TRUE;
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
//			508646:  rp to UTC_Offset fails in a script
//
//			Comparing the returned UTC_Offset value in a script to EPICS failed because the data type
//			used in VTS's internal structures for UTC_Offset was a float instead of int.  This has been
//			corrected by changing it's internal type to 'ssint' and processing accordingly.
//
//			508643:  VTS3 unable to read device-address-binding(RP, RPM)
//
//			'Property value not known' was returned correctly when comparing device-address-binding to EPICS when
//			the value was '?'.  It returned this error incorrectly when the EPICS value was defined as empty ().
//			It now compares the valid empty condition.  This error would also show up on other non-primitive data types
//			whose value could be empty ().
//
//			508640, Array bugs in rp (1E,1F,1G) 
//			
//			Case 1e has been fixed by creating a BACnet type class structure that will facilitate easier type
//			comparisons.  (See below).   Cases 1F and 1G are errors in the script parser that don't deal with array
//			indices.
//
//			Reworked implementation for comparing data in EXPECT statements from values retrieved from EPICS and
//			specified in scripts.   Any AL value being compared to EPICS data was using a method to match encoded
//			streams.  This scheme would fail in cases where numbers used different encoding sizes, different type
//			comparisons that are legal (such as float vs. null in priority arrays) and many other.  More detailed
//			comparisons were impossible with this method because VTS had no way of determining the type of data 
//			and could not determine the proper end of the stream, among other reasons.  
//
//			To implement type specific comparisons for all possible BACnet data types, including arrays, lists and 
//			complex structures, the c++ class representation for BACnet data types has been extended.  These 
//			classes serve as a foundation that allows the use of more tools (MFC, polymorphism, etc.) to 
//			facilitate feature additions and more flexibility where BACnet data type are concerned.  Type specific 
//			matching methods, display methods, decoding, assignment and comparative operators are now available.  
//
//			Using these classes to perform decoding and comparisons in the heart of EXPECT statements required 
//			the need for a retreval method to access the internal EPICS structures and convert them to BACnet 
//			type classes.   The immediate result of these changes can be seen in more meaningful text in test 
//			fail results.  In addition, comparisons can now be made to the following data types:
//
//				Unsigned values - all sizes.
//				Integer values - all sizes.
//				Real and doubles.
//				Priority arrays for floats, reals, enumerated and NULL choices.  Indexes not supported yet from 
//							script parser yet but comparisons are ready.
//				Boolean.
//				Bit strings.
//				Object IDs.
//				Character strings - any size.
//				Enumerated values.
//				Date values - all comparison operators.
//				Time values - all comparison operators.
//				DateTime values - all comparison operators.
//				DateRange - all comparison operators (where applicable).
//				Calendar Entries and lists.
//				Device Address Bindings.
//				Array of object identifiers.
//				Arrays and lists of unsigned values.
//				Arrays of text for active/inactive.
//
//			The following type comparisons are not implemented yet:
//				List of read access specs.
//				Action commands and arrays of action commands.
//				VT classes.
//				Event parameters.
//				Session keys.
//				Time Synch recipients.
//				Recipients.
//				List of Recipients.
//				Array of exceptions schedule events.
//				Array of weekly schedules.
//				List of object property references.
//				Setpoint reference.
//				List of active VT sessions.
//
//			Adding support for these remaining types is simply a matter of constructing the object from the 
//			EPICS internal structures and decoding the object from a stream.  The mechanisms for performing the 
//			comparisons are already in place.  The development of support for proprietary objects in EPICS 
//			and scripts will be assisted by BACnet classes as well.
//
//			Other benefits of this work will be seen later as the EPICS store gradually converts to store 
//			data internally in these classes.  The greatest benefit to this foundation work will be seen as data 
//			becomes extractable from Complex Acks either compared, tested or even assigned to variables of complex
//			types in scripts.  Additionally, script parsing will be effected by these changes once all of the 
//			data types are supported and proper scanning methods for each will allow a single source code source 
//			for the creation of such objects.
//
//			Most of the SEND methods have been altered to retrieve EPICS data using these classes as well as 
//			creation and encoding of send streams from these object types.  Specifically, the following classes 
//			have been added or altered:
//				All BACnet classes enhanced with Runtime type declaratives, ToString and Match virtuals.  
//				Most classes now have constructors that take an APDUDecoder to construct the value from an 
//				encoded stream.  BACnetEncodeable now decends from CObject, added Match and ToString virtuals.  
//				BACnetAddr, BACnetUnsigned, BACnetCharacterString, BACnetBinaryPriV, BACnetCalendarArray, 
//				BACnetCalendarEntry, BACnetDate, BACnetDateTime, BACnetDateRange, BACnetTime, BACnetTimeStamp, 
//				BACnetWeekNDay, BACnetObjectIDList, BACnetGenericArray, BACnetObjectContainer, 
//				BACnetAnyValue (to facility "any" type flexibility - but mostly to encapsulate dynamically 
//				allocated objects for proper destruction when there's so much 'throwing' goings on...).
//
//			617618:  RPM MAX script crashes VTS
//
//			This problem was caused during an attempt to reallocate an APDU buffer to accomodate a larger
//			size APDU.  The problem has been fixed.  VTS no longer crashes.  Proper segmentation should occur.
//
//			508640:  Array bugs in rp(1E, 1F, 1G)
//
//			Cases 1F (test for size of array [0]) and 1G (test of single index value in array [int]) have
//			now been fixed.  Indexing into array properties is now supported in the following manner:
//					{property[int]}, {property[var]}
//			The index must either be a literal integer or a parameter which resolves to an integer.  Errors will
//			be reported upon execution otherwise.  Index references as properties, other indexed properties or even
//			indexes into parms that are arrays may be easily added in the future.  At present, specifications
//			such as these:
//					{property[{property}]}, {property[{property[int]}]}, etc.
//			are not supported.
//
//			618176: EXPECT needs "Don't Care" arguments
//
//			Expressions within an EXPECT statement now support the don't care operation.  Equations and values
//			can be specified as don't care in one of the following ways:
//
//				Method 1: Replace data in expression with '?',  Keyword = ?
//				EX:   Unsigned = ?, Bitstring = ?, PrimitiveType = ?
//
//				Data does not have to be valid for the given type.  '?' is also useful for replacing addresses
//				returned for EXPECT statements when applied to:  WriteBDT, BVLLResult, ReadBDTAck, ForwardedNPDU,
//				RegisterFD, ReadFDTAck, DeleteFDTEntry, WhoIsRouterToNetwork, IAmRouterToNetwork,
//				ICouldBeRouterToNetwork, RejectMsgToNetwork, RouterBusyToNetwork, RouterAvailToNetwork,
//				InitializeRoutingTable, EstablishConnectionToNetwork, DisconnectConnectionToNetwork.  
//				This format for don't care is not valid for Device responses and is also not valid for AL type data
//				where the datatype is determined by the EPICS reference on the right of the expression.
//
//				Method 2: Use of the don't care operator:  ?=,   Keyword ?= data
//				EX:  Unsigned ?= 20, Bitstring ?= F, T, AL ?= {EPICSReference}
//
//				Use of the don't care operator within an EXPECT statement will cause the data stream to be
//				parsed correctly according to the data type but the value will not be tested.  This operator
//				works on primitive types (Unsigned ?= 20) as well as the AL data.  The difference here is that
//				the value on the right of the operator must be valid for the datatype.  EPICS references for AL
//				expressions must be valid in order to determine the data stream type.  Exeptions to this are 
//				expressions where both the ?= operator AND the ? don't care data case both be used (although
//				redundant):  Bitstring ?= ?.  An advantage to using the operator is in keywords.  Object IDs can
//				be ignored (Object ?= 0, OBJECTVAR), and the like.
//
//				Date / Time improvements
//
//				Several changes have taken place in the handling of the primitive Date and Time data types.
//				Unless noted, both the Date and Time data type 	handling were affected equally by
//				these corrections and enhancements.
//
//				- The Date and Time data types were not accepting a script variable substitution when used
//				  in Send and Expect statements.  Full support for receiving Date and Time values from
//				  variables has been added.
//				- The Date and Time data types now compare don't care ('?', '*') values properly.
//				- Added support for the don't care value that other data types recently support.
//				- Date and Time values specified in EPICS that had don't care values would not be
//				  allowed in references. This restriction has been removed and now supports all
//				  combinations of don't care specifications (?/?/?).
//				- Full comparison support has been added to the Date and Time primitive types. 
//				- Various parsing problems in Date/Time have been fixed.
//
//				Text representations of complex data types now require the use of brackets '[]' to separate
//				their data from other parameters.  The brackets are required even if no other parameters
//				in an expression are present.  The Date and Time text representations now take the following
//				form:
//						Date = [dow, mm/dd/yy]		( '-' can also be used to separate date values)
//						Time = [hh:mm:ss.HH]
//						Where:
//								dow = 3 characters (case insensitive) for MON, TUE, WED, THU, FRI, SAT, SUN.
//									  This value must be present and can be substituted with ? as in
//									  [?, mm/dd/yy].  A comma separates this value and leading and trailing
//									  spaces between each atomic type are ignored 
//									  (i.e. [?   ,   mm  /  dd  / yy] is the same as [?,mm/dd/yy]).
//								mm, dd, yy, hh, mm, ss, HH = numeric values with leading or no zeros.
//							          Values are checked for validity in every case except invalid dates as
//									  a whole (Feb 31st).
//								yy = Year specification given as either the full year (2002) or a partial
//									  year (02).  Values interpreted as partial years range from 1941 to 2040.
//								HH =  hundredth value in the range 0 - 99.  This value is optional in the text
//									  representation.  If the period is present in the text, the value MUST follow
//									  or a parsing error is generated and the Time is invalid.
//
//				Any and all values can be substituted with the don't care value ?.   [12:?:44.?]
//				Enclosing these complex representations inside brackets gives clarity to expressions with
//				multiple parameters as in the case of tag specifications:  Date = tag, [TUE, 11/4/02]
//				-or-  Date = tag, VAR  where VAR = [TUE, 11/4/02].   Bracketing this compound data can be
//				nested as in the case of the text representation for a DateTime and DateRange:
//						DateTime = [DATE, TIME] = [[dow, mm/dd/yy], [hh:mm:ss]]
//						DateRange = [DATE1, DATE2] = [[dow, mm/dd/yy], [dow, mm/dd/yy]]
//
//				DateTime and DateRange are not considered primitive types and currently can only be used
//				when referencing EPICS data and so a text representation for these types has not been needed
//				as yet.  Like quotes for strings, the brackets are only necessary in situations where other
//				types of data may be read as well.  Brackets are not necessary when date and time values
//				are specified inside edit fields within the VTS user interface. 
//				They are, however, necessary in scripts and subsequently, like quotes, necessary inside
//				script parameter data, since this data is partially parsed.
//
//				[444200] Extract data from incoming packets
//
//				Support for stuffing incoming data elements into script variables has been added throughout
//				the EXPECT statement, except where noted.  Syntax for the new operator '>>' in assignments
//				take the following form:
//						Keyword >> VAR
//						Keyword >> tag, VAR
//				This will place a text representation of the incoming data given by the keyword into the
//				script parameter VAR.  The VAR parameter MUST be previously created in the SETUP section
//				of the script or an error will be reported.
//
//				Existing support for other keywords is as follows:
//					Object >> tag, VAR	Assigns:  OBJECT-TYPE, Number
//					Object >> VAR
// 				  This assignment will place the object type keyword AND the object instance number
//				  inside the script parameter value. 
//					Device >> VAR		Assigns:  DEVICE, Number
//				  This assignment will also place the keyword 'DEVICE', followed by a comma, and the
//				  instance number inside the script variable.  Brackets will not accompany the data.
//					Property >> tag, VAR	Assigns:  Number or PropertyKeyword
//				  The property keyword (i.e. PRESENT_VALUE) will be placed inside the variable. 
//				  This will be properly decoded and compared in other equality expressions in the current
//				  or future EXPECT statements.
//
//				Special notes about other keywords:
//				MAXSIZE >> VAR		Assigns: Enumeration of ("50", "128", "206", "480", "1024", "1476")  or Number if < 16.
//						 Variable supplied as number, converts to code before SEND.
//				NETWORK >> VAR		Assigns: CharacterString for currently defined port (in quotes).
//				BVLCI >> VAR
//					where type BVLCI type is: READ-BROADCAST-DISTRIBUTION-TABLE-ACK {,host/net:port}*
//				MESSAGE >> VAR, ?, ?, ?
//					These keywords have parameters that follow which dictate how the values are extracted
//					from the received packet.   The parameters that follows must still accompany the expression.
//
//				An error was discovered and corrected concerning the specification of DNET/DADR and SNET/SADR
//				pairs in the script.  The test for whether or not these pairs were present in the received
//				packet was being determined by whether or not the DNET/DADR keywords were supplied in the
//				script.
//
//  3.1.10		Date / Time handling correction for '*'
//
//				Added support in recent changes for Date/Time handling to make a distinction between the
//				'?' value (matches to anything for equality) and the '*' value (MUST match the anything value).   
//				Comparisons were affected in the following ways:
//
//					'?' appearing in any field means that field (m/d/y, dow) or (h:m:s.h) will match with
//					any other value in a comparison.  <, > tests will result in failure for that piece of
//					the date/time - which will effect the entire comparison.
//
//					'*' appearing in any field means that the field it's matching to MUST be either '*' or '?'.
//					
//  3.2.0		[618172] Conditional flow control for scripts
//
//				Added conditional script compilation with use of IFDEF type keyword.  IF is a pre-processor
//				conditional used during the "Syntax Check" function.  The expression
//				in the conditional will not be evaluated at script execution.  Syntax is as follows:
//
//					IF (expression {[OR | AND] expression} )
//						.
//					{ELSEIF (expression {[OR | AND] expression} )}
//						.
//					{ELSE} 
//						.
//					ENDIF
//
//				The pre-processor will evaluate the expressions contained within the perenthesis following
//				the 'IF' keyword before scanning the next line.  If the expression evaluates to true, all
//				of the following lines up to the ENDIF keyword (or ELSEIF or ELSE, if supplied) will be
//				included in the compile.  Multiple expressions can be evaluated.  Evaluation will occur left
//				to right until a false condition is detected.
//
//				See Sourceforge item 618172 for details regarding usage of the IF statement.
//
//				[44152] New CHECK statement
//
//				Added CHECK statement to script commands that will popup a modal dialog box and allow the user
//				to PASS or FAIL the script test.  Script execution is halted while the dialog is up.  Syntax
//				for the CHECK statement is as follows:
//
//				CHECK "Title" (
//					"Text line"
//					...
//					"Text line"
//					)
//
//				Usage details can be found in a usage document attached to the sourceforge item 44153 for MAKE.
//
//				[44153] New non-modal MAKE statement
//
//				Added MAKE statement to script commands.  It will pop up a dialog with a supplied message and
//				allow the user to alter a condition that generates a receive packet.  When followed by an
//				EXPECT statement, script execution continues with indefinately extended EXPECT timers.  Syntax
//				is exactly like the CHECK statement with the exception of the keyword MAKE.  Usage details can
//				be found in the document attached to the sourceforge item.
//
//				
//  3.2.1   [567046] 
//          Now Detail pane has been changed into treelist pane, we expanded some nodes by 
//          default to display the informations that we think they are very important.
//
//	3.3		[485733] Add RP/WP support to internal device objects.  Minor version number changed to reflect
//			change to the database (the device descriptor now has an ID of a list of object/property/value 
//			components.
//
//          [626601] Crashes when openning a read-only VDB
//		    A message is now returned when an attempt to open a read-only VDB is made.
//
//          Fixed all of the child popups decending from the Send dialog to chain properly.  Each (tedious) 
//          ocurrance was altered to report to the proper parent such that all of the popups are now modal
//          to the modeless Send dialog.  Multiple Send dialogs can now be up simultaneously.
//
//			Trapped corrupted and inconsistent database opens, report the problem and control document close.
//
//	3.4		- Removed internal database and replaced with workspace files (.cfg).
//			- Replaced BACMACNT driver with WinPcap library calls.  No changes to the database format, just the
//			- Workspaces menu added to main menu.
//			- VTS can switch workspaces without shutting down.
//			- Only one workspace can be loaded at once.
//			- VTS will open the most recently used workspace file upon VTS start.
//			- Menu items have been changed to reflect workspace handling (on separate menu) and document handling (on main File menu).
//			- VTS will not associate .VDB files with VTS upon startup anymore.
//			- Scripts are opened in a separate frame window with a menu and button bar appropriate for script handling.
//			- VTS main window remembers packet view column sizes.
//			- Packet summary window now has timestamp and port name columns. 
//			- Button bar has been expanded on main VTS packet view and menu as been trimmed to commands relative to main VTS (not scripts).
//			- Port names on Send menu are grayed if inactive and show port type.
//			- Refresh added to reload packet view.  This is useful if you have changed name/address mappings.
//			- Revamped 'Export' function.
//			- Added progress bar on status bar that shows during lengthy operations (loading large packet files, exporting, etc.).
//			- Changed main list view to handle virtually unlimited number of packets.
//			- Delete all packets reduces packet file to zero and does not affect configuration file.
//			- VTS can switch packet files via Change Packet file menu item.
//			- Changed port configuration for usability. 
//			- Changed device configuration for usability.
//			- Added ability to edit object, property and values to devices.
//			- Added ReadProperty and WriteProperty support to devices.
//			- Added automatic RP and WP to parameters of configured devices.
//			- Changed names configuration for usability.
//			- VTS had a registry problem confusing an INI path with key names.
//			- Added preferences dialog.  Preferences are stored in "Setting" subkey inside registry.
//			- Added preference setting for specification of number of slots used for main packet view cache.
//			- Added timeout for autoscroll.
//			- Added preference setting for number of seconds auto-scroll is to remain inactive.
//			- Added preference for relative vs. absolute packet file path association in workspace.
//
//
//  3.4.1
//			[ 747747 ] EPICS loading problems:
//			Fixed this problem loading bitstrings larger than one byte. 
//			Additional EPICS loading problems have been fixed:
//			
//				- VTS would cause an exception when property names were not found in AVG, Trend-Log and
//				  Multistate Value objects.
//				- VTS would not properly recognize property names if comments followed on the same line.
//				- EPICS parser was incorrectly looking for the following in Supported Services and Objects: 
//						"Read Range"  -> now changed to "ReadRange"
//						"UTC-Time-Synchronization"  -> now changed to "UTCTimeSynchronization"
//						"Trend-Log"  -> now changed to "Trend Log"
//						"trendlog"  -> now changed to "Trend-Log" for object type specification
//				- Fixed VTS to skip consistency check (and subsequent dialog box) if user cancels at
//				  any encountered parsing error.
//				- Fixed EPICS parsing error notifications to parent script window properly.
//				- Fixed problem where EPICS wouldn't load again if user canceled a prior EPICS load attempt
//				  without without restarting VTS.
//				- Fixed EPICS parser to barf on malformed object property reference in list-of-object-property-reference.
//				  If delimeter is not present, parsing notifies with an error instead of continuing and
//				  subsequently attempts to recognize extraneous garbage as property names.
//				- Fixed whitespace trim from right function - which never worked correctly.  This may fix
//				  spurrious recognition problems throughout parsing.
//
//			Added option in preferences for verification on delete packets.  Defaults to true.
//
//
//  3.4.2
//			[681223] - Changed the default delay in EXPECT and SEND packets to 3 seconds and changed the maximum
//			delay to 2 minutes.  The default and max were previously set to 1.8 seconds, which was a bug.
//
//			Added support to parse "Event-Time-Stamps" property.  Unfortunately this is not yet a property
//			that can be handled inside a script... It only parses EPICS correctly for these objects:
//				- MSI, MSO, MSV, AI, AO, AV, BI, BO, EE, LOOP, BV, BV
//				- TR object support already existed, but it was the only one.
//
//			[618217] - INCLUDE directive added to scripts.  Syntax for including files:
//
//					INCLUDE "filename.vts"
//
//				Usage of the INCLUDE directive follows these rules:
//					- The INCLUDE keyword must be the first item on a line (excluding whitespace) and is 
//					  followed by a filename to include in quotes.
//					- The extension for included files is the same as script files.
//					- Any number of files can be included.
//					- Included files can contain nested includes.
//					- The file name should include a relative path that is always relative to the location
//					  of the file that contains the include statement.  This is also true for each included
//					  file.  This means that file specifications included from the base file will be
//					  relative to the base file but files that are included from an include file will be
//					  relative to the location of the include file.  For example:  If included files are
//					  in a subdirectory called 'inc', only the base file, which is located one directory up
//					  from 'inc' will have the statement:  INCLUDE "inc\file1.vts".
//					  If file1.vts contains include statements that pull in files from the inc directory as
//					  well, the statement will be:  INCLUDE "file2.vts".
//					- Absolute paths may be used as well if specified properly (C:\, \, etc.)
//					- INCLUDE statements can occur anywhere within the script file with the exception of 
//					  inside statements.  This means that they cannot appear within SEND or EXPECT
//					  statements even though these statements may span several lines.  CHECK, MAKE and
//					  other multi-line statements are part of this restriction.
//					- Circular includes are not allowed.
//					- If variables are defined and initialized in included files that are already
//					  defined in the base file, the values will be overridden with the last encountered declaration.
//
//				There are a few things to note when files are included:
//					- Include files are opened and parsed during the "Check Syntax" command.
//					- When errors are encountered within an included file during parsing, an error message 
//					  will present (as usual) and the line and text that produced the error will be 
//					  highlighted in a separate editing window that allows the imediate editing and 
//					  saving of the included file.  This window is modal and parsing execution halts.
//					  A quick correction followed by another Syntax Check will have you on your way.
//					- Nodes created on the tree (following a successful syntax check) from include files
//					  will appear in blue text.  Clicking once on these nodes will display the fully
//					  qualified path to the containing include file in the editor's status bar.  Double
//					  clicking on these nodes will open a modal editing window and the statement's line
//					  will be highlighted.  This editing window will allow the file to be saved (no SaveAs).
//
//			A bug was fixed in the script editor that would not initially place the caret on the 
//			line if goto was used.
//
//			Moved Load EPICS command from Script frame to main frame under "EPICS" menu.  This menu will be populated
//			with other EPICS type functions in future versions.
//
//			Altered EPICS load results dialog to use static windows (instead of read only edit boxes) to do automatic
//			word wrap.
//			
//			Added "Load last EPICS on startup" to preferences section.
//
//			Fixed a bug in the specification of the EPICS file to load.  It was using the filename only, even if
//			the file specified from the file dialog found the file in other directories.


const int kReleaseVersion = 2;

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
//	m_Version = _T("");
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
//	VTSApp::CheckWinPcapVersion
//
//	Thanks to Thomas Weller and www.codeguru.com, this function checks to see if 
//	WinPcap is installed and checks the version.
//

void VTSApp::CheckWinPcapVersion( void )
{
	CFileVersionInfo	m_info
	;

#if 0
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
#endif
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

		switch (pMsg->message)
		{
			case WM_VTS_RCOUNT:
//MAD_DB		((VTSDocPtr)(pMsg->lParam))->NewPacketCount();
				((VTSDocPtr)(pMsg->lParam))->ProcessPacketStoreChange();
				break;

			case WM_VTS_MAXPACKETS:

				// maximum packets have been reached... we're here because the main app thread
				// has to tell us about it...

				AfxMessageBox(IDS_ERR_MAXPACKETS, MB_ICONSTOP | MB_OK);
				break;

			case WM_VTS_PORTSTATUS:
				((VTSDocPtr)(pMsg->lParam))->PortStatusChange();
				break;

			case WM_VTS_EXECMSG:
				{
				ScriptExecMsg * pexecmsg;

				while ((pexecmsg = gExecutor.ReadMsg()) != NULL)
				{
					switch( pexecmsg->GetType() )
					{
						case ScriptExecMsg::msgStatus:

							{
							ScriptMsgStatus * pmsgstatus = (ScriptMsgStatus *) pexecmsg;
							pmsgstatus->m_pdoc->SetImageStatus( pmsgstatus->m_pbase, pmsgstatus->m_nStatus );
							}
							break;

						case ScriptExecMsg::msgMakeDlg:

							{
							ScriptMsgMake * pmsgmake = (ScriptMsgMake *) pexecmsg;

							switch( pmsgmake->m_maketype )
							{
								case ScriptMsgMake::msgmakeCreate:

									pmsgmake->m_pdlg->DoModeless();
									break;

								case ScriptMsgMake::msgmakeDestroy:

									pmsgmake->m_pdlg->DestroyWindow();
									delete pmsgmake->m_pdlg;
							}
							}
							break;

						default:
							TRACE("UNKOWN Exec Msg type");
							ASSERT(0);
					}

					delete pexecmsg;
				}
				}
		}

		// we completely processed this message
		return true;
	}

	// pass along to regular processing
	return CWinApp::PreTranslateMessage(pMsg);
}

//Added by Yajun Zhou, 2002-11-22
CRecentFileList* VTSApp::GetRecentFileList()
{
	return m_pRecentFileList;
}
///////////////////////////////
