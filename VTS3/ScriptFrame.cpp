// ScriptFrame.cpp : implementation of the ScriptFrame class
//

#include "stdafx.h"
#include "VTS.h"

#include "ScriptDocument.h"
#include "ScriptFrame.h"
#include "ScriptBase.h"
#include "ScriptExecutor.h"
#include "ScriptSelectSession.h"

//Added by Yajun Zhou, 2002-11-4
#include "ReadAllPropSettingsDlg.h"

#define MAX_LENGTH_OF_OBJECTTYPE		30
#define MAX_LENGTH_OF_PROPERTYNAME		100
#define MAX_DIGITS_OF_INSTANCENUMBER	10

#ifndef dword
typedef unsigned long dword;
#endif
#ifndef word
typedef unsigned short word;
#endif
#ifndef octet
typedef unsigned char octet;
#endif

CString GetObject(dword object_id);
///////////////////////////////
namespace PICS {

#include "db.h" 
#include "service.h"
#include "vtsapi.h"
#include "props.h"
#include "bacprim.h"
#include "dudapi.h"

#include "propid.h"

}

PICS::PICSdb *gPICSdb = 0;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ScriptFrame

IMPLEMENT_DYNCREATE(ScriptFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(ScriptFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(ScriptFrame)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_CHECK_SYNTAX, OnUpdateScriptCheckSyntax)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_LOADEPICS, OnUpdateScriptLoadEPICS)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_ENV, OnUpdateScriptEnvironment)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_RUN, OnUpdateScriptRun)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_HALT, OnUpdateScriptHalt)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_STEP, OnUpdateScriptStep)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_STEPPASS, OnUpdateScriptStepPass)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_STEPFAIL, OnUpdateScriptStepFail)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_KILL, OnUpdateScriptKill)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_RESET, OnUpdateScriptReset)
	ON_COMMAND(ID_SCRIPT_CHECK_SYNTAX, OnScriptCheckSyntax)
	ON_COMMAND(ID_SCRIPT_LOADEPICS, OnScriptLoadEPICS)
	ON_COMMAND(ID_SCRIPT_ENV, OnScriptEnvironment)
	ON_COMMAND(ID_SCRIPT_RUN, OnScriptRun)
	ON_COMMAND(ID_SCRIPT_HALT, OnScriptHalt)
	ON_COMMAND(ID_SCRIPT_STEP, OnScriptStep)
	ON_COMMAND(ID_SCRIPT_STEPPASS, OnScriptStepPass)
	ON_COMMAND(ID_SCRIPT_STEPFAIL, OnScriptStepFail)
	ON_COMMAND(ID_SCRIPT_KILL, OnScriptKill)
	ON_COMMAND(ID_SCRIPT_RESET, OnScriptReset)
	ON_COMMAND(ID_SCRIPT_READALLPROPERTY, OnReadAllProperty)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_READALLPROPERTY, OnUpdateScriptReadallproperty)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScriptFrame construction/destruction

ScriptFrame::ScriptFrame()
	: m_pDoc(0)
{
}

ScriptFrame::~ScriptFrame()
{
}

BOOL ScriptFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// ScriptFrame diagnostics

#ifdef _DEBUG
void ScriptFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void ScriptFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// ScriptFrame message handlers

BOOL ScriptFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// save a pointer to the document
	m_pDoc = (ScriptDocument*)pContext->m_pCurrentDoc;

	// create a splitter with 2 rows, 1 column
	if (!m_wndSplit1.CreateStatic(this, 2, 1))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	// add the first splitter pane - the edit view in row 0
//	Changed by Yajun Zhou, 2002-4-22
//	if (!m_wndSplit1.CreateView(0, 0,
//		RUNTIME_CLASS(CEditView), CSize(0, 300), pContext))
	if (!m_wndSplit1.CreateView(0, 0,
		RUNTIME_CLASS(ScriptEdit), CSize(0, 300), pContext))
////////////////////////////////////////////////////////////
	{
		TRACE0("Failed to create edit pane\n");
		return FALSE;
	}
	
	// nice to have around
//	Changed by Yajun Zhou, 2002-4-22
//	m_pEditView = (CEditView*)m_wndSplit1.GetPane( 0, 0 );
	m_pEditView = (ScriptEdit*)m_wndSplit1.GetPane( 0, 0 );
