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
