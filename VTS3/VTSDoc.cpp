// VTSDoc.cpp : implementation of the VTSDoc class
//

#include "stdafx.h"
#include <afxmt.h>
#include <direct.h>

#include "VTS.h"
#include "VTSPreferences.h"
#include "Mainfrm.h"

#include "VTSDoc.h"
// MAD_DB #include "VTSValue.h"

#include "VTSPortDlg.h"
//#include "VTSPortIPDialog.h"
//#include "VTSPortEthernetDialog.h"
#include "VTSNamesDlg.h"
//#include "VTSDevicesDlg.h"
#include "VTSDevicesTreeDlg.h"
#include "VTSFiltersDlg.h"

//#include "PacketFileDlg.h"
#include "FrameContext.h"

#include "Send.h"
#include "SendPage.h"

#include "ScriptExecutor.h"

#include "VTSStatisticsCollector.h"
#include "VTSStatisticsDlg.h"

//Xiao Shiyuan 2002-9-23
#include "VTSWinPTPPort.h"
//#include "VTSPortPTPDialog.h"
#include "PropID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define NT_DebugMemState	0

/////////////////////////////////////////////////////////////////////////////
// VTSDoc

LPCSTR VTSDoc::m_pszDefaultFilename = "vts3.cfg";


IMPLEMENT_DYNCREATE(VTSDoc, CDocument)

BEGIN_MESSAGE_MAP(VTSDoc, CDocument)
	//{{AFX_MSG_MAP(VTSDoc)
	ON_COMMAND(ID_VIEW_STATISTICS, OnViewStatistics)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSDoc construction/destruction

#if NT_DebugMemState
_CrtMemState		s1, s2, s3;
#endif

//create the statistics dialog object
//VTSStatisticsDlg	dlg(NULL);

VTSDoc::VTSDoc()
//MAD_DB	: m_PacketCount(0), m_pDB(0), m_FrameContexts(0)
	: m_FrameContexts(0)
	, m_pPortDlg(0), m_postMessages(false), m_pStatitiscsDlg(0)
	, m_nPacketCount(0)				// MAD_DB
	, m_strLogFilename("vts3.vpk")
{
#if NT_DebugMemState
	_CrtMemCheckpoint( &s1 );
#endif
	m_bStatisticsDlgInUse=false;
	m_pStatisticsCollector=NULL;

	m_apPackets.SetSize(0, 400);		// reallocate in 1600 byte chunks (for DWORD size pointer)
	m_fLoadPackets = true;

//	m_pStatitiscsDlg=&dlg;
//	m_pStatitiscsDlg->m_pDoc=this;
}

//
//	VTSDoc::~VTSDoc
//
//	The only responsibility of the destructor is cleaning up after itself.
//	When a new document is canceled, the document object is destroyed before 
//	the clients, so they really don't know their context has gone away.
//

VTSDoc::~VTSDoc()
{
	while (m_FrameContexts)
		UnbindFrameContext( m_FrameContexts );

	DestroyPacketArray();

#if NT_DebugMemState
	_CrtMemCheckpoint( &s2 );
	_CrtMemDumpAllObjectsSince( &s1 );
	_CrtMemDifference( &s3, &s1, &s2 );
	_CrtMemDumpStatistics( &s3 );
#endif
}


// switch CWD to path supplied by relative...  Even if we're not doing relative paths
// the CWD doesn't matter...

void VTSDoc::ChangeCWDToWorkspace( LPCSTR lpszWksFile /* = NULL */ )
{
	CString strPath = lpszWksFile;
	
	StripToPath(&strPath);

	// now change directories if there is any left
	if ( !strPath.IsEmpty() )
		_chdir(strPath);
}





LPCSTR VTSDoc::StripToPath( CString * pstr )
{
	if ( pstr == NULL )
		return NULL;

	// if no path supplied, get it from the document... might be null if in document open
	// hence the supplied path

	if ( pstr->IsEmpty() )
		*pstr = GetPathName();

	// reduce string to path only
	int n = pstr->GetLength();
	for ( char * p = pstr->GetBuffer(1); p != NULL && n > 0 && p[n-1] != '\\'; n-- )
		p[n-1] = 0;

	pstr->ReleaseBuffer();
	return *pstr;
}



//
//	VTSDoc::OnNewDocument
//

BOOL VTSDoc::OnNewDocument()
{
	// be nice to the base class, but it is not used
	if (!CDocument::OnNewDocument())
		return FALSE;

	// must have this here to add to recent workspace list and
	// tell document what name we're using...

	SetPathName(m_pszDefaultFilename);

	// Initialize default data for vts3.cfg
	SetTitle(m_pszDefaultFilename);

	// Always make sure we're in the workspace directory before attempting to open the 
	// logfile... this way the logfile name, if relative, will open correctly.  If not relative,
	// then, CWD doesn't matter.

	ChangeCWDToWorkspace();
	if ( !m_PacketDB.Open(m_strLogFilename) )
		return FALSE;

	// create a new database object and initialize it
//	m_pDB = new VTSDB();   MAD_DB
//	m_pDB->NewFile( fd.GetPathName().GetBuffer(0) );
//	m_pDB->NewFile( "..\\danner.vdb" );
	
	// MAD_DB
	DestroyPacketArray();

	// it should be empty!
//MAD_DB	SetPacketCount( pDB->GetPacketCount() );
	
	// bind to the (empty) device list
//MAD_DB	m_Devices.Load( this );

	// bind to the name list
//MAD_DB	m_Names.Load( this );

	// bind to the (empty) port list
//	m_Ports.Load( this );

	// We've got to save the file with the defaults in there...
	if ( !OnSaveDocument(m_pszDefaultFilename) )
		return FALSE;

	// associate with the global list of documents
// MAD_DB	gDocList.Append( this );

	// ready for messages
	m_postMessages = true;

	// create a new statistics collector
	m_pStatisticsCollector=new VTSStatisticsCollector();
	//gStatisticsCollector=new VTSStatisticsCollector();

	// it should be empty!
	m_nPacketCount = 0;

	ScheduleForProcessing();
	
	return TRUE;
}

//
//	VTSDoc::OnOpenDocument
//

BOOL VTSDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// We need to check to see if the file exists... if not, or there was an error opening the file,
	// then open the default.  

	CFileStatus	fs;
	if ( CFile::GetStatus(lpszPathName, fs) == FALSE )
		return FALSE;

	// Calls serialization mechanisms...  After complete, all data will be loaded (except packets)
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// Always make sure we're in the workspace directory before attempting to open the 
	// logfile... this way the logfile name, if relative, will open correctly.  If not relative,
	// then, CWD doesn't matter.

	// Try to open packet file... create if not exist and report if problem...
	ChangeCWDToWorkspace(lpszPathName);
	if ( !m_PacketDB.Open(m_strLogFilename)  &&  !DoPacketFileNameDialog(false) )
		return FALSE;

	// create a new statistics collector
	m_pStatisticsCollector=new VTSStatisticsCollector();
//	gStatisticsCollector=new VTSStatisticsCollector();

//	m_pDB = new VTSDB();  MAD_DB

	DestroyPacketArray();

//	try
//	{
//		m_pDB->OpenFile( (char *)lpszPathName );
//		m_pDB->OpenFile( "..\\danner.vdb"	);
//	catch (char *errMsg) {
//		AfxMessageBox( errMsg );
//		delete m_pDB;
//		m_pDB = 0;
//		return FALSE;

//	try
//	{

		// MAD_DB load previous packets
//		m_nPacketCount = LoadPacketArray();
	LoadPacketArray();

		// bind to the port list and open the ports
//MAD_DB		m_Devices.Load( this );

		// bind to the name list
//MAD_DB		m_Names.Load( this );

		// bind to the port list and open the ports
//		m_Ports.Load( this );

		// link all in memory structures together
	FixupPortToDeviceLinks();
	FixupNameToPortLinks();
	FixupFiltersToPortLinks();

	try
	{
		for ( int n = 0; n < m_devices.GetSize(); n++ )
			m_devices[n]->Activate();

		for ( int i = 0; i < m_ports.GetSize(); i++ )
			m_ports[i]->Refresh();
	}
	catch (...)
	{
		AfxMessageBox("An unexpected exception ocurred initializing ports and devices.");
	}


		// associate with the global list of documents
//MAD_DB		gDocList.Append( this );

		// ready for messages
	m_postMessages = true;

	ScheduleForProcessing();

/* MAD_DB
		//get the statistics from the loading db file
		for(int i=0;i<m_pDB->GetPacketCount();i++)
		{
			VTSPacket	pkt;
			m_pDB->ReadPacket(i,pkt);
			BACnetPIInfo	summary( true, false );
			summary.Interpret( (BACnetPIInfo::ProtocolType)pkt.packetHdr.packetProtocolID
				, (char *)pkt.packetData
				, pkt.packetLen);
			m_pStatisticsCollector->Update(summary.summaryLine,pkt.packetLen,pkt.packetHdr.packetType,pkt.packetHdr.packetProtocolID);
		//	gStatisticsCollector->Update(summary.summaryLine,pkt.packetLen,pkt.packetHdr.packetType,pkt.packetHdr.packetProtocolID);
		}
*/

		// get the packet count
//		SetPacketCount( m_pDB->GetPacketCount() );		MAD_DB

	return TRUE;
//	}
//	catch (char *errMsg)
//	{
//		AfxMessageBox( errMsg );
//	}
//	catch (...)
//	{
//		AfxMessageBox("An unexpected exception ocurred loading the database.  The database is most likely corrupted and cannot be opened due to port initialization problems.  The document will be closed and removed from the recently used list.");
//
//		// OnDocumentClose will automatically be called by MFC after a FALSE is returned here...
//	}
//	return FALSE;
}

//
//	VTSDoc::OnCloseDocument
//

void VTSDoc::OnCloseDocument() 
{
	// delete the statistics collector
	if (m_pStatisticsCollector)
		delete m_pStatisticsCollector;

	// no more messages please
	m_postMessages = false;

	// if the executor is associated with this document, kill it
	if (gExecutor.IsBound(this))
		gExecutor.Kill();

	// unload the port list
//	m_Ports.Unload();

	// unload the device list
//MAD_DB	m_Devices.Unload();

	// disassociate with the global list of documents
//MAD_DB	gDocList.Remove( this );

	for ( int i = 0; i < m_ports.GetSize(); i++ )
		m_ports[i]->Deactivate();

	for ( int n = 0; n < m_devices.GetSize(); n++ )
		m_devices[n]->Deactivate();

	// close the database
//	if (m_pDB) {
//		m_pDB->Close();
//		delete m_pDB;
//	}
	m_PacketDB.Close();

	// MAD_DB
	DestroyPacketArray();

	// pass along to the framework
	CDocument::OnCloseDocument();

//	delete gStatisticsCollector;
}


// This method is called by the framework during an open...
// We need to override it so we call it telling it NOT to add this document
// to the standard document MRU list...
// Add it to our own for workspaces...

void VTSDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	CDocument::SetPathName(lpszPathName, FALSE);
	
	// Path has been made full and stored in document...
	// Now add this document to recent workspaces...

	((VTSApp *) AfxGetApp())->AddToRecentWorkspaceList(GetPathName());
}


//
//	VTSDoc::DoPortsDialog
//

void VTSDoc::DoPortsDialog( void )
{
//MAD_DB	VTSPortDlg		dlg( &m_Ports, &m_Devices );
	VTSPortDlg		dlg(this);

	m_pPortDlg = &dlg;

	if ( dlg.DoModal() == IDOK )
		SaveConfiguration();

	m_pPortDlg = NULL;
}

//
//	VTSDoc::DoNamesDialog
//

void VTSDoc::DoNamesDialog( void )
{
	VTSNamesDlg		dlg( GetNames(), GetPorts() );

	if ( dlg.DoModal() == IDOK )
		SaveConfiguration();
}

//
//	VTSDoc::DoCaptureFiltersDialog
//

void VTSDoc::DoCaptureFiltersDialog( void )
{
	VTSFiltersDlg		dlg( GetCaptureFilters(), GetPorts() );

	if ( dlg.DoModal() == IDOK )
		SaveConfiguration();
}

//
//	VTSDoc::DoDisplayFiltersDialog
//

void VTSDoc::DoDisplayFiltersDialog( void )
{
	VTSFiltersDlg		dlg( GetDisplayFilters(), GetPorts() );

	if ( dlg.DoModal() == IDOK ) {
		SaveConfiguration();

		// reload all the packets, applying the (new) filter
		ReloadPacketStore();

		// tell the application the list has changed
		ScheduleForProcessing();
	}
}


void VTSDoc::ReActivateAllPorts()
{
	for ( int i = 0; i < m_ports.GetSize(); i++ )
		if ( m_ports[i]->IsDirty() )
			m_ports[i]->Refresh();
}


void VTSDoc::SetNewCacheSize(void)
{
	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();

	for (CFrameContext* cur = m_FrameContexts; cur; cur = cur->m_NextFrame)
		cur->UpdatePrefs();

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}


// Called by the list view in driving what ought to be cached...
// This allows us to prep our virtual list of VTSPackets with a caching scheme if we're going
// virtual...
// nFrom is the item number (lo) to start loading cache for and nTo is the high.

void VTSDoc::CacheHint(int nFrom, int nTo)
{
	// for now do nothing since our m_apPacket pointer array holds pointers to all of our packets
	// when we go virtual we'll do something else about this...
}



//
//	VTSDoc::DoDevicesDialog
//

void VTSDoc::DoDevicesDialog( void )
{
//	VTSDevicesDlg		dlg( &m_Devices, &m_devices );
	VTSDevicesTreeDlg	dlg( &m_devices );

	if ( dlg.DoModal() == IDOK )
		SaveConfiguration();
}


void VTSDoc::SaveConfiguration()
{
	OnSaveDocument(GetPathName());
}


bool VTSDoc::DoPacketFileNameDialog( bool fReload /* = true */ )
{
	CString newName =  _T("NewPacketFile.vpk");
	CString strPath;

	CFileDialog dlgFile(TRUE);

	dlgFile.m_ofn.Flags |= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

	CString strFilter(_T("VTS Packet Files (*.vpk)"));
	strFilter += (TCHAR)'\0';
	strFilter += _T("*.vpk");
	strFilter += (TCHAR)'\0';
	strFilter += _T("All Files (*.*)");
	strFilter += (TCHAR)'\0';
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';
	dlgFile.m_ofn.nMaxCustFilter = 2;

	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = "Specify Packet File";
	dlgFile.m_ofn.lpstrFile = newName.GetBuffer(_MAX_PATH);
	dlgFile.m_ofn.lpstrInitialDir = StripToPath(&strPath);

	CString strStatusText;
	strStatusText.Format("Current logfile: %s", m_strLogFilename );
	((CFrameWnd *) AfxGetApp()->m_pMainWnd)->SetMessageText(strStatusText);

	// Change CWD and also specify lpstrInitialDir for 2000 and XP...
	// This always starts the search for a new logfile from the workspace directory

	ChangeCWDToWorkspace();					// for W98 ??
	int nResult = dlgFile.DoModal();
	newName.ReleaseBuffer();
	ChangeCWDToWorkspace();					// for W98 ??

	// packet file open auto closes old packet file if it was opened... it leaves everything as it was
	// if the open failed and it even reports an error upon fail.

	if ( nResult != IDOK  )
		return false;

	if ( gVTSPreferences.Setting_IsPacketFileRelative() )
		ConvertPathToRelative(&newName);

	// AFter path name conversion... only attempt open if the new file is diff from old
	// This is a problem if they specify the same file and they've switched between relative and absolute

	if ( !m_strLogFilename.CompareNoCase(newName) ||  !m_PacketDB.Open(newName) )
		return false;

	m_strLogFilename = newName;
	SaveConfiguration();

	// Some usage of this function is only to re-specify loading of lost packet file linkage
	// during loading...  Don't bother reloading list because that's our calling context

	if ( !fReload )
		return true;

	strStatusText.Format(IDS_NEWPACKETFILENAME, m_strLogFilename);
	ReloadPacketStore();
	AfxMessageBox(strStatusText);

	// Must call this here.. for some reason we won't see the main list view update until we let this
	// message box go, at which time the message in the queue will have been lost (why?)

	ScheduleForProcessing();

	strStatusText.Format("Loaded logfile: %s", m_strLogFilename );
	((CFrameWnd *) AfxGetApp()->m_pMainWnd)->SetMessageText(strStatusText);

	return true;
}