//////////////////////////////////////////////////////////

	// pass it along to the document
	m_pDoc->m_editData = &m_pEditView->GetEditCtrl();

	// add the second splitter pane - which is a nested splitter with 2 columns
	if (!m_wndSplit2.CreateStatic(
		&m_wndSplit1,     // our parent window is the first splitter
		1, 2,               // the new splitter is 1 row, 2 columns
		WS_CHILD | WS_VISIBLE | WS_BORDER,  // style, WS_BORDER is needed
		m_wndSplit1.IdFromRowCol(1, 0)
			// new splitter is in the second row, first column of first splitter
	   ))
	{
		TRACE0("Failed to create nested splitter\n");
		return FALSE;
	}
	
	// now create the two views inside the nested splitter
	int cxHalf = max(lpcs->cx / 2, 20);
	
	// tree view gets left half
	if (!m_wndSplit2.CreateView(0, 0,
		RUNTIME_CLASS(ScriptContentTree), CSize(cxHalf, 0), pContext))
	{
		TRACE0("Failed to create tree pane\n");
		return FALSE;
	}
	
	// nice to have around, and pass along to the document
	m_pContentTree = (ScriptContentTree*)m_wndSplit2.GetPane( 0, 0 );
	m_pContentTree->m_pDoc = m_pDoc;
	m_pDoc->m_pContentTree = m_pContentTree;
//	Added by Yajun Zhou, 2002-6-20
	m_pContentTree->m_pEditView = m_pEditView;
//////////////////////////////////////////////////////////

	// list view gets right half
	if (!m_wndSplit2.CreateView(0, 1,
		RUNTIME_CLASS(ScriptParmList), CSize(0, 0), pContext))
	{
		TRACE0("Failed to create parameter list pane\n");
		return FALSE;
	}

	// nice to have around, and pass along to the document
	m_pParmList = (ScriptParmList*)m_wndSplit2.GetPane( 0, 1 );
	m_pDoc->m_pParmList = m_pParmList;

	// success
	return TRUE;
}

//
//	ScriptFrame::OnUpdateScriptCheckSyntax
//
//	The syntax for a script can only be checked if there is no executor,
//	that is to say, nothing is running.
//

afx_msg void ScriptFrame::OnUpdateScriptCheckSyntax(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( gExecutor.IsIdle() );
}

//
//	ScriptFrame::OnUpdateScriptLoadEPICS
//
//	It's kinda lame, but the menu item is checked when an EPICS has been 
//	successfully loaded.
//

afx_msg void ScriptFrame::OnUpdateScriptLoadEPICS(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( true );
	pCmdUI->SetCheck( gPICSdb != 0 );
}

//
//	ScriptFrame::OnUpdateScriptEnvironment
//
//	The menu is checked when the parameter list in this frame is the 
//	current environment.
//

afx_msg void ScriptFrame::OnUpdateScriptEnvironment(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( true );
	pCmdUI->SetCheck( m_pParmList == gCurrentEnv );
}

//
//	ScriptFrame::OnUpdateScriptRun
//
//	The run command is enabled if the executor is idle and there is 
//	content to run or the executor is bound to this document and it is
//	not running.
//
//	If a particular test is selected, then the run command is just for 
//	that test, otherwise it is for all tests in the script.
//

afx_msg void ScriptFrame::OnUpdateScriptRun(CCmdUI* pCmdUI)
{
	if (gExecutor.IsIdle() && m_pDoc->m_pContentTree) {
		pCmdUI->Enable( true );

		if (m_pDoc->m_pSelectedTest) {
			CString	str( "&Run " );

			str += m_pDoc->m_pSelectedTest->baseLabel;
			str += "\tF5";

			pCmdUI->SetText( str );
		} else
			pCmdUI->SetText( "&Run all tests\tF5" );
	} else
	if (gExecutor.IsBound(m_pDoc)) {
		pCmdUI->SetText( "&Resume" );
		pCmdUI->Enable( !gExecutor.IsRunning() );
	} else {
		pCmdUI->SetText( "Run\tF5" );
		pCmdUI->Enable( false );
	}
}

//
//	ScriptFrame::OnUpdateScriptHalt
//
//	The halt menu option is only enabled if there is a running script associated with 
//	this frame.
//