void VTSDoc::ConvertPathToRelative( CString * pstr )
{
	CString strPathRelative;
	CString strOld = *pstr;

	// Loads the workspace directory.. which is what we want to make the supplied filename relative to
	StripToPath(&strPathRelative);

	int nLen1 = strPathRelative.GetLength();
	int nLen2 = strOld.GetLength();
	int nStart = 0;	

	// scan start of both strings and compare... continue while equal
	// or length runs out and increment position where directories start...

	for ( int n = 0; n < nLen1 && n < nLen2 && strPathRelative[n] == strOld[n]; n++ )
		if ( strPathRelative[n] == '\\' )
			nStart = n+1;

	// if we can't replace anything... no sense in making relative because the drive letters
	// must be different...

	if ( nStart == 0 )
		return;

	// now count how many directories in the base path we have to substitute
	int nBack = 0;
	for ( int nDir = nStart; nDir < nLen1; nDir++ )
		if ( strPathRelative[nDir] == '\\' )
			nBack++;

	// now build a new path by substituting different paths and appending remaining
	pstr->Empty();

	for ( int i = 0; i < nBack; i++ )
		*pstr += "..\\";
	*pstr += strOld.Right(nLen2 - nStart);
}


//
//	VTSDoc::PortStatusChange
//

void VTSDoc::PortStatusChange( void )
{
	if (m_pPortDlg)
		m_pPortDlg->PortStatusChange();
}

//
//	VTSDoc::DoSendWindow
//

extern int gSelectedGroup, gSelectedItem;

void VTSDoc::DoSendWindow( int iGroup, int iItem )
{
	CSend		*sendp = new CSend( "Send" )
	;
	
	// pass the group and item through globals
	gSelectedGroup = iGroup;
	gSelectedItem = iItem;

	// create the window and show it
	// madanner 6/03, changed back to be child of main window, not the desktop.  This was originally an attempt
	// to satisfy feature request 444238. 
    sendp->Create( AfxGetApp()->m_pMainWnd );
//	sendp->Create(); //Make send window a client window, Xiao Shiyuan 2002-10-24
	sendp->ShowWindow( SW_SHOW );
	
	// reset the selection
	gSelectedGroup = -1;
	gSelectedItem = -1;
}

/////////////////////////////////////////////////////////////////////////////
// VTSDoc serialization

void VTSDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_strLogFilename;
	}
	else
	{
		ar >> m_strLogFilename;
	}

	m_ports.Serialize(ar);
	m_names.Serialize(ar);
	m_devices.Serialize(ar);
	m_captureFilters.Serialize(ar);
	m_displayFilters.Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// VTSDoc diagnostics

#ifdef _DEBUG
void VTSDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void VTSDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// VTSDoc commands

//
//	VTSDoc::BindFrameContext
//
//	Binding a frame context to a document is the mechanism that changes to a
//	document can be forwarded to all of the views.
//

void VTSDoc::BindFrameContext( CFrameContext *pfc )
{
	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();

	// let it know its bound to this document
	pfc->m_pDoc = this;

	// add it to the list of other frame contexts
	pfc->m_NextFrame = m_FrameContexts;
	m_FrameContexts = pfc;

	// initialize the frame count, such as it is.  Note that this bind function
	// might be called before the document is properly initialized because the 
	// clients are created before OnNewDocument() or OnOpenDocument() is called.
	pfc->m_PacketCount = m_nPacketCount;

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}

//
//	VTSDoc::UnbindFrameContext
//
//	Remove a frame context from the documents list of contexts.  This is called 
//	when a view goes away.
//

void VTSDoc::UnbindFrameContext( CFrameContext *pfc )
{
	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();

	// let it know its properly unbound
	pfc->m_pDoc = 0;

	// add it to the list of other frame contexts
	for (CFrameContext **cur = &m_FrameContexts; *cur; cur = &(*cur)->m_NextFrame)
		if (*cur == pfc) {
			*cur = (*cur)->m_NextFrame;
			break;
		}

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}

//
//	VTSDoc::SetFrameCount
//
//	Tell all of the frame contexts that the frame count of the document 
//	has changed.  The frame context will pass the message along to 
//	all of its associated views.
//

void VTSDoc::SetPacketCount( int count )
{
	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();
	
	// make sure there is really a change
	if (m_nPacketCount != count)
	{
		// change the local copy
		m_nPacketCount = count;

		// tell the frame contexts that the frame count has changed
		for (CFrameContext* cur = m_FrameContexts; cur; cur = cur->m_NextFrame)
			cur->SetPacketCount( count );
	}

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}


// returns index of newly added packet
// Usually called from another thread... don't do more here than write to the database
// and wake up the main thread for this document to continue processing

int VTSDoc::WritePacket( VTSPacket & pkt )
{
	int		nIndex = -1;
	bool	schedule = false;

	m_FrameContextsCS.Lock();

	// make sure packet passes
	if (m_captureFilters.TestPacket( pkt )) {
		// save it in the database
		m_PacketDB.WritePacket(pkt);

		// see if we should display the packet
		if (m_displayFilters.TestPacket( pkt )) {
			// have we halted packet loads?
			VTSPacketPtr ppkt = NULL;
			try
			{
				ppkt = m_fLoadPackets ? new VTSPacket() : NULL;

				if ( ppkt != NULL )
				{
					*ppkt = pkt;
					nIndex = m_apPackets.Add(ppkt);
				}
			}
			catch(CMemoryException e)
			{
				if ( ppkt != NULL )
					delete ppkt;

				if ( m_fLoadPackets )
					::PostThreadMessage( AfxGetApp()->m_nThreadID, WM_VTS_MAXPACKETS, (WPARAM) 0, (LPARAM) this );
				m_fLoadPackets = false;
			}

			schedule = true;
		}
	}

	// release the list back to other threads
	m_FrameContextsCS.Unlock();

	// if we saved the packet, tell the main thread to update
	if (schedule)
		ScheduleForProcessing();

	return nIndex;
}


void VTSDoc::ScheduleForProcessing()
{
	// tell the application
	if ( m_postMessages )
		::PostThreadMessage( AfxGetApp()->m_nThreadID, WM_VTS_RCOUNT, (WPARAM) 0, (LPARAM) this );
}



int VTSDoc::GetPacketCount()
{
	return m_apPackets.GetSize();
}


VTSPacketPtr VTSDoc::GetPacket( int nIndex )
{
	if ( nIndex >= m_apPackets.GetSize() || nIndex < 0 )
		return NULL;

	return m_apPackets[nIndex];
}



//
//	VTSDoc::DeletePackets
//

void VTSDoc::DeletePackets( void )
{
	m_PacketDB.DeletePackets();

	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();

	DestroyPacketArray();

	// make sure there is really a change
	if (m_nPacketCount != 0)
	{
		// change the local copy
		m_nPacketCount = 0;

		// tell the frame contexts that the frame count has changed
		for (CFrameContext* cur = m_FrameContexts; cur; cur = cur->m_NextFrame) {
			cur->SetPacketCount( 0 );
			cur->SetCurrentPacket( -1 );
		}
	}

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}


void VTSDoc::ReloadPacketStore( void )
{
	// make sure nobody else is using the list
	m_FrameContextsCS.Lock();

	DestroyPacketArray();
	m_nPacketCount = 0;

	for (CFrameContext* cur = m_FrameContexts; cur; cur = cur->m_NextFrame)
	{
		cur->SetPacketCount( 0 );
		cur->SetCurrentPacket( -1 );
	}

	LoadPacketArray();
	ScheduleForProcessing();

	// release the list back to other threads
	m_FrameContextsCS.Unlock();
}



int VTSDoc::LoadPacketArray( void )
{
	try
	{
		VTSPacketPtr ppkt = new VTSPacket();

		// put everything into the packet list that matches the display filter
		for ( long lNextPosition = 0; ppkt != NULL && (lNextPosition = m_PacketDB.ReadNextPacket(*ppkt, lNextPosition)) != -1; )
			if (m_displayFilters.TestPacket(*ppkt)) {
				m_apPackets.Add(ppkt);
				ppkt = new VTSPacket();
			}

		// Always allocated one more than we needed... so kill it
		if ( ppkt != NULL )
			delete ppkt;
	}
	catch(CMemoryException e)
	{
		AfxMessageBox(IDS_ERR_MAXPACKETS, MB_ICONSTOP | MB_OK);
		m_fLoadPackets = false;
	}

	return m_apPackets.GetSize();
}


void VTSDoc::DestroyPacketArray( void )
{
	// MAD_DB
	// Kill all packets stored in memory

	for ( int i = 0; i < m_apPackets.GetSize(); i++ )
		delete m_apPackets[i];

	m_apPackets.RemoveAll();
	m_fLoadPackets = true;			// start again if there was an error
}


//	This func is called as a result of a message on this application queue... The message
//  was placed there by IO threads performing a packet write through this document.
//  This function performs any processing necessary when new packets come in beyond
//  writing them to the database and updating this doc's packet array.
//  The idea is that the main window thread takes care of this processing because of
//  PreTranslateMessage... then we'll tell all the UI type CFrameContextListeners to
//  deal with the addition as well.

void VTSDoc::ProcessPacketStoreChange( void )
{
	// This was called because the packets in the store have changed.  See if we need to 
	// read the rest of these things...

	VTSPacketPtr ppkt = NULL;
	int nNewIndex;

	// update stats for the unread packets

	// See if this update operation (catching up the views from their current packet size to 
	// the new packet size) will take a long time...  If so (arbitrary > 200), then 
	// invoke the progress bar in the status bar

	int nPacketCountBehind = GetPacketCount() - m_nPacketCount;
	CProgressCtrl * pprogress = nPacketCountBehind > 200 ? ((CMainFrame *) AfxGetApp()->m_pMainWnd)->InitializeProgress() : NULL;

	if ( pprogress != NULL )
		pprogress->SetRange(0, nPacketCountBehind);

	for( nNewIndex = m_nPacketCount;  nNewIndex < GetPacketCount() && (ppkt = GetPacket(nNewIndex)) != NULL; nNewIndex++ )
	{
		if ( pprogress != NULL )
			pprogress->StepIt();

		BACnetPIInfo	summary( true, false );
		summary.Interpret( (BACnetPIInfo::ProtocolType) ppkt->packetHdr.packetProtocolID, (char *) ppkt->packetData, ppkt->packetLen );

		m_pStatisticsCollector->Update(summary.summaryLine, ppkt->packetLen, ppkt->packetHdr.packetType, ppkt->packetHdr.packetProtocolID );

		if ( m_bStatisticsDlgInUse )
			m_pStatitiscsDlg->Update(summary.summaryLine);
	}

	//OK... this doc has finished processing... now tell listening frame contexts to process as well
	SetPacketCount( nNewIndex );		// index now represents count

	if ( pprogress != NULL )
		((CMainFrame *) AfxGetApp()->m_pMainWnd)->ReleaseProgress();
}


//This is called after all objects have loaded...  We'll fixup stored names with active pointers

void VTSDoc::FixupNameToPortLinks( bool fCheckForExistingLink /* = true */ )
{
	// resolve port links in name list
	int i;
	for ( i = 0; i < m_names.GetSize(); i++ )
	{
		// May have already been munged, possibly by TD redefine
		// pportLink will be NULL if fixup needed, so check name for value

		if ( !fCheckForExistingLink || m_names[i]->m_pportLink == NULL )
			m_names[i]->m_pportLink = m_ports.Find(m_names[i]->GetPortName());
	}
}


void VTSDoc::FixupPortToDeviceLinks( bool fCheckForExistingLink /* = true */ )
{
	for ( int i = 0; i < m_ports.GetSize(); i++ )
	{
		if ( !fCheckForExistingLink || m_ports[i]->m_pdevice == NULL )
			m_ports[i]->m_pdevice = m_devices.Find(m_ports[i]->GetDeviceName());
	}
}


void VTSDoc::FixupFiltersToPortLinks( bool fCheckForExistingLink /* = true */ )
{
	// resolve port links in name list
	int i;

	for ( i = 0; i < m_captureFilters.GetSize(); i++ )
	{
		if ( !fCheckForExistingLink || m_captureFilters[i]->m_pportLink == NULL )
			m_captureFilters[i]->m_pportLink = m_ports.Find(m_captureFilters[i]->GetPortName());
	}

	for ( i = 0; i < m_displayFilters.GetSize(); i++ )
	{
		if ( !fCheckForExistingLink || m_displayFilters[i]->m_pportLink == NULL )
			m_displayFilters[i]->m_pportLink = m_ports.Find(m_displayFilters[i]->GetPortName());
	}
}


void VTSDoc::UnbindPortsToDevices()
{
	for ( int i = 0; i < m_ports.GetSize(); i++ )
		m_ports[i]->UnbindDevice();
}


void VTSDoc::BindPortsToDevices()
{
	for ( int i = 0; i < m_ports.GetSize(); i++ )
		m_ports[i]->BindDevice();
}


bool VTSDoc::LoadNamedAddress( BACnetAddress * pbacnetaddr, LPCSTR lpszNameLookup )
{
	int i = m_names.FindIndex(lpszNameLookup);

	if ( i == -1 )
		return false;

	*pbacnetaddr = ((VTSName *) m_names[i])->m_bacnetaddr;
	return true;
}


//
//	VTSDoc::NewPacketCount
//
//	This function is called when an IO thread has told the main application that 
//	there are new packets in a database.  The VTSApp::PreTranslateMessage function 
//	calls this function, which gets the count out of the database and then tells 
//	all of the frame contexts.
//

/* MAD_DB
void VTSDoc::NewPacketCount( void )
{

	int NewPacket=m_pDB->GetPacketCount()-m_PacketCount;

	// Edited By Hu Meng 
	// Notice: I made some changes here.
	// Sometimes when this function is called, the packet has not been written into the
	// vdb file. So the NewPacket will be 0,and I will ignore it. 
	// Otherwise the last packet  in the db file will be counted in the statistics collector 
	// for two times
	// The new coming packet will be counted in the statistics collector the next time this
	// function is called.

	if (NewPacket>0)
	{
		SetPacketCount( m_pDB->GetPacketCount() );
		
		for(int i=1;i<=NewPacket;i++)
		{
			VTSPacket	pkt;
			m_pDB->ReadPacket(m_PacketCount-i,pkt);

			BACnetPIInfo	summary( true, false );
			summary.Interpret( (BACnetPIInfo::ProtocolType)pkt.packetHdr.packetProtocolID
				, (char *)pkt.packetData
				, pkt.packetLen);

			m_pStatisticsCollector->Update(summary.summaryLine,pkt.packetLen,pkt.packetHdr.packetType,pkt.packetHdr.packetProtocolID);
			//gStatisticsCollector->Update(summary.summaryLine,pkt.packetLen,pkt.packetHdr.packetType,pkt.packetHdr.packetProtocolID);
			if(m_bStatisticsDlgInUse)
			{
				m_pStatitiscsDlg->Update(summary.summaryLine);
			}
		}
		
	}
}
*/


//LPCSTR VTSDoc::AddrToName( const BACnetAddress &addr, objId portID /* = 0 */ )
LPCSTR VTSDoc::AddrToName( const BACnetAddress &addr, const char * pszPortName /* = NULL */ )
{
	return m_names.AddrToName(addr, m_ports.Find(pszPortName));
}


void VTSDoc::DefineTD( VTSPort * pport, const BACnetOctet *addr, int len )
{
	m_names.InitializeTD(pport, addr, len);
}


//
//	VTSDoc::OnViewStatistics
//

void VTSDoc::OnViewStatistics() 
{
	m_bStatisticsDlgInUse=true;
	VTSStatisticsDlg dlg;
	m_pStatitiscsDlg=&dlg;
	m_pStatitiscsDlg->m_pDoc=this;
	dlg.DoModal();
	m_bStatisticsDlgInUse=false;
}

/////////////////////////////////////////////////////////////////////////////
// VTSDocList

// MAD_DB No longer needed
/*
VTSDocList	gDocList;

//
//	VTSDocList::VTSDocList
//

VTSDocList::VTSDocList( void )
{
}

//
//	VTSDocList::~VTSDocList
//

VTSDocList::~VTSDocList( void )
{
}

//
//	VTSDocList::Append
//

void VTSDocList::Append( VTSDocPtr vdp )
{
	// add to the end of the list
	AddTail( vdp );
}

//
//	VTSDocList::Remove
//
//	This simply finds the document pointer in the list and removes it.  Note that
//	if the document creation or open was canceled this member function is still 
//	called by the framework, so pos could be NULL and it is not an error.
//

void VTSDocList::Remove( VTSDocPtr vdp )
{
	POSITION	pos = Find( vdp )
	;

	if (pos != NULL)
		RemoveAt( pos );
}

//
//	VTSDocList::Length
//
//	Return the number of open documents.
//

int VTSDocList::Length( void )
{
	return CList<VTSDocPtr,VTSDocPtr>::GetCount();
}

//
//	VTSDocList::Child
//
//	Return a pointer to a specific document.  This is used to build a list of the 
//	open documents so the user can select one for test results.
//

VTSDocPtr VTSDocList::Child( int indx )
{
	POSITION	pos = FindIndex( indx )
	;

	if (pos != NULL )
		return (VTSDocPtr)GetAt( pos );
	else
		return 0;
}
*/

/////////////////////////////////////////////////////////////////////////////
// VTSPort

//MAD_DB VTSPortList gMasterPortList;


const char * VTSPort::m_pszPortTypes[] = { "Null", "IP", "Ethernet", "ARCNET", "MS/TP", "PTP" };

//
//	VTSPort::VTSPort
//

//MAD_DB VTSPort::VTSPort( VTSDocPtr dp, objId id )
//MAD_DB	: portDoc(dp), portSendGroup(0), portDescID(id)
VTSPort::VTSPort(void)
	: portSendGroup(0)