afx_msg void ScriptFrame::OnUpdateScriptHalt(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( gExecutor.IsRunning() && gExecutor.IsBound(m_pDoc) );
}

//
//	ScriptFrame::OnUpdateScriptStep
//
//	The step command is similar to the run command.  It will allow a single step to 
//	be processed when this script is associated with the executor but is not running.
//

afx_msg void ScriptFrame::OnUpdateScriptStep(CCmdUI* pCmdUI)
{
	if (gExecutor.IsIdle() && m_pDoc->m_pContentTree) {
		if (m_pDoc->m_pSelectedTest) {
			CString	str( "&Step " );

			str += m_pDoc->m_pSelectedTest->baseLabel;
			str += "\tF6";

			pCmdUI->SetText( str );
			pCmdUI->Enable( true );
		} else {
			pCmdUI->SetText( "Step\tF6" );
			pCmdUI->Enable( false );
		}
	} else
	if (gExecutor.IsBound(m_pDoc)) {
		pCmdUI->SetText( "&Step\tF6" );
		pCmdUI->Enable( !gExecutor.IsRunning() );
	} else {
		pCmdUI->SetText( "Step\tF6" );
		pCmdUI->Enable( false );
	}
}

//
//	ScriptFrame::OnUpdateScriptStepPass
//
//	This step command is similar to the simple step, but the executor must be stopped.
//

afx_msg void ScriptFrame::OnUpdateScriptStepPass(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( gExecutor.IsStopped() && gExecutor.IsBound(m_pDoc) );
}

//
//	ScriptFrame::OnUpdateScriptStepFail
//
//	This step command is similar to the simple step, but the executor must be stopped.
//

afx_msg void ScriptFrame::OnUpdateScriptStepFail(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( gExecutor.IsStopped() && gExecutor.IsBound(m_pDoc) );
}

//
//	ScriptFrame::OnUpdateScriptKill
//
//	If there is a running script associated with this document, allow it to be 
//	killed (even if it has not been halted first).
//

afx_msg void ScriptFrame::OnUpdateScriptKill(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( gExecutor.IsBound(m_pDoc) );
}

//
//	ScriptFrame::OnUpdateScriptReset
//
//	The reset command can only be executed if there is no script running.
//

afx_msg void ScriptFrame::OnUpdateScriptReset(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( gExecutor.IsIdle() );
}

//
//	ScriptFrame::OnScriptCheckSyntax
//

void ScriptFrame::OnScriptCheckSyntax() 
{
	m_pDoc->CheckSyntax();
}

//
//	ScriptFrame::OnScriptLoadEPICS
//

void ScriptFrame::OnScriptLoadEPICS() 
{
	int				errc;
	int             errPICS;
	
	if (gPICSdb) {
		// delete the database
		PICS::MyDeletePICSObject( gPICSdb->Database );

		// toss the rest
		delete gPICSdb;
		gPICSdb = 0;
	}

	// prep a dialog
	CFileDialog	fd( TRUE, "tpi", NULL, OFN_FILEMUSTEXIST, "EPICS (*.tpi)|*.tpi||" )
	;
	
	// if not acceptable, exit
	if (fd.DoModal() != IDOK)
		return;

	// make a new database
	gPICSdb = new PICS::PICSdb;

	// read in the EPICS
	PICS::ReadTextPICS( (char *)(LPCSTR)fd.GetFileName(), gPICSdb, &errc,&errPICS );
	TRACE1( "error count = %d\n", errc );
    //Added by Liangping Xu,2002-11
	CCheckEPICSCons dlgEPICSCons;
    if(errPICS){
		errc+=errPICS;
	    dlgEPICSCons.DoModal();
	}
    /////////////////////////////////
	
	// display the results
	if (errc == 0) {
		ScriptLoadResults().DoModal();
	} else {
		// delete the database
		PICS::MyDeletePICSObject( gPICSdb->Database );

		// toss the rest
		delete gPICSdb;
		gPICSdb = 0;
	}
}

//
//	ScriptFrame::OnScriptEnvironment
//