//	, portStatus(1), portStatusDesc(0)
//	, portEndpoint(0), portFilter(0)
//	, portBTR(0), portBBMD(0), portBIPSimple(0), portBIPForeign(0), portBindPoint(0)
//	, portDevice(0)
{
	portDoc = (VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace();
	m_fDirty = true;

	portStatus = 0;
	portStatusDesc = "New";
	portEndpoint = NULL;
	portFilter = NULL;
	portBTR = NULL;
	portBBMD = NULL;
	portBIPSimple = NULL;
	portBIPForeign = NULL;
	portBindPoint = NULL;
	m_pdevice = NULL;

	// add this to the master list
//MAD_DB	AddToMasterList();

	m_nPortType = nullPort;
	m_fEnabled = FALSE;
	m_strConfigParms = _T("");
	m_strName = _T("Untitled");
	m_nNet = -1;

	// don't worry about device... it will reattach bound by name at load and save
	// not new...

	// most ports are bound to a document and have a descriptor
//MAD_DB	if (dp && id) {
		// read in the descriptor
//MAD_DB		ReadDesc();

		// see if it can be turned on
		//Refresh();

		// madanner 3/03
		// shouldn't call Refresh here... it throws and that aborts the new assignment, even though
		// the object was actually created, which results in no reference and no destroy.
		// Call refresh from caller
//MAD_DB	}
}

//
//	VTSPort::~VTSPort
//

VTSPort::~VTSPort( void )
{
	// remove this from the master list
//MAD_DB	RemoveFromMasterList();

	Deactivate();
}


void VTSPort::Deactivate(void)
{
	UnbindDevice();
//	if (m_pdevice)
//		m_pdevice->Unbind( this );

	// if there is an endpoint, delete it
	if (portEndpoint)
		delete portEndpoint;

	// if there is a filter, delete it
	if (portFilter)
		delete portFilter;

	// if there is a BTR, delete it
	if (portBTR)
		delete portBTR;

	// if there is a BBMD, delete it
	if (portBBMD)
		delete portBBMD;

	// if there is a simple BIP object, delete it
	if (portBIPSimple)
		delete portBIPSimple;

	// if there is a foreign BIP object, delete it
	if (portBIPForeign)
		delete portBIPForeign;

//	m_pdevice = NULL;
	portEndpoint = NULL;
	portFilter = NULL;
	portBTR = NULL;
	portBBMD = NULL;
	portBIPSimple = NULL;
	portBIPForeign = NULL;
}


//
//	VTSPort::ReadDesc
//

/* MAD_DB
void VTSPort::ReadDesc( void )
{
	int		stat
	;

	// ask the database for the descriptor
	stat = portDoc->m_pDB->pObjMgr->ReadObject( portDescID, &portDesc );

	// make sure it's the correct version
	if (portDesc.objSize != kVTSPortDescSize) {
		// initialize the new variables
		portDesc.portNet = -1;
		portDesc.portDeviceObjID = 0;

		// update the size
		portDesc.objSize = kVTSPortDescSize;

		// write it back
		stat = portDoc->m_pDB->pObjMgr->WriteObject( portDescID, &portDesc );
	}
}

//
//	VTSPort::WriteDesc
//

void VTSPort::WriteDesc( void )
{
	int		stat
	;

	// save the descriptor in the database
	stat = portDoc->m_pDB->pObjMgr->WriteObject( portDescID, &portDesc );

	// if there's a filter, pass along the name
	if (portFilter)
		strcpy( portFilter->filterName, portDesc.portName );
}

//
//	VTSPort::AddToMasterList
//

void VTSPort::AddToMasterList( void )
{
	// add this to the end, regardless of its status
	gMasterPortList.AddTail( this );
}

//
//	VTSPort::RemoveFromMasterList
//

void VTSPort::RemoveFromMasterList( void )
{
	POSITION pos = gMasterPortList.Find( this );

//MAD_DB	ASSERT( pos != NULL );

	if ( pos != NULL )
		gMasterPortList.RemoveAt( pos );
}
*/

//
//	VTSPort::Refresh
//

void VTSPort::Refresh( void )
{
	// unbind from the device, if bound
	UnbindDevice();

//	if ( m_pdevice ) {
//		m_pdevice->Unbind( this );
//		m_pdevice = 0;
//	}

	// shutdown the existing endpoint, if any
	if (portEndpoint) {
		// unbind from the filter
		Unbind( portFilter, portEndpoint );

		// delete the port object
		delete portEndpoint;

		portStatus = 3;
		portStatusDesc = "Port shut down";
		portEndpoint = 0;
	}

	// if there is a filter, delete it
	if (portFilter) {
		delete portFilter;
		portFilter = 0;
	}

	// if there is a BTR, delete it
	if (portBTR) {
		delete portBTR;
		portBTR = 0;
	}

	// if there is a BBMD, delete it
	if (portBBMD) {
		delete portBBMD;
		portBBMD = 0;
	}

	// if there is a simple BIP object, delete it
	if (portBIPSimple) {
		delete portBIPSimple;
		portBIPSimple = 0;
	}

	// if there is a foreign BIP object, delete it
	if (portBIPForeign) {
		delete portBIPForeign;
		portBIPForeign = 0;
	}

	// see if the port should be enabled
//	if (!portDesc.portEnabled) {
	if ( !m_fEnabled ) {
		portStatus = 0;
		portStatusDesc = "Disabled";
		return;
	}

	// null ports cannot be enabled
//	if (portDesc.portType == nullPort) {
	if ( m_nPortType == nullPort) {
		portStatus = 3;
		portStatusDesc = "Null ports cannot be enabled";
		return;
	}

	// see if a port is already active that matches this configuration
//	for (POSITION pos = gMasterPortList.GetHeadPosition(); pos; ) {
//		VTSPortPtr	cur = gMasterPortList.GetNext( pos );
	for ( int i = 0; portDoc != NULL && i < portDoc->GetPorts()->GetSize(); i++ )
	{
		VTSPortPtr cur = (*portDoc->GetPorts())[i];

		// skip this port if it's not loaded
		if ((cur->portStatus != 1) || (!cur->portEndpoint))
			continue;

		// skip if port types don't match
//MAD_DB if (cur->portDesc.portType != portDesc.portType)
		if (cur->m_nPortType != m_nPortType )
			continue;
		
		// if config strings match, flag this one
//MAD_DB		if (strcmp(portDesc.portConfig,cur->portDesc.portConfig) == 0) {
		if ( cur->m_strConfigParms.CompareNoCase(m_strConfigParms) == 0 ) {
			portStatus = 2;
			portStatusDesc = "Existing port with this configuration";
			return;
		}
	}

	// port should be one of those listed below
	portStatus = 3;
	portStatusDesc = "Unknown port type";

	// enable the port
//	switch (portDesc.portType) {
	switch (m_nPortType) {
		case ipPort: {
				VTSWinIPPortPtr		pp
				;

				portStatus = 1;
				portStatusDesc = "IP started";
				portEndpoint = pp = new VTSWinIPPort( this );
				
				// define the TD name
//				portDoc->m_Names.DefineTD( portDescID, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
				portDoc->DefineTD( this, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
				break;
			}

		case ethernetPort: {
				VTSWinWinPcapPortPtr	pp
				;
				
				portStatus = 1;
				portStatusDesc = "Ethernet started";
//				portEndpoint = pp = new VTSWinPacket32Port( this );
				portEndpoint = pp = new VTSWinWinPcapPort( this );
				
				// define the TD name
//				portDoc->m_Names.DefineTD( portDescID, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
				portDoc->DefineTD( this, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
				break;
			}

		case arcnetPort: {
//				VTSWinPacket32PortPtr	pp;
				
//				portStatus = 2;		// good luck
				portStatusDesc = "ARCNET unsupported";
//				portEndpoint = pp = new VTSWinPacket32Port( this );
				
				// define the TD name
//MAD_DB			portDoc->m_Names.DefineTD( portDescID, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
//				portDoc->DefineTD( this, pp->portLocalAddr.addrAddr, pp->portLocalAddr.addrLen );
				break;
			}		

		case ptpPort: {
			VTSWinPTPPortPtr pp;
			portStatus = 1;		// supported by Xiao Shiyuan 2002-4-22
			portStatusDesc = "PTP started";
			portEndpoint = pp = new VTSWinPTPPort( this );
			break;
		}
	}

	// see if a filter should be set up
	if (portEndpoint) {
		// create a script filter and in the darkness bind them
//MAD_DB		portFilter = new ScriptNetFilter( portDesc.portName );
		portFilter = new ScriptNetFilter( GetName() );
		Bind( portFilter, portEndpoint );

		// default bind point is the filter
		portBindPoint = portFilter;

		// if this is an IP port, we have more work to do
//		if (portDesc.portType == ipPort) {
		if ( m_nPortType == ipPort ) {
			int		argc = 0;
			char	config[kVTSPortConfigLength], *src,	*argv[16];

			// copy the configuration, split it up
//MAD_DB			strcpy( config, portDesc.portConfig );
			strcpy( config, m_strConfigParms );
			for (src = config; *src; ) {
				argv[argc++] = src;
				while (*src && (*src != ','))
					src++;
				if (*src == ',')
					*src++ = 0;
			}
			for (int i = argc; i < 16; i++)
				argv[i] = src;

			if (argc == 1) {
			} else {
				int option = atoi( argv[1] );
				switch (option) {
					case 0:
						break;
					case 1:
						portBTR = new BACnetBTR();
						Bind( portBTR, portFilter );
						portBindPoint = portBTR;
						break;
					case 2:
						portBBMD = new BACnetBBMD( portEndpoint->portLocalAddr );
						Bind( portBBMD, portFilter );
						portBindPoint = portBBMD;
						break;
					case 3:
						portBIPSimple = new BACnetBIPSimple();
						Bind( portBIPSimple, portFilter );
						portBindPoint = portBIPSimple;
						break;
					case 4: {
							unsigned long	host
							;
							unsigned short	port
							;
							int				ttl
							;

							portBIPForeign = new BACnetBIPForeign();
							Bind( portBIPForeign, portFilter );
							portBindPoint = portBIPForeign;

							BACnetIPAddr::StringToHostPort( argv[2], &host, 0, &port );
							ttl = atoi( argv[3] );
							portBIPForeign->Register( host, port, ttl );
							break;
						}
				}
			}
		}
	}

	// see if we can bind to a device

	BindDevice();
	SetDirty(false);

/*MAD_DB
	if (portDesc.portDeviceObjID != 0) {
		portDevice = portDoc->m_Devices.FindDevice( portDesc.portDeviceObjID );
		if (portDevice)
			portDevice->Bind( this, portDesc.portNet );
	}
*/
}

void VTSPort::BindDevice()
{
	if ( m_pdevice != NULL )
		m_pdevice->Bind(this, m_nNet);
}


void VTSPort::UnbindDevice()
{
	if ( m_pdevice != NULL )			// already fixed up but not activated yet
		m_pdevice->Unbind(this);
}


//
//	VTSPort::SendData
//
//	This is a common passthru function that allows the application to point to 
//	a VTSPort and gives it access to the SendData function provided by the 
//	endpoint (if one has been successfully established).
//

void VTSPort::SendData( BACnetOctet *data, int len )
{
	if (portEndpoint)
		portEndpoint->SendData( data, len );
}



const VTSPort& VTSPort::operator=(const VTSPort& rportSrc)
{
	// Set dirty to false... this will only be the case if we're copying from another port
	SetDirty(false);

	portDoc = rportSrc.portDoc;
	portStatus = rportSrc.portStatus;
	portStatusDesc = rportSrc.portStatusDesc;

	// don't copy the owned devices
	portEndpoint = NULL;			// we'll refresh all this stuff later
	portFilter = NULL;
	portBTR = NULL;
	portBBMD = NULL;
	portBIPSimple = NULL;
	portBIPForeign = NULL;
	portBindPoint = NULL;

	m_pdevice = rportSrc.m_pdevice;

	m_nPortType = rportSrc.m_nPortType;
	m_fEnabled = rportSrc.m_fEnabled;
	m_strConfigParms = rportSrc.m_strConfigParms;
	m_strName = rportSrc.m_strName;
	m_nNet = rportSrc.m_nNet;
	
	return *this;
}



void VTSPort::SetName( LPCSTR lpszName )
{
	if ( m_strName.Compare(lpszName) != 0 )
	{
		SetDirty();
		m_strName = lpszName;
	}
}


void VTSPort::SetEnabled( BOOL fEnabled /* = true */ )
{
	if ( m_fEnabled != fEnabled)
	{
		SetDirty();
		m_fEnabled = fEnabled;
	}
}


void VTSPort::SetConfig( LPCSTR lpszConfig )
{
	if ( m_strConfigParms.CompareNoCase(lpszConfig) != 0 )
	{
		SetDirty();
		m_strConfigParms = lpszConfig;
	}
}


void VTSPort::SetPortType( VTSPortType nType )
{
	if ( m_nPortType != nType )
	{
		SetDirty();
		m_nPortType = nType;
	}
}


void VTSPort::SetNetwork( int nNetwork )
{
	if ( m_nNet != nNetwork )
	{
		SetDirty();
		m_nNet = nNetwork;
	}
}


void VTSPort::SetDevice( VTSDevice * pdevice )
{
	if ( m_pdevice != pdevice )
	{
		SetDirty();
		m_pdevice = pdevice;
	}
}


void VTSPort::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_strName;
		ar << m_nPortType;
		ar << m_fEnabled;
		ar << m_nNet;

		if ( m_pdevice == NULL )
			m_strDevice.Empty();
		else
			m_strDevice = m_pdevice->GetName();

		ar << m_strDevice;
		ar << m_strConfigParms;
	}
	else
	{
		ar >> m_strName;
		ar >> (int &) m_nPortType;
		ar >> m_fEnabled;
		ar >> m_nNet;

		// just unlink (or leave unlinked) the device here... since we're loading the fixups will be done later
		m_pdevice = NULL;
		ar >> m_strDevice;
		ar >> m_strConfigParms;
	}

	SetDirty(false);
}

IMPLEMENT_SERIAL(VTSPort, CObject, 1);

IMPLEMENT_VTSPTRARRAY(VTSPorts, VTSPort);


/////////////////////////////////////////////////////////////////////////////
// VTSPortList

/*MAD_DB PortList not needed anymore

//
//	VTSPortList::VTSPortList
//
//	This empty constructor is used to build the master port list.  At the time 
//	the master list is created there is no database to read configurations from 
//	and no INI file.
//

VTSPortList::VTSPortList( void )
	: m_pDoc(0)
{
}

//
//	VTSPortList::~VTSPortList
//

VTSPortList::~VTSPortList( void )
{
}

//
//	VTSPortList::Load
//

void VTSPortList::Load( VTSDocPtr docp )
{
	int			stat
	;
	objId		curID
	;
	JDBListPtr	plist
	;
	VTSPortPtr	cur
	;

	// keep track of the database
	m_pDoc = docp;

	// get a pointer to the list
	plist = &m_pDoc->m_pDB->dbPortList;

	// intialize the port list
	for (int i = 0; i < plist->Length(); i++) {
		// get the port descriptor ID's
		stat = plist->ReadElem( i, &curID );
		ASSERT( stat == 0 );

		// create a new port object to match
		cur = new VTSPort( m_pDoc, curID );

		try
		{
			if ( cur )
				cur->Refresh();

			// add it to our list of ports
			AddTail( cur );
		}
		catch(...)
		{
			// Who knows what nasty throws this gives off... it does too much to tell.
			// Just clean up the memory and move on...
			if ( cur )
				delete cur;
			throw;
		}
	}
}

//
//	VTSPortList::Unload
//

void VTSPortList::Unload( void )
{
	for (POSITION pos = GetHeadPosition(); pos; )
		delete GetNext( pos );
}

//
//	VTSPortList::Add
//
//	This function is called when the user has requested a new port in the VTSPortDlg
//	dialog.  It defaults to an untitled port.  The port descriptor must be created 
//	in the database before the VTSPort object can be created (its ID is in the ctor).
//

void VTSPortList::Add( void )
{
	int				stat, elemLoc
	;
	objId			newDescID
	;
	VTSPortPtr		cur
	;
	VTSPortDesc		newDesc
	;

	// create a new descriptor
	stat = m_pDoc->m_pDB->pObjMgr->NewObject( &newDescID, &newDesc, kVTSPortDescSize );

	// initalize it
	newDesc.portType = nullPort;
	newDesc.portEnabled = 0;
	newDesc.portConfig[0] = 0;
	strcpy( newDesc.portName, "Untitled" );
	newDesc.portNet = -1;			// no network, or local
	newDesc.portDeviceObjID = 0;	// no bound device/router object

	// save it
	stat = m_pDoc->m_pDB->pObjMgr->WriteObject( newDescID, &newDesc );
	ASSERT( stat == 0 );

	// add the object ID on the end of the port list
	elemLoc = 0x7FFFFFFF;
	stat = m_pDoc->m_pDB->dbPortList.NewElem( &elemLoc, &newDescID );
	ASSERT( stat == 0 );

	// create a new port object to match
	cur = new VTSPort( m_pDoc, newDescID );

	try
	{
		if ( cur )
			cur->Refresh();

		// add it to our list of ports
		AddTail( cur );
	}
	catch(...)
	{
		// Who knows what nasty throws this gives off... it does too much to tell.
		// Just clean up the memory and move on...
		if ( cur )
			delete cur;
		throw;
	}
}

//
//	VTSPortList::Remove
//
//	This function is currently not called.  If a port is deleted there must be some 
//	way of telling all of the packets and names that the port they used to refer to 
//	is gone.  This is not overly complicated, but I'm putting it off for now.  Note 
//	that there is no delete button in the port dialog box!
//

void VTSPortList::Remove( int i )
{
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	delete GetAt( pos );
	RemoveAt( pos );
}

//
//	VTSPortList::FindPort
//
//	This function is called by the scripting engine to find a port with a given 
//	name as described in the packet description in the script.  Used for sending 
//	low level messages directly to a port.
//

VTSPortPtr VTSPortList::FindPort( const char *name )
{
	for (POSITION pos = GetHeadPosition(); pos; ) {
		VTSPortPtr cur = (VTSPortPtr)GetNext( pos );

		if (strcmp(name,cur->portDesc.portName) == 0)
			return cur;
	}

	// failed to find it
	return 0;
}


VTSPortPtr VTSPortList::FindPort( objId portID )
{
	if ( portID == 0 )		// MAD_DB
		return NULL;

	for (POSITION pos = GetHeadPosition(); pos; )
	{
		VTSPortPtr cur = (VTSPortPtr)GetNext( pos );

		if ( cur->portDescID == portID )
			return cur;
	}

	// failed to find it
	return NULL;
}


//
//	VTSPortList::Length
//

int VTSPortList::Length( void )
{
	return CList<VTSPortPtr,VTSPortPtr>::GetCount();
}

//
//	VTSPortList::operator []
//
//	When the user selects a port from the list of defined ports in the VTSPortDlg, this 
//	function is called to return a pointer to the VTSPort object.  
//

VTSPortPtr VTSPortList::operator []( int i )
{
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	return GetAt( pos );
}

/////////////////////////////////////////////////////////////////////////////
// VTSNameList

//
//	VTSNameList::VTSNameList
//
//	The name list object acts as a simple interface to the JDB object functions 
//	that handle the names.  Unlike ports, where the port list contains objId's 
//	of port descriptions, the names list is actually a list of the name 
//	descriptions themselves.  This is so that name lookups are faster (the 
//	complete list is loaded into one contiguous block) and simpler.
//

VTSNameList::VTSNameList( void )
	: m_pDoc(0)
{
}

//
//	VTSNameList::~VTSNameList
//

VTSNameList::~VTSNameList( void )
{
}

//
//	VTSNameList::Load
//
//	Loading names is easy, the document has already loaded the list from the 
//	database.
//

void VTSNameList::Load( VTSDocPtr docp )
{
	m_pDoc = docp;
}

//
//	VTSNameList::Add
//

void VTSNameList::Add( void )
{
	int				stat, elemLoc
	;
	VTSNameDesc		newDesc
	;

	// initalize it
	strcpy( newDesc.nameName, "Untitled" );
	newDesc.nameAddr.addrType = nullAddr;
	newDesc.nameAddr.addrNet = 0;
	newDesc.nameAddr.addrLen = 0;
	memset( newDesc.nameAddr.addrAddr, 0, kMaxAddressLen );
	newDesc.namePort = 0;

	// add it on the end of the name list
	elemLoc = 0x7FFFFFFF;
	stat = m_pDoc->m_pDB->dbNameList.NewElem( &elemLoc, &newDesc );
}

//
//	VTSNameList::Remove
//
//	This simply deletes the name from the database list.  It could turn around and 
//	save the list, but it doesn't.  I could see about a 'Cancel' button in the Names
//	dialog, but it doesn't seem worth it right now.
//

void VTSNameList::Remove( int i )
{
	int			stat
	;

	stat = m_pDoc->m_pDB->dbNameList.DeleteElem( i );
}

//
//	VTSNameList::Length
//

int VTSNameList::Length( void )
{
	//MAD_DB
	ASSERT(0);

	return m_pDoc->m_pDB->dbNameList.Length();
}

//
//	VTSNameList::AddrToName
//
//	Given an address and a port return the first match that is found.  If there's 
//	no match, return null.  Note that this returns a pointer to an internal static
//	buffer, so be sure to make a copy of the results before calling the function 
//	again.
//

const char* VTSNameList::AddrToName( const BACnetAddress &addr, objId portID )
{
	//MAD_DB
	ASSERT(0);
	int					len = Length()
	;
	static VTSNameDesc	addrRec
	;

	for (int i = 0; i < len; i++) {
		// load the record
		ReadName( i, &addrRec );

		// if the name record requires a specific port and it's not this one, continue
		if ((addrRec.namePort != 0) && (addrRec.namePort != portID))
			continue;

		// overloaded comparison operator
		if (!(addrRec.nameAddr == addr))
			continue;

		// must be the one we're looking for
		return addrRec.nameName;
	}

	// nothing found
	return 0;
}

//
//	VTSNameList::ReadName
//

void VTSNameList::ReadName( int i, VTSNameDescPtr ndp )
{
	//MAD_DB
	ASSERT(0);

	int		stat
	;

	stat = m_pDoc->m_pDB->dbNameList.ReadElem( i, ndp );
}

//
//	VTSNameList::WriteName
//

void VTSNameList::WriteName( int i, VTSNameDescPtr ndp )
{
	//MAD_DB
	ASSERT(0);

	int		stat
	;

	stat = m_pDoc->m_pDB->dbNameList.WriteElem( i, ndp );
}

//
//	VTSNameList::DefineTD
//

void VTSNameList::DefineTD( objId port, const BACnetOctet *addr, int len )
{
	int				stat, elemLoc
	;

	// define the name
	strcpy( searchName.nameName, "TD" );
	searchName.namePort = port;
	searchName.nameAddr.addrType = localStationAddr;
	searchName.nameAddr.addrNet = 0;
	memcpy( searchName.nameAddr.addrAddr, addr, len );
	searchName.nameAddr.addrLen = len;

	// look for an existing desc
	elemLoc = m_pDoc->m_pDB->dbNameList.FindElem( 0, (JDBListCompFnPtr)TDSearch );

	// if no match was found, add a new name
	if (elemLoc == -1) {
		// add it on the end of the name list
		elemLoc = 0x7FFFFFFF;
		stat = m_pDoc->m_pDB->dbNameList.NewElem( &elemLoc, &searchName );
	} else
		// save new version
		stat = m_pDoc->m_pDB->dbNameList.WriteElem( elemLoc, &searchName );
}

//
//	VTSNameList::FindTD
//

const BACnetAddress *VTSNameList::FindTD( objId port )
{
	//MAD_DB
	ASSERT(0);
	int		stat, elemLoc
	;

	// set up for the search
	searchName.namePort = port;

	// look for a desc
	elemLoc = m_pDoc->m_pDB->dbNameList.FindElem( 0, (JDBListCompFnPtr)TDSearch );

	// if not found, bail out
	if (elemLoc == -1)
		return 0;

	// read the description
	stat = m_pDoc->m_pDB->dbNameList.ReadElem( elemLoc, &searchName );

	// return a pointer to just the address portion
	return &searchName.nameAddr;
}

//
//	VTSNameList::TDSearch
//
//	You would expect the matching function to return non-zero for a successful 
//	match and zero to mean different.  In this case I wanted the return value to 
//	be the same as what memcmp() would return.
//

VTSNameDesc VTSNameList::searchName;

int VTSNameList::TDSearch( const VTSNameDescPtr, const VTSNameDescPtr ndp )
{
	//MAD_DB
	ASSERT(0);
	if ((ndp->namePort == searchName.namePort) && (strcmp(ndp->nameName,"TD") == 0))
		return 0;
	else
		return 1;
}
*/


IMPLEMENT_SERIAL(VTSName, CObject, 1);

VTSName::VTSName( void )
{
	m_strName = "Untitled";

	m_bacnetaddr.addrType = nullAddr;
	m_bacnetaddr.addrNet = 0;
	m_bacnetaddr.addrLen = 0;
	memset( m_bacnetaddr.addrAddr, 0, kMaxAddressLen );

	m_pportLink = NULL;
}


VTSName::VTSName( LPCSTR pszname )
		:m_strName(pszname)
{
	m_bacnetaddr.addrType = nullAddr;
	m_bacnetaddr.addrNet = 0;
	m_bacnetaddr.addrLen = 0;
	memset( m_bacnetaddr.addrAddr, 0, kMaxAddressLen );

	m_pportLink = NULL;
}


VTSName::~VTSName( void )
{
}


const VTSName& VTSName::operator=(const VTSName& rnameSrc)
{
	m_strName = rnameSrc.m_strName;
	memcpy( &m_bacnetaddr, &rnameSrc.m_bacnetaddr, sizeof(m_bacnetaddr) );
	m_pportLink = rnameSrc.m_pportLink;
	
	return *this;
}


void VTSName::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// Switch on schema for differnent implementations during load
		ar << m_strName;
		try
		{
			ar.Write(&m_bacnetaddr, sizeof(m_bacnetaddr));
		}
		catch( CFileException e )
		{
		}

		if ( m_pportLink != NULL )
			m_strPortNameTemp = m_pportLink->GetName();

		ar << m_strPortNameTemp;
	}
	else
	{
		ar >> m_strName;
		ar.Read(&m_bacnetaddr, sizeof(m_bacnetaddr));
		ar >> m_strPortNameTemp;

		// This will be fixed up later...
		m_pportLink = NULL;
	}
}



bool VTSName::IsAddressMatch( const BACnetAddress &addr, VTSPort * pport )
{
	return (m_pportLink ==  NULL || m_pportLink == pport) &&  m_bacnetaddr == addr;
}



/////////////////////////////////////////////////////////////////////////////
// VTSNames


IMPLEMENT_VTSPTRARRAY(VTSNames, VTSName);


void VTSNames::Remove( int i )
{
	ASSERT(i >= 0 && i < GetSize());

	if ( i >= 0 && i < GetSize() )
	{
		delete (VTSName *) GetAt(i);
		RemoveAt(i);
	}
}



LPCSTR VTSNames::AddrToName( const BACnetAddress &addr, VTSPort * pport )
{
	for (int i = 0; i < GetSize(); i++)
		if ( ((VTSName *) GetAt(i))->IsAddressMatch(addr, pport) )
			return ((VTSName *) GetAt(i))->m_strName;

	return NULL;
}


int VTSNames::FindIndex( LPCSTR lpszName )
{
	for ( int i = 0; i < GetSize(); i++ )
		if ( ((VTSName *) GetAt(i))->m_strName.CompareNoCase(lpszName) == 0 )
			return i;
		
	return -1;
}


void VTSNames::InitializeTD( VTSPort * pport, const BACnetOctet *addr, int len )
{
	VTSName * pnameNew = new VTSName("TD");

	if ( pnameNew == NULL )
		return;

	pnameNew->m_pportLink = pport;
	pnameNew->m_bacnetaddr.addrType = localStationAddr;
	memcpy( pnameNew->m_bacnetaddr.addrAddr, addr, len );
	pnameNew->m_bacnetaddr.addrLen = len;

	int i = FindIndex(pnameNew->m_strName);
	
	if ( i == -1 )
		Add(pnameNew);
	else
	{
		delete (VTSName *) GetAt(i);
		SetAt(i, pnameNew);
	}
}




//
//	SetLookupContext
//

namespace NetworkSniffer {

const char * g_pszPortName = NULL;

// MAD_DB JDBListPtr	gNameListSearchList;
// MAD_DB VTSNameDesc	gNameListSearchName;

void SetLookupContext( const char * pszPortName )
{
	// Sadly, this is a two part call.  This function is called to set the name of the port 
	// that received a packet that is about to be picked apart by the sniffer window... it will
	// call the LookupName many time.  Why not just change the sniffer window?  Dunno.

	g_pszPortName = pszPortName;
}

//void SetLookupContext( objId port, JDBListPtr list )
//{
//	gNameListSearchName.namePort = port;
//	gNameListSearchList = list;
//}

//
//	LookupName
//
//	This function is called by the interpreters to lookup an address and see if it
//	can be translated into a name.  It is assumed that the global list and port 
//	will be set correctly before the interpreter is called.
//

const char* LookupName( int net, const BACnetOctet *addr, int len )
{
	const char * pszName = NULL;

	BACnetAddress	bacnetAddrSearch(net, addr, len);

	if ( net == 65535 )
		bacnetAddrSearch.addrType = globalBroadcastAddr;

	VTSDoc * pdoc = (VTSDoc * ) ((VTSApp *) AfxGetApp())->GetWorkspace();
	if ( pdoc != NULL )
		pszName = pdoc->AddrToName(bacnetAddrSearch, g_pszPortName);

	return pszName;
}

/* MAD_DB
const char* LookupName( int net, const BACnetOctet *addr, int len )
{
	int					stat, elemLoc;
	static VTSNameDesc	searchRslts;

	// set up the search block
	gNameListSearchName.nameAddr.addrNet = net;
	memcpy( gNameListSearchName.nameAddr.addrAddr, addr, len );
	gNameListSearchName.nameAddr.addrLen = len;

	// look for a desc
	elemLoc = gNameListSearchList->FindElem( 0, (JDBListCompFnPtr)NameSearch );

	// if not found, bail out
	if (elemLoc == -1)
		return 0;

	// read the description
	stat = gNameListSearchList->ReadElem( elemLoc, &searchRslts );

	// return a pointer to just the name portion
	return searchRslts.nameName;
}

//
//	NameSearch
//

int NameSearch( const VTSNameDescPtr, const VTSNameDescPtr ndp )
{
	// see if the port is a match
	if ((ndp->namePort != 0) && (gNameListSearchName.namePort != ndp->namePort))
		return 1;

	// if a network was specified and its 65535, match for a global broadcast
	if (gNameListSearchName.nameAddr.addrNet == 65535)
		if (ndp->nameAddr.addrType == globalBroadcastAddr)
			return 0;
		else
			return 1;

	// network numbers must match (0 implies local station)
	if (gNameListSearchName.nameAddr.addrNet != ndp->nameAddr.addrNet)
		return 1;

	// address length must match (zero for broadcast)
	if (gNameListSearchName.nameAddr.addrLen != ndp->nameAddr.addrLen)
		return 1;

	// address must match
	return memcmp( gNameListSearchName.nameAddr.addrAddr, ndp->nameAddr.addrAddr, ndp->nameAddr.addrLen );
}
*/

}

/////////////////////////////////////////////////////////////////////////////
// VTSFilter

IMPLEMENT_SERIAL(VTSFilter, CObject, 1);

VTSFilter::VTSFilter( void )
{
	m_type = 0;
	m_addr = 0;
	m_addrType = 0;

	m_filteraddr.addrType = nullAddr;
	m_filteraddr.addrNet = 0;
	m_filteraddr.addrLen = 0;
	memset( m_filteraddr.addrAddr, 0, kMaxAddressLen );

	m_fnGroup = 0;

	m_pportLink = NULL;
}

VTSFilter::~VTSFilter( void )
{
}

const VTSFilter& VTSFilter::operator=(const VTSFilter& rfilterSrc)
{
	m_type = rfilterSrc.m_type;
	m_addr = rfilterSrc.m_addr;
	m_addrType = rfilterSrc.m_addrType;
	memcpy( &m_filteraddr, &rfilterSrc.m_filteraddr, sizeof(m_filteraddr) );
	m_fnGroup = rfilterSrc.m_fnGroup;

	m_pportLink = rfilterSrc.m_pportLink;

	return *this;
}

void VTSFilter::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_type;

		if ( m_pportLink != NULL )
			m_strPortNameTemp = m_pportLink->GetName();

		ar << m_strPortNameTemp;

		ar << m_addr;
		ar << m_addrType;

		try
		{
			ar.Write(&m_filteraddr, sizeof(m_filteraddr));
		}
		catch( CFileException e )
		{
		}

		ar << m_fnGroup;
	}
	else
	{
		ar >> m_type;
		ar >> m_strPortNameTemp;
		ar >> m_addr;
		ar >> m_addrType;
		ar.Read(&m_filteraddr, sizeof(m_filteraddr));
		ar >> m_fnGroup;

		// This will be fixed up later...
		m_pportLink = NULL;
	}
}

//
//	VTSFilter::TestAddress
//