void ScriptFrame::OnScriptEnvironment()
{
	ScriptParmListPtr	oldEnv
	;

	// remember the current environment
	oldEnv = gCurrentEnv;

	// reset the current environment and unload everyone else
	if (gCurrentEnv) {
		gCurrentEnv->ResetCurrentEnv();

		for (int i = 0; i < gScriptParmLists.Length(); i++) {
			ScriptParmListPtr splp = gScriptParmLists[i];

			if (splp != gCurrentEnv)
				splp->UnloadEnv();
		}

		gCurrentEnv = 0;
	}

	// if this becomes the new environment, set it and load everyone else
	if (m_pParmList != oldEnv) {
		gCurrentEnv = m_pParmList;

		gCurrentEnv->SetCurrentEnv();

		for (int j = 0; j < gScriptParmLists.Length(); j++) {
			ScriptParmListPtr splp = gScriptParmLists[j];

			if (splp != gCurrentEnv)
				splp->LoadEnv();
		}
	}
}

//
//	ScriptFrame::OnScriptRun
//

void ScriptFrame::OnScriptRun() 
{
	// if the test is already running, ignore this (probably sync issue)
	if (gExecutor.IsRunning()) {
		TRACE0( "Already running\n" );
		return;
	}

	// if the test has been stopped, allow it to continue
	if (gExecutor.IsStopped()) {
		TRACE0( "Resume\n" );
		gExecutor.Resume();
		return;
	}

	// new test to run
	TRACE0( "New run\n" );

	// if it can be set up, let it run
	if (DoInitialSetup())
		gExecutor.Run();
}

//
//	ScriptFrame::OnScriptHalt
//

void ScriptFrame::OnScriptHalt() 
{
	// make sure the executor is bound and running
	if (gExecutor.IsIdle()) {
		TRACE0( "Error: executor is idle\n" );
		return;
	}
	if (!gExecutor.IsRunning()) {
		TRACE0( "Error: executor not running\n" );
		return;
	}

	// stop the executor
	gExecutor.Halt();
}

//
//	ScriptFrame::OnScriptStep
//

void ScriptFrame::OnScriptStep() 
{
	// if it is idle, bind it to the selected test and single step
	if (gExecutor.IsIdle()) {
		// new test to run
		TRACE0( "Step into new run\n" );

		// get the executor ready
		if (!DoInitialSetup())
			return;
	} else
	// if it is running, toss an error
	if (gExecutor.IsRunning()) {
		TRACE0( "Error: executor running\n" );
		return;
	}

	// let it run for a packet
	gExecutor.Step();
}

//
//	ScriptFrame::OnScriptStepPass
//

void ScriptFrame::OnScriptStepPass() 
{
	if (!gExecutor.IsStopped()) {
		// new test to run
		TRACE0( "Error: executor is not stopped\n" );
		return;
	}

	// assume current packet passed
	gExecutor.Step( true );
}

//
//	ScriptFrame::OnScriptStepFail
//

void ScriptFrame::OnScriptStepFail() 
{
	if (!gExecutor.IsStopped()) {
		// new test to run
		TRACE0( "Error: executor is not stopped\n" );
		return;
	}

	// assume current packet failed
	gExecutor.Step( false );
}

//
//	ScriptFrame::OnScriptKill
//

void ScriptFrame::OnScriptKill() 
{
	// verify the executor is running or stopped
	if (gExecutor.IsIdle()) {
		TRACE0( "Error: executor is idle\n" );
		return;
	}

	// kill the test, executor returns to idle
	gExecutor.Kill();
}

//
//	ScriptFrame::OnScriptReset
//

void ScriptFrame::OnScriptReset() 
{
	// make sure the executor is idle
	if (!gExecutor.IsIdle()) {
		TRACE0( "Error: executor is not idle\n" );
		return;
	}

	// let the document reset
	m_pDoc->Reset();
}

//
//	ScriptFrame::DoInitialSetup
//

bool ScriptFrame::DoInitialSetup( void )
{
	VTSDocPtr			vdp
	;
	ScriptDocumentPtr	sdp
	;
	ScriptTestPtr		stp = 0
	;
	
	// find a database for test messages
	if (gDocList.Length() == 0) {
		// alert the user
		AfxMessageBox( _T("No session database available to receive test results") );
		return false;
	} else
	if (gDocList.Length() == 1)
		vdp = gDocList.Child( 0 );
	else {
		ScriptSelectSession	sss
		;

		if (sss.DoModal() && sss.m_SelectedDoc)
			vdp = sss.m_SelectedDoc;
		else {
			TRACE0( "User canceled from selecting session database\n" );
			return false;
		}
	}

	// the document to run is this one
	sdp = m_pDoc;

	// if there is a selected test, use it
	if (m_pDoc->m_pSelectedTest)
		stp = m_pDoc->m_pSelectedTest;

	// tell the executor this new found knowledge
	gExecutor.Setup( vdp, sdp, stp );

	// success
	return true;
}