bool VTSFilter::TestAddress( const BACnetAddress &addr )
{
	switch (m_addrType) {
		case 0:
			return true;

		case 1:		// local station
			return ((addr == m_filteraddr) ? true : false);

		case 2:		// local network
			return (addr.addrType == localStationAddr);

		case 3:		// local broadcast
			return (addr.addrType == localBroadcastAddr);

		case 4:		// remote station
			return ((addr == m_filteraddr) ? true : false);

		case 5:		// remote network
			return ((addr.addrType == remoteStationAddr) && (addr.addrNet == m_filteraddr.addrNet));

		case 6:		// remote broadcast
			return ((addr.addrType == remoteBroadcastAddr) && (addr.addrNet == m_filteraddr.addrNet));

		case 7:		// global broadcast
			return (addr.addrType == globalBroadcastAddr);
	}

	// should never get here
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// VTSFilters

IMPLEMENT_SERIAL(VTSFilters, CPtrArray, 1);

VTSFilters::VTSFilters( void )
{
}

//
//	VTSFilters::~VTSFilters
//

VTSFilters::~VTSFilters( void )
{
	KillContents();
}

//
//	VTSFilters::KillContents
//

void VTSFilters::KillContents( void )
{
	for ( int i = 0; i < GetSize(); i++ )
		if ( GetAt(i) != NULL )
			delete GetAt(i);
	RemoveAll();
}

//
//	VTSFilters::DeepCopy
//

void VTSFilters::DeepCopy( const VTSFilters * psrc )
{
	KillContents();

	for ( int i = 0; i < psrc->GetSize(); i++ )	{
		VTSFilter * pelement = new VTSFilter();
		*pelement = *(*psrc)[i];
		Add(pelement);
	}
}

//
//	VTSFilters::Serialize
//

void VTSFilters::Serialize( CArchive& ar )
{
	if (ar.IsStoring())	{
		ar << GetSize();
		for ( int i = 0; i < GetSize(); i++ )
			GetAt(i)->Serialize(ar);
	} else {
		KillContents();

		if (ar.IsBufferEmpty())
			return;

		int iSize;
		for ( ar >> iSize; iSize > 0; iSize-- ) {
			VTSFilter * p = new VTSFilter();
			p->Serialize(ar);
			Add(p);
		}
	}
}

//
//	VTSFilters::Remove
//

void VTSFilters::Remove( int i )
{
	ASSERT(i >= 0 && i < GetSize());

	if ( i >= 0 && i < GetSize() )
	{
		delete (VTSFilter *) GetAt(i);
		RemoveAt(i);
	}
}

//
//	VTSFilters::TestPacket
//
//	Return true iff packet should be accepted.
//

bool VTSFilters::TestPacket( const VTSPacket& packet )
{
	// accept the packet if there are no filters
	if (GetSize() == 0)
		return true;

	// always accept script messages
	if (packet.packetHdr.packetType == msgData)
		return true;

	// get ready to dig into packet
	int				pktLen = packet.packetLen
	,				pktFnGroup = 0
	;
	BACnetOctet		*pktData = packet.packetData
	;
	BACnetAddress	pktSrcAddr = packet.packetHdr.packetSource
	,				pktDestAddr = packet.packetHdr.packetDestination
	;

	// find the first octet in the packet
	switch ((BACnetPIInfo::ProtocolType)packet.packetHdr.packetProtocolID) {
		case BACnetPIInfo::ipProtocol:
			// skip the fake ip header, address (4), and port (2)
			pktData += 6;
			pktLen -= 6;

			// check for a BVLL header
			if (*pktData == 0x81) {
				pktLen--,pktData++;

				// extract the function
				int bipFn = (pktLen--,*pktData++);

				// extract the length
				int len = (pktLen--,*pktData++);
				len = (len << 8) + (pktLen--,*pktData++);

				// set the function group
				switch ((BVLCFunction)bipFn) {
					case bvlcResult:
					case blvcWriteBroadcastDistributionTable:
					case blvcReadBroadcastDistributionTable:
					case blvcReadBroadcastDistributionTableAck:
					case bvlcRegisterForeignDevice:
					case bvlcReadForeignDeviceTable:
					case bvlcReadForeignDeviceTableAck:
					case bvlcDeleteForeignDeviceTableEntry:
						pktFnGroup = 1;
						break;

					case blvcForwardedNPDU:
						// extract the original source
						pktSrcAddr.addrType = localStationAddr;
						memcpy( pktSrcAddr.addrAddr, pktData, 6 );
						pktSrcAddr.addrLen = 6;

						pktData += 6;
						pktLen -= 6;
						break;

						// dig deeper into these
					case bvlcDistributeBroadcastToNetwork:
					case bvlcOriginalUnicastNPDU:
					case bvlcOriginalBroadcastNPDU:
						break;
				}
			}
			break;

		case BACnetPIInfo::ethernetProtocol:
			// skip over source (6), destination (6), length (2), and SAP (3)
			pktData += 17;
			pktLen -= 17;
			break;

		case BACnetPIInfo::arcnetProtocol:
			// skip over source (1), destination (1), SAP (3), LLC (1)
			pktData += 6;
			pktLen -= 6;
			break;

		case BACnetPIInfo::mstpProtocol:
			// skip over preamble
			pktData += 2;
			pktLen -= 2;

			// look at the frame type
			switch (pktLen--,*pktData++) {
				case 0x00:		// Token Frame
				case 0x01:		// Poll For Master Frame
				case 0x02:		// Reply To Poll For Master Frame
				case 0x03:		// Test Request Frame
				case 0x04:		// Test Response Frame
				case 0x07:		// Reply Postponed Frame
					pktFnGroup = 2;
					break;

				case 0x05:		// Data Expecting Reply Frame
				case 0x06:		// Data Not Expecting Reply Frame
					break;
			};

			// skip over destination (1), source (1), length (2) and CRC (1)
			pktData += 5;
			pktLen -= 5;
			break;

		case BACnetPIInfo::ptpProtocol:
			// ### interpreter is painful
			pktFnGroup = 3;
			break;

		default:
			return true;
	}

	// if the function group hasn't been set, dig into the NPCI header
	if (pktFnGroup == 0) {
		int					netLayerMessage, dnetPresent, snetPresent
		;

		// check the length
		if (pktLen < 2)
			return true;	// invalid length
		
		// only version 1 messages supported
		if (*pktData++ != 0x01)
			return true;	// version 1 only
		
		// extract the flags
		netLayerMessage = (*pktData & 0x80);
		dnetPresent = (*pktData & 0x20);
		snetPresent = (*pktData & 0x08);
//		expectingReply = (*pktData & 0x04);			// might be nice to check these someday
//		networkPriority = (*pktData & 0x03);		// perhaps filter all critical messages?
		pktData += 1;
		pktLen -= 1;
		
		// extract the destination address
		if (dnetPresent) {
			int		dnet, dlen
			;
			
			dnet = (pktLen--,*pktData++);
			dnet = (dnet << 8) + (pktLen--,*pktData++);
			dlen = (pktLen--,*pktData++);
			
			if (dnet == 0xFFFF)
				pktDestAddr.GlobalBroadcast();
			else {
				if (dlen == 0)
					pktDestAddr.RemoteBroadcast( dnet );
				else {
					pktDestAddr.RemoteStation( dnet, pktData, dlen );
					pktData += dlen;
				}
			}
		}
		
		// extract the source address, or copy the one from the endpoint
		if (snetPresent) {
			int		snet, slen
			;
			
			snet = (pktLen--,*pktData++);
			snet = (snet << 8) + (pktLen--,*pktData++);
			slen = (pktLen--,*pktData++);
			
			pktSrcAddr.RemoteStation( snet, pktData, slen );
			pktData += slen;
		}
		
		// skip the hop count
		if (dnetPresent)
			pktLen--, pktData++;
		
		// all done for network layer messages
		if (netLayerMessage)
			pktFnGroup = 4;
	}

	// if the function group hasn't been set, dig into the application layer
	if (pktFnGroup == 0) {
		switch ((BACnetAPDUType)((*pktData) >> 4)) {
			case confirmedRequestPDU:
				pktFnGroup = ConfirmedServiceFnGroup( (pktData[0] & 0x08) ? pktData[5] : pktData[3] );
				break;
			case unconfirmedRequestPDU:
				pktFnGroup = UnconfirmedServiceFnGroup( pktData[1] );
				break;
			case simpleAckPDU:
				pktFnGroup = ConfirmedServiceFnGroup( pktData[2] );
				break;
			case complexAckPDU:
				pktFnGroup = ConfirmedServiceFnGroup( (pktData[0] & 0x08) ? pktData[4] : pktData[2] );
				break;

			case segmentAckPDU:
				// not a part of a function group
				break;

			case errorPDU:
				pktFnGroup = ConfirmedServiceFnGroup( pktData[2] );
				break;

			case rejectPDU:
			case abortPDU:
				pktFnGroup = 10;
				break;
		}
	}


	// look through the filters
	for ( int i = 0; i < GetSize(); i++ ) {
		VTSFilter	*fp = (VTSFilter *)GetAt(i)
		;

		// check the port
		if ((strlen(fp->m_strPortNameTemp) != 0) && (strcmp(fp->m_strPortNameTemp,packet.packetHdr.m_szPortName) != 0))
			continue;

		// check address
		if (fp->m_addr != 0) {
			if (fp->m_addr == 1) {
				if (!fp->TestAddress(pktSrcAddr))	// if source doesn't match, try next filter
					continue;
			} else
			if (fp->m_addr == 2) {
				if (!fp->TestAddress(pktDestAddr))	// if destination doesn't match, try next filter
					continue;
			} else
			if (fp->m_addr == 3) {
				if (fp->TestAddress(pktSrcAddr))	// if source matches, ok
					;
				else
				if (fp->TestAddress(pktDestAddr))	// if destination matches, ok
					;
				else
					continue;						// try next filter
			} else
				;
		}

		// check function group
		if (fp->m_fnGroup && (fp->m_fnGroup != pktFnGroup))
			continue;

		// everything matched, accept or reject it
		return ((fp->m_type == 0) ? true : false);
	}

	// none of the filters matched, default is accept
	return true;
}

//
//	VTSFilters::ConfirmedServiceFnGroup
//

int VTSFilters::ConfirmedServiceFnGroup( int service )
{
	int		pktFnGroup = 0
	;

	switch ((BACnetConfirmedServiceChoice)service) {
		// Alarm and Event Services
		case acknowledgeAlarm:
		case confirmedCOVNotification:
		case confirmedEventNotification:
		case getAlarmSummary:
		case getEnrollmentSummary:
		case subscribeCOV:
			pktFnGroup = 5;
			break;

		// File Access Services
		case atomicReadFile:
		case atomicWriteFile:
			pktFnGroup = 6;
			break;

		// Object Access Services
		case addListElement:
		case removeListElement:
		case createObject:
		case deleteObject:
		case readProperty:
		case readPropertyConditional:
		case readPropertyMultiple:
		case writeProperty:
		case writePropertyMultiple:
			pktFnGroup = 7;
			break;

		// Remote Device Management Services
		case deviceCommunicationControl:
		case confirmedTextMessage:
		case reinitializeDevice:
			pktFnGroup = 8;
			break;

		// Virtual Terminal Services
		case vtOpen:
		case vtClose:
		case vtData:
			pktFnGroup = 9;
			break;

		case confirmedPrivateTransfer:
		case authenticate:
		case requestKey:
			// no function group for now
			break;
	}

	return pktFnGroup;
}

//
//	VTSFilters::UnconfirmedServiceFnGroup
//

int VTSFilters::UnconfirmedServiceFnGroup( int service )
{
	int		pktFnGroup = 0
	;

	switch ((BACnetUnconfirmedServiceChoice)service) {
		// Alarm and Event Services
		case unconfirmedCOVNotification:
		case unconfirmedEventNotification:
			pktFnGroup = 5;
			break;

		// Remote Device Management Services
		case iAm:
		case iHave:
		case unconfirmedTextMessage:
		case timeSynchronization:
		case whoHas:
		case whoIs:
			pktFnGroup = 8;
			break;

		case unconfirmedPrivateTransfer:
			// no function group for now
			break;
	}

	return pktFnGroup;
}

/////////////////////////////////////////////////////////////////////////////
// VTSClient

//
//	VTSClient::VTSClient
//

VTSClient::VTSClient( VTSDevicePtr dp )
: clientDev(dp), BACnetClient( &dp->devDevice )
{
}

//
//	VTSClient::Confirmation
//

void VTSClient::Confirmation( const BACnetAPDU &apdu )
{
	// pass it along to the executor for a match
	gExecutor.ReceiveAPDU( apdu );
}

//
//	VTSDevice::IAm
//

void VTSClient::IAm( void )
{
	BACnetUnconfirmedServiceAPDU	hereIAm( iAm )
	;

/*MAD_DB
	BACnetObjectIdentifier( 8, clientDev->devDesc.deviceInstance ).Encode( hereIAm );
	BACnetUnsigned( clientDev->devDesc.deviceMaxAPDUSize ).Encode( hereIAm );
	BACnetEnumerated( clientDev->devDesc.deviceSegmentation ).Encode( hereIAm );
	BACnetUnsigned( clientDev->devDesc.deviceVendorID ).Encode( hereIAm );
*/
	BACnetObjectIdentifier( 8, clientDev->m_nInstance ).Encode( hereIAm );
	BACnetUnsigned( clientDev->m_nMaxAPDUSize ).Encode( hereIAm );
	BACnetEnumerated( clientDev->m_segmentation ).Encode( hereIAm );
	BACnetUnsigned( clientDev->m_nVendorID ).Encode( hereIAm );

	hereIAm.apduAddr.GlobalBroadcast();
	Request( hereIAm );
}

/////////////////////////////////////////////////////////////////////////////
// VTSServer

//
//	VTSServer::VTSServer
//

VTSServer::VTSServer( VTSDevicePtr dp )
: serverDev(dp), BACnetServer( &dp->devDevice )
{
}

//
//	VTSServer::Indication
//

void VTSServer::Indication( const BACnetAPDU &apdu )
{
	// ### log the message

	// pass it along to the executor for a match
	gExecutor.ReceiveAPDU( apdu );

	// process it
	if (apdu.apduType == unconfirmedRequestPDU) {
		switch ((BACnetUnconfirmedServiceChoice)apdu.apduService) {
			case whoIs:
				WhoIs( apdu );
				break;
			case iAm:
				IAm( apdu );
				break;
		}
	} else
	if (apdu.apduType == confirmedRequestPDU) {
		switch ((BACnetConfirmedServiceChoice)apdu.apduService) {
			case readProperty:
				ReadProperty( apdu );
				break;
			case writeProperty:
				WriteProperty( apdu );
				break;
			default:
				// madanner 5/03, needs address for return
				{
				BACnetRejectAPDU	rejectNoService(apdu.apduInvokeID, unrecognizedService);
				rejectNoService.apduAddr = apdu.apduAddr;

				Response(rejectNoService);
//				Response( BACnetRejectAPDU( apdu.apduInvokeID, unrecognizedService ) );
				}
		}
	} else
		;
}

//
//	VTSServer::Response
//
//	DEVICE_LOOPBACK is used to allow the developer to send requests directly to the
//	internal device object.  There is currently no way to log messages that the device 
//	object sends and receives, so responses are tossed.
//

void VTSServer::Response( const BACnetAPDU &pdu )
{
	// ### log the message

	// filter out null addresses
	if (pdu.apduAddr.addrType == nullAddr)
		return;

	BACnetAppServer::Response( pdu );
}

//
//	VTSServer::WhoIs
//

void VTSServer::WhoIs( const BACnetAPDU &apdu )
{
	BACnetAPDUDecoder	dec( apdu );
	BACnetUnsigned		loLimit, hiLimit;
//	unsigned int		myInst = serverDev->devDesc.deviceInstance;
	unsigned int		myInst = serverDev->m_nInstance;

	try {
		if (dec.pktLength != 0) {
			loLimit.Decode( dec );
			hiLimit.Decode( dec );

			TRACE2( "WhoIs %d..%d\n", loLimit.uintValue, hiLimit.uintValue );

			if ((myInst < loLimit.uintValue) || (myInst > hiLimit.uintValue))
				return;
		}

		BACnetUnconfirmedServiceAPDU hello( iAm );

		// send it to everyone
		hello.apduAddr.GlobalBroadcast();

		// encode the parameters
/*MAD_DB
		BACnetObjectIdentifier( 8, serverDev->devDesc.deviceInstance ).Encode( hello );
		BACnetUnsigned( serverDev->devDesc.deviceMaxAPDUSize ).Encode( hello );
		BACnetEnumerated( serverDev->devDesc.deviceSegmentation ).Encode( hello );
		BACnetUnsigned( serverDev->devDesc.deviceVendorID ).Encode( hello );
*/
		BACnetObjectIdentifier( 8, serverDev->m_nInstance ).Encode( hello );
		BACnetUnsigned( serverDev->m_nMaxAPDUSize ).Encode( hello );
		BACnetEnumerated( serverDev->m_segmentation ).Encode( hello );
		BACnetUnsigned( serverDev->m_nVendorID ).Encode( hello );

		Response( hello );
	}
	catch (...) {
		TRACE0( "WhoIs decoding error\n" );
	}
}

//
//	VTSServer::IAm
//

void VTSServer::IAm( const BACnetAPDU &apdu )
{
	// reserved (add to device address binding list?)
}

//
//	VTSServer::ReadProperty
//

void VTSServer::ReadProperty( const BACnetAPDU &apdu )
{
	BACnetAPDUDecoder		dec( apdu )
	;
	BACnetObjectIdentifier	objId
	;
	BACnetEnumerated		propId
	;
	BACnetUnsigned			arryIndx
	;
	bool					gotIndex = false
	;

	try {
		objId.Decode( dec );
		propId.Decode( dec );

		if (dec.pktLength != 0) {
			gotIndex = true;
			arryIndx.Decode( dec );
		}

		TRACE3( "ReadProperty %d, %d, %d\n", objId.objID, propId.enumValue, arryIndx.uintValue );

		// build an ack
		BACnetComplexAckAPDU	ack( readProperty, apdu.apduInvokeID );

		// send the response back to where the request came from
		ack.apduAddr = apdu.apduAddr;

		// encode the properties from the request
		objId.Encode( ack, 0 );
		propId.Encode( ack, 1 );
		if (gotIndex)
			arryIndx.Encode( ack, 2 );

		// encode the result
	    BACnetOpeningTag().Encode( ack, 3 );

		/* MAD_DB  Find proper object/property/value
		serverDev->devObjPropValueList->Encode( objId.objID, propId.enumValue
			, (gotIndex ? arryIndx.uintValue : -1)
			, ack
			);
		*/

		// I'm not sure how this hooks up to the device...  Just go with it.
		int nErr = serverDev->ReadProperty(&objId, &propId, gotIndex ? arryIndx.uintValue : -1, &ack );
		if ( nErr != 0 )
			throw(nErr);
			
		BACnetClosingTag().Encode( ack, 3 );

		// send it
		Response( ack );
	}
	catch (int errCode) {
		TRACE1( "ReadProperty execution error - %d\n", errCode );

		BACnetErrorAPDU error( readProperty, apdu.apduInvokeID );
		error.apduAddr = apdu.apduAddr;

		// encode the Error Class
		if ((errCode == 2) || (errCode == 25)) // configuration-in-progress, operational-problem
			BACnetEnumerated( 0 ).Encode( error );		// DEVICE
		else
		if (errCode == 42) // invalid-array-index
			BACnetEnumerated( 2 ).Encode( error );		// PROPERTY
		else
			BACnetEnumerated( 1 ).Encode( error );		// OBJECT

		// encode the Error Code
		BACnetEnumerated( errCode ).Encode( error );

		Response( error );
	}
	catch (...) {
		TRACE0( "ReadProperty execution error\n" );

		BACnetRejectAPDU goAway( apdu.apduInvokeID, otherReject );
		goAway.apduAddr = apdu.apduAddr;

		Response( goAway );
	}
}

//
//	VTSServer::WriteProperty
//

void VTSServer::WriteProperty( const BACnetAPDU &apdu )
{
	BACnetAPDUTag			t
	;
	BACnetAPDUDecoder		dec( apdu )
	;
	BACnetObjectIdentifier	objId
	;
	BACnetEnumerated		propId
	;
	BACnetUnsigned			arryIndx
	;
	bool					gotIndex = false
	;

	try {
		objId.Decode( dec );
		propId.Decode( dec );

		// look at the next tag
		dec.ExamineTag( t );

		if ((t.tagClass == contextTagClass) && (t.tagNumber == 2)) {
			gotIndex = true;
			arryIndx.Decode( dec );

			// look at the next tag
			dec.ExamineTag( t );
		}

		TRACE3( "WriteProperty %d, %d, %d\n", objId.objID, propId.enumValue, arryIndx.uintValue );

		// build an ack
		BACnetSimpleAckAPDU	ack( writeProperty, apdu.apduInvokeID );

		// send the response back to where the request came from
		ack.apduAddr = apdu.apduAddr;

		// make sure we have an opening tag
		if ((t.tagClass == openingTagClass) && (t.tagNumber == 3)) {
			// remove it from the decoder
			BACnetOpeningTag().Decode( dec );
		} else
			throw (invalidTagReject);

		// decode the contents
		/* MAD_DB 
		serverDev->devObjPropValueList->Decode( objId.objID, propId.enumValue
			, (gotIndex ? arryIndx.uintValue : -1)
			, dec
			);
		*/

		int nErr = serverDev->WriteProperty( &objId, &propId, gotIndex ? arryIndx.uintValue : -1, &dec );
		if ( nErr != 0 )
			throw(nErr);

		// look at the next tag
		dec.ExamineTag( t );

		// make sure it's a closing tag
		if ((t.tagClass == closingTagClass) && (t.tagNumber == 3)) {
			// remove it from the decoder
			BACnetClosingTag().Decode( dec );
		} else
			throw (invalidTagReject);

		// send it
		Response( ack );
	}
	catch (int errCode) {
		TRACE1( "WriteProperty execution error - %d\n", errCode );

		BACnetErrorAPDU error( writeProperty, apdu.apduInvokeID );
		error.apduAddr = apdu.apduAddr;

		// encode the Error Class
		if ((errCode == 2) || (errCode == 25)) // configuration-in-progress, operational-problem
			BACnetEnumerated( 0 ).Encode( error );		// DEVICE
		else
		if (errCode == 42) // invalid-array-index
			BACnetEnumerated( 2 ).Encode( error );		// PROPERTY
		else
			BACnetEnumerated( 1 ).Encode( error );		// OBJECT

		// encode the Error Code
		BACnetEnumerated( errCode ).Encode( error );

		Response( error );
	}
	catch (BACnetRejectReason rr) {
		TRACE0( "WriteProperty execution error\n" );

		BACnetRejectAPDU goAway( apdu.apduInvokeID, rr );
		goAway.apduAddr = apdu.apduAddr;

		Response( goAway );
	}
	catch (...) {
		TRACE0( "WriteProperty execution error\n" );

		BACnetRejectAPDU goAway( apdu.apduInvokeID, otherReject );
		goAway.apduAddr = apdu.apduAddr;

		Response( goAway );
	}
}

/////////////////////////////////////////////////////////////////////////////
// VTSDevice

//MAD_DB no need to create a global list because multiple documents for config are history...
//VTSDeviceList gMasterDeviceList;

//
//	VTSDevice::VTSDevice
//

IMPLEMENT_SERIAL(VTSDevice, CObject, 1);


#pragma warning( disable : 4355 )

VTSDevice::VTSDevice()
	: devClient(this), devServer(this)
{
	m_strName = "Untitled";
	m_nInstance = 0;
	m_fRouter = FALSE;
	m_segmentation = noSegmentation;
	m_nSegmentSize = 1024;
	m_nWindowSize = 1;
	m_nMaxAPDUSize = 1024;
	m_nNextInvokeID = 0;
	m_nAPDUTimeout = 5000;
	m_nAPDUSegmentTimeout = 1000;
	m_nAPDURetries = 3;
	m_nVendorID = 15;				// default to Cornell

	devPort = NULL;
	devPortEndpoint = NULL;
//MAD_DB	devObjPropValueList = NULL;
}


void VTSDevice::Activate()
{
	// bind the device to the router (global Bind(), not our local one)
	::Bind( &devDevice, &devRouter );

// Not sure what all this is about !!
//ASSERT(0);
	// create a dummy port
//	devPort = new VTSPort( 0, 0 );
	devPort = new VTSPort();

	// the port name matches the device name
//	strcpy( devPort->portDesc.portName, devDesc.deviceName );
	devPort->SetName(GetName());

	// create a funny endpoint that redirects requests back to this device
	devPortEndpoint = new VTSDevicePort( devPort, this );
}



void VTSDevice::Deactivate()
{
	// Don't need to kill this object, the created port object will destroy
	// the endpoint...

//	if ( devPortEndpoint != NULL )
//		delete devPortEndpoint;

	devPortEndpoint = NULL;

	if ( devPort != NULL )
		delete devPort;

	devPort = NULL;
}


/* MAD_DB
VTSDevice::VTSDevice( VTSDocPtr dp, objId id )
	: devDoc(dp), devDescID(id)
	, devClient(this), devServer(this)
{
	// read in the descriptor
	ReadDesc();

	// bind the device to the router (global Bind(), not our local one)
	::Bind( &devDevice, &devRouter );

	// add this to the master list
	AddToMasterList();

	// create a dummy port
	devPort = new VTSPort( 0, 0 );

	// the port name matches the device name
	strcpy( devPort->portDesc.portName, devDesc.deviceName );

	// create a funny endpoint that redirects requests back to this device
	devPortEndpoint = new VTSDevicePort( devPort, this );

	// read in the defined objects, properties and values
	devObjPropValueList = new VTSObjPropValueList( dp, devDesc.deviceObjPropValueListID );
}
*/

#pragma warning( default : 4355 )

//
//	VTSDevice::~VTSDevice
//

VTSDevice::~VTSDevice( void )
{
	// remove this from the master list
//MAD_DB	RemoveFromMasterList();

	Deactivate();

	// delete the dummy port which will also delete the endpoint
	if ( devPort != NULL )
		delete devPort;

	// toss the object, properties and values manager
// Already handled by destructor for propery values arrays
//	if ( devObjPropValueList != NULL )
//		delete devObjPropValueList;
}

//
//	VTSDevice::AddToMasterList
//

/* MAD_DB
void VTSDevice::AddToMasterList( void )
{
	// add this to the end, regardless of its status
	gMasterDeviceList.AddTail( this );
}

//
//	VTSDevice::RemoveFromMasterList
//

void VTSDevice::RemoveFromMasterList( void )
{
	POSITION pos = gMasterDeviceList.Find( this )
	;

//MAD_DB	ASSERT( pos != NULL );
	if ( pos != NULL )
		gMasterDeviceList.RemoveAt( pos );
}

//
//	VTSDevice::ReadDesc
//

void VTSDevice::ReadDesc( void )
{
	int		stat
	;

	// ask the database for the descriptor
	stat = devDoc->m_pDB->pObjMgr->ReadObject( devDescID, &devDesc );

	// make sure the device matches the descriptor contents
	devDevice.deviceInst = devDesc.deviceInstance;
	devDevice.deviceSegmentSize = devDesc.deviceSegmentSize;
	devDevice.deviceWindowSize = devDesc.deviceWindowSize;
	devDevice.deviceNextInvokeID = devDesc.deviceNextInvokeID;
	devDevice.deviceMaxAPDUSize = devDesc.deviceMaxAPDUSize;
	devDevice.deviceAPDUTimeout = devDesc.deviceAPDUTimeout;
	devDevice.deviceAPDUSegmentTimeout = devDesc.deviceAPDUSegmentTimeout;
	devDevice.deviceAPDURetries = devDesc.deviceAPDURetries;
}

//
//	VTSDevice::WriteDesc
//

void VTSDevice::WriteDesc( void )
{
	int		stat
	;

	// save the descriptor in the database
	stat = devDoc->m_pDB->pObjMgr->WriteObject( devDescID, &devDesc );

	// make sure the device matches the descriptor contents
	devDevice.deviceInst = devDesc.deviceInstance;
	devDevice.deviceSegmentSize = devDesc.deviceSegmentSize;
	devDevice.deviceWindowSize = devDesc.deviceWindowSize;
	devDevice.deviceNextInvokeID = devDesc.deviceNextInvokeID;
	devDevice.deviceMaxAPDUSize = devDesc.deviceMaxAPDUSize;
	devDevice.deviceAPDUTimeout = devDesc.deviceAPDUTimeout;
	devDevice.deviceAPDUSegmentTimeout = devDesc.deviceAPDUSegmentTimeout;
	devDevice.deviceAPDURetries = devDesc.deviceAPDURetries;

	// make sure the dummy port has the correct name
	strcpy( devPort->portDesc.portName, devDesc.deviceName );
}
*/


//
//	VTSDevice::Bind
//
//	A binding operation is pretty simple, just pass the request along 
//	to the built-in router.
//

void VTSDevice::Bind( VTSPortPtr pp, int net )
{
	devRouter.BindToEndpoint( pp->portBindPoint, net );

	//madanner 5/03, uncommented SetLocaAddress to enable device RP/WP system
	devRouter.SetLocalAddress( net, pp->portEndpoint->portLocalAddr );
}

//
//	VTSDevice::Unbind
//

void VTSDevice::Unbind( VTSPortPtr pp )
{
	devRouter.UnbindFromEndpoint( pp->portBindPoint );
}

//
//	VTSDevice::IAm
//

void VTSDevice::IAm( void )
{
	devClient.IAm();
}

//
//	VTSDevice::SendAPDU
//

void VTSDevice::SendAPDU( const BACnetAPDU &apdu )
{
	bool	asClient
	;

	switch (apdu.apduType) {
		case confirmedRequestPDU:
		case unconfirmedRequestPDU:
			asClient = true;
			break;

		case simpleAckPDU:
		case complexAckPDU:
			asClient = false;
			break;

		case segmentAckPDU:
			asClient = (apdu.apduSrv == 0);
			break;

		case errorPDU:
		case rejectPDU:
			asClient = false;
			break;

		case abortPDU:
			asClient = (apdu.apduSrv == 0);
			break;

	}

	try {
#if DEVICE_LOOPBACK
		// null address is like a loopback
		if (apdu.apduAddr.addrType == nullAddr) {
			if (asClient)
				devServer.Indication( apdu );
			else
				devClient.Confirmation( apdu );
		} else
#endif
		if (asClient)
			devClient.Request( apdu );
		else
			devServer.Response( apdu );
	}
	catch (...) {
		TRACE0( "---------- Something went wrong ----------\n" );
	}
}


void VTSDevice::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// Switch on schema when loading for 
		// Schema 1

		ar << m_strName;
		ar << m_nInstance;
		int n = m_fRouter ? 1 : 0;
		ar << m_fRouter;
		ar << m_segmentation;
		ar << m_nSegmentSize;
		ar << m_nWindowSize;
		ar << m_nMaxAPDUSize;
		ar << m_nNextInvokeID;
		ar << m_nAPDUTimeout;
		ar << m_nAPDUSegmentTimeout;
		ar << m_nAPDURetries;
		ar << m_nVendorID;
	}
	else
	{
		ar >> m_strName;
		ar >> m_nInstance;
		ar >> m_fRouter;
		int n;
		ar >> n;
		m_segmentation = (BACnetSegmentation) n;
		ar >> m_nSegmentSize;
		ar >> m_nWindowSize;
		ar >> m_nMaxAPDUSize;
		ar >> m_nNextInvokeID;
		ar >> m_nAPDUTimeout;
		ar >> m_nAPDUSegmentTimeout;
		ar >> m_nAPDURetries;
		ar >> m_nVendorID;
	}

	m_devobjects.Serialize(ar);
}