//
//	ScriptFrame::Serialize
//

void ScriptFrame::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	// storing code
	}
	else
	{	// loading code
	}
}

//	Added by Yajun Zhou, 2002-11-1
//
//	ScriptFrame::OnReadAllProperty
//
void ScriptFrame::OnReadAllProperty() 
{
	if (gPICSdb)
	{
		if(!CreateScriptFile())
			return;

		CMultiDocTemplate* pDocTemp = (CMultiDocTemplate*)GetActiveDocument()->GetDocTemplate();
		ScriptDocument* pDoc = (ScriptDocument*)pDocTemp->OpenDocumentFile(m_strFileFullName);

		if(DeleteFile(m_strFileFullName))
		{
			CRecentFileList* pRecentFileList = ((VTSApp*)AfxGetApp())->GetRecentFileList();
			pRecentFileList->Remove(0);
		}
		else
		{
			AfxMessageBox("The temporary file didn't be deleted!");
			return;
		}
	}
	else
		AfxMessageBox("No EPICS Loaded!");
}

void ScriptFrame::OnUpdateScriptReadallproperty(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(gPICSdb != NULL);
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-11-3
//	Purpose:	Write a new line in m_readAllPropFile
//	In:			CString strNewLine: The content to be added
//	Out:		void
//******************************************************************
void ScriptFrame::SetLine(CString strNewLine)
{
	m_readAllPropFile.Write(strNewLine.GetBuffer(0), strNewLine.GetLength());
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-11-4
//	Purpose:	Create a script file to read all the properties
//	In:			void
//	Out:		BOOL: TRUE when success
//					  FALSE when fault
//******************************************************************
BOOL ScriptFrame::CreateScriptFile()
{

	GetCurrentDirectory(_MAX_PATH, m_strFileFullName.GetBufferSetLength(_MAX_PATH));

	strcat(m_strFileFullName.GetBufferSetLength(_MAX_PATH),
		"\\ReadAllProperties");
	
	int nNameLength = m_strFileFullName.GetLength();

	try
	{
		m_readAllPropFile.Open(m_strFileFullName.GetBuffer(nNameLength), 
			CFile::modeCreate|CFile::modeWrite);
		
		CReadAllPropSettingsDlg SettingsDlg;
		
		if(SettingsDlg.DoModal() == IDOK)
		{
			SetLine(" ;Read All Property Tests\r\n");
			
			//Setup
			SetLine(" ;-------------------------------------------------------------------------------------\r\n");
			SetLine("  Setup ReadProperty Tests\r\n");
			SetLine(" ;-------------------------------------------------------------------------------------\r\n");
			SetLine("\r\n");
			
			CString strIUTIPSetup = "  IUT_IP = ";
			strIUTIPSetup += SettingsDlg.m_strIUTIP;
			strIUTIPSetup += "\r\n";
			SetLine(strIUTIPSetup);
			SetLine("\r\n");
			
			PICS::generic_object far *Database = gPICSdb->Database;
			dword object_id;
			CString strObjectSetup;
			char cObj[15];
			int nObjNum;
			for(nObjNum = 0; Database->next != NULL; nObjNum++)
			{
				object_id = Database->object_id;
				
				strObjectSetup = "  ";
				sprintf(cObj,"OBJECT%d", nObjNum+1);
				strObjectSetup += cObj;
				strObjectSetup += " = ";
				strObjectSetup += GetObject(object_id);
				strObjectSetup += "\r\n";
				
				SetLine( strObjectSetup);
				Database = (PICS::generic_object far *)Database->next;
			}
			SetLine("\r\n");
			
			//SECTION
			CString strSection;
			CString strNetwork;
			CString strObjBuffer, strPropBuffer, strALBuffer;
			char cTest[25 + MAX_LENGTH_OF_PROPERTYNAME];
			char cPropName[MAX_LENGTH_OF_PROPERTYNAME];
			
			word object_type;
			dword PropId;
			Database = gPICSdb->Database;				
			
			for(int j = 0; j < nObjNum; j++)
			{
				object_type = Database->object_type;
				
				sprintf(cObj,"OBJECT%d", j+1);
				strSection = "  SECTION Read Properties of ";
				strSection += cObj;
				strSection += "\r\n";
				
				SetLine(" ;-------------------------------------------------------------------------------------\r\n");
				SetLine(strSection);
				SetLine(" ;-------------------------------------------------------------------------------------\r\n");
				
				//Test
				for(word k = 0; k < 64; k++)
				{
					if((Database->propflags[k]&ValueUnknown) == ValueUnknown)	// PropertyValue is unspecified
						continue;												// skip
					
					if(PICS::GetPropNameSupported(cPropName, k, object_type, Database->propflags, &PropId) > 0)
					{
						sprintf(cTest,"  TEST#%d.%d - Read %s\r\n", j+1, k+1, cPropName);
						SetLine(cTest);
						
						//DEPENDENCIES
						SetLine("\r\n");
						SetLine("  DEPENDENCIES None\r\n");
						SetLine("\r\n");
						
						//SEND
						SetLine("    SEND (\r\n");
						strNetwork = "\tNETWORK = \"";
						strNetwork += SettingsDlg.m_strNetwork;
						strNetwork += "\"\r\n";
						SetLine(strNetwork);
						SetLine("\tDA = IUT_IP\r\n");
						SetLine("\tDER = TRUE\r\n");
						SetLine("\tBVLCI = ORIGINAL-UNICAST-NPDU\r\n");
						SetLine("\tPDU = Confirmed-Request\r\n");
						SetLine("\tService = ReadProperty\r\n");
						SetLine("\tInvokeID = 1\r\n");
						SetLine("\tSegMsg = 0\r\n");
						SetLine("\tSegResp = 0\r\n");
						SetLine("\tMaxResp = 1470\r\n");
						strObjBuffer = "\tObject = 0, "; 
						strObjBuffer += cObj;
						strObjBuffer += "\r\n";
						SetLine(strObjBuffer);
						strPropBuffer = "\tProperty = 1, ";
						strPropBuffer += cPropName;
						strPropBuffer += "\r\n";
						SetLine(strPropBuffer);
						SetLine("    )\r\n");
						SetLine("\r\n");
						
						//EXPECT
						SetLine("    EXPECT (\r\n");
						SetLine(strNetwork);
						SetLine("\tSA = IUT_IP\r\n");
						SetLine("\tDER = FALSE\r\n");
						SetLine("\tBVLCI = ORIGINAL-UNICAST-NPDU\r\n");
						SetLine("\tPDU = ComplexAck\r\n");
						SetLine("\tService = ReadProperty\r\n");
						SetLine(strObjBuffer);
						SetLine(strPropBuffer);
						SetLine("\tOpenTag 3\r\n");
						strALBuffer = "\t\tAL = {";
						strALBuffer += cPropName;
						strALBuffer += "}\r\n";
						SetLine(strALBuffer);
						SetLine("\tCloseTag 3\r\n");
						SetLine("    )\r\n");
						SetLine(" ;-------------------------------------------------------------------------------------\r\n");
					}
				}
				Database = (PICS::generic_object far *)Database->next;
			}
		}
		else
			return FALSE;
		
		m_readAllPropFile.Close();
	}
	catch(CFileException e)
	{
		CString str;
		e.GetErrorMessage(str.GetBuffer(1), 1);
		AfxMessageBox(str);
		return FALSE;
	}

	return TRUE;
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-11-4
//	Purpose:	Get Object
//	In:			dword object_id: The object's identifier
//	Out:		CString: a string describe the object's type and instance
//						 number like this format: "object_type, instance_number"
//******************************************************************
CString GetObject(dword object_id)
{
	word object_type = (word)(object_id >> 22);
	unsigned long instance_number = (object_id & 0x003fffff);

	CString strObject;
	char cObjType[MAX_LENGTH_OF_OBJECTTYPE];

	PICS::GetObjType(object_type, cObjType);

	char cInsNum[MAX_DIGITS_OF_INSTANCENUMBER];
	itoa(instance_number, cInsNum, 10);

	strObject += cObjType;
	strObject += ", ";
	strObject += cInsNum;

	return strObject;
}