CString VTSDevice::GetDescription(void)
{
	CString strInstance;
	strInstance.Format("%s, %d", GetName(), m_nInstance);
	return strInstance;
}


// First look for supported object in our array

VTSDevObject * VTSDevice::FindObject( unsigned int nObjID )
{
	for (int i = 0; i < m_devobjects.GetSize(); i++ )
		if ( m_devobjects[i]->GetID() == nObjID )
			return m_devobjects[i];

	return NULL;
}



VTSDevProperty * VTSDevice::FindProperty( VTSDevObject * pobject, int nPropID )
{
	for ( int i = 0; pobject != NULL && i < pobject->GetProperties()->GetSize(); i++ )
		if ( (*pobject->GetProperties())[i]->GetID() == nPropID )
			return (*pobject->GetProperties())[i];

	return NULL;
}


int VTSDevice :: InternalReadProperty( BACnetObjectIdentifier * pbacnetobjectid, BACnetEnumerated * pbacnetpropid, BACnetAPDUEncoder * pAPDUEncoder )
{
	BACnetObjectIdentifier	bacnetobjidThis(8, m_nInstance);

	if ( pbacnetobjectid->objID != bacnetobjidThis.objID )
		return 31;												// object not found

	switch( pbacnetpropid->enumValue )
	{
		case OBJECT_NAME:
						{
						BACnetCharacterString(m_strName).Encode(*pAPDUEncoder);
						}
						break;

		case VENDOR_IDENTIFIER:
						{
						BACnetUnsigned(m_nVendorID).Encode( *pAPDUEncoder );
						}
						break;

		case MAX_APDU_LENGTH_ACCEPTED:
						{
						BACnetUnsigned(m_nMaxAPDUSize).Encode( *pAPDUEncoder );
						}
						break;

		case APDU_TIMEOUT:
						{
						BACnetUnsigned(m_nAPDUTimeout).Encode( *pAPDUEncoder );
						}
						break;

		case APDU_SEGMENT_TIMEOUT:
						{
						BACnetUnsigned(m_nAPDUSegmentTimeout).Encode( *pAPDUEncoder );
						}
						break;

		case NUMBER_OF_APDU_RETRIES:
						{
						BACnetUnsigned(m_nAPDURetries).Encode( *pAPDUEncoder );
						}
						break;

		case SEGMENTATION_SUPPORTED:
						{
						BACnetUnsigned((int) m_segmentation).Encode( *pAPDUEncoder );
						}
						break;

		case LOCAL_DATE:
						{
						BACnetDate().Encode( *pAPDUEncoder );
						}
						break;

		case LOCAL_TIME:
						{
						BACnetTime().Encode( *pAPDUEncoder );
						}
						break;

		// might be nice to support others implicitly with device declaration... 

		default:
						return 32;			// property not found
	}

	return 0;		// no errors
}



int VTSDevice :: InternalWriteProperty( BACnetObjectIdentifier * pbacnetobjectid, BACnetEnumerated * pbacnetpropid, BACnetAPDUDecoder * pdec )
{
	BACnetObjectIdentifier	bacnetobjidThis(8, m_nInstance);

	if ( pbacnetobjectid->objID != bacnetobjidThis.objID )
		return 31;												// object not found

	try
	{
		BACnetUnsigned	bacnetunsigned;

		switch( pbacnetpropid->enumValue )
		{
			case OBJECT_NAME:
							{
							BACnetCharacterString str(*pdec);
							strncpy(m_strName.GetBuffer(str.strLen+1), (const char *) str.strBuff, str.strLen);
							m_strName.SetAt(str.strLen, 0);
							m_strName.ReleaseBuffer();
							}
							break;

			case VENDOR_IDENTIFIER:

							bacnetunsigned.Decode(*pdec);
							m_nVendorID = bacnetunsigned.uintValue;
							break;

			case MAX_APDU_LENGTH_ACCEPTED:

							bacnetunsigned.Decode(*pdec);
							m_nMaxAPDUSize = bacnetunsigned.uintValue;
							break;

			case APDU_TIMEOUT:

							bacnetunsigned.Decode(*pdec);
							m_nAPDUTimeout = bacnetunsigned.uintValue;
							break;

			case APDU_SEGMENT_TIMEOUT:

							bacnetunsigned.Decode(*pdec);
							m_nAPDUSegmentTimeout = bacnetunsigned.uintValue;
							break;

			case NUMBER_OF_APDU_RETRIES:

							bacnetunsigned.Decode(*pdec);
							m_nAPDURetries = bacnetunsigned.uintValue;
							break;

			case SEGMENTATION_SUPPORTED:

							bacnetunsigned.Decode(*pdec);

							if ( bacnetunsigned.uintValue > (unsigned int) noSegmentation )
								return 37;			// value out of range

							m_segmentation = (BACnetSegmentation) bacnetunsigned.uintValue;
							break;

			// might be nice to support others implicitly with device declaration... 
			default:
							return 32;			// property not found
		}
	}
	catch(...)
	{
		return 9;			// invalid data type
	}

	return 0;		// no errors
}


// Possible errors:
// 2  = device configuration in progress
// 7  = value is not an array but array indexing requested
// 25 = internal problems...
// 31 = object not found
// 32 = property not found
// 42 = array index out of bounds

int VTSDevice :: ReadProperty( BACnetObjectIdentifier * pbacnetobjectid, BACnetEnumerated * pbacnetpropid, int nIndex, BACnetAPDUEncoder * pAPDUEncoder )
{
	VTSDevObject *		pobject = NULL;
	VTSDevProperty *	pprop = NULL;

	// First attempt to locate property in question from list... if property is for this device this will 
	// allow for overrides in device object property configuration

	if ( (pobject = FindObject(pbacnetobjectid->objID)) == NULL )
		return InternalReadProperty(pbacnetobjectid, pbacnetpropid, pAPDUEncoder);

	if ( (pprop = FindProperty(pobject, pbacnetpropid->enumValue)) == NULL )
		return InternalReadProperty(pbacnetobjectid, pbacnetpropid, pAPDUEncoder);

	// IF they've requested either the length of the array or a specific element and this isn't
	// an array, we need to tell them

	if ( !pprop->IsArray() && nIndex != -1 )
		return 7;											// property is not an array

	if ( nIndex == 0 )
	{
		BACnetUnsigned( pprop->GetValues()->GetSize() ).Encode( *pAPDUEncoder );
		return 0;
	}

	int nStart;

	// get all values, array or not
	if ( nIndex == -1 )
	{
		nStart = 0;
		nIndex = pprop->GetValues()->GetSize();
	}
	else
	{
		// we might be trying to check for out of bounds array...
		// Remember that the request is base 1
		if ( nIndex > pprop->GetValues()->GetSize() )
			return 42;										// array index out of bounds
		nStart = nIndex-1;
	}

	for ( ; nStart < nIndex; nStart++ )
	{
		VTSDevValue * pvalue = (*pprop->GetValues())[nStart];

		// There may be some problems simply encoding value after value here..
		// It seems like there needs to be a distinction between sets and arrays (I know there is,
		// just can't think of it).  Outer loop is for arrays, inner is for set values.

		try
		{
			pAPDUEncoder->Append(pvalue->m_abContent, pvalue->m_nLength);
			VTSDevValues * pvaluesComponents = pvalue->GetValueList();
			
			for ( int i = 0; pvaluesComponents != NULL && i < pvaluesComponents->GetSize(); i++ )
				pAPDUEncoder->Append((*pvaluesComponents)[i]->m_abContent, (*pvaluesComponents)[i]->m_nLength);
		}
		catch(...)
		{
			return 25;		// database problem...  Append has issues.
		}
	}

	return 0;
}


// Possible errors:
// 2  = device configuration in progress
// 7  = value is not an array but array indexing requested
// 25 = internal problems...
// 31 = object not found
// 32 = property not found
// 42 = array index out of bounds

int VTSDevice :: WriteProperty( BACnetObjectIdentifier * pbacnetobjectid, BACnetEnumerated * pbacnetpropid, int nIndex, BACnetAPDUDecoder * pdec )
{
	VTSDevObject *		pobject = NULL;
	VTSDevProperty *	pprop = NULL;

	// First attempt to locate property in question from list... if property is for this device this will 
	// allow for overrides in device object property configuration

	if ( (pobject = FindObject(pbacnetobjectid->objID)) == NULL )
		return InternalWriteProperty(pbacnetobjectid, pbacnetpropid, pdec);

	if ( (pprop = FindProperty(pobject, pbacnetpropid->enumValue)) == NULL )
		return InternalWriteProperty(pbacnetobjectid, pbacnetpropid, pdec);

	// IF they've requested to change either the length of the array or a specific element and this isn't
	// an array, we need to tell them.  This is not physically necessary, but the property must have been 
	// marked as an array before we should be allowed to make it so by adding elements

	if ( !pprop->IsArray() && nIndex != -1 )
		return 7;											// property is not an array

	// They want to change the length of the array... We'll just fill it with null values or cut off
	// others...

	VTSDevValues * pvalues = pprop->GetValues();

	if ( nIndex == 0 )
	{
		BACnetUnsigned	bacnetunsignedLength(*pdec);			// auto decodes

		if ( bacnetunsignedLength.uintValue < 0 )
			return 42;	 // invalid-array-index

		// Now we'll go through all the elements and add some or remove some...
		unsigned long nExistingCount = pvalues->GetSize();

		if ( bacnetunsignedLength.uintValue < nExistingCount )
		{
			// need to kill a few existing values... start at the end and work backwards
			for ( ; nExistingCount > bacnetunsignedLength.uintValue; nExistingCount-- )
			{
				delete (*pvalues)[nExistingCount-1];
				pvalues->RemoveAt(nExistingCount-1);
			}
		}
		else if ( bacnetunsignedLength.uintValue > nExistingCount )
		{
			// need to add some null elements... they'll be changed later (I guess in another bacnet message)
			for ( ; nExistingCount < bacnetunsignedLength.uintValue; nExistingCount++ )
				pvalues->Add(new VTSDevValue());
		}

		return 0;
	}

	// OK... they want to change either an existing element or the whole thing...
	// all of the nodes have been added so it's just a matter of changing the contents of each node

	int nStart;

	// replace all values, array or not
	if ( nIndex == -1 )
	{
		nStart = 0;
		nIndex = pvalues->GetSize();
	}
	else
	{
		// we might be trying to check for out of bounds array...
		// Remember that the request is base 1

		if ( nIndex > pvalues->GetSize() )
			return 42;										// array index out of bounds
		nStart = nIndex-1;
	}

	// This really doesn't work too well... Something about arrays, sets and whatnot is going
	// to change this code.  For now, just go through what you find in the encoded stream and stuff
	// each value in the array

	BACnetAPDUTag	t;
	int nNestCount = 0;
	bool fFinish = false;

	while ( nStart < nIndex  &&  !fFinish && pdec->pktLength > 0 )
	{
		VTSDevValue * pvalue = (*pvalues)[nStart];

		// There may be some problems simply encoding value after value here..
		// It seems like there needs to be a distinction between sets and arrays (I know there is,
		// just can't think of it).  Outer loop is for arrays, inner is for set values.

		try
		{
			pdec->ExamineTag(t);

			switch (t.tagClass)
			{
				case contextTagClass:
					// set this as an octet string.  Because it is context tagged, the datatype connot
					// be determined, the context isn't maintained.  It could be, because we know the 
					// object type and property, and we could even validate the datatype along the 
					// way, but that will be saved for future work.
					pvalue->m_nType = octetStringAppTag;
					pvalue->m_nContext = (int) t.tagNumber;
					break;
					
				case openingTagClass:

					nNestCount++;
					pvalue->m_nType = 13;	// opening tag
					pvalue->m_nContext = (int) t.tagNumber;
					break;
					
				case closingTagClass:

					if ( !nNestCount--)
					{
						fFinish = true;
						break;
					}

					pvalue->m_nType = 14;	// closing tag
					pvalue->m_nContext = (int) t.tagNumber;
					break;
				
				case applicationTagClass:

					pvalue->m_nType = t.tagNumber;	// tag number matches type
					pvalue->m_nContext = -1;
					break;
			}
		}
		catch (...)
		{
			return 25;			// decoding error
		}

		// done yet?
		if ( !fFinish )
			pvalue->m_nLength = pdec->ExtractTagData(pvalue->m_abContent);

		nStart++;
	}

	return 0;
}


const VTSDevice& VTSDevice::operator=(const VTSDevice& rdeviceSrc)
{
	m_strName = rdeviceSrc.m_strName;
	m_nInstance = rdeviceSrc.m_nInstance;
	m_fRouter = rdeviceSrc.m_fRouter;
	m_segmentation = rdeviceSrc.m_segmentation;
	m_nSegmentSize = rdeviceSrc.m_nSegmentSize;
	m_nWindowSize = rdeviceSrc.m_nWindowSize;
	m_nMaxAPDUSize = rdeviceSrc.m_nMaxAPDUSize;
	m_nNextInvokeID = rdeviceSrc.m_nNextInvokeID;
	m_nAPDUTimeout = rdeviceSrc.m_nAPDUTimeout;
	m_nAPDUSegmentTimeout = rdeviceSrc.m_nAPDUSegmentTimeout;
	m_nAPDURetries = rdeviceSrc.m_nAPDURetries;
	m_nVendorID = rdeviceSrc.m_nVendorID;

	m_devobjects.DeepCopy(&rdeviceSrc.m_devobjects);
	
	return *this;
}


IMPLEMENT_VTSPTRARRAY(VTSDevices, VTSDevice);



/* MAD_DB

/////////////////////////////////////////////////////////////////////////////
// VTSDeviceList

//
//	VTSDeviceList::VTSDeviceList
//

VTSDeviceList::VTSDeviceList( void )
	: m_pDoc(0)
{
}

//
//	VTSDeviceList::~VTSDeviceList
//

VTSDeviceList::~VTSDeviceList( void )
{
}

//
//	VTSDeviceList::Load
//

void VTSDeviceList::Load( VTSDocPtr docp )
{
	int				stat
	;
	objId			curID
	;
	JDBListPtr		plist
	;
	VTSDevicePtr	cur
	;

	// keep track of the database
	m_pDoc = docp;

	// get a pointer to the list
	plist = &m_pDoc->m_pDB->dbDeviceList;

	// intialize the port list
	for (int i = 0; i < plist->Length(); i++) {
		// get the port descriptor ID's
		stat = plist->ReadElem( i, &curID );
		ASSERT( stat == 0 );

		// create a new device object to match
		cur = new VTSDevice( m_pDoc, curID );

		// add it to our list of ports
		AddTail( cur );
	}
}

//
//	VTSDeviceList::Unload
//

void VTSDeviceList::Unload( void )
{
	for (POSITION pos = GetHeadPosition(); pos; )
		delete GetNext( pos );
}

//
//	VTSDeviceList::Add
//

void VTSDeviceList::Add( void )
{
	int					stat, elemLoc
	;
	objId				newDescID
	;
	VTSDevicePtr		cur
	;
	VTSDeviceDesc		newDesc
	;
	JDBListPtr			lp
	;

	// create a new descriptor
	stat = m_pDoc->m_pDB->pObjMgr->NewObject( &newDescID, &newDesc, kVTSDeviceDescSize );

	// initalize it
	strcpy( newDesc.deviceName, "Untitled" );
	newDesc.deviceInstance = 0;
	newDesc.deviceRouter = 0;
	newDesc.deviceSegmentation = noSegmentation;
	newDesc.deviceSegmentSize = 1024;
	newDesc.deviceWindowSize = 1;
	newDesc.deviceMaxAPDUSize = 1024;
	newDesc.deviceNextInvokeID = 0;
	newDesc.deviceAPDUTimeout = 5000;
	newDesc.deviceAPDUSegmentTimeout = 1000;
	newDesc.deviceAPDURetries = 3;
	newDesc.deviceVendorID = 15;				// default to Cornell

	// create a new object and property list
	lp = new JDBList();
	stat = m_pDoc->m_pDB->pListMgr->NewList( *lp, kVTSObjPropValueSize );
	newDesc.deviceObjPropValueListID = lp->objID;
	delete lp;

	// save it
	stat = m_pDoc->m_pDB->pObjMgr->WriteObject( newDescID, &newDesc );
	ASSERT( stat == 0 );

	// add it on the end of the device list
	elemLoc = 0x7FFFFFFF;
	stat = m_pDoc->m_pDB->dbDeviceList.NewElem( &elemLoc, &newDescID );

	// create a new device object to match
	cur = new VTSDevice( m_pDoc, newDescID );

	// add it to our list of ports
	AddTail( cur );
}

//
//	VTSDeviceList::Remove
//
//	This function is currently not called.  Note there is no delete button in 
//	the device dialog box.
//

void VTSDeviceList::Remove( int i )
{
//	ASSERT( 0 );
	int			stat
	;
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	delete GetAt( pos );
	RemoveAt( pos );

	stat = m_pDoc->m_pDB->dbDeviceList.DeleteElem( i );
}

//
//	VTSPortList::FindPort
//
//	This function is called by the scripting engine to find a port with a given 
//	name as described in the packet description in the script.  Used for sending 
//	low level messages directly to a port.
//

VTSDevicePtr VTSDeviceList::FindDevice( const char *name )
{
	for (POSITION pos = GetHeadPosition(); pos; ) {
		VTSDevicePtr cur = (VTSDevicePtr)GetNext( pos );

		if (strcmp(name,cur->devDesc.deviceName) == 0)
			return cur;
	}

	// failed to find it
	return 0;
}

//
//	VTSDeviceList::FindDevice
//
//	This function is used by a port to bind itself properly to a device object.
//

VTSDevicePtr VTSDeviceList::FindDevice( objId id )
{
	for (POSITION pos = GetHeadPosition(); pos; ) {
		VTSDevicePtr cur = (VTSDevicePtr)GetNext( pos );

		if (cur->devDescID == id)
			return cur;
	}

	// failed to find it
	return 0;
}

//
//	VTSDeviceList::Length
//

int VTSDeviceList::Length( void )
{
	return CList<VTSDevicePtr,VTSDevicePtr>::GetCount();
}

//
//	VTSDeviceList::operator []
//
//	When the user selects a port from the list of defined ports in the VTSPortDlg, this 
//	function is called to return a pointer to the VTSPort object.  
//

VTSDevicePtr VTSDeviceList::operator []( int i )
{
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	return GetAt( pos );
}

*/  


/////////////////////////////////////////////////////////////////////////////
// VTSWinWinPcapPort

//
//	VTSWinWinPcapPort::VTSWinWinPcapPort
//

extern CSendGroupPtr gEthernetGroupList[];
extern CSendGroupPtr gARCNETGroupList[];

//VTSWinPacket32Port::VTSWinPacket32Port( VTSPortPtr pp )
//	: WinPacket32( pp->m_strConfigParms )
VTSWinWinPcapPort::VTSWinWinPcapPort( VTSPortPtr pp )
	: WinWinPcap( pp->m_strConfigParms )
//	: WinWinPcap( pp->portDesc.portConfig )
	, m_pPort( pp )
{
	// let the port know which send group to use
	m_pPort->portSendGroup = gEthernetGroupList;
}

//
//	VTSWinWinPcapPort::~VTSWinWinPcapPort
//

VTSWinWinPcapPort::~VTSWinWinPcapPort( void )
{
	// reset the send group
	m_pPort->portSendGroup = 0;
}

//
//	VTSWinWinPcapPort::FilterData
//

void VTSWinWinPcapPort::FilterData( BACnetOctet *data, int len, BACnetPortDirection dir )
{
	VTSPacket	pkt
	;
	
	// fill in the packet header
//	pkt.packetHdr.packetPortID = m_pPort->portDescID;
	strncpy(pkt.packetHdr.m_szPortName, m_pPort->GetName(), sizeof(pkt.packetHdr.m_szPortName)-1);

	pkt.packetHdr.packetFlags = 0;
	pkt.packetHdr.packetType = (dir == portSending) ? txData : rxData;

	// parse the source and destination addresses
	pkt.packetHdr.packetProtocolID = (int)BACnetPIInfo::ProtocolType::ethernetProtocol;

	// check for broadcast destination
	if ((data[0] == 0xFF) && (data[1] == 0xFF) && (data[2] == 0xFF)
			&& (data[3] == 0xFF) && (data[4] == 0xFF) && (data[5] == 0xFF))
		pkt.packetHdr.packetDestination.LocalBroadcast();
	else
		pkt.packetHdr.packetDestination.LocalStation( data, 6 );

	// source is always a station
	pkt.packetHdr.packetSource.LocalStation( data + 6, 6 );

	// skip processing packets from myself
	if ((dir == portReceiving) && (pkt.packetHdr.packetSource == portLocalAddr))
		return;

	// let the packet refer to the pdu contents, cast away const
	pkt.NewDataRef( data, len );

	// save it in the database;
//	m_pPort->portDoc->m_pDB->WritePacket( -1, pkt );			// MAD_DB
	m_pPort->portDoc->WritePacket( pkt );

//MAD_DB  This is now called from the Doc's WritePacket
/*
	// tell the application
	if (m_pPort->portDoc->m_postMessages)
		::PostThreadMessage( AfxGetApp()->m_nThreadID
			, WM_VTS_RCOUNT, (WPARAM)0, (LPARAM)m_pPort->portDoc
			);
*/
}

//
//	VTSWinWinPcapPort::PortStatusChange
//

void VTSWinWinPcapPort::PortStatusChange( void )
{
	// set the VTSPort info to reflect this status
	if (portStatus == 0) {
		m_pPort->portStatus = 0;
		m_pPort->portStatusDesc = 0;
	} else {
		m_pPort->portStatus = 2;
		m_pPort->portStatusDesc = "Something is wrong";
	}

	// tell the application that something changed.
	::PostThreadMessage( AfxGetApp()->m_nThreadID
		, WM_VTS_PORTSTATUS, (WPARAM)0, (LPARAM)m_pPort->portDoc
		);
}

/////////////////////////////////////////////////////////////////////////////
// VTSWinIPPort

//
//	VTSWinIPPort::VTSWinIPPort
//

extern CSendGroupPtr gIPGroupList[];

VTSWinIPPort::VTSWinIPPort( VTSPortPtr pp )
	: WinIP( pp->m_strConfigParms )
	, m_pPort( pp )
{
	// let the port know which send group to use
	m_pPort->portSendGroup = gIPGroupList;
}

//
//	VTSWinIPPort::~VTSWinIPPort
//

VTSWinIPPort::~VTSWinIPPort( void )
{
	// reset the send group
	m_pPort->portSendGroup = 0;
}

//
//	VTSWinIPPort::FilterData
//

void VTSWinIPPort::FilterData( BACnetOctet *data, int len, BACnetPortDirection dir )
{
	VTSPacket	pkt
	;

	// fill in the packet header
//	pkt.packetHdr.packetPortID = m_pPort->portDescID;
	strncpy(pkt.packetHdr.m_szPortName, m_pPort->GetName(), sizeof(pkt.packetHdr.m_szPortName)-1);

	pkt.packetHdr.packetProtocolID = (int)BACnetPIInfo::ProtocolType::ipProtocol;
	pkt.packetHdr.packetFlags = 0 /* (pdu.pduExpectingReply << 8) + pdu.pduNetworkPriority */;
	pkt.packetHdr.packetType = (dir == portSending) ? txData : rxData;
	
	// parse the header and suck out the source or destination
	if (dir == portSending)
		pkt.packetHdr.packetDestination.LocalStation( data, 6 );
	else
	if (dir == portReceiving) {
		pkt.packetHdr.packetSource.LocalStation( data, 6 );
	} else
		;

	// let the packet refer to the pdu contents, cast away const
	pkt.NewDataRef( data, len );
	
	// save it in the database;
//	m_pPort->portDoc->m_pDB->WritePacket( -1, pkt );			// MAD_DB
	m_pPort->portDoc->WritePacket( pkt );
	
//MAD_DB  This is now called from the Doc's WritePacket
/*
	// tell the application there is a new packet count
	if (m_pPort->portDoc->m_postMessages)
		::PostThreadMessage( AfxGetApp()->m_nThreadID
			, WM_VTS_RCOUNT, (WPARAM)0, (LPARAM)m_pPort->portDoc
			);
*/
}

//
//	VTSWinIPPort::PortStatusChange
//

void VTSWinIPPort::PortStatusChange( void )
{
	// set the VTSPort info to reflect this status
	if (portStatus == 0) {
		m_pPort->portStatus = 0;
		m_pPort->portStatusDesc = 0;
	} else {
		m_pPort->portStatus = 2;
		m_pPort->portStatusDesc = "Something is wrong";
	}

	// tell the application that something changed.
	::PostThreadMessage( AfxGetApp()->m_nThreadID
		, WM_VTS_PORTSTATUS, (WPARAM)0, (LPARAM)m_pPort->portDoc
		);
}

/////////////////////////////////////////////////////////////////////////////
// VTSDevicePort

//
//	VTSDevicePort::VTSDevicePort
//

VTSDevicePort::VTSDevicePort( VTSPortPtr pp, VTSDevicePtr dp )
	: m_pPort(pp), m_pDevice(dp)
{
	// the port should think this is its endpoint
	pp->portEndpoint = this;

	// let the port know which send group to use
	pp->portSendGroup = gDeviceGroupList;
}

//
//	VTSDevicePort::~VTSDevicePort
//

VTSDevicePort::~VTSDevicePort( void )
{
	// reset the send group
	m_pPort->portSendGroup = 0;
}

//
//	VTSDevicePort::Indication
//
//	This function is required because a VTSDevicePort derives from BACnetNetServer.
//	It should never be called, these objects aren't bound to anything.
//

void VTSDevicePort::Indication( const BACnetNPDU &pdu )
{
	ASSERT( 0 );
}

//
//	VTSDevicePort::SendData
//

void VTSDevicePort::SendData( BACnetOctet *data, int len )
{
	int			net, addrLen
	;
	BACnetAPDU	apdu
	;

	// rip apart the address that was so carefully encoded by CSendDevice::EncodePage
	apdu.apduAddr.addrType = (BACnetAddressType)(len--,*data++);

	net = (len--,*data++);
	net = (net << 8) + (len--,*data++);
	apdu.apduAddr.addrNet = net;

	addrLen = (len--,*data++);
	apdu.apduAddr.addrLen = addrLen;
	for (int i = 0; i < addrLen; i++)
		apdu.apduAddr.addrAddr[i] = (len--,*data++);

	// build a temporary decoder out of the rest
	BACnetAPDUDecoder dec( data, len )
	;

	// turn it into an APDU
	apdu.Decode( dec );

	// pass it along to the device to process
	m_pDevice->SendAPDU( apdu );
}

