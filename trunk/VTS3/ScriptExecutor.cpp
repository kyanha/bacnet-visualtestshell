// ScriptExecutor.cpp: implementation of the ScriptExecutor class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "VTS.h"
#include "VTSDoc.h"
#include "VTSQueue.h"



#include "PI.h"

#include "BACnetIP.hpp"

#include "ScriptExecutor.h"
#include "ScriptSelectSession.h"
#include "ScriptParmList.h"

#include "ScriptKeywords.h"

namespace PICS {

#include "db.h" 
#include "service.h"
#include "vtsapi.h"
#include "props.h"
#include "bacprim.h"
#include "dudapi.h"
#include "dudtool.h"
#include "propid.h"

//extern struct generic_object;
//class BACnetAnyValue;
extern "C" void CreatePropertyFromEPICS( PICS::generic_object * pObj, int PropId, BACnetAnyValue * pbacnetAnyValue );

}

extern PICS::PICSdb *gPICSdb;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// global defines

ScriptExecutor		gExecutor;
ScriptFilterList	gMasterFilterList;

//******Added by Liangping Xu, 2002-8-5********//

int   nArrayIndx = -1;

//******Ended by Liangping Xu, 2002-8-5********//

//Added by Yiping Xu, 2002-8-5
namespace NetworkSniffer {
	extern char *BACnetConfirmedServiceChoice[];
//	extern char *BACnetErrorClass[];
//	extern char *BACnetErrorCode[];
}
//Ended by Yiping Xu, 2002-8-5

// some useful matching functions

//Added by Yajun Zhou, 2002-7-16
#include "math.h"
const float FLOAT_EPSINON = 1e-005;
const double DOUBLE_EPSINON = 1e-010;
/////////////////////////////


bool Match( int op, int a, int b );
//madanner 9/24/02, needed for expanded BACnetUsigned internal value
//bool Match( int op, unsigned long a, unsigned long b );
bool Match( int op, unsigned long a, unsigned long b );
bool Match( int op, float a, float b );
bool Match( int op, double a, double b );
bool Match( int op, CTime &timeThis, CTime &timeThat );
LPCSTR OperatorToString(int iOperator);



//	VTSQueue<ScriptExecMsg>

// Why is this CPP file here?  Should be able to do all from header file. - STK
//#include "VTSQueue.cpp"
// I moved all the code from VTSQueue.CPP to VTSQueue.H (okay for templates). - STK
 
//	This declaration below instantiates a version of the VTSQueue that contains
//	executor messages.  This will generate a warning that the template has already 
//	been instantiated, but don't believe it.  I wouldn't be suprised if the 
//	template code is defined someplace but not accessable to the linker. - Joel?

// This is instantiated in the class ScriptExecutor as execMsgQueue - STK
// I wouldn't have typedef'd it - that really confuses the programmer. - STK
//template class VTSQueue<ScriptExecMsg>;

//
//	ScriptFilter::ScriptFilter
//

ScriptFilter::ScriptFilter( ScriptFilterType typ, char *name )
	: filterType(typ)
{
	strcpy( filterName, name );
	gMasterFilterList.AddFilter( this );
}

//
//	ScriptFilter::~ScriptFilter
//

ScriptFilter::~ScriptFilter( void )
{
	gMasterFilterList.RemoveFilter( this );
}

//
//	ScriptFilterList::ScriptFilterList
//

ScriptFilterList::ScriptFilterList( void )
{
}

//
//	ScriptFilterList::~ScriptFilterList
//
//	If the list goes away, make sure all of the filters go away.
//

ScriptFilterList::~ScriptFilterList( void )
{
	for (POSITION xpos = GetHeadPosition(); xpos; )
		delete GetNext( xpos );
}

//
//	ScriptFilterList::AddFilter
//

void ScriptFilterList::AddFilter( ScriptFilterPtr fp )
{
	// add it to our list of ports
	AddTail( fp );
}

//
//	ScriptFilterList::RemoveFilter
//

void ScriptFilterList::RemoveFilter( ScriptFilterPtr fp )
{
	for (POSITION xpos = GetHeadPosition(); xpos; ) {
		POSITION		cur = xpos;
		ScriptFilterPtr curfp = (ScriptFilterPtr)GetNext( xpos );

		if (curfp == fp) {
			RemoveAt( cur );
			return;
		}
	}
}

//
//	ScriptFilterList::FindFilter
//

ScriptFilterPtr ScriptFilterList::FindFilter( const char *name )
{
	for (POSITION xpos = GetHeadPosition(); xpos; ) {
		ScriptFilterPtr cur = (ScriptFilterPtr)GetNext( xpos );

		if (strcmp(name,cur->filterName) == 0)
			return cur;
	}

	// failed to find it
	return 0;
}

//
//	ScriptFilterList::Length
//

int ScriptFilterList::Length( void )
{
	return CList<ScriptFilterPtr,ScriptFilterPtr>::GetCount();
}

//
//	ScriptFilterList::operator []
//

ScriptFilterPtr ScriptFilterList::operator []( int i )
{
	POSITION	xpos = FindIndex( i )
	;

	ASSERT( xpos != NULL );
	return GetAt( xpos );
}

//
//	ScriptNetFilter::ScriptNetFilter
//
//	The only thing that the filter ctor must do is save its name.
//

ScriptNetFilter::ScriptNetFilter( char *name )
	: ScriptFilter( scriptNPDUFilter, name )
{
}

//
//	ScriptNetFilter::~ScriptNetFilter
//
//	When the configuration of VTS changes, these filter objects will be deleted.  These
//	filter objects are private to the filter list, so there's nothing special to do 
//	in the dtor.
//

ScriptNetFilter::~ScriptNetFilter( void )
{
}

//
//	ScriptNetFilter::Indication
//
//	When a filter gets an indication is acting as a server to some higher level object.
//	This function is also called when the executor has some NPDU that needs to be sent 
//	down to the port.  This server is simple, it just passes it to the bound client.
//

void ScriptNetFilter::Indication( const BACnetNPDU &npdu )
{
	// pass down to server
	Request( npdu );
}

//
//	ScriptNetFilter::Confirmation
//
//	When a filter gets a confirmation it is bound to some object that wants this message 
//	passed up the chain.  Before doing that, make sure the executor gets a chance to look 
//	at it and see if it passes any EXPECT conditions that a running script established.
//	If the executor isn't running, don't bother.
//

void ScriptNetFilter::Confirmation( const BACnetNPDU &npdu )
{
	// make a copy for the executor thread
	if (gExecutor.IsRunning())
		new ScriptNetPacket( this, npdu );

	// pass up to client
	Response( npdu );
}

//
//	ScriptNetPacket::ScriptNetPacket
//

ScriptNetPacket::ScriptNetPacket( ScriptFilterPtr fp, const BACnetNPDU &npdu )
	: BACnetTask( BACnetTask::oneShotDeleteTask )
	, packetFilter(fp)
	, packetNPDU( npdu.pduAddr
		, 0, npdu.pduLen
		, npdu.pduExpectingReply, npdu.pduNetworkPriority
		)
{
	// timestamp the delivery
	::GetSystemTimeAsFileTime( &packetTime );

	// make a copy of the data
	packetData = new BACnetOctet[ npdu.pduLen ];
	memcpy( packetData, npdu.pduData, npdu.pduLen );

	// point to the new copy
	packetNPDU.pduData = packetData;

	// let it run when it gets a chance
	InstallTask();
}

//
//	ScriptNetPacket::~ScriptNetPacket
//

ScriptNetPacket::~ScriptNetPacket( void )
{
	// delete the content
	delete[] packetData;
}

//
//	ScriptNetPacket::ProcessTask
//

void ScriptNetPacket::ProcessTask( void )
{
	// pass this along
	gExecutor.ReceiveNPDU( (ScriptNetFilterPtr)packetFilter, packetNPDU );
}

//
//	ScriptDebugNetFilter::ScriptDebugNetFilter
//

ScriptDebugNetFilter::ScriptDebugNetFilter( char *name )
	: ScriptFilter( scriptNPDUFilter, name ), filterPeer(0)
{
}

//
//	ScriptDebugNetFilter::~ScriptDebugNetFilter
//

ScriptDebugNetFilter::~ScriptDebugNetFilter( void )
{
}

//
//	ScriptDebugNetFilter::Indication
//

void ScriptDebugNetFilter::Indication( const BACnetNPDU &npdu )
{
	// make a copy for the executor thread
	if (filterPeer && gExecutor.IsRunning())
		new ScriptNetPacket( filterPeer, npdu );
}

//
//	ScriptDebugNetFilter Bind
//

void Bind( ScriptDebugNetFilterPtr fp1, ScriptDebugNetFilterPtr fp2 )
{
	fp1->filterPeer = fp2;
	fp2->filterPeer = fp1;
}

//--------------------------------------------------

//
//	ScriptAppFilter::ScriptAppFilter
//
//	The only thing that the filter ctor must do is save its name.
//

ScriptAppFilter::ScriptAppFilter( char *name )
	: ScriptFilter( scriptAPDUFilter, name )
{
}

//
//	ScriptAppFilter::~ScriptAppFilter
//

ScriptAppFilter::~ScriptAppFilter( void )
{
}

//
//	ScriptAppFilter::Indication
//

void ScriptAppFilter::Indication( const BACnetAPDU &adpu )
{
	// pass down to server
	Request( adpu );
}

//
//	ScriptAppFilter::Confirmation
//

void ScriptAppFilter::Confirmation( const BACnetAPDU &apdu )
{
#if 0
	// give it to the executor
	if (gExecutor.IsRunning())
		gExecutor.ReceiveAPDU( this, apdu );
#endif

	// pass up to client
	Response( apdu );
}

//
//	ScriptExecutor::ExecError::ExecError
//
//	When some deeply nested function needs to throw an error message it tosses one of 
//	these.
//

ScriptExecutor::ExecError::ExecError( const char *msg, int lineNo )
	: errMsg(msg), errLineNo(lineNo)
{
}


ScriptExecutor::ExecError::ExecError( CString &str, int lineNo )
	: errLineNo(lineNo)
{
	lstrcpy(szBuf, (LPCSTR)str);
	errMsg = szBuf;
}

//
//	ScriptExecutor::ScriptExecutor
//

ScriptExecutor::ScriptExecutor( void )
	: execState(execIdle)
	, execAllTests(false), execSingleStep(false), execStepForced(0)
	, execFailContinue(false)
	, execDB(0), execDoc(0), execTest(0), execPacket(0)
{
}

//
//	ScriptExecutor::~ScriptExecutor
//

ScriptExecutor::~ScriptExecutor()
{
}

//
//	ScriptExecutor::Setup
//
//	This function is used by the application to initialize the executor
//	for running.  It does not change the execState, that will be done when 
//	the application calls Run() or Step().
//
//	By default if a specific test is being run, then only that test will 
//	run and execSingleTest will be set.  The application can reset it at any 
//	before it completes and the executor will continue to the end of the 
//	script.
//

void ScriptExecutor::Setup( VTSDocPtr vdp, ScriptDocumentPtr sdp, ScriptTestPtr stp )
{
	// if the executor is already initialized, exit
	if (execState != execIdle) {
		TRACE0( "ScriptExecutor::Setup() error, not idle\n" );
		return;
	}

	// initialize
	execDB = vdp;
	execDoc = sdp;
	execTest = stp;

	// see if all tests should execute, default to run 
	execAllTests = (stp == 0);
	execSingleStep = false;
	execPending = false;

	// there is no current packet
	execPacket = 0;

	// let the document know that a script will be writing messages
	execDoc->BindExecutor();
}

//
//	ScriptExecutor::Cleanup
//
//	This member function is called when the executor has completed the 
//	testing process for some reason (success or failure).  It will let 
//	the bound document know that it will no longer receive messages.
//

void ScriptExecutor::Cleanup( void )
{
	TRACE0( "Executor::Cleanup()\n" );

	// make sure we don't get rescheduled
	SuspendTask();

	// check to run the next test
	if (execAllTests && execTest->testNext) {
		// move to the next test
		execTest = execTest->testNext;
		execPacket = 0;

		// install the task
		taskType = oneShotTask;
		taskInterval = 0;
		InstallTask();
	} else {
		// release from the document
		execDoc->UnbindExecutor();

		// clear all the rest of the execution vars
		execState = execIdle;
		execDB = 0;
		execDoc = 0;
		execTest = 0;
		execPacket = 0;
	}
}

//
//	ScriptExecutor::IsBound
//
//	This function returns true iff the executor is bound to a specific 
//	document.  Unlike the other simpler functions IsIdle(), IsRunning()
//	and IsStopped(), this one has two things to check and just to make 
//	sure the executor state doesn't change in the middle of the call, 
//	this is locked (in the main thread context).
//

bool ScriptExecutor::IsBound( ScriptDocumentPtr sdp )
{
	execCS.Lock();
	bool rslt = ((execState != execIdle) && (execDoc == sdp));
	execCS.Unlock();

	return rslt;
}

//
//	ScriptExecutor::IsBound
//
//	This function returns true iff the executor is bound to a specific 
//	database.  This is used when the database is closed, the test will be 
//	killed if it is.
//

bool ScriptExecutor::IsBound( VTSDocPtr vdp )
{
	execCS.Lock();
	bool rslt = ((execState != execIdle) && (execDB == vdp));
	execCS.Unlock();

	return rslt;
}

//
//	ScriptExecutor::Msg
//
//	Save a messsage related to script execution.  There are three "severity code" levels:
//
//		1	- test start/halt/pass/fail
//		2	- operator action (step, step/pass/fail)
//		3	- script info (delay expired, parm mismatch)
//

void ScriptExecutor::Msg( int sc, int line, const char *msg )
{
	unsigned char	buff[256], *dst
	;
	VTSPacket		pkt
	;
	
	TRACE3( "[%d:%d] %s\n", sc, line, msg );

	// save the severity code
	buff[0] = sc;

	// save the line number
	buff[1] = (line >> 24) & 0xFF;
	buff[2] = (line >> 16) & 0xFF;
	buff[3] = (line >> 8) & 0xFF;
	buff[4] = (line & 0xFF);

	// save the digest
	memcpy( buff+5, execDoc->m_digest, 16 );

	// for test level messages start with test name
	dst = buff + 21;
	if (sc == 1) {
		strcpy( (char *)dst, "Test " );
		dst += 5;

		strcpy( (char *)dst, execTest->baseLabel );
		while (*dst) dst++;

		// add a space
		*dst++ = ' ';
	}
	strcpy( (char *)dst, msg );
	while (*dst) dst++;

	// fill in the packet header
	pkt.packetHdr.packetPortID = 0;
	pkt.packetHdr.packetProtocolID = (int)BACnetPIInfo::ProtocolType::msgProtocol;
	pkt.packetHdr.packetFlags = 0;
	pkt.packetHdr.packetType = msgData;

	// let the packet refer to the pdu contents, cast away const
	pkt.NewDataRef( buff, (dst - buff) + 1 );

	// save it in the database;
	execDB->m_pDB->WritePacket( -1, pkt );

	// tell the application
	if (execDB->m_postMessages)
		::PostThreadMessage( AfxGetApp()->m_nThreadID
			, WM_VTS_RCOUNT, (WPARAM)0, (LPARAM)execDB
			);
}

//
//	ScriptExecutor::Run
//
//	This member function is called by the main thread in response to a run 
//	request (as selected in the menu).  The application has called Setup() 
//	to bind the executor to the correct document.
//

void ScriptExecutor::Run( void )
{
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// verify the correct state
	if (execState != execIdle) {
		TRACE0( "Error: invalid executor state\n" );
		return;
	}

	// clear the single step flag
	execSingleStep = false;

	// install the task
	taskType = oneShotTask;
	taskInterval = 0;
	InstallTask();

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::Halt
//

void ScriptExecutor::Halt( void )
{
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// verify the correct state
	if (execState != execRunning) {
		TRACE0( "Error: invalid executor state\n" );
		return;
	}

	Msg( 1, execTest->baseLineStart, "Halt" );

	// change the state to stopped
	execState = execStopped;

	// pull the task from the installed list (effecively canceling a timer)
	SuspendTask();
	execPending = false;

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::Step
//

void ScriptExecutor::Step( void )
{
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// verify the correct state
	if (execState == execRunning) {
		TRACE0( "Error: invalid executor state\n" );
		return;
	}

	Msg( 2, execTest->baseLineStart, "Step" );

	// set the single step flag
	execSingleStep = true;
	execPending = false;

	// install the task
	taskType = oneShotTask;
	taskInterval = 0;
	InstallTask();

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::Step
//

void ScriptExecutor::Step( bool pass )
{
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// verify the correct state
	if (execState != execStopped) {
		TRACE0( "Error: invalid executor state\n" );
		return;
	}

	if (pass)
		Msg( 2, execTest->baseLineStart, "Step Pass" );
	else
		Msg( 2, execTest->baseLineStart, "Step Fail" );

	// set the single step flag
	execSingleStep = true;
	execPending = false;
	execStepForced = (pass ? 1 : 2);

	// install the task
	taskType = oneShotTask;
	taskInterval = 0;
	InstallTask();

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::Resume
//

void ScriptExecutor::Resume( void )
{
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// verify the correct state
	if (execState != execStopped) {
		TRACE0( "Error: invalid executor state\n" );
		return;
	}

	Msg( 2, execTest->baseLineStart, "Resume" );

	// clear the single step flag
	execSingleStep = false;
	execPending = false;

	// install the task
	taskType = oneShotTask;
	taskInterval = 0;
	InstallTask();

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::Kill
//

void ScriptExecutor::Kill( void )
{
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// verify the correct state
	if (execState == execIdle) {
		TRACE0( "Error: invalid executor state\n" );
		return;
	}

	// tell the database the test was killed
	Msg( 1, execTest->baseLineStart, "Kill" );

	// pull the task from the installed list (effecively canceling a timer)
	SuspendTask();

	// set the status
	SetPacketStatus( execPacket, 3 );
	SetTestStatus( execTest, 3 );

	// make sure the doc knows we're done
	Cleanup();

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::ReadMsg
//
//	This function provides the application access to the executor message queue 
//	which is protected.
//

ScriptExecMsgPtr ScriptExecutor::ReadMsg( void )
{
	return execMsgQueue.Read();
}

//
//	ScriptExecutor::ProcessTask
//
//	This member function is called by the task manager when the task 
//	should do something that it was scheduled to do.  In the case of 
//	the executor, this means that a timer has expired and a receive 
//	packet attempt has failed.
//

void ScriptExecutor::ProcessTask( void )
{
	int				now = ::GetTickCount()
	;
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// if no test, get the first one
	if (!execTest) {
		// must be bound to a document
		if (!execDoc) {
			// alert the user
			TRACE0( "***** No document bound" );
			return;
		}

		// make sure the document has content
		if (!execDoc->m_pContentTree || !execDoc->m_pContentTree->m_pScriptContent) {
			// alert the user
			Msg( 2, 0, "No test to run, check syntax" );

			// go back to idle
			Cleanup();
			return;
		}

		// find the first test of section that has one
		ScriptBasePtr sbp = execDoc->m_pContentTree->m_pScriptContent;
		for (int i = 0; i < sbp->Length(); i++) {
			ScriptSectionPtr ssp = (ScriptSectionPtr)sbp->Child( i );

			if (ssp->Length() != 0) {
				execTest = (ScriptTestPtr)ssp->Child( 0 );
				break;
			}
		}
		if (!execTest) {
			// alert the user
			Msg( 2, 0, "No test to run, check syntax" );

			// go back to idle
			Cleanup();
			return;
		}
	}

	// if no packet, get first one
	if (!execPacket) {
		// reset the test
		ResetTest( execTest );

		// calculate the new status
		int newStatus = CalcTestStatus( execTest );
		if (newStatus) {
			// alert the user
			Msg( 1, execTest->baseLineStart, "failed, check dependencies" );

			// set the status
			SetTestStatus( execTest, newStatus );

			// go back to idle
			Cleanup();
			return;
		}

		// extract the first packet
		execPacket = execTest->testFirstPacket;

		// if no packets in test, it succeeds easily
		if (!execPacket) {
			// alert the user
			Msg( 1, execTest->baseLineStart, "trivial test successful" );

			// set the status
			SetTestStatus( execTest, 1 );

			// go back to idle
			Cleanup();
			return;
		}

		// message to the database
		Msg( 1, execTest->baseLineStart, "started" );
		Msg( 3, execTest->baseLineStart, execDoc->GetPathName() );

		// set the test status to running
		SetTestStatus( execTest, 2 );

		// if single stepping, go to stopped
		if (execSingleStep) {
			// set the packet status to running
			SetPacketStatus( execPacket, 2 );

			execState = execStopped;
			return;
		}
	}

	// executor running
	execState = execRunning;

keepGoing:
	// execute the current packet
	if (execStepForced == 1) {
		execStepForced = 0;
		NextPacket( true );
	} else
	if (execStepForced == 2) {
		execStepForced = 0;
		NextPacket( false );
	} else
	if (execPacket->packetType == ScriptPacket::sendPacket) {
		if (execPending || (execPacket->packetDelay == 0)) {
			execPending = false;

			// do the packet
			NextPacket( SendPacket() );

			// might be more to do.  The goto sucks, but rather than reschedule with 
			// a zero delay, keep the executor locked down until the transition to 
			// the next packet can be completed.  This prevents packets that come in 
			// faster than the reschedule from being processed by ReceiveNPDU.
			if (execPacket)
				goto keepGoing;
		} else {
			if (execPacket->packetSubtype == ScriptPacket::rootPacket) {
				TRACE2( "Send %08X root time %d\n", execPacket, now );
				execRootTime = now;
			}

			int delay = execPacket->packetDelay - (now - execRootTime);

			// proposed delay may have expired
			if (delay < 0) {
				TRACE1( "Send delay expired %d (2)\n", delay );
				Msg( 3, execPacket->baseLineStart, "Delay expired" );
				NextPacket( false );
			} else {
				// set the status pending
				SetPacketStatus( execPacket, 2 );

				// come back later
				TRACE1( "Send delay %d (2)\n", delay );
				execPending = true;
				taskType = oneShotTask;
				taskInterval = delay;
				InstallTask();
			}
		}
	} else
	if (execPacket->packetType == ScriptPacket::expectPacket) {
		if (execPending) {
			bool gotOne = false;

			// fail all the packets that have expired and min delay of rest
			int minDelay1 = kMaxPacketDelay + 1;
			for (ScriptPacketPtr pp1 = execPacket; pp1; pp1 = pp1->packetFail)
				if (pp1->baseStatus == 2)
					if (pp1->packetDelay <= (now - execRootTime))
						SetPacketStatus( pp1, 3 );
					else {
						gotOne = true;
						minDelay1 = (pp1->packetDelay < minDelay1 ? pp1->packetDelay : minDelay1);
					}

			// still one that hasn't failed?
			if (!gotOne) {
				Msg( 1, execTest->baseLineStart, "failed" );
				SetTestStatus( execTest, 3 );

				// go back to idle
				Cleanup();
			} else {
				// subtract time already expired
				minDelay1 -= (now - execRootTime);

				// schedule for the next delay
				TRACE1( "Expect more delay %d\n", minDelay1 );
				execPending = true;
				taskType = oneShotTask;
				taskInterval = minDelay1;
				InstallTask();
			}
		} else {
			// set the root time, always a root packet here
			TRACE2( "Expect %08X root time %d\n", execPacket, now );
			execRootTime = now;

			// set all the packets pending and find min delay
			int minDelay2 = kMaxPacketDelay + 1;
			for (ScriptPacketPtr pp2 = execPacket; pp2; pp2 = pp2->packetFail) {
				SetPacketStatus( pp2, 2 );
				minDelay2 = (pp2->packetDelay < minDelay2 ? pp2->packetDelay : minDelay2);
			}
			
			// schedule for the next delay
			TRACE1( "Expect delay %d\n", minDelay2 );
			execPending = true;
			taskType = oneShotTask;
			taskInterval = minDelay2;
			InstallTask();
		}
	}

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::NextPacket
//

void ScriptExecutor::NextPacket( bool okPacket )
{
	TRACE2( "Packet %08X %s\n", execPacket, (okPacket ? "succeeded" : "failed") );

	// set the current packet status
	SetPacketStatus( execPacket, okPacket ? 1 : 3 );

	// move to next packet
	execPacket = (okPacket ? execPacket->packetPass : execPacket->packetFail);

	// if no next packet, set test status
	if (!execPacket) {
		// set the test status
		if (okPacket) {
			Msg( 1, execTest->baseLineStart, "passed" );
			SetTestStatus( execTest, 1 );
		} else {
			Msg( 1, execTest->baseLineStart, "failed" );
			SetTestStatus( execTest, 3 );
		}

		// go back to idle
		Cleanup();
	} else {
		// set the current packet status to running
		SetPacketStatus( execPacket, 2 );

		// if the user is single stepping, stop
		if (execSingleStep)
			execState = execStopped;
		else {
			// schedule it to keep running
			taskType = oneShotTask;
			taskInterval = 0;
			InstallTask();
		}
	}
}

//
//	ScriptExecutor::ResolveExpr
//
//	Given a comma separated list of values and paramter names, this function 
//	turns it into an array of tokens.  If a parameter name is found, the 
//	parameter value is scanned recursively.
//

void ScriptExecutor::ResolveExpr( const char *expr, int exprLine, ScriptTokenList &lst )
{
	ScriptScanner	scan( expr )
	;
	ScriptToken		tok
	;
	ScriptParmPtr	pp
	;
	
	// start at the beginning
	scan.Next( tok );
	if (tok.tokenType == scriptEOL)
		return;

	for (;;) {
		// check the value
		switch (tok.tokenType) {
			case scriptKeyword:
				// resolve the keyword as a parameter?
				if ((pp = execDoc->m_pParmList->LookupParm( tok.tokenSymbol )) != 0) {
					ResolveExpr( pp->parmValue, pp->parmLine, lst );
					break;
				}

			case scriptValue:
				// save the value
				lst.Append( tok );
				break;

			case scriptReference:
				// look up reference in EPICS database

				lst.Append( tok );
				break;

			default:
				throw ExecError( "Value expected", exprLine );
		}

		// check for EOL
		scan.Next( tok );
		if (tok.tokenType == scriptEOL)
			break;

		// check for more
		if ((tok.tokenType != scriptSymbol) || (tok.tokenSymbol != ','))
			throw ExecError( "End-of-line or comma expected in value list", exprLine );

		// get ready for next value
		scan.Next( tok );
	}

	// token list is built.  Now go through and resolve all of the index items on each token and
	// set that final index value into the top level token.

	// allow cases of {property[int]}, {property[var]}, {property[{property}]}, {property[{property[int]}]}, etc.
	// Don't allow cases (yet) of var[var], var[int], var[etc.]

	for ( int i = 0; i < lst.GetCount(); i++ )
		lst[i].ResolveIndex(execDoc->m_pParmList);
}




//
//	ScriptExecutor::GetEPICSProperty
//
//	Given a property keyword, this function will attempt to get the value of the 
//	property from the EPICS database.
//  Throw a string exception if there is a problem

void ScriptExecutor::GetEPICSProperty( int prop, BACnetAnyValue * pbacnetAny, int nIndex /* = -1 */ )
{
	int						pid, pindx;
	PICS::generic_object	*pobj;

	// find the property
	pid = ScriptToken::Lookup( prop, ScriptPropertyMap );
	if (pid < 0)
		throw "No such property";

	// verify the context
	if (execObjID == 0xFFFFFFFF)
		throw "No object reference";

	// make sure there's something to search
	if (!gPICSdb)
		throw "No EPICS information loaded";

	// get a pointer to the object database record
	pobj = PICS::GetpObj( gPICSdb->Database, execObjID );
	if (!pobj)
		throw "Object not defined in EPICS";

	// get the index of the property flags
	pindx = PICS::GetPropIndex( pobj->object_type, pid );
	if (pindx < 0)
		throw "Property not defined for this object type";

	// see if the value is not available
	if (pobj->propflags[pindx] & ValueUnknown)
		throw "EPICS property is not specified";

	// Access DUDAPI call to point to yucky part of EPICS internal data structures
	PICS::CreatePropertyFromEPICS( pobj, pid, pbacnetAny );

	// assign real value to this array pointer... doesn't matter here if it's an actual
	// array or not... just pretent.  Sure.  I knew you could.

	BACnetGenericArray * pbacnetarray = (BACnetGenericArray *) pbacnetAny->GetObject();

	if ( pbacnetarray == NULL )
		throw "Property value not known or comparison for this type not implemented";

	// see if caller really wants just an element here.  If so, (won't be -1), then
	// grab a copy of the element and destroy the rest (taken care of by resetting object).
	// only do this for list and array types.
	// ALL must decend from GenericArray

	if ( nIndex != -1  &&  pbacnetarray->IsKindOf(RUNTIME_CLASS(BACnetGenericArray)) )
	{
		if ( nIndex == 0 )				// return number of elements only
			pbacnetAny->SetObject(uw, new BACnetUnsigned(pbacnetarray->GetSize()) );
		else
		{
			if ( nIndex > pbacnetarray->GetSize() )			// out of bounds?  Remember index is base 1
			{
				CString str;
				str.Format(IDS_SCREX_INDEXBOUNDS, nIndex, pbacnetarray->GetSize());
				throw CString(str);
			}

			pbacnetAny->SetObject((*pbacnetarray)[nIndex-1].clone());
		}
	}
}


//
//	ScriptExecutor::SendPacket
//
//	Construct a packet out of the information in execPacket and send it.  Return true 
//	if the send was successful.  The only time it should fail is if there is something 
//	wrong with its construction (or something like the network missing).
//

bool ScriptExecutor::SendPacket( void )
{
	int						bipMsgID = -1, nlMsgID = -1, alMsgID = -1
	;
	BACnetOctet				ctrl = 0
	;
	CByteArray				packet
	;
	ScriptFilterPtr			sfp
	;
	ScriptNetFilterPtr		nfp
	;
	ScriptPacketExprPtr		pNet, bipMsg, nlMsg, nlDA, alMsg
	,						pVersion, pDNET, pDADR, pHopCount, pSNET, pSADR
	,						pDER, pPriority
	;
	BACnetAddress			nlDestAddr
	;
	BACnetBoolean			nlDER
	;
	BACnetInteger			nlVersion, nlDNET, nlSNET
	,						nlPriority, nlHopCount
	;
	BACnetOctetString		nlDest, nlDADR, nlSADR, nlData, alData
	;
	BACnetCharacterString	nlNetwork
	;

	try {
		// see if the network is provided
		pNet = GetKeywordValue( kwNETWORK, nlNetwork );

		// see if this has a BVLCI header
		bipMsg = execPacket->packetExprList.Find( kwBVLCI );

		// see if this is a network layer message
		nlMsg = execPacket->packetExprList.Find( kwMESSAGE );
		if (!nlMsg)
			nlMsg = execPacket->packetExprList.Find( kwMSG );

		// see if this is an application layer message
		alMsg = execPacket->packetExprList.Find( kwPDU );

		// see if this should be sent in the context of a device
		if (!pNet && !bipMsg && !nlMsg && alMsg) {
			SendDevPacket();
			return true;
		}

		// look for the network (actually the filter name)
		if (gMasterFilterList.Length() == 1) {
			sfp = gMasterFilterList[0];
			if (pNet && !nlNetwork.Equals(sfp->filterName))
				throw ExecError( "Port not found", pNet->exprLine );
		} else {
			if (!pNet)
				throw ExecError( "Network required", execPacket->baseLineStart );

			for (int i = 0; i < gMasterFilterList.Length(); i++) {
				sfp = gMasterFilterList[i];
				if (nlNetwork.Equals(sfp->filterName))
					break;
			}
			if (i >= gMasterFilterList.Length())
				throw ExecError( "Port not found", pNet->exprLine );
		}

		// make sure its a network filter for now
		if (sfp->filterType != scriptNPDUFilter)
			throw ExecError( "NPDU filter only", pNet->exprLine );
		nfp = (ScriptNetFilterPtr)sfp;

		// get the destination
		nlDA = execPacket->packetExprList.Find( kwDA );
		if (!nlDA) {
			for (int i = 0; i < execDB->m_Names.Length(); i++ ) {
				VTSNameDesc		nameDesc;

				execDB->m_Names.ReadName( i, &nameDesc );
				if (stricmp(nameDesc.nameName,"IUT") == 0) {
					nlDestAddr = nameDesc.nameAddr;
					break;
				}
			}
			if (i >= execDB->m_Names.Length())
				throw ExecError( "Default destination address IUT not found", execPacket->baseLineStart );
		} else {
			ScriptTokenList			daList
			;

			// resolve the expressions
			ResolveExpr( nlDA->exprValue, nlDA->exprLine, daList );
			if (daList.Length() < 1)
				throw ExecError( "Address, name or keyword expected", nlDA->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = daList[0];

			// check to see if this is a keyword
			if (t.tokenType == scriptKeyword) {
				if ((t.tokenSymbol == kwBROADCAST) || (t.tokenSymbol == kwLOCALBROADCAST))
					nlDestAddr.LocalBroadcast();
				else
					throw ExecError( "Unrecognized keyword", nlDA->exprLine );
			} else
			// it might be a name
			if ((t.tokenType == scriptValue) && (t.tokenEnc == scriptASCIIEnc)) {
				CString tvalu = t.RemoveQuotes();
				for (int i = 0; i < execDB->m_Names.Length(); i++ ) {
					VTSNameDesc		nameDesc;

					execDB->m_Names.ReadName( i, &nameDesc );
					if (stricmp(nameDesc.nameName,tvalu) == 0) {
						nlDestAddr = nameDesc.nameAddr;
						break;
					}
				}
				if (i >= execDB->m_Names.Length())
					throw ExecError( "Destination address name not found", nlDA->exprLine );
			} else
			// it might be an IP address
			if ((t.tokenType == scriptValue) && (t.tokenEnc == scriptIPEnc)) {
				BACnetIPAddr nlIPAddr( t.tokenValue );
				nlDestAddr = nlIPAddr;
			} else
			// it might be an explicit octet string
			if (t.IsEncodeable( nlDest )) {
				nlDestAddr.LocalStation( nlDest.strBuff, nlDest.strLen );
			} else
				throw ExecError( "Destination address expected", nlDA->exprLine );
		}

		// get some interesting keywords in case they should be used in BIP messages
		pDER		= GetKeywordValue( kwDER, nlDER );
		pPriority	= GetKeywordValue( kwPRIORITY, nlPriority );
		if (!pPriority)
			pPriority = GetKeywordValue( kwPRIO, nlPriority );
		if (!pPriority)
			nlPriority.intValue = 0;
		else
		if ((nlPriority.intValue < 0) || (nlPriority.intValue > 3))
			throw ExecError( "Priority out of range 0..3", pPriority->exprLine );

		// if this is a BVLL specific message, encode and send it
		if (bipMsg) {
			ScriptTokenList	bvllList;

			// resolve the expressions
			ResolveExpr( bipMsg->exprValue, bipMsg->exprLine, bvllList );
			if (bvllList.Length() < 1)
				throw ExecError( "BIP message keyword expected", bipMsg->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = bvllList[0];

			// check to see if this is a keyword
			if (t.tokenType == scriptKeyword) {
				bipMsgID = t.Lookup( t.tokenSymbol, ScriptBIPMsgTypeMap );
				if (bipMsgID < 0)
					throw ExecError( "Unrecognized keyword", bipMsg->exprLine );
			} else
				throw ExecError( "Keyword expected", bipMsg->exprLine );

			// based on the number, check for other parameters
			try {
				// encode the message type and function
				packet.Add( 0x81 );
				packet.Add( bipMsgID );

				// hold space for the length
				packet.Add( 0x00 );
				packet.Add( 0x00 );

				switch (bipMsgID) {
					case 0:						// BVLC-RESULT, rslt
						SendBVLCResult( bvllList, packet );
						break;
					case 1:						// WRITE-BROADCAST-DISTRIBUTION-TABLE {,host/net:port}*
						SendWriteBDT( bvllList, packet );
						break;
					case 2:						// READ-BROADCAST-DISTRIBUTION-TABLE
						if (bvllList.Length() != 1)
							throw "BVLC function takes no parameters";
						break;
					case 3:						// READ-BROADCAST-DISTRIBUTION-TABLE-ACK {,host/net:port}*
						SendReadBDTAck( bvllList, packet );
						break;
					case 4:						// FORWARDED-NPDU, host:port
						SendForwardedNPDU( bvllList, packet );
						break;
					case 5:						// REGISTER-FOREIGN-DEVICE, ttl
						SendRegisterFD( bvllList, packet );
						break;
					case 6:						// READ-FOREIGN-DEVICE-TABLE
						if (bvllList.Length() != 1)
							throw "BVLC function takes no parameters";
						break;
					case 7:						// READ-FOREIGN-DEVICE-TABLE-ACK {,host:port,ttl,tr}*
						SendReadFDTAck( bvllList, packet );
						break;
					case 8:						// DELETE-FOREIGN-DEVICE-TABLE-ENTRY
						SendDeleteFDTEntry( bvllList, packet );
						break;
					case 9:						// DISTRIBUTE-BROADCAST-TO-NETWORK
					case 10:					// ORIGINAL-UNICAST-NPDU
					case 11:					// ORIGINAL-BROADCAST-NPDU
						if (bvllList.Length() != 1)
							throw "BVLC function takes no parameters";
						break;
				}

				// clean up the length
				int len = packet.GetSize();
				packet[2] = (len >> 8);
				packet[3] = (len & 0xFF);
			}
			catch (const char *errMsg) {
				// one of the functions had an error
				throw ExecError( errMsg, bipMsg->exprLine );
			}

			// if this doesn't have an NPDU that follows, send it
			if ((bipMsgID != 4) && (bipMsgID != 9) && (bipMsgID != 10) && (bipMsgID != 11)) {
				nfp->Request(
					BACnetNPDU( nlDestAddr
						, packet.GetData(), packet.GetSize()
						, (pDER && nlDER.boolValue ? 1 : 0)
						, nlPriority.intValue
						)
					);

				// success
				return true;
			}
		}

		// there must be a network or application layer message
		if (!nlMsg && !alMsg)
			throw "Network or application message keyword required";
		if (nlMsg && alMsg)
			throw "Specify network or application message but not both";

		// find the interesting keywords
		pVersion	= GetKeywordValue( kwVERSION, nlVersion );
		pDNET		= GetKeywordValue( kwDNET, nlDNET );
		pDADR		= GetKeywordValue( kwDADR, nlDADR );
		pHopCount	= GetKeywordValue( kwHOPCOUNT, nlHopCount );
		pSNET		= GetKeywordValue( kwSNET, nlSNET );
		pSADR		= GetKeywordValue( kwSADR, nlSADR );

		// validate version
		if (pVersion && ((nlVersion.intValue < 0) || (nlVersion.intValue > 255)))
			throw ExecError( "Version out of range 0..255", pVersion->exprLine );

		if (pDNET || pDADR) {
			if (!pDNET)
				throw ExecError( "Destination network required (65535 = global broadcast)", pDADR->exprLine );
			if ((nlDNET.intValue < 0) || (nlDNET.intValue > 65535))
				throw ExecError( "Destination network out of range 0..65535", pDNET->exprLine );
//			if ((nlDNET.intValue != 65535) && ((!pDADR) || (nlDADR.strLen == 0)))
//				throw ExecError( "DADR required when DNET is not global broadcast", pDNET->exprLine );
			if (!pHopCount) {
				Msg( 3, execPacket->baseLineStart, "Hopcount defaulting to 255" );
				nlHopCount.intValue = 255;
			} else
			if ((nlHopCount.intValue < 0) || (nlHopCount.intValue > 255))
				throw ExecError( "Hop count out of range 0..255", pHopCount->exprLine );
		}
		if (pSNET || pSADR) {
			if (!pSNET)
				throw ExecError( "Source network required", pSADR->exprLine );
			if ((nlSNET.intValue < 0) || (nlSNET.intValue > 65534))
				throw ExecError( "Source network out of range 0..65534", pSNET->exprLine );
			if ((!pSADR) || (nlSADR.strLen == 0))
				throw ExecError( "SADR required when SNET is present", pSNET->exprLine );
		}

		// start with the version
		packet.Add( pVersion ? nlVersion.intValue : 1 );

		// build the control field
		if (nlMsg)
			ctrl |= 0x80;
		if (pDNET)
			ctrl |= 0x20;
		if (pSNET)
			ctrl |= 0x08;
		if (pDER && nlDER.boolValue)
			ctrl |= 0x04;
		ctrl |= nlPriority.intValue;

		// add the control field
		packet.Add( ctrl );

		// add the destination stuff
		if (pDNET) {
			packet.Add( nlDNET.intValue >> 8 );
			packet.Add( nlDNET.intValue & 0x0FF );
			if (pDADR) {
				packet.Add( nlDADR.strLen );
				for (int i = 0; i < nlDADR.strLen; i++)
					packet.Add( nlDADR.strBuff[i] );
			} else
				packet.Add( 0x00 );
		}

		// add the source stuff
		if (pSNET) {
			packet.Add( nlSNET.intValue >> 8 );
			packet.Add( nlSNET.intValue & 0x0FF );
			packet.Add( nlSADR.strLen );
			for (int i = 0; i < nlSADR.strLen; i++)
				packet.Add( nlSADR.strBuff[i] );
		}

		// add the hop count
		if (pDNET)
			packet.Add( nlHopCount.intValue );

		// if this is a network layer message, encode it
		if (nlMsg) {
			ScriptTokenList	nlList;

			// resolve the expressions
			ResolveExpr( nlMsg->exprValue, nlMsg->exprLine, nlList );
			if (nlList.Length() < 1)
				throw ExecError( "NL message keyword expected", nlMsg->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = nlList[0];

			// check to see if this is a keyword
			if (t.tokenType == scriptKeyword) {
				nlMsgID = t.Lookup( t.tokenSymbol, ScriptNLMsgTypeMap );
				if (nlMsgID < 0)
					throw ExecError( "Unrecognized keyword", nlMsg->exprLine );
			} else
			// it might be an explicit number
			if (t.IsInteger( nlMsgID )) {
				if ((nlMsgID < 0) || (nlMsgID > 255))
					throw ExecError( "Network message number out of range (0..255)", nlMsg->exprLine );
			} else
				throw ExecError( "Keyword or network message expected", nlMsg->exprLine );

			// add the message to the packet
			packet.Add( nlMsgID );

			// based on the number, check for other parameters
			try {
				switch (nlMsgID) {
					case 0:						// WHO-IS-ROUTER-TO-NETWORK
						SendWhoIsRouterToNetwork( nlList, packet );
						break;
					case 1:						// I-AM-ROUTER-TO-NETWORK
						SendIAmRouterToNetwork( nlList, packet );
						break;
					case 2:						// I-COULD-BE-ROUTER-TO-NETWORK
						SendICouldBeRouterToNetwork( nlList, packet );
						break;
					case 3:						// REJECT-MESSAGE-TO-NETWORK
						SendRejectMessageToNetwork( nlList, packet );
						break;
					case 4:						// ROUTER-BUSY-TO-NETWORK
						SendRouterBusyToNetwork( nlList, packet );
						break;
					case 5:						// ROUTER-AVAILABLE-TO-NETWORK
						SendRouterAvailableToNetwork( nlList, packet );
						break;
					case 6:						// INITIALIZE-ROUTING-TABLE
					case 7:						// INITIALIZE-ROUTING-TABLE-ACK
						SendInitializeRoutingTable( nlList, packet );
						break;
					case 8:						// ESTABLISH-CONNECTION-TO-NETWORK
						SendEstablishConnectionToNetwork( nlList, packet );
						break;
					case 9:						// DISCONNECT-CONNECTION-TO-NETWORK
						SendDisconnectConnectionToNetwork( nlList, packet );
						break;
				}
			}
			catch (const char *errMsg) {
				// one of the functions had an error
				throw ExecError( errMsg, nlMsg->exprLine );
			}

			// look for optional network layer data
			if (GetKeywordValue( kwNLDATA, nlData ) || GetKeywordValue( kwNL, nlData )) {
				TRACE0( "Got additional network layer octet string\n" );
				for (int i = 0; i < nlData.strLen; i++)
					packet.Add( nlData.strBuff[i] );
			}
		}

		// if this is an application layer message, encode it
		if (alMsg) {
			ScriptTokenList	alList;

			// force property references to fail until context established
			execObjID = 0xFFFFFFFF;

			// resolve the expressions
			ResolveExpr( alMsg->exprValue, alMsg->exprLine, alList );
			if (alList.Length() < 1)
				throw ExecError( "AL message keyword expected", alMsg->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = alList[0];

			// check to see if this is a keyword
			if (t.tokenType == scriptKeyword) {
				alMsgID = t.Lookup( t.tokenSymbol, ScriptALMsgTypeMap );
				if (alMsgID < 0)
					throw ExecError( "Unrecognized keyword", alMsg->exprLine );
			} else
				throw ExecError( "Keyword expected", alMsg->exprLine );

			// based on the number, check for other parameters
			try {
				switch (alMsgID) {
					case 0:						// CONFIRMED-REQUEST
						SendConfirmedRequest( packet );
						break;
					case 1:						// UNCONFIRMED-REQUEST
						SendUnconfirmedRequest( packet );
						break;
					case 2:						// SIMPLEACK
						SendSimpleACK( packet );
						break;
					case 3:						// COMPLEXACK
						SendComplexACK( packet );
						break;
					case 4:						// SEGMENTACK
						SendSegmentACK( packet );
						break;
					case 5:						// ERROR
						SendError( packet );
						break;
					case 6:						// REJECT
						SendReject( packet );
						break;
					case 7:						// ABORT
						SendAbort( packet );
						break;
				}
			}
			catch (const char *errMsg) {
				// one of the functions had an error
				throw ExecError( errMsg, alMsg->exprLine );
			}
		}

		// check to see if there was a BVCL prefix
		if ((bipMsgID == 4) || (bipMsgID == 9) || (bipMsgID == 10) || (bipMsgID == 11)) {
			// clean up the length
			int len = packet.GetSize();
			packet[2] = (len >> 8);
			packet[3] = (len & 0xFF);
		}

		// send it
		nfp->Request(
			BACnetNPDU( nlDestAddr
				, packet.GetData(), packet.GetSize()
				, (pDER && nlDER.boolValue ? 1 : 0)
				, nlPriority.intValue
				)
			);
	}
	catch (const ExecError &err) {
		// failed
		Msg( 3, err.errLineNo, err.errMsg );
		return false;
	}
	catch (const char *errMsg) {
		// failed
		Msg( 3, -1, errMsg );
		return false;
	}

	// success
	return true;
}

//
//	ScriptExecutor::SendBVLCResult
//

void ScriptExecutor::SendBVLCResult( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;

	TRACE0( "SendBVLCResult\n" );

	switch (tlist.Length()) {
		case 1:
			throw "Result code expected";
		case 2:
			{
				const ScriptToken &dnet = tlist[1];

				if (dnet.tokenType != scriptValue)
					throw "Result code expected";
				if (!dnet.IsInteger(valu))
					throw "Result code invalid format";
				if ((valu < 0) || (valu > 65535))
					throw "Result code out of range (0..65535)";

				TRACE1( "    Result code %d\n", valu );
				packet.Add( valu >> 8 );
				packet.Add( valu & 0x0FF );
				break;
			}
		default:
			throw "Too many values in BVLC-Result message";
	}
}

//
//	ScriptExecutor::SendWriteBDT
//

void ScriptExecutor::SendWriteBDT( ScriptTokenList &tlist, CByteArray &packet )
{
	unsigned long		host, mask
	;
	unsigned short		port
	;

	TRACE0( "SendWriteBDT\n" );

	for (int i = 1; i < tlist.Length(); i++ ) {
		const ScriptToken &t = tlist[i];

		if (t.tokenType != scriptValue)
			throw "IP address expected";
		if (t.tokenEnc != scriptIPEnc)
			throw "IP address required";

		// convert to host, port and network mask
		BACnetIPAddr::StringToHostPort( t.tokenValue, &host, &mask, &port );

		// encode the host
		for (int j = 3; j >= 0; j--)
			packet.Add( (unsigned char)((host >> (j * 8)) & 0xFF) );

		// encode the port
		packet.Add( port >> 8 );
		packet.Add( port & 0xFF );

		// encode the mask
		for (int k = 3; k >= 0; k--)
			packet.Add( (unsigned char)((mask >> (k * 8)) & 0xFF) );
	}
}

//
//	ScriptExecutor::SendReadBDTAck
//

void ScriptExecutor::SendReadBDTAck( ScriptTokenList &tlist, CByteArray &packet )
{
	unsigned long		host, mask
	;
	unsigned short		port
	;

	TRACE0( "SendReadBDTAck\n" );

	for (int i = 1; i < tlist.Length(); i++ ) {
		const ScriptToken &t = tlist[i];

		if (t.tokenType != scriptValue)
			throw "IP address expected";
		if (t.tokenEnc != scriptIPEnc)
			throw "IP address required";

		// convert to host, port and network mask
		BACnetIPAddr::StringToHostPort( t.tokenValue, &host, &mask, &port );

		// encode the host
		for (int j = 3; j >= 0; j--)
			packet.Add( (unsigned char)((host >> (j * 8)) & 0xFF) );

		// encode the port
		packet.Add( port >> 8 );
		packet.Add( port & 0xFF );

		// encode the mask
		for (int k = 3; k >= 0; k--)
			packet.Add( (unsigned char)((mask >> (k * 8)) & 0xFF) );
	}
}

//
//	ScriptExecutor::SendForwardedNPDU
//

void ScriptExecutor::SendForwardedNPDU( ScriptTokenList &tlist, CByteArray &packet )
{
	TRACE0( "SendForwardedNPDU\n" );

	switch (tlist.Length()) {
		case 1:
			throw "IP address required";
		case 2:
			{
				unsigned long		host, mask
				;
				unsigned short		port
				;

				const ScriptToken &t = tlist[1];

				if (t.tokenType != scriptValue)
					throw "IP address expected";
				if (t.tokenEnc != scriptIPEnc)
					throw "IP address required";

				// convert to host, port and network mask
				BACnetIPAddr::StringToHostPort( t.tokenValue, &host, &mask, &port );

				// encode the host
				for (int j = 3; j >= 0; j--)
					packet.Add( (unsigned char)((host >> (j * 8)) & 0xFF) );

				// encode the port
				packet.Add( port >> 8 );
				packet.Add( port & 0xFF );

				break;
			}
		default:
			throw "Too many values";
	}
}

//
//	ScriptExecutor::SendRegisterFD
//

void ScriptExecutor::SendRegisterFD( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;

	TRACE0( "SendRegisterFD\n" );

	switch (tlist.Length()) {
		case 1:
			throw "Time-to-live required";
		case 2:
			{
				const ScriptToken &ttl = tlist[1];

				if (ttl.tokenType != scriptValue)
					throw "Time-to-live expected";
				if (!ttl.IsInteger(valu))
					throw "Time-to-live invalid format";
				if ((valu < 0) || (valu > 65535))
					throw "Time-to-live out of range (0..65535)";

				TRACE1( "    TTL %d\n", valu );
				packet.Add( valu >> 8 );
				packet.Add( valu & 0x0FF );
				break;
			}
		default:
			throw "Too many values";
	}
}

//
//	ScriptExecutor::SendReadFDTAck
//

void ScriptExecutor::SendReadFDTAck( ScriptTokenList &tlist, CByteArray &packet )
{
	int						valu
	;

	TRACE0( "SendReadFDTAck\n" );

	for (int i = 1; i < tlist.Length(); ) {
		unsigned long		host, mask
		;
		unsigned short		port
		;

		const ScriptToken &t = tlist[i++];

		if (t.tokenType != scriptValue)
			throw "IP address expected";
		if (t.tokenEnc != scriptIPEnc)
			throw "IP address required";

		// convert to host, port and network mask
		BACnetIPAddr::StringToHostPort( t.tokenValue, &host, &mask, &port );

		// encode the host
		for (int j = 3; j >= 0; j--)
			packet.Add( (unsigned char)((host >> (j * 8)) & 0xFF) );

		// encode the port
		packet.Add( port >> 8 );
		packet.Add( port & 0xFF );

		if (i >= tlist.Length())
			throw "Time-to-live expected";
		const ScriptToken &ttl = tlist[i++];

		if (ttl.tokenType != scriptValue)
			throw "Time-to-live expected";
		if (!ttl.IsInteger(valu))
			throw "Time-to-live invalid format";

		TRACE1( "    Time-to-live %d\n", valu );

		// encode the value
		packet.Add( valu >> 8 );
		packet.Add( valu & 0xFF );

		if (i >= tlist.Length())
			throw "Time remaining expected";
		const ScriptToken &tr = tlist[i++];

		if (tr.tokenType != scriptValue)
			throw "Time remaining expected";
		if (!tr.IsInteger(valu))
			throw "Time remaining invalid format";

		TRACE1( "    Time remaining %d\n", valu );

		// encode the value
		packet.Add( valu >> 8 );
		packet.Add( valu & 0xFF );
	}
}

//
//	ScriptExecutor::SendDeleteFDTEntry
//

void ScriptExecutor::SendDeleteFDTEntry( ScriptTokenList &tlist, CByteArray &packet )
{
	TRACE0( "SendDeleteFDTEntry\n" );

	switch (tlist.Length()) {
		case 1:
			throw "Foreign device IP address required";
		case 2:
			{
				unsigned long		host, mask
				;
				unsigned short		port
				;

				const ScriptToken &t = tlist[1];

				if (t.tokenType != scriptValue)
					throw "IP address expected";
				if (t.tokenEnc != scriptIPEnc)
					throw "IP address required";

				// convert to host, port and network mask
				BACnetIPAddr::StringToHostPort( t.tokenValue, &host, &mask, &port );

				// encode the host
				for (int j = 3; j >= 0; j--)
					packet.Add( (unsigned char)((host >> (j * 8)) & 0xFF) );

				// encode the port
				packet.Add( port >> 8 );
				packet.Add( port & 0xFF );

				break;
			}
		default:
			throw "Too many values";
	}
}

//
//	ScriptExecutor::SendWhoIsRouterToNetwork
//

void ScriptExecutor::SendWhoIsRouterToNetwork( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;

	TRACE0( "SendWhoIsRouterToNetwork\n" );

	switch (tlist.Length()) {
		case 1:
			break;
		case 2:
			{
				const ScriptToken &dnet = tlist[1];

				if (dnet.tokenType != scriptValue)
					throw "DNET expected";
				if (!dnet.IsInteger(valu))
					throw "DNET invalid format";
				if ((valu < 0) || (valu > 65534))
					throw "DNET out of range (0..65534)";

				TRACE1( "    Network %d\n", valu );
				packet.Add( valu >> 8 );
				packet.Add( valu & 0x0FF );
				break;
			}
		default:
			throw "Too many values in Who-Is-Router-To-Network message";
	}
}

//
//	ScriptExecutor::SendIAmRouterToNetwork
//

void ScriptExecutor::SendIAmRouterToNetwork( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;

	TRACE0( "SendIAmRouterToNetwork\n" );

	if (tlist.Length() == 1)
		throw "At least one network number required";

	for (int i = 1; i < tlist.Length(); i++ ) {
		const ScriptToken &t = tlist[i];

		if (t.tokenType != scriptValue)
			throw "Network number expected";
		if (!t.IsInteger(valu))
			throw "Network number invalid format";
		if ((valu < 0) || (valu > 65534))
			throw "DNET out of range (0..65534)";

		TRACE1( "    Network %d\n", valu );
		packet.Add( valu >> 8 );
		packet.Add( valu & 0x0FF );
	}
}

//
//	ScriptExecutor::SendICouldBeRouterToNetwork
//

void ScriptExecutor::SendICouldBeRouterToNetwork( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;

	TRACE0( "SendICouldBeRouterToNetwork\n" );

	if (tlist.Length() != 3)
		throw "DNET and performance index required";

	const ScriptToken &dnet = tlist[1];

	if (dnet.tokenType != scriptValue)
		throw "DNET expected";
	if (!dnet.IsInteger(valu))
		throw "DNET invalid format";
	if ((valu < 0) || (valu > 65534))
		throw "DNET out of range (0..65534)";

	TRACE1( "    Network %d\n", valu );
	packet.Add( valu >> 8 );
	packet.Add( valu & 0x0FF );

	const ScriptToken &perf = tlist[2];

	if (perf.tokenType != scriptValue)
		throw "Performance index expected";
	if (!perf.IsInteger(valu))
		throw "Performance index invalid format";
	if ((valu < 0) || (valu > 255))
		throw "Performance index out of range (0..255)";

	TRACE1( "    Performance %d\n", valu );
	packet.Add( valu );
}

//
//	ScriptExecutor::SendRejectMessageToNetwork
//

void ScriptExecutor::SendRejectMessageToNetwork( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;

	TRACE0( "SendRejectMessageToNetwork\n" );

	if (tlist.Length() != 3)
		throw "Reject reason and DNET required";

	const ScriptToken &reason = tlist[1];

	if (reason.tokenType != scriptValue)
		throw "Reject reason expected";
	if (!reason.IsInteger(valu))
		throw "Reject reason invalid format";
	if ((valu < 0) || (valu > 255))
		throw "Reject reason out of range (0..255)";

	TRACE1( "    Reject Reason %d\n", valu );
	packet.Add( valu );

	const ScriptToken &dnet = tlist[2];

	if (dnet.tokenType != scriptValue)
		throw "DNET expected";
	if (!dnet.IsInteger(valu))
		throw "DNET invalid format";
	if ((valu < 0) || (valu > 65534))
		throw "DNET out of range (0..65534)";

	TRACE1( "    Network %d\n", valu );
	packet.Add( valu >> 8 );
	packet.Add( valu & 0x0FF );
}

//
//	ScriptExecutor::SendRouterBusyToNetwork
//

void ScriptExecutor::SendRouterBusyToNetwork( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;
	
	TRACE0( "SendRouterBusyToNetwork\n" );
	
	for (int i = 1; i < tlist.Length(); i++ ) {
		const ScriptToken &dnet = tlist[i];

		if (dnet.tokenType != scriptValue)
			throw "Network expected";
		if (!dnet.IsInteger(valu))
			throw "Network invalid format";
		if ((valu < 0) || (valu > 65534))
			throw "Network out of range (0..65534)";

		TRACE1( "    Network %d\n", valu );
		packet.Add( valu >> 8 );
		packet.Add( valu & 0x0FF );
	}
}

//
//	ScriptExecutor::SendRouterAvailableToNetwork
//

void ScriptExecutor::SendRouterAvailableToNetwork( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;
	
	TRACE0( "SendRouterAvailableToNetwork\n" );

	for (int i = 1; i < tlist.Length(); i++ ) {
		const ScriptToken &dnet = tlist[i];

		if (dnet.tokenType != scriptValue)
			throw "DNET expected";
		if (!dnet.IsInteger(valu))
			throw "DNET invalid format";
		if ((valu < 0) || (valu > 65534))
			throw "DNET out of range (0..65534)";

		TRACE1( "    Network %d\n", valu );
		packet.Add( valu >> 8 );
		packet.Add( valu & 0x0FF );
	}
}

//
//	ScriptExecutor::SendInitializeRoutingTable
//

void ScriptExecutor::SendInitializeRoutingTable( ScriptTokenList &tlist, CByteArray &packet )
{
	int						valu
	;
	BACnetCharacterString	cstr
	;

	TRACE0( "SendInitializeRoutingTable[Ack]\n" );

	// append the number of ports
	packet.Add( (tlist.Length() - 1) / 3 );

	for (int i = 1; i < tlist.Length(); ) {
		const ScriptToken &dnet = tlist[i++];

		if (dnet.tokenType != scriptValue)
			throw "DNET expected";
		if (!dnet.IsInteger(valu))
			throw "DNET invalid format";
		if ((valu < 0) || (valu > 65534))
			throw "DNET out of range (0..65534)";

		TRACE1( "    Network %d\n", valu );
		packet.Add( valu >> 8 );
		packet.Add( valu & 0x0FF );

		if (i >= tlist.Length())
			throw "Port ID expected";
		const ScriptToken &portID = tlist[i++];

		if (portID.tokenType != scriptValue)
			throw "Port ID expected";
		if (!portID.IsInteger(valu))
			throw "Port ID invalid format";

		TRACE1( "    Port ID %d\n", valu );
		packet.Add( valu );

		if (i >= tlist.Length())
			throw "Port information expected";
		const ScriptToken &portInfo = tlist[i++];

		if (portInfo.tokenType != scriptValue)
			throw "Port information expected";
		if (!portInfo.IsEncodeable( cstr ))
			throw "Port information invalid format";
		if (cstr.strEncoding != 0)
			throw "Port information must be ASCII encoded";

		packet.Add( cstr.strLen );
		for (unsigned int j = 0; j < cstr.strLen; j++)
			packet.Add( cstr.strBuff[j] );
	}
}

//
//	ScriptExecutor::SendEstablishConnectionToNetwork
//

void ScriptExecutor::SendEstablishConnectionToNetwork( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;

	TRACE0( "SendEstablishConnectionToNetwork\n" );

	if (tlist.Length() != 3)
		throw "DNET and termination time required";

	const ScriptToken &dnet = tlist[1];

	if (dnet.tokenType != scriptValue)
		throw "DNET expected";
	if (!dnet.IsInteger(valu))
		throw "DNET invalid format";
	if ((valu < 0) || (valu > 65534))
		throw "DNET out of range (0..65534)";

	TRACE1( "    Network %d\n", valu );
	packet.Add( valu >> 8 );
	packet.Add( valu & 0x0FF );

	const ScriptToken &term = tlist[2];

	if (term.tokenType != scriptValue)
		throw "Termination time expected";
	if (!term.IsInteger(valu))
		throw "Termination time invalid format";

	TRACE1( "    Termination time %d\n", valu );
	packet.Add( valu );
}

//
//	ScriptExecutor::SendDisconnectConnectionToNetwork
//

void ScriptExecutor::SendDisconnectConnectionToNetwork( ScriptTokenList &tlist, CByteArray &packet )
{
	int		valu
	;

	TRACE0( "SendDisconnectConnectionToNetwork\n" );

	if (tlist.Length() != 2)
		throw "DNET required";

	const ScriptToken &dnet = tlist[1];

	if (dnet.tokenType != scriptValue)
		throw "DNET expected";
	if (!dnet.IsInteger(valu))
		throw "DNET invalid format";
	if ((valu < 0) || (valu > 65534))
		throw "DNET out of range (0..65534)";

	TRACE1( "    Network %d\n", valu );
	packet.Add( valu >> 8 );
	packet.Add( valu & 0x0FF );
}

//
//	ScriptExecutor::SendDevPacket
//

void ScriptExecutor::SendDevPacket( void )
{
	int						alMsgID, valu
	;
	BACnetAPDU				apdu
	;
	BACnetOctetString		nlDest
	;
	BACnetAddress			nlDestAddr
	;
	BACnetBoolean			nlDER
	;
	BACnetInteger			nlPriority, nlDNET
	;
	ScriptPacketExprPtr		nlDA, alMsg
	,						pDER, pPriority
	;
	ScriptTokenList			alList
	;

	// we already know this is an application layer message
	alMsg = execPacket->packetExprList.Find( kwPDU );

	// make sure we have a device
	if (gMasterDeviceList.Length() == 0)
		throw ExecError( "No defined devices", alMsg->exprLine );

	// make it an error to have more than one for now
	if (gMasterDeviceList.Length() > 1)
		throw ExecError( "Too many defined devices, there can be only one", alMsg->exprLine );

	// get the destination.  The IUT will be the default for all outbound
	// messages, regardless of the message type (for example, a message that 
	// would normally be sent as a global broadcast will be sent directly to
	// the IUT).
	nlDA = execPacket->packetExprList.Find( kwDA );
	if (!nlDA) {
		for (int i = 0; i < execDB->m_Names.Length(); i++ ) {
			VTSNameDesc		nameDesc;

			execDB->m_Names.ReadName( i, &nameDesc );
			if (stricmp(nameDesc.nameName,"IUT") == 0) {
				nlDestAddr = nameDesc.nameAddr;
				break;
			}
		}
		if (i >= execDB->m_Names.Length())
			throw ExecError( "Default destination address IUT not found", execPacket->baseLineStart );
	} else {
		ScriptTokenList			daList
		;

		// resolve the expressions
		ResolveExpr( nlDA->exprValue, nlDA->exprLine, daList );
		if (daList.Length() < 1)
			throw ExecError( "Address, name or keyword expected", nlDA->exprLine );

		// get a reference to the first parameter
		const ScriptToken &t = daList[0];

		// check to see if this is a keyword
		if (t.tokenType == scriptKeyword) {
			if ((t.tokenSymbol == kwBROADCAST) || (t.tokenSymbol == kwLOCALBROADCAST))
				nlDestAddr.LocalBroadcast();
			else
			if (t.tokenSymbol == kwREMOTEBROADCAST) {
				if (daList.Length() == 1)
					throw ExecError( "DNET expected", nlDA->exprLine );

				const ScriptToken &dnet = daList[1];

				if (dnet.tokenType != scriptValue)
					throw "DNET expected";
				if (!dnet.IsInteger(valu))
					throw "DNET invalid format, integer required";
				if ((valu < 0) || (valu > 65534))
					throw "DNET out of range (0..65534)";

				nlDestAddr.RemoteBroadcast( valu );
			} else
			if (t.tokenSymbol == kwGLOBALBROADCAST)
				nlDestAddr.GlobalBroadcast();
			else
				throw ExecError( "Unrecognized keyword", nlDA->exprLine );
		} else
		if (daList.Length() == 1) {
			// it might be a name
			if ((t.tokenType == scriptValue) && (t.tokenEnc == scriptASCIIEnc)) {
				CString tvalu = t.RemoveQuotes();
				for (int i = 0; i < execDB->m_Names.Length(); i++ ) {
					VTSNameDesc		nameDesc;

					execDB->m_Names.ReadName( i, &nameDesc );
					if (stricmp(nameDesc.nameName,tvalu) == 0) {
						nlDestAddr = nameDesc.nameAddr;
						break;
					}
				}
				if (i >= execDB->m_Names.Length())
					throw ExecError( "Destination address name not found", nlDA->exprLine );
			} else
			// it might be an IP address
			if ((t.tokenType == scriptValue) && (t.tokenEnc == scriptIPEnc)) {
				BACnetIPAddr nlIPAddr( t.tokenValue );
				nlDestAddr = nlIPAddr;
			} else
			// it might be an explicit octet string
			if (t.IsEncodeable( nlDest )) {
				nlDestAddr.LocalStation( nlDest.strBuff, nlDest.strLen );
			} else
				throw ExecError( "Destination address expected", nlDA->exprLine );
		} else
		if (daList.Length() == 2) {
			if (t.tokenType != scriptValue)
				throw "DNET expected";
			if (!t.IsInteger(valu))
				throw "DNET invalid format, integer required";
			if ((valu < 0) || (valu > 65534))
				throw "DNET out of range (0..65534)";

			const ScriptToken &dadr = daList[1];

			// it might be an IP address
			if ((dadr.tokenType == scriptValue) && (dadr.tokenEnc == scriptIPEnc)) {
				BACnetIPAddr nlIPAddr( dadr.tokenValue );
				nlDestAddr = nlIPAddr;
			} else
			// it might be an explicit octet string
			if (dadr.IsEncodeable( nlDest )) {
				nlDestAddr.LocalStation( nlDest.strBuff, nlDest.strLen );
			} else
				throw ExecError( "Destination address expected", nlDA->exprLine );

			nlDestAddr.addrType = remoteStationAddr;
			nlDestAddr.addrNet = valu;
		} else
			throw ExecError( "Destination address expected", nlDA->exprLine );
	}

	// copy the destination address into the apdu
	apdu.apduAddr = nlDestAddr;

	// force property references to fail until context established
	execObjID = 0xFFFFFFFF;

	// resolve the expressions
	ResolveExpr( alMsg->exprValue, alMsg->exprLine, alList );
	if (alList.Length() < 1)
		throw ExecError( "AL message keyword expected", alMsg->exprLine );

	// get a reference to the first parameter
	const ScriptToken &t = alList[0];

	// check to see if this is a keyword
	if (t.tokenType == scriptKeyword) {
		alMsgID = t.Lookup( t.tokenSymbol, ScriptALMsgTypeMap );
		if (alMsgID < 0)
			throw ExecError( "Unrecognized keyword", alMsg->exprLine );
	} else
		throw ExecError( "Keyword expected", alMsg->exprLine );

	// based on the number, check for other parameters
	try {
		switch (alMsgID) {
			case 0:						// CONFIRMED-REQUEST
				SendDevConfirmedRequest( apdu );
				break;
			case 1:						// UNCONFIRMED-REQUEST
				SendDevUnconfirmedRequest( apdu );
				break;
			case 2:						// SIMPLEACK
				SendDevSimpleACK( apdu );
				break;
			case 3:						// COMPLEXACK
				SendDevComplexACK( apdu );
				break;
			case 4:						// SEGMENTACK
				SendDevSegmentACK( apdu );
				break;
			case 5:						// ERROR
				SendDevError( apdu );
				break;
			case 6:						// REJECT
				SendDevReject( apdu );
				break;
			case 7:						// ABORT
				SendDevAbort( apdu );
				break;
		}
	}
	catch (const char *errMsg) {
		// one of the functions had an error
		throw ExecError( errMsg, alMsg->exprLine );
	}

	// get some interesting keywords that can override the default
	pDER = GetKeywordValue( kwDER, nlDER );
	if (pDER)
		apdu.apduExpectingReply = nlDER.boolValue;

	pPriority	= GetKeywordValue( kwPRIORITY, nlPriority );
	if (!pPriority)
		pPriority = GetKeywordValue( kwPRIO, nlPriority );
	if (pPriority) {
		if ((nlPriority.intValue < 0) || (nlPriority.intValue > 3))
			throw ExecError( "Priority out of range 0..3", pPriority->exprLine );
		apdu.apduNetworkPriority = nlPriority.intValue;
	}

	// pass along to the device object
	gMasterDeviceList[0]->SendAPDU( apdu );
}

//
//	ScriptExecutor::SendDevConfirmedRequest
//

void ScriptExecutor::SendDevConfirmedRequest( BACnetAPDU &apdu )
{
	CByteArray				packet
	;
	ScriptPacketExprPtr		pService
	;
	BACnetInteger			alService
	;

	// set the packet type
	apdu.apduType = confirmedRequestPDU;
	apdu.apduExpectingReply = true;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (!pService)
		throw "Service choice (SERVICE) keyword required";

	// encode it
	apdu.apduService = alService.intValue;

	// encode the rest
	SendALData( packet );

	// set the apdu contents
	apdu.Append( packet.GetData(), packet.GetSize() );
}

//
//	ScriptExecutor::SendDevUnconfirmedRequest
//

void ScriptExecutor::SendDevUnconfirmedRequest( BACnetAPDU &apdu )
{
	CByteArray				packet
	;
	ScriptPacketExprPtr		pService
	;
	BACnetInteger			alService
	;

	// set the packet type
	apdu.apduType = unconfirmedRequestPDU;
	apdu.apduExpectingReply = false;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALUnconfirmedServiceMap );
	if (!pService)
		throw "Service choice (SERVICE) keyword required";

	// encode it
	apdu.apduService = alService.intValue;

	// encode the rest
	SendALData( packet );

	// set the apdu contents
	apdu.Append( packet.GetData(), packet.GetSize() );
}

//
//	ScriptExecutor::SendDevSimpleACK
//

void ScriptExecutor::SendDevSimpleACK( BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pInvokeID, pService
	;
	BACnetInteger			alInvokeID, alService
	;

	// set the packet type
	apdu.apduType = simpleAckPDU;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (!pService)
		throw "Service-ACK-choice (SERVICE) keyword required";

	// encode it
	apdu.apduService = alService.intValue;

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	apdu.apduInvokeID = alInvokeID.intValue;
}

//
//	ScriptExecutor::SendDevComplexACK
//

void ScriptExecutor::SendDevComplexACK( BACnetAPDU &apdu )
{
	CByteArray				packet
	;
	ScriptPacketExprPtr		pInvokeID, pService
	;
	BACnetInteger			alInvokeID, alService
	;

	// set the packet type
	apdu.apduType = complexAckPDU;
	apdu.apduExpectingReply = false;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (!pService)
		throw "Service-ACK-choice (SERVICE) keyword required";

	// encode it
	apdu.apduService = alService.intValue;

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	apdu.apduInvokeID = alInvokeID.intValue;

	// encode the rest
	SendALData( packet );

	// set the apdu contents
	apdu.Append( packet.GetData(), packet.GetSize() );
}

//
//	ScriptExecutor::SendDevSegmentACK
//

void ScriptExecutor::SendDevSegmentACK( BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pNegativeACK, pServer, pInvokeID, pSeqNumber, pWindowSize
	;
	BACnetBoolean			alNegativeACK, alServer
	;
	BACnetInteger			alInvokeID, alSeqNumber, alWindowSize
	;

	// set the packet type
	apdu.apduType = segmentAckPDU;
	apdu.apduExpectingReply = false;

	// get the ACK
	pNegativeACK = GetKeywordValue( kwNEGATIVEACK, alNegativeACK );
	if (!pNegativeACK) throw "Negative-ACK keyword required";
	apdu.apduNak = (alNegativeACK.boolValue != 0);

	// get the SERVER
	pServer = GetKeywordValue( kwSERVER, alServer );
	if (!pServer) throw "Server keyword required";
	apdu.apduSrv = (alServer.boolValue != 0);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";
	apdu.apduInvokeID = alInvokeID.intValue;

	// get the sequence number
	pSeqNumber = GetKeywordValue( kwSEQUENCENR, alSeqNumber );
	if (!pSeqNumber)
		throw "Sequence number (SEQUENCENR) keyword required";
	if ((alSeqNumber.intValue < 0) || (alSeqNumber.intValue > 255))
		throw "Sequence number out of range (0..255)";
	apdu.apduSeq = alSeqNumber.intValue;

	// get the actual window size
	pWindowSize = GetKeywordValue( kwWINDOWSIZE, alWindowSize );
	if (!pWindowSize)
		throw "Actual window size (WINDOWSIZE) keyword required";
	if ((alWindowSize.intValue < 0) || (alWindowSize.intValue > 255))
		throw "Actual window size out of range (0..255)";
	apdu.apduWin = alWindowSize.intValue;
}

//
//	ScriptExecutor::SendDevError
//

void ScriptExecutor::SendDevError( BACnetAPDU &apdu )
{
	CByteArray				packet
	;
	ScriptPacketExprPtr		pInvokeID, pService
	;
	BACnetInteger			alInvokeID, alService
	;

	// set the packet type
	apdu.apduType = errorPDU;
	apdu.apduExpectingReply = false;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (!pService)
		throw "Service choice (SERVICE) keyword required";

	// encode it
	apdu.apduService = alService.intValue;

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	apdu.apduInvokeID = alInvokeID.intValue;

	// encode the rest
	SendALData( packet );

	// set the apdu contents
	apdu.Append( packet.GetData(), packet.GetSize() );
}

//
//	ScriptExecutor::SendDevReject
//

void ScriptExecutor::SendDevReject( BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pInvokeID, pReason
	;
	BACnetInteger			alInvokeID, alReason
	;

	// set the packet type
	apdu.apduType = rejectPDU;
	apdu.apduExpectingReply = false;

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	apdu.apduInvokeID = alInvokeID.intValue;

	// get the reject reason
	pReason = GetKeywordValue( kwREJECTREASON, alReason, ScriptALRejectReasonMap );
	if (!pReason)
		throw "Reject reason keyword required";
	if ((alReason.intValue < 0) || (alReason.intValue > 255))
		throw "Reject reason out of range (0..255)";

	// encode it
	apdu.apduAbortRejectReason = alReason.intValue;
}

//
//	ScriptExecutor::SendDevAbort
//

void ScriptExecutor::SendDevAbort( BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pServer, pInvokeID, pReason
	;
	BACnetBoolean			alServer
	;
	BACnetInteger			alInvokeID, alReason
	;

	// set the packet type
	apdu.apduType = abortPDU;
	apdu.apduExpectingReply = false;

	// see if this is being sent as a server
	pServer = GetKeywordValue( kwSERVER, alServer );
	if (!pServer) throw "Server keyword required";

	// encode it
	apdu.apduSrv = (alServer.boolValue != 0);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	apdu.apduInvokeID = alInvokeID.intValue;

	// get the reject reason
	pReason = GetKeywordValue( kwABORTREASON, alReason, ScriptALAbortReasonMap );
	if (!pReason)
		throw "Abort reason keyword required";
	if ((alReason.intValue < 0) || (alReason.intValue > 255))
		throw "Abort reason out of range (0..255)";

	// encode it
	apdu.apduAbortRejectReason = alReason.intValue;
}

//
//	ScriptExecutor::SendConfirmedRequest
//

void ScriptExecutor::SendConfirmedRequest( CByteArray &packet )
{
	ScriptPacketExprPtr		pSegMsg, pMOR, pSegResp, pMaxResp, pInvokeID, pSeq, pWindow, pService
	;
	BACnetBoolean			alSegMsg, alMOR, alSegResp
	;
	BACnetInteger			alMaxResp, alInvokeID, alSeq, alWindow, alService
	;

	// find segmentation inforamtion
	pSegMsg = GetKeywordValue( kwSEGMSG, alSegMsg );
	if (!pSegMsg)
		pSegMsg = GetKeywordValue( kwSEGMENTEDMESSAGE, alSegMsg );
	if (!pSegMsg)
		throw "Segmented-message keyword required";

	if (alSegMsg.boolValue) {
		pMOR = GetKeywordValue( kwMOREFOLLOWS, alMOR );
		if (!pMOR)
			throw "More-follows keyword required";
	} else
		alMOR.boolValue = BACnetBoolean::bFalse;

	pSegResp = GetKeywordValue( kwSEGRESP, alSegResp );
	if (!pSegResp)
		pSegResp = GetKeywordValue( kwSEGRESPACCEPTED, alSegResp );
	if (!pSegResp)
		throw "Segmented-response-accepted keyword required";

	// encode it
	packet.Add( (0 << 4)
			+ ((alSegMsg.boolValue ? 1 : 0) << 3)
			+ ((alMOR.boolValue ? 1 : 0) << 2)
			+ ((alSegResp.boolValue ? 1 : 0) << 1)
		);

	// lots of options for maximum response size
	pMaxResp = GetKeywordValue( kwMAXRESP, alMaxResp );
	if (!pMaxResp)
		pMaxResp = GetKeywordValue( kwMAXRESPONSE, alMaxResp );
	if (!pMaxResp)
		pMaxResp = GetKeywordValue( kwMAXSIZE, alMaxResp );
	if (!pMaxResp)
		throw "Max response size keyword required";

	// translate the max response size into the code
	if (alMaxResp.intValue < 16)
		;
	else
	if (alMaxResp.intValue <= 50)
		alMaxResp.intValue = 0;
	else
	if (alMaxResp.intValue <= 128)
		alMaxResp.intValue = 1;
	else
	if (alMaxResp.intValue <= 206)
		alMaxResp.intValue = 2;
	else
	if (alMaxResp.intValue <= 480)
		alMaxResp.intValue = 3;
	else
	if (alMaxResp.intValue <= 1024)
		alMaxResp.intValue = 4;
	else
	if (alMaxResp.intValue <= 1476)
		alMaxResp.intValue = 5;
	else
		alMaxResp.intValue = 6;

	// encode it
	packet.Add( alMaxResp.intValue );

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	packet.Add( alInvokeID.intValue );

	// look up segmented message stuff
	if (alSegMsg.boolValue) {
		pSeq		= GetKeywordValue( kwSEQUENCENR, alSeq );
		if (!pSeq) throw "Sequence number (SEQUENCENR) keyword required";
		pWindow		= GetKeywordValue( kwWINDOWSIZE, alWindow );
		if (!pWindow) throw "Window size keyword required";

		// encode it
		packet.Add( alSeq.intValue );
		packet.Add( alWindow.intValue );
	}

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (!pService)
		throw "Service choice (SERVICE) keyword required";

	// encode it
	packet.Add( alService.intValue );

	// encode the rest
	SendALData( packet );
}

//
//	ScriptExecutor::SendUnconfirmedRequest
//

void ScriptExecutor::SendUnconfirmedRequest( CByteArray &packet )
{
	ScriptPacketExprPtr		pService
	;
	BACnetInteger			alService
	;

	// encode the header
	packet.Add( 1 << 4 );

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALUnconfirmedServiceMap );
	if (!pService)
		throw "Service choice (SERVICE) keyword required";

	// encode it
	packet.Add( alService.intValue );

	// encode the rest
	SendALData( packet );
}

//
//	ScriptExecutor::SendSimpleACK
//

void ScriptExecutor::SendSimpleACK( CByteArray &packet )
{
	ScriptPacketExprPtr		pInvokeID, pService
	;
	BACnetInteger			alInvokeID, alService
	;

	// encode the header
	packet.Add( 2 << 4 );

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	packet.Add( alInvokeID.intValue );

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (!pService)
		throw "Service-ACK-choice (SERVICE) keyword required";

	// encode it
	packet.Add( alService.intValue );
}

//
//	ScriptExecutor::SendComplexACK
//

void ScriptExecutor::SendComplexACK( CByteArray &packet )
{
	ScriptPacketExprPtr		pSegMsg, pMOR, pInvokeID, pSeq, pWindow, pService
	;
	BACnetBoolean			alSegMsg, alMOR
	;
	BACnetInteger			alInvokeID, alSeq, alWindow, alService
	;

	// find segmentation inforamtion
	pSegMsg = GetKeywordValue( kwSEGMSG, alSegMsg );
	if (!pSegMsg)
		pSegMsg = GetKeywordValue( kwSEGMENTEDMESSAGE, alSegMsg );
	if (!pSegMsg)
		alSegMsg.boolValue = BACnetBoolean::bFalse;
	if (alSegMsg.boolValue) {
		pMOR = GetKeywordValue( kwMOREFOLLOWS, alMOR );
		if (!pMOR)
			throw "More-follows keyword required";
	} else
		alMOR.boolValue = BACnetBoolean::bFalse;

	// encode it
	packet.Add( (3 << 4)
			+ ((alSegMsg.boolValue ? 1 : 0) << 3)
			+ ((alMOR.boolValue ? 1 : 0) << 2)
		);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Original invoke ID (INVOKEID) keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Original invoke ID out of range (0..255)";

	// encode it
	packet.Add( alInvokeID.intValue );

	// if this is segmented
	if (alSegMsg.boolValue) {
		// get the sequence number
		pSeq = GetKeywordValue( kwSEQUENCENR, alSeq );
		if (!pSeq)
			throw "Sequence number (SEQUENCENR) keyword required";
		if ((alSeq.intValue < 0) || (alSeq.intValue > 255))
			throw "Sequence number out of range (0..255)";

		// encode it
		packet.Add( alSeq.intValue );

		// get the proposed window size
		pWindow = GetKeywordValue( kwWINDOWSIZE, alWindow );
		if (!pWindow)
			throw "Proposed window size keyword required";
		if ((alWindow.intValue < 1) || (alWindow.intValue > 127))
			throw "Proposed window size out of range (1..127)";

		// encode it
		packet.Add( alWindow.intValue );
	}

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (!pService)
		throw "Service choice (SERVICE) keyword required";

	// encode it
	packet.Add( alService.intValue );

	// encode the rest
	SendALData( packet );
}

//
//	ScriptExecutor::SendSegmentACK
//

void ScriptExecutor::SendSegmentACK( CByteArray &packet )
{
	ScriptPacketExprPtr		pNegativeACK, pServer, pInvokeID, pSeqNumber, pWindowSize
	;
	BACnetBoolean			alNegativeACK, alServer
	;
	BACnetInteger			alInvokeID, alSeqNumber, alWindowSize
	;

	// find the interesting keywords
	pNegativeACK = GetKeywordValue( kwNEGATIVEACK, alNegativeACK );
	if (!pNegativeACK) throw "Negative-ACK keyword required";
	pServer = GetKeywordValue( kwSERVER, alServer );
	if (!pServer) throw "Server keyword required";

	// encode it
	packet.Add( (4 << 4)
			+ ((alNegativeACK.boolValue ? 1 : 0) << 1)
			+ (alServer.boolValue ? 1 : 0)
		);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	packet.Add( alInvokeID.intValue );

	// get the sequence number
	pSeqNumber = GetKeywordValue( kwSEQUENCENR, alSeqNumber );
	if (!pSeqNumber)
		throw "Sequence number (SEQUENCENR) keyword required";
	if ((alSeqNumber.intValue < 0) || (alSeqNumber.intValue > 255))
		throw "Sequence number out of range (0..255)";

	// encode it
	packet.Add( alSeqNumber.intValue );

	// get the actual window size
	pWindowSize = GetKeywordValue( kwWINDOWSIZE, alWindowSize );
	if (!pWindowSize)
		throw "Actual window size (WINDOWSIZE) keyword required";
	if ((alWindowSize.intValue < 0) || (alWindowSize.intValue > 255))
		throw "Actual window size out of range (0..255)";

	// encode it
	packet.Add( alWindowSize.intValue );
}

//
//	ScriptExecutor::SendError
//

void ScriptExecutor::SendError( CByteArray &packet )
{
	ScriptPacketExprPtr		pInvokeID, pService
	;
	BACnetInteger			alInvokeID, alService
	;

	// encode the header
	packet.Add( 5 << 4 );

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	packet.Add( alInvokeID.intValue );

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (!pService)
		throw "Service choice (SERVICE) keyword required";

	// encode it
	packet.Add( alService.intValue );

	// encode the rest
	SendALData( packet );
}

//
//	ScriptExecutor::SendReject
//

void ScriptExecutor::SendReject( CByteArray &packet )
{
	ScriptPacketExprPtr		pInvokeID, pReason
	;
	BACnetInteger			alInvokeID, alReason
	;

	// encode the header
	packet.Add( 6 << 4 );

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	packet.Add( alInvokeID.intValue );

	// get the reject reason
	pReason = GetKeywordValue( kwREJECTREASON, alReason, ScriptALRejectReasonMap );
	if (!pReason)
		throw "Reject reason keyword required";
	if ((alReason.intValue < 0) || (alReason.intValue > 255))
		throw "Reject reason out of range (0..255)";

	// encode it
	packet.Add( alReason.intValue );
}

//
//	ScriptExecutor::SendAbort
//

void ScriptExecutor::SendAbort( CByteArray &packet )
{
	ScriptPacketExprPtr		pServer, pInvokeID, pReason
	;
	BACnetBoolean			alServer
	;
	BACnetInteger			alInvokeID, alReason
	;

	// see if this is being sent as a server
	pServer = GetKeywordValue( kwSERVER, alServer );
	if (!pServer) throw "Server keyword required";

	// encode it
	packet.Add( (7 << 4)
			+ (alServer.boolValue ? 1 : 0)
		);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (!pInvokeID)
		throw "Invoke ID keyword required";
	if ((alInvokeID.intValue < 0) || (alInvokeID.intValue > 255))
		throw "Invoke ID out of range (0..255)";

	// encode it
	packet.Add( alInvokeID.intValue );

	// get the reject reason
	pReason = GetKeywordValue( kwABORTREASON, alReason, ScriptALAbortReasonMap );
	if (!pReason)
		throw "Abort reason keyword required";
	if ((alReason.intValue < 0) || (alReason.intValue > 255))
		throw "Abort reason out of range (0..255)";

	// encode it
	packet.Add( alReason.intValue );
}

//
//	ScriptExecutor::SendALData
//
//	There are two flavors of application layer variable encoding stuff.  One is with a keyword 
//	on the front of the type with a bunch of parameters, the other is ALDATA followed by a 
//	hex string.
//

void ScriptExecutor::SendALData( CByteArray &packet )
{
	int						indx, len
	;
	BACnetOctetString		ostr
	;

	// get the index of the first data
	indx = execPacket->packetExprList.FirstData();
	if (indx < 0)
		return;		// in some cases, like Who-Is, there might not be any parameters

	// get the length
	len = execPacket->packetExprList.Length();

	for (; indx < len; indx++) {
		ScriptPacketExprPtr spep = execPacket->packetExprList.Child( indx );
		if (!spep)
			throw "Application variable element expected";

		// check for raw data
		if ((spep->exprKeyword == kwAL) || (spep->exprKeyword == kwALDATA)) {
			// translate the expression, resolve parameter names into values
			ScriptTokenList tlist;
			ResolveExpr( spep->exprValue, spep->exprLine, tlist );

			// just a little error checking
			if (tlist.Length() != 1)
				throw "ALDATA requires an octet string";

			// reference or data?
			if (tlist[0].tokenType == scriptReference) {
				BACnetAnyValue		bacnetEPICSProperty;

				GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

				BACnetAPDUEncoder	enc(2048);		// big buffer size
				bacnetEPICSProperty.Encode(enc);

				// copy the contents into the byte array
				for (int i = 0; i < enc.pktLength; i++)
					packet.Add( enc.pktBuffer[i] );
			} else {
				if (!tlist[0].IsEncodeable( ostr ))
					throw "Octet string expected";

				// copy the contents into the byte array
				for (int j = 0; j < ostr.strLen; j++)
					packet.Add( ostr.strBuff[j] );
			}

			continue;
		}

		// skip things that aren't data
		if (!spep->exprIsData)
			continue;

		// do the type specific encoding
		try {
			int rslt = ScriptToken::Lookup( spep->exprKeyword, ScriptALMap );
			if (rslt < 0)
				throw "Invalid keyword";

			switch (rslt) {
				case 1:								// NULL
					SendALNull( spep, packet );
					break;
				case 2:								// BOOLEAN
					SendALBoolean( spep, packet );
					break;
				case 3:								// UNSIGNED
					SendALUnsigned( spep, packet );
					break;
				case 4:								// INTEGER
					SendALInteger( spep, packet );
					break;
				case 5:								// REAL
					SendALReal( spep, packet );
					break;
				case 6:								// DOUBLE
					SendALDouble( spep, packet );
					break;
				case 7:								// OCTETSTRING
					SendALOctetString( spep, packet );
					break;
				case 8:								// CHARACTERSTRING
					SendALCharacterString( spep, packet );
					break;
				case 9:								// BITSTRING
					SendALBitString( spep, packet );
					break;
				case 10:							// ENUMERATED
					SendALEnumerated( spep, packet );
					break;
				case 11:							// DATE
					SendALDate( spep, packet );
					break;
				case 12:							// TIME
					SendALTime( spep, packet );
					break;
				case 13:							// OBJECTIDENTIFIER
					SendALObjectIdentifier( spep, packet );
					break;
				case 14:							// DEVICEIDENTIFIER
					SendALDeviceIdentifier( spep, packet );
					break;
				case 15:							// PROPERTYIDENTIFIER
					SendALPropertyIdentifier( spep, packet );
					break;
				case 16:							// OPENINGTAG
					SendALOpeningTag( spep, packet );
					break;
				case 17:							// CLOSINGTAG
					SendALClosingTag( spep, packet );
					break;
			}
		}
		catch (const char *errMsg) {
			throw ExecError( errMsg, spep->exprLine );
		}
	}
}

//
//	ScriptExecutor::SendALNull
//

void ScriptExecutor::SendALNull( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					context = kAppContext
	;
	BACnetNull			nullData
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 0)
		;
	else
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
	} else
		throw "Too many keyword values";

	// encode it
	nullData.Encode( enc, context );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALBoolean
//

void ScriptExecutor::SendALBoolean( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					indx, context = kAppContext
	;
	BACnetBoolean		boolData
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
	} else
		throw "Boolean keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference)
	{
		BACnetAnyValue bacnetAny;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetAny, tlist[indx].m_nIndex);

		// verify the type
		if (bacnetAny.GetType() != ebool)
			throw "Boolean property value expected in EPICS";

		bacnetAny.Encode(enc, context);
	} else
	{
		if (!tlist[indx].IsEncodeable( boolData ))
			throw "Boolean value expected";

		boolData.Encode( enc, context );
	}

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALUnsigned
//

void ScriptExecutor::SendALUnsigned( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					indx, context = kAppContext
	;
	BACnetUnsigned		uintData
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	//Added by Yajun Zhou, 2002-8-16
	spep->exprValue.MakeUpper();
	////////////////////////////////
	
	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
	} else
		throw "Unsigned keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference)
	{
		BACnetAnyValue bacnetAny;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetAny, tlist[indx].m_nIndex);

		// verify the type
		if (bacnetAny.GetType() != uw )
			throw "Unsigned property value expected in EPICS";

		uintData.uintValue = ((BACnetUnsigned *) bacnetAny.GetObject())->uintValue;
	} else
	if (!tlist[indx].IsEncodeable( uintData ))
		throw "Unsigned value expected";

	// check for wierd versions
	if (spep->exprKeyword == kwUNSIGNED8) {
		if ((uintData.uintValue < 0) || (uintData.uintValue > 255))
			throw "Unsigned8 value out of range (0..255)";
		if (tlist.Length() == 2)
			throw "Unsigned8 keyword cannot be context encoded";
		packet.Add( (unsigned char) uintData.uintValue );
	} else
	if (spep->exprKeyword == kwUNSIGNED16) {
		if ((uintData.uintValue < 0) || (uintData.uintValue > 65535))
			throw "Unsigned16 value out of range (0..65535)";
		if (tlist.Length() == 2)
			throw "Unsigned16 keyword cannot be context encoded";
		packet.Add( (unsigned char) ((uintData.uintValue >> 8) & 0xFF) );
		packet.Add( (unsigned char) (uintData.uintValue & 0xFF) );
	} else
	if (spep->exprKeyword == kwUNSIGNED32) {
		if (tlist.Length() == 2)
			throw "Unsigned32 keyword cannot be context encoded";
		packet.Add( (unsigned char) ((uintData.uintValue >> 24) & 0xFF) );
		packet.Add( (unsigned char) ((uintData.uintValue >> 16) & 0xFF) );
		packet.Add( (unsigned char) ((uintData.uintValue >>  8) & 0xFF) );
		packet.Add( (unsigned char) (uintData.uintValue & 0xFF) );
	} else {
		// encode it
		uintData.Encode( enc, context );

		// copy the encoding into the byte array
		for (int i = 0; i < enc.pktLength; i++)
			packet.Add( enc.pktBuffer[i] );
	}
}

//
//	ScriptExecutor::SendALInteger
//

void ScriptExecutor::SendALInteger( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					indx, context = kAppContext
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	//Added by Yajun Zhou, 2002-8-17
	spep->exprValue.MakeUpper();
	////////////////////////////////

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
	} else
		throw "Integer keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue bacnetAny;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetAny, tlist[indx].m_nIndex);

		// verify the type
		if (!bacnetAny.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetUnsigned)) && !bacnetAny.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetInteger)) )
			throw "Integer property value expected in EPICS";

		bacnetAny.Encode(enc, context);
	} else
	{
		BACnetInteger intData;
		if (!tlist[indx].IsEncodeable( intData ))
			throw "Integer value expected";

		// encode it
		intData.Encode( enc, context );
	}

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALReal
//

void ScriptExecutor::SendALReal( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					indx, context = kAppContext
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
	} else
		throw "Real keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference)
	{
		BACnetAnyValue bacnetAny;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetAny, tlist[indx].m_nIndex );

		// verify the type
		if ( bacnetAny.GetType() != flt )
			throw "Real property value expected in EPICS";

		bacnetAny.Encode(enc, context);
	} else
	{
		BACnetReal realData;

		if (!tlist[indx].IsEncodeable( realData ))
			throw "Real value expected";
		
		realData.Encode( enc, context );
	}

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALDouble
//

void ScriptExecutor::SendALDouble( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					indx, context = kAppContext
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
	} else
		throw "Double keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue bacnetAny;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetAny, tlist[indx].m_nIndex);

		// verify the type
		if (bacnetAny.GetType() != flt )
			throw "Floating point property value expected in EPICS";

		bacnetAny.Encode(enc, context);
	} else
	{
		BACnetDouble		doubleData;

		if (!tlist[indx].IsEncodeable( doubleData ))
			throw "Floating point value expected";

		doubleData.Encode( enc, context );
	}

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALOctetString
//

void ScriptExecutor::SendALOctetString( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					indx, context = kAppContext
	;
	BACnetOctetString	ostrData
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
	} else
		throw "Double keyword requires 1 or 2 parameters";

	// no references
	if (tlist[indx].tokenType == scriptReference)
		throw "Octet string property references not supported";
	if (!tlist[indx].IsEncodeable( ostrData ))
		throw "Octet string expected";

	// encode it
	ostrData.Encode( enc, context );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALCharacterString
//
//	The character string format has been extended a little.  The basic idea is to make 
//	it easier to use regular strings, a novel concept.
//
//		CHARACTERSTRING	[ op ] cstring
//		CHARACTERSTRING	[ op ] ostring
//		CHARACTERSTRING	[ op ] { ref }
//		CHARACTERSTRING	[ op ] char-set ',' ( cstring | ostring )
//		CHARACTERSTRING	[ op ] tag ',' char-set ',' ( cstring | ostring )
//		CHARACTERSTRING	[ op ] tag ',' { ref }
//
//	Most of ugly cases involves putting the last two tokens back together and passing the 
//	whole thing off to Decode().  What a hack!  I don't want to allow the script to 
//	specify a different encoding than what might be in the EPICS without (a) supporting 
//	translation from one encoding to another or (b) requiring that the EPICS content 
//	match the encoding specified in the script.  (A) is a pain if it's even possible, and 
//	(b) doesn't seem to have much point.
//

void ScriptExecutor::SendALCharacterString( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int						indx = -1, context = kAppContext
	;
	BACnetCharacterString	cstrData
	;
	BACnetAPDUEncoder		enc
	;
	ScriptTokenList			tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (tlist[0].tokenType == scriptReference)
			indx = 0;
		else
		if (!tlist[0].IsEncodeable( cstrData ))
			throw "ASCII string value expected";
	} else
	if (tlist.Length() == 2) {
		if (tlist[1].tokenType == scriptReference) {
			if (!tlist[0].IsInteger( context ))
				throw "Tag number expected";
			indx = 1;
		} else {
			if (ScriptToken::Lookup( tlist[0].tokenSymbol, ScriptCharacterTypeMap ) < 0)
				throw "Unknown encoding";

			CString buff;
			buff.Format( "%s, %s", tlist[0].tokenValue, tlist[1].tokenValue );
			cstrData.Decode( buff );
		}
	} else
	if (tlist.Length() == 3) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		if (ScriptToken::Lookup( tlist[1].tokenSymbol, ScriptCharacterTypeMap ) < 0)
			throw "Unknown encoding";

		CString buff;
		buff.Format( "%s, %s", tlist[1].tokenValue, tlist[2].tokenValue );
		cstrData.Decode( buff );
	} else
		throw "Missing requred parameters";

	// see if a reference was used
	if (indx >= 0) {
		BACnetAnyValue bacnetAny;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetAny, tlist[indx].m_nIndex);

		// verify the type
		if ( !bacnetAny.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetCharacterString)) )
			throw "Character string property value expected in EPICS";

		bacnetAny.Encode(enc, context);
	}
	else
		cstrData.Encode( enc, context );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALBitString
//
//	Yet another custom extension, bit strings can be specified as a list of T/F values or 
//	B'1010100111'.
//

void ScriptExecutor::SendALBitString( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					context = kAppContext, bit, i;
	BACnetBitString		bstrData
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 0)
		throw "Bit string keyword values required";

	const ScriptToken &data = tlist[0];

	// must only be one bit or a bit string
	if (tlist.Length() == 1) {
		if (data.tokenType == scriptReference) {
			BACnetAnyValue		bacnetEPICSProperty;

			GetEPICSProperty( data.tokenSymbol, &bacnetEPICSProperty, data.m_nIndex);

			// verify the type
			if ( bacnetEPICSProperty.GetType() != bits)
				throw "Bit string property value expected in EPICS";

			bstrData = *((BACnetBitString *) bacnetEPICSProperty.GetObject());
		} else
		if (data.tokenEnc == scriptBinaryEnc) {
			if (!data.IsEncodeable( bstrData ))
				throw "Bit string value expected";
		} else
		if (data.tokenType == scriptKeyword) {
			if (data.IsInteger( bit, ScriptBooleanMap ))
				bstrData += bit;
		} else
			throw "Bit list expected";
	} else {
		i = 0;
		// check for a context
		if (data.tokenEnc != scriptBinaryEnc)
			if (data.IsInteger( context ))
				i += 1;

		if (tlist[i].tokenType == scriptReference) {
			BACnetAnyValue		bacnetEPICSProperty;

			GetEPICSProperty( tlist[i].tokenSymbol, &bacnetEPICSProperty, tlist[i].m_nIndex);

			// verify the type
			if ( bacnetEPICSProperty.GetType() != bits)
				throw "Bit string property value expected in EPICS";

			bstrData = *((BACnetBitString *) bacnetEPICSProperty.GetObject());
		} else
		if (tlist[i].IsEncodeable( bstrData ))
			;
		else {
			int count = 0;
			while (i < tlist.Length())
				if (tlist[i++].IsInteger( bit, ScriptBooleanMap ))
					bstrData.SetBit( count++, bit );
				else
					throw "Bit value expected";
		}
	}

	// encode it
	bstrData.Encode( enc, context );

	// copy the encoding into the byte array
	for (i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALEnumerated
//

void ScriptExecutor::SendALEnumerated( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					indx, context = kAppContext
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
	} else
		throw "Enumerated keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue bacnetAny;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetAny, tlist[indx].m_nIndex);

		// verify the type
		if ( bacnetAny.GetType() != et )
			throw "Enumerated property value expected in EPICS";

		bacnetAny.Encode(enc, context);
	} else {
		BACnetEnumerated	enumData;

		try {
			enumData.Decode( tlist[indx].tokenValue );
		}
		catch (...) {
			throw "Integer value expected";
		}

		enumData.Encode( enc, context );
	}


	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALDate
//
//		DATE [ op ] [ tag ',' ] dow [ ',' ] mon '/' day '/' year
//		DATE [ op ] [ tag ',' ] { ref }
//
//		dow ::= MONDAY | ... | SUNDAY | '*'
//		mon ::= number | '*'
//		day ::= number | '*'
//		year ::= number | '*'
//
//	In the future it would be nice to support these:
//
//		DATE [ op ] [ tag ',' ] dow [ ',' ] month [ [ ',' ] day ',' year ]
//		DATE [ op ] [ tag ',' ] dow [ ',' ] day '-' month [ '-' year ]
//
//	which requires changes to BACnetDate::Decode(const char *).
//

void ScriptExecutor::SendALDate( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int				context = kAppContext
	;
	BACnetDate		dateData
	;
	ScriptScanner	scan( spep->exprValue )
	;
	ScriptToken		tok
	;

	// get something from the front
	scan.Next( tok );

	// encode it
	BACnetAPDUEncoder enc;

	// if it is a number, it must be a tag
	if ((tok.tokenType == scriptValue) && (tok.IsInteger( context ))) {
		scan.Peek( tok );
		if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == ','))
			scan.Next( tok );

		if (tok.tokenType == scriptReference) {
			BACnetAnyValue bacnetAny;

			GetEPICSProperty( tok.tokenSymbol, &bacnetAny, tok.m_nIndex);

			// verify the type
			if ( bacnetAny.GetType() != ptDate )
				throw "Date property value expected in EPICS";

			bacnetAny.Encode(enc, context);
		} else
		{
			dateData.Decode( scan.scanSrc );
			dateData.Encode( enc, context );
		}
	} else
	{
		dateData.Decode( spep->exprValue );
		dateData.Encode( enc, context );
	}

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}




//
//	ScriptExecutor::SendALTime
//
//		TIME [ op ] [ tag ',' ] hour ':' min ':' sec '.' hund
//		TIME [ op ] [ tag ',' ] { ref }
//
//	The trick to parsing a time is the comma after the tag.  All four parts are 
//	reqired, the '*' may be used for dont-care value.
//

void ScriptExecutor::SendALTime( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int				holdContext, context = kAppContext
	;
	BACnetTime		timeData
	;
	ScriptScanner	scan( spep->exprValue )
	;
	ScriptToken		tok
	;

	// get something from the front
	scan.Next( tok );

	// encode it
	BACnetAPDUEncoder enc;

	try {
		// if it is a number
		if ((tok.tokenType == scriptValue) && (tok.IsInteger( holdContext ))) {
			scan.Peek( tok );
			if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == ',')) {
				context = holdContext;
				scan.Next( tok );

				if (tok.tokenType == scriptReference) {
					BACnetAnyValue bacnetAny;

					GetEPICSProperty( tok.tokenSymbol, &bacnetAny, tok.m_nIndex);

					// verify the type
					if ( bacnetAny.GetType() != ptTime )
						throw "Time property value expected in EPICS";

					bacnetAny.Encode(enc, context);
				} else
					timeData.Decode( scan.scanSrc );	// do the rest as a time
					timeData.Encode( enc, context );
			} else
				timeData.Decode( spep->exprValue );	// do whole thing
				timeData.Encode( enc, context );
		} else
			throw "Time keyword parameter format invalid";
	}
	catch (...) {
		throw "Time keyword parameter format invalid";
	}

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALObjectIdentifier
//

void ScriptExecutor::SendALObjectIdentifier( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int						indx, context = kAppContext, typ, inst
	;
	BACnetObjectIdentifier	id
	;
	BACnetAPDUEncoder		enc
	;
	ScriptTokenList			tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tlist[0].tokenType != scriptReference)
			throw "Object identifier keyword expects an EPICS reference";
	} else
	if (tlist.Length() == 2) {
		indx = 0;
	} else
	if (tlist.Length() == 3) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
	} else
		throw "Object identifier keyword requires 1, 2 or 3 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue bacnetAny;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetAny, tlist[indx].m_nIndex);

		if ( bacnetAny.GetType() != ob_id )
			throw "Object identifier property value expected in EPICS";

		id.objID = ((BACnetObjectIdentifier *) bacnetAny.GetObject())->objID;
	} else {
		if (!tlist[indx].IsInteger( typ, ScriptObjectTypeMap ))
			throw "Object identifier type expected";
		if (!tlist[indx+1].IsInteger( inst ))
			throw "Object identifier instance expected";

		id.SetValue( (BACnetObjectType)typ, inst );
	}

	// encode it
	id.Encode( enc, context );

	// stash the context for later property references
	execObjID = id.objID;

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALDeviceIdentifier
//

void ScriptExecutor::SendALDeviceIdentifier( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int						context = kAppContext, inst
	;
	BACnetAPDUEncoder		enc
	;
	ScriptTokenList			tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( inst ))
			throw "Device identifier instance value expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		if (!tlist[1].IsInteger( inst ))
			throw "Device identifier instance value expected";
	} else
		throw "Device identifier keyword requires 1 or 2 parameters";

	// encode it
	BACnetObjectIdentifier( 8, inst ).Encode( enc, context );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALPropertyIdentifier
//

void ScriptExecutor::SendALPropertyIdentifier( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					context = kAppContext, valu
	;
	BACnetEnumerated	enumData
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( valu, ScriptPropertyMap ))
			throw "Property name expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		if (!tlist[1].IsInteger( valu, ScriptPropertyMap ))
			throw "Property name expected";
	} else
		throw "Property identifier keyword requires 1 or 2 parameters";

	// encode it
	enumData.enumValue = valu;
	enumData.Encode( enc, context );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALOpeningTag
//

void ScriptExecutor::SendALOpeningTag( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					context
	;
	BACnetOpeningTag	openTag
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
	} else
		throw "Opening tag requires tag number";

	// encode it
	openTag.Encode( enc, context );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::SendALClosingTag
//

void ScriptExecutor::SendALClosingTag( ScriptPacketExprPtr spep, CByteArray &packet )
{
	int					context
	;
	BACnetClosingTag	closeTag
	;
	BACnetAPDUEncoder	enc
	;
	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
	} else
		throw "Closing tag requires tag number";

	// encode it
	closeTag.Encode( enc, context );

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		packet.Add( enc.pktBuffer[i] );
}

//
//	ScriptExecutor::ExpectPacket
//

bool ScriptExecutor::ExpectPacket( ScriptNetFilterPtr fp, const BACnetNPDU &npdu )
{
	int						bipMsgID = -1, nlMsgID = -1, alMsgID = -1
	,						valu
	;
	BACnetOctet				ctrl = 0
	;
	CByteArray				packet
	;
	ScriptFilterPtr			sfp
	;
	ScriptNetFilterPtr		nfp
	;
	ScriptPacketExprPtr		pNet, bipMsg, nlMsg, nlSA, alMsg
	,						pVersion, pDNET, pDADR, pHopCount, pSNET, pSADR
	,						pDER, pPriority
	;
	BACnetAddress			nlSrcAddr
	;
	BACnetBoolean			nlDER
	;
	BACnetInteger			nlVersion, nlDNET, nlSNET
	,						nlPriority, nlHopCount
	;
	BACnetOctetString		nlSrc, nlDADR, nlSADR, nlData, alData
	;
	BACnetCharacterString	nlNetwork
	;

	try {
		// see if the netework is provided
		pNet = GetKeywordValue( kwNETWORK, nlNetwork );

		// see if this should have a BVLCI header
		bipMsg = execPacket->packetExprList.Find( kwBVLCI );

		// see if we're looking for a network layer message
		nlMsg = execPacket->packetExprList.Find( kwMESSAGE );
		if (!nlMsg)
			nlMsg = execPacket->packetExprList.Find( kwMSG );

		// see if we're looking for an application layer message
		alMsg = execPacket->packetExprList.Find( kwPDU );

		// See if matching should come from device layer.  This is a quiet fail,
		// no message is going to be saved saying that this combination was 
		// skipped.  At some point the packet is going to be passed up through 
		// to a VTSServer or VTSClient and that will call ReceiveAPDU().
		if (!pNet && !bipMsg && !nlMsg && alMsg)
			return false;

		// look for the network (actually the filter name)
		if (gMasterFilterList.Length() == 1) {
			sfp = gMasterFilterList[0];
			if (pNet && !nlNetwork.Equals(sfp->filterName))
				throw ExecError( "Port not found", pNet->exprLine );
		} else {
			if (!pNet)
				throw ExecError( "Network required", execPacket->baseLineStart );

			if (pNet->exprOp != '=')
				throw ExecError( "Equality operator required for network keyword", pNet->exprLine );
			
			for (int i = 0; i < gMasterFilterList.Length(); i++) {
				sfp = gMasterFilterList[i];
				if (nlNetwork.Equals(sfp->filterName))
					break;
			}
			if (i >= gMasterFilterList.Length())
				throw ExecError( "Port not found", pNet->exprLine );
		}

		// make sure its a network filter for now
		if (sfp->filterType != scriptNPDUFilter)
			throw ExecError( "NPDU filter only", pNet->exprLine );
		nfp = (ScriptNetFilterPtr)sfp;

		// did this packet come in on the network?
		if (nfp != fp)
			throw "Network mismatch";

		// get the source
		nlSA = execPacket->packetExprList.Find( kwSA );
		if (!nlSA) {
			for (int i = 0; i < execDB->m_Names.Length(); i++ ) {
				VTSNameDesc		nameDesc;

				execDB->m_Names.ReadName( i, &nameDesc );
				if (stricmp(nameDesc.nameName,"IUT") == 0) {
					nlSrcAddr = nameDesc.nameAddr;
					break;
				}
			}
			if (i >= execDB->m_Names.Length())
				throw ExecError( "Default source address IUT not found", execPacket->baseLineStart );
		} else {
			ScriptTokenList			saList
			;

			if (nlSA->exprOp != '=')
				throw ExecError( "Equality operator required for source address keyword", nlSA->exprLine );

			// resolve the expressions
			ResolveExpr( nlSA->exprValue, nlSA->exprLine, saList );
			if (saList.Length() < 1)
				throw ExecError( "Address, name or keyword expected", nlSA->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = saList[0];

			// check to see if this is a keyword
			if (t.tokenType == scriptKeyword) {
				if ((t.tokenSymbol == kwBROADCAST) || (t.tokenSymbol == kwLOCALBROADCAST))
					nlSrcAddr.LocalBroadcast();
				else
					throw ExecError( "Unrecognized keyword", nlSA->exprLine );
			} else
			// it might be a name
			if ((t.tokenType == scriptValue) && (t.tokenEnc == scriptASCIIEnc)) {
				CString tvalu = t.RemoveQuotes();
				for (int i = 0; i < execDB->m_Names.Length(); i++ ) {
					VTSNameDesc		nameDesc;

					execDB->m_Names.ReadName( i, &nameDesc );
					if (stricmp(nameDesc.nameName,tvalu) == 0) {
						nlSrcAddr = nameDesc.nameAddr;
						break;
					}
				}
				if (i >= execDB->m_Names.Length())
					throw ExecError( "Source address name not found", nlSA->exprLine );
			} else
			// it might be an IP address
			if ((t.tokenType == scriptValue) && (t.tokenEnc == scriptIPEnc)) {
				BACnetIPAddr nlIPAddr( t.tokenValue );
				nlSrcAddr = nlIPAddr;
			} else
			// it might be an explicit octet string
			if (t.IsEncodeable( nlSrc )) {
				nlSrcAddr.LocalStation( nlSrc.strBuff, nlSrc.strLen );
			} else
				throw ExecError( "Source address expected", nlSA->exprLine );
		}

		// did this packet come from the expected station?
		if (!(npdu.pduAddr == nlSrcAddr))
			throw "Source address mismatch";

		// match the DER
		pDER = GetKeywordValue( kwDER, nlDER );
		if (pDER && !Match(pDER->exprOp,npdu.pduExpectingReply,nlDER.boolValue))
			throw ExecError( "Data expecting reply mismatch", pDER->exprLine );

		// match the priority
		pPriority = GetKeywordValue( kwPRIORITY, nlPriority );
		if (pPriority && !Match(pPriority->exprOp,npdu.pduNetworkPriority,nlPriority.intValue))
			throw ExecError( "Priority mismatch", pPriority->exprLine );

		// the rest of this code will need a decoder
		BACnetAPDUDecoder	dec( npdu.pduData, npdu.pduLen );

		// if this is a BVLL specific message, match it
		if (bipMsg) {
			ScriptTokenList	bvllList;

			if (bipMsg->exprOp != '=')
				throw ExecError( "Equality operator required for BVLCI", bipMsg->exprLine );

			// resolve the expressions
			ResolveExpr( bipMsg->exprValue, bipMsg->exprLine, bvllList );
			if (bvllList.Length() < 1)
				throw ExecError( "BIP message keyword expected", bipMsg->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = bvllList[0];

			// check to see if this is a keyword
			if (t.tokenType == scriptKeyword) {
				bipMsgID = t.Lookup( t.tokenSymbol, ScriptBIPMsgTypeMap );
				if (bipMsgID < 0)
					throw ExecError( "Unrecognized keyword", bipMsg->exprLine );
			} else
				throw ExecError( "Keyword expected", bipMsg->exprLine );

			// based on the number, check for other parameters
			try {
				// verify the message type and function
				if (0x81 != (dec.pktLength--,*dec.pktBuffer++))
					throw ExecError( "BVLCI expected", bipMsg->exprLine );
				if (bipMsgID != (dec.pktLength--,*dec.pktBuffer++))
					throw ExecError( "BVLCI message mismatch", bipMsg->exprLine );

				// verify the length
				int len;
				len = (dec.pktLength--,*dec.pktBuffer++);
				len = (len << 8) + (dec.pktLength--,*dec.pktBuffer++);
				if (len != npdu.pduLen)
					throw "BVLCI length incorrect";

				switch (bipMsgID) {
					case 0:						// BVLC-RESULT, rslt
						ExpectBVLCResult( bvllList, dec );
						break;
					case 1:						// WRITE-BROADCAST-DISTRIBUTION-TABLE {,host/net:port}*
						ExpectWriteBDT( bvllList, dec );
						break;
					case 2:						// READ-BROADCAST-DISTRIBUTION-TABLE
						if (bvllList.Length() != 1)
							throw "BVLC function takes no parameters";
						break;
					case 3:						// READ-BROADCAST-DISTRIBUTION-TABLE-ACK {,host/net:port}*
						ExpectReadBDTAck( bvllList, dec );
						break;
					case 4:						// FORWARDED-NPDU, host:port
						ExpectForwardedNPDU( bvllList, dec );
						break;
					case 5:						// REGISTER-FOREIGN-DEVICE, ttl
						ExpectRegisterFD( bvllList, dec );
						break;
					case 6:						// READ-FOREIGN-DEVICE-TABLE
						if (bvllList.Length() != 1)
							throw "BVLC function takes no parameters";
						break;
					case 7:						// READ-FOREIGN-DEVICE-TABLE-ACK {,host:port,ttl,tr}*
						ExpectReadFDTAck( bvllList, dec );
						break;
					case 8:						// DELETE-FOREIGN-DEVICE-TABLE-ENTRY
						ExpectDeleteFDTEntry( bvllList, dec );
						break;
					case 9:						// DISTRIBUTE-BROADCAST-TO-NETWORK
					case 10:					// ORIGINAL-UNICAST-NPDU
					case 11:					// ORIGINAL-BROADCAST-NPDU
						if (bvllList.Length() != 1)
							throw "BVLC function takes no parameters";
						break;
				}
			}
			catch (const char *errMsg) {
				// one of the functions had an error
				throw ExecError( errMsg, bipMsg->exprLine );
			}

			// if this doesn't have an NPDU that follows, we're done
			if ((bipMsgID != 4) && (bipMsgID != 9) && (bipMsgID != 10) && (bipMsgID != 11)) {
				// success
				return true;
			}
		}

		// there should be a network or application layer message
		if (!nlMsg && !alMsg)
			return true;
		if (nlMsg && alMsg)
			throw "Specify network or application message but not both";

		// match the version
		valu = (dec.pktLength--,*dec.pktBuffer++);
		pVersion = GetKeywordValue( kwVERSION, nlVersion );
		if (pVersion && !Match(pVersion->exprOp,valu,nlPriority.intValue))
			throw ExecError( "Version mismatch", pVersion->exprLine );

		// suck out the control field
		ctrl = (dec.pktLength--,*dec.pktBuffer++);

		// check to see if we got what we're looking for
		if (nlMsg && ((ctrl & 0x80) == 0))
			throw ExecError( "Not a network message", nlMsg->exprLine );
		if (alMsg && ((ctrl & 0x80) != 0))
			throw ExecError( "Not an application message", alMsg->exprLine );

		// if we have a DNET/DADR, check it
		if ((ctrl & 0x20) != 0) {
			// suck out the DNET
			valu = (dec.pktLength--,*dec.pktBuffer++);
			valu = (valu << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// are we looking for a specific DNET?
			pDNET = GetKeywordValue( kwDNET, nlDNET );
			if (pDNET && !Match(pDNET->exprOp,valu,nlDNET.intValue))
				throw ExecError( "DNET mismatch", pDNET->exprLine );

			// suck out the length
			int len = (dec.pktLength--,*dec.pktBuffer++);

			// looking for a specific address?
			pDADR = GetKeywordValue( kwDADR, nlDADR );
			if (pDADR) {
				if (pDADR->exprOp != '=')
					throw ExecError( "Equality operator required for DADR", pDADR->exprLine );
				if (len == 0)
					throw ExecError( "No DADR in packet", pDADR->exprLine );
				if (len != nlDADR.strLen)
					throw ExecError( "DADR mismatch", pDADR->exprLine );
				for (int ii = 0; ii < len; ii++)
					if (nlDADR.strBuff[ii] != (dec.pktLength--,*dec.pktBuffer++))
						throw ExecError( "DADR mismatch", pDADR->exprLine );
			}
		}

		// if we have an SNET/SADR, check it
		if ((ctrl & 0x08) != 0) {
			// suck out the SNET
			valu = (dec.pktLength--,*dec.pktBuffer++);
			valu = (valu << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// are we looking for a specific SNET?
			pSNET = GetKeywordValue( kwSNET, nlSNET );
			if (pSNET && !Match(pSNET->exprOp,valu,nlSNET.intValue))
				throw ExecError( "SNET mismatch", pVersion->exprLine );

			// suck out the length
			int len = (dec.pktLength--,*dec.pktBuffer++);

			// looking for a specific address?
			pSADR = GetKeywordValue( kwSADR, nlSADR );
			if (pSADR) {
				if (pSADR->exprOp != '=')
					throw ExecError( "Equality operator required for SADR", pSADR->exprLine );
				if (len == 0)
					throw ExecError( "No SADR in packet", pSADR->exprLine );
				if (len != nlSADR.strLen)
					throw ExecError( "SADR mismatch", pSADR->exprLine );
				for (int ii = 0; ii < len; ii++)
					if (nlSADR.strBuff[ii] != (dec.pktLength--,*dec.pktBuffer++))
						throw ExecError( "SADR mismatch", pSADR->exprLine );
			}
		}

		// if we have a DNET/DADR, check the hop count
		if ((ctrl & 0x20) != 0) {
			// suck out the hop count
			valu = (dec.pktLength--,*dec.pktBuffer++);
			pHopCount = GetKeywordValue( kwHOPCOUNT, nlHopCount );
			if (pHopCount && !Match(pHopCount->exprOp,valu,nlHopCount.intValue))
				throw ExecError( "Hop count mismatch", pHopCount->exprLine );
		}

		// if this is a network layer message, check it
		if (nlMsg) {
			ScriptTokenList	nlList;

			if (nlMsg->exprOp != '=')
				throw ExecError( "Equality operator required for network message", nlMsg->exprLine );

			// resolve the expressions
			ResolveExpr( nlMsg->exprValue, nlMsg->exprLine, nlList );
			if (nlList.Length() < 1)
				throw ExecError( "Network message keyword expected", nlMsg->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = nlList[0];

			// check to see if this is a keyword
			if (t.tokenType == scriptKeyword) {
				nlMsgID = t.Lookup( t.tokenSymbol, ScriptNLMsgTypeMap );
				if (nlMsgID < 0)
					throw ExecError( "Unrecognized keyword", nlMsg->exprLine );
			} else
				throw ExecError( "Keyword expected", nlMsg->exprLine );

			// based on the number, check for other parameters
			try {
				// verify the message type
				if (nlMsgID != (dec.pktLength--,*dec.pktBuffer++))
					throw ExecError( "Netork message mismatch", nlMsg->exprLine );

				switch (nlMsgID) {
					case 0:						// WHO-IS-ROUTER-TO-NETWORK
						ExpectWhoIsRouterToNetwork( nlList, dec );
						break;
					case 1:						// I-AM-ROUTER-TO-NETWORK
						ExpectIAmRouterToNetwork( nlList, dec );
						break;
					case 2:						// I-COULD-BE-ROUTER-TO-NETWORK
						ExpectICouldBeRouterToNetwork( nlList, dec );
						break;
					case 3:						// REJECT-MESSAGE-TO-NETWORK
						ExpectRejectMessageToNetwork( nlList, dec );
						break;
					case 4:						// ROUTER-BUSY-TO-NETWORK
						ExpectRouterBusyToNetwork( nlList, dec );
						break;
					case 5:						// ROUTER-AVAILABLE-TO-NETWORK
						ExpectRouterAvailableToNetwork( nlList, dec );
						break;
					case 6:						// INITIALIZE-ROUTING-TABLE
					case 7:						// INITIALIZE-ROUTING-TABLE-ACK
						ExpectInitializeRoutingTable( nlList, dec );
						break;
					case 8:						// ESTABLISH-CONNECTION-TO-NETWORK
						ExpectEstablishConnectionToNetwork( nlList, dec );
						break;
					case 9:						// DISCONNECT-CONNECTION-TO-NETWORK
						ExpectDisconnectConnectionToNetwork( nlList, dec );
						break;
				}
			}
			catch (const char *errMsg) {
				// one of the functions had an error
				throw ExecError( errMsg, nlMsg->exprLine );
			}
		}
		
		// if this is an application layer message, check it
		if (alMsg) {
			ScriptTokenList	alList;

			// force property references to fail until context established
			execObjID = 0xFFFFFFFF;

			// resolve the expressions
			ResolveExpr( alMsg->exprValue, alMsg->exprLine, alList );
			if (alList.Length() < 1)
				throw ExecError( "AL message keyword expected", alMsg->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = alList[0];

			// check to see if this is a keyword
			if (t.tokenType == scriptKeyword) {
				alMsgID = t.Lookup( t.tokenSymbol, ScriptALMsgTypeMap );
				if (alMsgID < 0)
					throw ExecError( "Unrecognized keyword", alMsg->exprLine );
			} else
				throw ExecError( "Keyword expected", alMsg->exprLine );

			// based on the number, check for other parameters
			try {
				switch (alMsgID) {
					case 0:						// CONFIRMED-REQUEST
						ExpectConfirmedRequest( dec );
						break;
					case 1:						// UNCONFIRMED-REQUEST
						ExpectUnconfirmedRequest( dec );
						break;
					case 2:						// SIMPLEACK
						ExpectSimpleACK( dec );
						break;
					case 3:						// COMPLEXACK
						ExpectComplexACK( dec );
						break;
					case 4:						// SEGMENTACK
						ExpectSegmentACK( dec );
						break;
					case 5:						// ERROR
						ExpectError( dec );
						break;
					case 6:						// REJECT
						ExpectReject( dec );
						break;
					case 7:						// ABORT
						ExpectAbort( dec );
						break;
				}
			}
			catch (const char *errMsg) {
				// one of the functions had an error
				throw ExecError( errMsg, alMsg->exprLine );
			}
			catch (CString strThrowMessage) 
			{
				throw ExecError( strThrowMessage, alMsg->exprLine );
			}
		}
	}
	catch (const ExecError &err) {
		// failed
		Msg( 3, err.errLineNo, err.errMsg );
		return false;
	}
	catch (const char *errMsg) {
		// failed
		Msg( 3, execPacket->baseLineStart, errMsg );
		return false;
	}

	// we made it!
	return true;
}

//
//	ScriptExecutor::ExpectBVLCResult
//

void ScriptExecutor::ExpectBVLCResult( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int		valu1, valu2
	;

	TRACE0( "ExpectBVLCResult\n" );

	switch (tlist.Length()) {
		case 1:
			throw "Result code expected";
		case 2:
			{
				const ScriptToken &dnet = tlist[1];

				if (dnet.tokenType != scriptValue)
					throw "Result code expected";

				// madanner 11/4/02
				// account for don't care case.  Not sure if this is a good thing or not.

				bool fDontCare = dnet.IsDontCare();

				if ( !fDontCare )
				{
					if ( !dnet.IsInteger(valu1))
						throw "Result code invalid format";
					if ((valu1 < 0) || (valu1 > 65535))
						throw "Result code out of range (0..65535)";
				}

				TRACE1( "    Result code %d\n", valu1 );

				// extract the value
				valu2 = (dec.pktLength--,*dec.pktBuffer++);
				valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

				// check for a match
				if ( !fDontCare && valu1 != valu2)
					throw "Result code mismatch";

				break;
			}
		default:
			throw "Too many values in BVLC-Result message";
	}
}

//
//	ScriptExecutor::ExpectWriteBDT
//

void ScriptExecutor::ExpectWriteBDT( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int					i
	;
	unsigned long		host1, host2, mask1, mask2
	;
	unsigned short		port1, port2
	;

	TRACE0( "ExpectWriteBDT\n" );

	try {
		for (i = 1; i < tlist.Length(); i++ ) {
			const ScriptToken &t = tlist[i];

			if (t.tokenType != scriptValue)
				throw "IP address expected";

			// madanner 11/4/02
			// account for don't care case.  Not sure if this is a good thing or not.

			bool fDontCare = t.IsDontCare();

			if ( !fDontCare )
			{
				if (t.tokenEnc != scriptIPEnc)
					throw "IP address required";

				// convert to host, port and network mask
				BACnetIPAddr::StringToHostPort( t.tokenValue, &host1, &mask1, &port1 );
			}

			// make sure there's enough data
			if (dec.pktLength < 10)
				throw "Additional BDT entries expected";

			// extract the host
			for (int j = 0; j < 4; j++)
				host2 = (host2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare && host1 != host2)
				throw "Host address mismatch";

			// extract the port
			port2 = (dec.pktLength--,*dec.pktBuffer++);
			port2 = (port2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if (!fDontCare && port1 != port2)
				throw "Port mismatch";

			// extract the mask
			for (int k = 0; k < 4; k++)
				mask2 = (mask2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare &&  mask1 != mask2)
				throw "Subnet mask mismatch";
		}

		if (dec.pktLength != 0)
			throw "Additional BDT entries not matched";
	}
	catch (char *msg) {
		static char msgBuff[80];

		sprintf( msgBuff, "%s (entry %d)", msg, i );
		throw msgBuff;
	}
}

//
//	ScriptExecutor::ExpectReadBDTAck
//

void ScriptExecutor::ExpectReadBDTAck( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int					i
	;
	unsigned long		host1, host2, mask1, mask2
	;
	unsigned short		port1, port2
	;

	TRACE0( "ExpectReadBDTAck\n" );

	try {
		for (i = 1; i < tlist.Length(); i++ ) {
			const ScriptToken &t = tlist[i];

			if (t.tokenType != scriptValue)
				throw "IP address expected";

			// madanner 11/4/02
			// account for don't care case.  Not sure if this is a good thing or not.

			bool fDontCare = t.IsDontCare();

			if ( !fDontCare )
			{
				if (t.tokenEnc != scriptIPEnc)
					throw "IP address required";

				// convert to host, port and network mask
				BACnetIPAddr::StringToHostPort( t.tokenValue, &host1, &mask1, &port1 );
			}

			// make sure there's enough data
			if (dec.pktLength < 10)
				throw "Additional BDT entries expected";

			// extract the host
			for (int j = 0; j < 4; j++)
				host2 = (host2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare && host1 != host2)
				throw "Host address mismatch";

			// extract the port
			port2 = (dec.pktLength--,*dec.pktBuffer++);
			port2 = (port2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare && port1 != port2)
				throw "Port mismatch";

			// extract the mask
			for (int k = 0; k < 4; k++)
				mask2 = (mask2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare && mask1 != mask2)
				throw "Subnet mask mismatch";
		}

		if (dec.pktLength != 0)
			throw "Additional BDT entries not matched";
	}
	catch (char *msg) {
		static char msgBuff[80];

		sprintf( msgBuff, "%s (entry %d)", msg, i );
		throw msgBuff;
	}
}

//
//	ScriptExecutor::ExpectForwardedNPDU
//

void ScriptExecutor::ExpectForwardedNPDU( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	TRACE0( "ExpectForwardedNPDU\n" );

	switch (tlist.Length()) {
		case 1:
			throw "IP address required";
		case 2:
			{
				unsigned long		host1, host2 = 0, mask1
				;
				unsigned short		port1, port2
				;

				const ScriptToken &t = tlist[1];

				if (t.tokenType != scriptValue)
					throw "IP address expected";

				// madanner 11/4/02
				// account for don't care case.  Not sure if this is a good thing or not.

				bool fDontCare = t.IsDontCare();

				if ( !fDontCare )
				{
					if (t.tokenEnc != scriptIPEnc)
						throw "IP address required";

					// convert to host, port and network mask
					BACnetIPAddr::StringToHostPort( t.tokenValue, &host1, &mask1, &port1 );
				}

				// make sure there's enough data
				if (dec.pktLength < 6)
					throw "B/IP address expected";

				// extract the host
				for (int j = 0; j < 4; j++)
					host2 = (host2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

				// see if they match
				if ( !fDontCare && host1 != host2)
					throw "Host address mismatch";

				// extract the port
				port2 = (dec.pktLength--,*dec.pktBuffer++);
				port2 = (port2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

				// see if they match
				if ( !fDontCare && port1 != port2)
					throw "Port mismatch";
				break;
			}
		default:
			throw "Too many values";
	}
}

//
//	ScriptExecutor::ExpectRegisterFD
//

void ScriptExecutor::ExpectRegisterFD( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int		valu1, valu2
	;

	bool fDontCare = false;

	TRACE0( "ExpectRegisterFD\n" );

	switch (tlist.Length()) {
		case 1:
			throw "Time-to-live required";
		case 2:
			{
				const ScriptToken &dnet = tlist[1];

				if (dnet.tokenType != scriptValue)
					throw "Time-to-live expected";

				// madanner 11/4/02
				// account for don't care case.  Not sure if this is a good thing or not.

				bool fDontCare = dnet.IsDontCare();

				if ( !fDontCare )
				{
					if (!dnet.IsInteger(valu1))
						throw "Time-to-live invalid format";
					if ((valu1 < 0) || (valu1 > 65535))
						throw "Time-to-live out of range (0..65535)";
				}

				TRACE1( "    TTL %d\n", valu1 );

				// extract the value
				valu2 = (dec.pktLength--,*dec.pktBuffer++);
				valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

				// check for a match
				if ( !fDontCare && valu1 != valu2)
					throw "Time-to-live mismatch";

				break;
			}
		default:
			throw "Too many values in Register-Foreign-Device message";
	}
}

//
//	ScriptExecutor::ExpectReadFDTAck
//

void ScriptExecutor::ExpectReadFDTAck( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int					i, entry, valu1, valu2
	;
	unsigned long		host1, host2, mask1
	;
	unsigned short		port1, port2
	;

	TRACE0( "ExpectReadFDTAck\n" );

	try {
		for (i = 1, entry = 1; i < tlist.Length(); entry++ ) {
			const ScriptToken &t = tlist[i++];

			if (t.tokenType != scriptValue)
				throw "IP address expected";

			// madanner 11/4/02
			// account for don't care case.  Not sure if this is a good thing or not.

			bool fDontCare = t.IsDontCare();

			if ( !fDontCare )
			{
				if (t.tokenEnc != scriptIPEnc)
					throw "IP address required";

				// convert to host, port and network mask
				BACnetIPAddr::StringToHostPort( t.tokenValue, &host1, &mask1, &port1 );
			}

			// make sure there's enough data
			if (dec.pktLength < 10)
				throw "Additional FDT entries expected";

			// extract the host
			for (int j = 0; j < 4; j++)
				host2 = (host2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare &&  host1 != host2)
				throw "Host address mismatch";

			// extract the port
			port2 = (dec.pktLength--,*dec.pktBuffer++);
			port2 = (port2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare &&  port1 != port2)
				throw "Port mismatch";

			// extract the ttl
			if (i >= tlist.Length())
				throw "Time-to-live expected";
			const ScriptToken &ttl = tlist[i++];

			if (ttl.tokenType != scriptValue)
				throw "Time-to-live expected";

			fDontCare = ttl.IsDontCare();

			if ( !fDontCare )
			{
				if (!ttl.IsInteger(valu1))
					throw "Time-to-live invalid format";
				if ((valu1 < 0) || (valu1 > 65535))
					throw "Time-to-live out of range (0..65535)";
			}

			TRACE1( "    TTL %d\n", valu1 );

			// extract the value
			valu2 = (dec.pktLength--,*dec.pktBuffer++);
			valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// check for a match
			if ( !fDontCare && valu1 != valu2)
				throw "Time-to-live mismatch";

			// extract the tr
			if (i >= tlist.Length())
				throw "Time remaining expected";
			const ScriptToken &tr = tlist[i++];

			if (tr.tokenType != scriptValue)
				throw "Time remaining expected";

			fDontCare = tr.IsDontCare();

			if ( !fDontCare )
			{
				if (!tr.IsInteger(valu1))
					throw "Time remaining invalid format";
				if ((valu1 < 0) || (valu1 > 65535))
					throw "Time remaining out of range (0..65535)";
			}

			TRACE1( "    TR %d\n", valu1 );

			// extract the value
			valu2 = (dec.pktLength--,*dec.pktBuffer++);
			valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// check for a match
			if ( !fDontCare && valu1 != valu2)
				throw "Time remaining mismatch";
		}

		if (dec.pktLength != 0)
			throw "Additional BDT entries not matched";
	}
	catch (char *msg) {
		static char msgBuff[80];

		sprintf( msgBuff, "%s (entry %d)", msg, entry );
		throw msgBuff;
	}
}

//
//	ScriptExecutor::ExpectDeleteFDTEntry
//

void ScriptExecutor::ExpectDeleteFDTEntry( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	TRACE0( "ExpectDeleteFDTEntry\n" );

	switch (tlist.Length()) {
		case 1:
			throw "IP address required";
		case 2:
			{
				unsigned long		host1, host2 = 0, mask1
				;
				unsigned short		port1, port2
				;

				const ScriptToken &t = tlist[1];

				if (t.tokenType != scriptValue)
					throw "IP address expected";

				bool fDontCare = t.IsDontCare();

				if ( !fDontCare )
				{
					if (t.tokenEnc != scriptIPEnc)
						throw "IP address required";

					// convert to host, port and network mask
					BACnetIPAddr::StringToHostPort( t.tokenValue, &host1, &mask1, &port1 );
				}

				// make sure there's enough data
				if (dec.pktLength < 6)
					throw "B/IP address expected";

				// extract the host
				for (int j = 0; j < 4; j++)
					host2 = (host2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

				// see if they match
				if ( !fDontCare && host1 != host2)
					throw "Entry address mismatch";

				// extract the port
				port2 = (dec.pktLength--,*dec.pktBuffer++);
				port2 = (port2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

				// see if they match
				if ( !fDontCare && port1 != port2)
					throw "Port mismatch";
				break;
			}
		default:
			throw "Too many values";
	}
}

//
//	ScriptExecutor::ExpectWhoIsRouterToNetwork
//

void ScriptExecutor::ExpectWhoIsRouterToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int		valu1, valu2
	;

	TRACE0( "ExpectWhoIsRouterToNetwork\n" );

	switch (tlist.Length()) {
		case 1:
			if (dec.pktLength != 0)
				throw "Network not matched";
			break;
		case 2:
			{
				const ScriptToken &dnet = tlist[1];

				if (dnet.tokenType != scriptValue)
					throw "Network expected";

				bool fDontCare = dnet.IsDontCare();

				if ( !fDontCare )
				{
					if (!dnet.IsInteger(valu1))
						throw "Network invalid format";
					if ((valu1 < 0) || (valu1 > 65534))
						throw "Network out of range (0..65534)";
				}

				TRACE1( "    Network %d\n", valu1 );

				// extract the value
				valu2 = (dec.pktLength--,*dec.pktBuffer++);
				valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

				// check for a match
				if ( !fDontCare && valu1 != valu2)
					throw "Network mismatch";

				break;
			}
		default:
			throw "Too many values in Who-Is-Router-To-Network message";
	}
}

//
//	ScriptExecutor::ExpectIAmRouterToNetwork
//

void ScriptExecutor::ExpectIAmRouterToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int					i, valu1, valu2
	;

	TRACE0( "ExpectIAmRouterToNetwork\n" );

	if (tlist.Length() == 1)
		throw "At least one network number required";

	try {
		for (i = 1; i < tlist.Length(); i++ ) {
			const ScriptToken &t = tlist[i];

			if (t.tokenType != scriptValue)
				throw "Network expected";

			bool fDontCare = t.IsDontCare();

			if ( !fDontCare )
			{
				if (!t.IsInteger(valu1))
					throw "Network invalid format";
				if ((valu1 < 0) || (valu1 > 65534))
					throw "Network out of range (0..65534)";
			}

			TRACE1( "    Network %d\n", valu1 );

			// got at least one more?
			if (dec.pktLength < 2)
				throw "Additional network expected";

			// extract the port
			valu2 = (dec.pktLength--,*dec.pktBuffer++);
			valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare && valu1 != valu2)
				throw "Network mismatch";
		}

		if (dec.pktLength != 0)
			throw "Additional network numbers not matched";
	}
	catch (char *msg) {
		static char msgBuff[80];

		sprintf( msgBuff, "%s (entry %d)", msg, i );
		throw msgBuff;
	}
}

//
//	ScriptExecutor::ExpectICouldBeRouterToNetwork
//

void ScriptExecutor::ExpectICouldBeRouterToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int		valu1, valu2
	;

	TRACE0( "ExpectICouldBeRouterToNetwork\n" );

	if (tlist.Length() != 3)
		throw "DNET and performance index required";

	const ScriptToken &dnet = tlist[1];

	if (dnet.tokenType != scriptValue)
		throw "DNET expected";

	bool fDontCare = dnet.IsDontCare();

	if ( !fDontCare )
	{
		if (!dnet.IsInteger(valu1))
			throw "DNET invalid format";
		if ((valu1 < 0) || (valu1 > 65534))
			throw "DNET out of range (0..65534)";
	}

	TRACE1( "    Network %d\n", valu1 );

	// extract the network
	valu2 = (dec.pktLength--,*dec.pktBuffer++);
	valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

	// see if they match
	if ( !fDontCare && valu1 != valu2)
		throw "Network mismatch";

	const ScriptToken &perf = tlist[2];

	if (perf.tokenType != scriptValue)
		throw "Performance index expected";

	fDontCare = perf.IsDontCare();

	if ( !fDontCare )
	{
		if (!perf.IsInteger(valu1))
			throw "Performance index invalid format";
		if ((valu1 < 0) || (valu1 > 255))
			throw "Performance index out of range (0..255)";
	}

	TRACE1( "    Performance %d\n", valu1 );

	// extract the network
	valu2 = (dec.pktLength--,*dec.pktBuffer++);

	// see if they match
	if ( !fDontCare && valu1 != valu2)
		throw "Performance index mismatch";
}

//
//	ScriptExecutor::ExpectRejectMessageToNetwork
//

void ScriptExecutor::ExpectRejectMessageToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int		valu1, valu2
	;

	TRACE0( "ExpectRejectMessageToNetwork\n" );

	if (tlist.Length() != 3)
		throw "Reject reason and DNET required";

	const ScriptToken &reason = tlist[1];

	if (reason.tokenType != scriptValue)
		throw "Reject reason expected";

	bool fDontCare = reason.IsDontCare();

	if ( !fDontCare )
	{
		if (!reason.IsInteger(valu1))
			throw "Reject reason invalid format";
		if ((valu1 < 0) || (valu1 > 255))
			throw "Reject reason out of range (0..255)";
	}

	TRACE1( "    Reject reason %d\n", valu1 );

	// extract the network
	valu2 = (dec.pktLength--,*dec.pktBuffer++);

	// see if they match
	if ( !fDontCare && valu1 != valu2)
		throw "Reject reason mismatch";

	const ScriptToken &dnet = tlist[2];

	if (dnet.tokenType != scriptValue)
		throw "DNET expected";

	fDontCare = dnet.IsDontCare();

	if ( !fDontCare )
	{
		if (!dnet.IsInteger(valu1))
			throw "DNET invalid format";
		if ((valu1 < 0) || (valu1 > 65534))
			throw "DNET out of range (0..65534)";
	}

	TRACE1( "    DNET %d\n", valu1 );

	// extract the network
	valu2 = (dec.pktLength--,*dec.pktBuffer++);
	valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

	// see if they match
	if ( !fDontCare && valu1 != valu2)
		throw "Network mismatch";
}

//
//	ScriptExecutor::ExpectRouterBusyToNetwork
//

void ScriptExecutor::ExpectRouterBusyToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int					i, valu1, valu2
	;

	TRACE0( "ExpectRouterBusyToNetwork\n" );

	try {
		for (i = 1; i < tlist.Length(); i++ ) {
			const ScriptToken &t = tlist[i];

			if (t.tokenType != scriptValue)
				throw "Network expected";

			bool fDontCare = t.IsDontCare();

			if ( !fDontCare )
			{
				if (!t.IsInteger(valu1))
					throw "Network invalid format";
				if ((valu1 < 0) || (valu1 > 65534))
					throw "Network out of range (0..65534)";
			}

			TRACE1( "    Network %d\n", valu1 );

			// got at least one more?
			if (dec.pktLength < 2)
				throw "Additional network expected";

			// extract the port
			valu2 = (dec.pktLength--,*dec.pktBuffer++);
			valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare && valu1 != valu2)
				throw "Network mismatch";
		}

		if (dec.pktLength != 0)
			throw "Additional network numbers not matched";
	}
	catch (char *msg) {
		static char msgBuff[80];

		sprintf( msgBuff, "%s (entry %d)", msg, i );
		throw msgBuff;
	}
}

//
//	ScriptExecutor::ExpectRouterAvailableToNetwork
//

void ScriptExecutor::ExpectRouterAvailableToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int					i, valu1, valu2
	;

	TRACE0( "ExpectRouterAvailableToNetwork\n" );

	try {
		for (i = 1; i < tlist.Length(); i++ ) {
			const ScriptToken &t = tlist[i];

			if (t.tokenType != scriptValue)
				throw "Network expected";

			bool fDontCare = t.IsDontCare();

			if ( !fDontCare )
			{
				if (!t.IsInteger(valu1))
					throw "Network invalid format";
				if ((valu1 < 0) || (valu1 > 65534))
					throw "Network out of range (0..65534)";
			}

			TRACE1( "    Network %d\n", valu1 );

			// got at least one more?
			if (dec.pktLength < 2)
				throw "Additional network expected";

			// extract the network
			valu2 = (dec.pktLength--,*dec.pktBuffer++);
			valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

			// see if they match
			if ( !fDontCare && valu1 != valu2)
				throw "Network mismatch";
		}

		if (dec.pktLength != 0)
			throw "Additional network numbers not matched";
	}
	catch (char *msg) {
		static char msgBuff[80];

		sprintf( msgBuff, "%s (entry %d)", msg, i );
		throw msgBuff;
	}
}

//
//	ScriptExecutor::ExpectInitializeRoutingTable
//

void ScriptExecutor::ExpectInitializeRoutingTable( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int						valu1, valu2
	;
	BACnetCharacterString	cstr
	;

	TRACE0( "ExpectInitializeRoutingTable[Ack]\n" );

	// extract the number of ports
	valu1 = (dec.pktLength--,*dec.pktBuffer++);
	if (valu1 != (tlist.Length() - 1) / 3 )
		throw "Port count mismatch";

	for (int i = 1; i < tlist.Length(); ) {
		const ScriptToken &dnet = tlist[i++];

		if (dnet.tokenType != scriptValue)
			throw "DNET expected";

		bool fDontCare = dnet.IsDontCare();

		if ( !fDontCare )
		{
			if (!dnet.IsInteger(valu1))
				throw "DNET invalid format";
			if ((valu1 < 0) || (valu1 > 65534))
				throw "DNET out of range (0..65534)";
		}

		TRACE1( "    Network %d\n", valu1 );

		// extract the network
		valu2 = (dec.pktLength--,*dec.pktBuffer++);
		valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

		// see if they match
		if ( !fDontCare &&  valu1 != valu2)
			throw "Network mismatch";

		if (i >= tlist.Length())
			throw "Port ID expected";
		const ScriptToken &portID = tlist[i++];

		if (portID.tokenType != scriptValue)
			throw "Port ID expected";

		fDontCare = portID.IsDontCare();

		if ( !fDontCare )
		{
			if (!portID.IsInteger(valu1))
				throw "Port ID invalid format";
		}

		TRACE1( "    Port ID %d\n", valu1 );

		// extract the port ID
		valu2 = (dec.pktLength--,*dec.pktBuffer++);

		// see if they match
		if ( !fDontCare && valu1 != valu2)
			throw "Port ID mismatch";

		if (i >= tlist.Length())
			throw "Port information expected";
		const ScriptToken &portInfo = tlist[i++];

		if (portInfo.tokenType != scriptValue)
			throw "Port information expected";

		fDontCare = portInfo.IsDontCare();

		if ( !fDontCare )
		{
			if (!portInfo.IsEncodeable( cstr ))
				throw "Port information invalid format";
			if (cstr.strEncoding != 0)
				throw "Port information must be ASCII encoded";
		}

		// check the port information content
		if (cstr.strLen != (dec.pktLength--,*dec.pktBuffer++))
			throw "Port information mismatch";
		for (unsigned j = 0; j < cstr.strLen; j++)
			if (cstr.strBuff[j] != (dec.pktLength--,*dec.pktBuffer++))
				if ( !fDontCare )
					throw "Port information mismatch";
	}
}

//
//	ScriptExecutor::ExpectEstablishConnectionToNetwork
//

void ScriptExecutor::ExpectEstablishConnectionToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int		valu1, valu2
	;

	TRACE0( "ExpectEstablishConnectionToNetwork\n" );

	if (tlist.Length() != 3)
		throw "DNET and termination required";

	const ScriptToken &dnet = tlist[1];

	if (dnet.tokenType != scriptValue)
		throw "DNET expected";

	bool fDontCare = dnet.IsDontCare();

	if ( !fDontCare )
	{
		if (!dnet.IsInteger(valu1))
			throw "DNET invalid format";
		if ((valu1 < 0) || (valu1 > 65534))
			throw "DNET out of range (0..65534)";
	}

	TRACE1( "    Network %d\n", valu1 );

	// extract the network
	valu2 = (dec.pktLength--,*dec.pktBuffer++);
	valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

	// see if they match
	if ( !fDontCare &&  valu1 != valu2)
		throw "Network mismatch";

	const ScriptToken &term = tlist[2];

	if (term.tokenType != scriptValue)
		throw "Termination time expected";

	fDontCare = term.IsDontCare();

	if ( !fDontCare )
	{
		if (!term.IsInteger(valu1))
			throw "Termination time invalid format";
	}

	TRACE1( "    Termination time %d\n", valu1 );

	// extract the network
	valu2 = (dec.pktLength--,*dec.pktBuffer++);

	// see if they match
	if ( !fDontCare &&  valu1 != valu2)
		throw "Termination time mismatch";
}

//
//	ScriptExecutor::ExpectDisconnectConnectionToNetwork
//

void ScriptExecutor::ExpectDisconnectConnectionToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec )
{
	int		valu1, valu2
	;

	TRACE0( "ExpectDisconnectConnectionToNetwork\n" );

	if (tlist.Length() != 2)
		throw "DNET required";

	const ScriptToken &dnet = tlist[1];

	if (dnet.tokenType != scriptValue)
		throw "DNET expected";

	bool fDontCare = dnet.IsDontCare();

	if ( !fDontCare )
	{
		if (!dnet.IsInteger(valu1))
			throw "DNET invalid format";
		if ((valu1 < 0) || (valu1 > 65534))
			throw "DNET out of range (0..65534)";
	}

	TRACE1( "    Network %d\n", valu1 );

	// extract the network
	valu2 = (dec.pktLength--,*dec.pktBuffer++);
	valu2 = (valu2 << 8) + (dec.pktLength--,*dec.pktBuffer++);

	// see if they match
	if ( !fDontCare &&  valu1 != valu2)
		throw "Network mismatch";
}

//
//	ScriptExecutor::ExpectDevPacket
//

bool ScriptExecutor::ExpectDevPacket( const BACnetAPDU &apdu )
{
	int						alMsgID, valu
	;
	BACnetOctetString		nlSource
	;
	BACnetAddress			nlSourceAddr
	;
	BACnetBoolean			nlDER
	;
	BACnetInteger			nlPriority, nlDNET
	;
	ScriptPacketExprPtr		pNet, bipMsg, nlMsg, alMsg
	,						nlSA, pDER, pPriority
	;
	ScriptTokenList			alList
	;

	try {
		// see if the netework is provided
		pNet = execPacket->packetExprList.Find( kwNETWORK );

		// see if this should have a BVLCI header
		bipMsg = execPacket->packetExprList.Find( kwBVLCI );

		// see if we're looking for a network layer message
		nlMsg = execPacket->packetExprList.Find( kwMESSAGE );
		if (!nlMsg)
			nlMsg = execPacket->packetExprList.Find( kwMSG );

		// see if we're looking for an application layer message
		alMsg = execPacket->packetExprList.Find( kwPDU );

		// see if we should be matching, it should only be an application
		// layer packet.  No need to throw anything.
		if (!(!pNet && !bipMsg && !nlMsg && alMsg))
			return false;

		// If the code supports multiple device objects in the future there 
		// needs to be a parameter that decribes which device object this 
		// packet should match.  Hunt through the gMasterDevice list for the 
		// one specified.

		// Get the source.  The IUT will be the default for all inbound messages.
		nlSA = execPacket->packetExprList.Find( kwDA );
		if (!nlSA) {
			for (int i = 0; i < execDB->m_Names.Length(); i++ ) {
				VTSNameDesc		nameDesc;

				execDB->m_Names.ReadName( i, &nameDesc );
				if (stricmp(nameDesc.nameName,"IUT") == 0) {
					nlSourceAddr = nameDesc.nameAddr;
					break;
				}
			}
			if (i >= execDB->m_Names.Length())
				throw ExecError( "Default source address IUT not found", execPacket->baseLineStart );
		} else {
			if (nlSA->exprOp != '=')
				throw ExecError( "Equality operator required for source address", nlSA->exprLine );
			
			ScriptTokenList			saList
			;

			// resolve the expressions
			ResolveExpr( nlSA->exprValue, nlSA->exprLine, saList );
			if (saList.Length() < 1)
				throw ExecError( "Address, or name expected", nlSA->exprLine );

			// get a reference to the first parameter
			const ScriptToken &t = saList[0];

			// check for name or octet string
			if (saList.Length() == 1) {
				// it might be a name
				if ((t.tokenType == scriptValue) && (t.tokenEnc == scriptASCIIEnc)) {
					CString tvalu = t.RemoveQuotes();
					for (int i = 0; i < execDB->m_Names.Length(); i++ ) {
						VTSNameDesc		nameDesc;

						execDB->m_Names.ReadName( i, &nameDesc );
						if (stricmp(nameDesc.nameName,tvalu) == 0) {
							nlSourceAddr = nameDesc.nameAddr;
							break;
						}
					}
					if (i >= execDB->m_Names.Length())
						throw ExecError( "Destination address name not found", nlSA->exprLine );
				} else
				// it might be an IP address
				if ((t.tokenType == scriptValue) && (t.tokenEnc == scriptIPEnc)) {
					BACnetIPAddr nlIPAddr( t.tokenValue );
					nlSourceAddr = nlIPAddr;
				} else
				// it might be an explicit octet string
				if (t.IsEncodeable( nlSource )) {
					nlSourceAddr.LocalStation( nlSource.strBuff, nlSource.strLen );
				} else
					throw ExecError( "Source address expected", nlSA->exprLine );
			} else
			if (saList.Length() == 2) {
				if (t.tokenType != scriptValue)
					throw "SNET expected";
				if (!t.IsInteger(valu))
					throw "SNET invalid format, integer required";
				if ((valu < 0) || (valu > 65534))
					throw "DNET out of range (0..65534)";

				const ScriptToken &sadr = saList[1];

				// it might be an IP address
				if ((sadr.tokenType == scriptValue) && (sadr.tokenEnc == scriptIPEnc)) {
					BACnetIPAddr nlIPAddr( sadr.tokenValue );
					nlSourceAddr = nlIPAddr;
				} else
				// it might be an explicit octet string
				if (sadr.IsEncodeable( nlSource )) {
					nlSourceAddr.LocalStation( nlSource.strBuff, nlSource.strLen );
				} else
					throw ExecError( "Source address expected", nlSA->exprLine );

				nlSourceAddr.addrType = remoteStationAddr;
				nlSourceAddr.addrNet = valu;
			} else
				throw ExecError( "Source address expected", nlSA->exprLine );
		}

		// make sure the addresses match
		if (!(apdu.apduAddr == nlSourceAddr))
			throw ExecError( "Source address mismatch"
					, (nlSA ? nlSA->exprLine : execPacket->baseLineStart)
					);

		// force property references to fail until context established
		execObjID = 0xFFFFFFFF;

		// resolve the expressions
		ResolveExpr( alMsg->exprValue, alMsg->exprLine, alList );
		if (alList.Length() < 1)
			throw ExecError( "AL message keyword expected", alMsg->exprLine );

		// get a reference to the first parameter
		const ScriptToken &t = alList[0];

		// check to see if this is a keyword
		if (t.tokenType == scriptKeyword) {
			alMsgID = t.Lookup( t.tokenSymbol, ScriptALMsgTypeMap );
			if (alMsgID < 0)
				throw ExecError( "Unrecognized keyword", alMsg->exprLine );
		} else
			throw ExecError( "Keyword expected", alMsg->exprLine );

		if (alMsgID != apdu.apduType)
			throw ExecError( "PDU type mismatch", alMsg->exprLine );

		// based on the number, check for other parameters
		try {
			switch (alMsgID) {
				case 0:						// CONFIRMED-REQUEST
					ExpectDevConfirmedRequest( apdu );
					break;
				case 1:						// UNCONFIRMED-REQUEST
					ExpectDevUnconfirmedRequest( apdu );
					break;
				case 2:						// SIMPLEACK
					ExpectDevSimpleACK( apdu );
					break;
				case 3:						// COMPLEXACK
					ExpectDevComplexACK( apdu );
					break;
				case 4:						// SEGMENTACK
					ExpectDevSegmentACK( apdu );
					break;
				case 5:						// ERROR
					ExpectDevError( apdu );
					break;
				case 6:						// REJECT
					ExpectDevReject( apdu );
					break;
				case 7:						// ABORT
					ExpectDevAbort( apdu );
					break;
			}
		}
		catch (const char *errMsg) {
			// one of the functions had an error
			throw ExecError( errMsg, alMsg->exprLine );
		}
		catch ( CString & strThrow )
		{
			throw ExecError( strThrow, alMsg->exprLine );
		}

		// get some interesting keywords that might match
		pDER = GetKeywordValue( kwDER, nlDER );
		if (pDER && !Match(pDER->exprOp,nlDER.boolValue,apdu.apduExpectingReply))
			throw ExecError( "Network priority mismatch", pDER->exprLine );

		pPriority	= GetKeywordValue( kwPRIORITY, nlPriority );
		if (!pPriority)
			pPriority = GetKeywordValue( kwPRIO, nlPriority );
		if (pPriority && !Match(pPriority->exprOp,nlPriority.intValue,apdu.apduNetworkPriority))
			throw ExecError( "Network priority mismatch", pPriority->exprLine );
	}
	catch (const ExecError &err) {
		// failed
		Msg( 3, err.errLineNo, err.errMsg );
		return false;
	}
	catch (const char *errMsg) {
		// failed
		Msg( 3, execPacket->baseLineStart, errMsg );
		return false;
	}

	// we made it!
	return true;
}

//
//	ScriptExecutor::ExpectDevConfirmedRequest
//

void ScriptExecutor::ExpectDevConfirmedRequest( const BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pService
	;
	BACnetInteger			alService
	;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (pService && !Match(pService->exprOp,apdu.apduService,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// expect the rest
	BACnetAPDUDecoder dec( apdu );
	ExpectALData( dec );

	// make sure all the data was matched
	if (dec.pktLength != 0)
		throw ExecError( "Additional application data not matched"
		, execPacket->baseLineStart
		);
}

//
//	ScriptExecutor::ExpectDevUnconfirmedRequest
//

void ScriptExecutor::ExpectDevUnconfirmedRequest( const BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pService
	;
	BACnetInteger			alService
	;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALUnconfirmedServiceMap );
	if (pService && !Match(pService->exprOp,apdu.apduService,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// expect the rest
	BACnetAPDUDecoder dec( apdu );
	ExpectALData( dec );

	// make sure all the data was matched
	if (dec.pktLength != 0)
		throw ExecError( "Additional application data not matched"
		, execPacket->baseLineStart
		);
}

//
//	ScriptExecutor::ExpectDevSimpleACK
//

void ScriptExecutor::ExpectDevSimpleACK( const BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pService, pInvokeID
	;
	BACnetInteger			alService, alInvokeID
	;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (pService && !Match(pService->exprOp,apdu.apduService,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// get the service choice
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,apdu.apduInvokeID,alInvokeID.intValue))
		throw "Service-choice (INVOKEID) mismatch";
}

//
//	ScriptExecutor::ExpectDevComplexACK
//

void ScriptExecutor::ExpectDevComplexACK( const BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pService, pInvokeID
	;
	BACnetInteger			alService, alInvokeID
	;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (pService && !Match(pService->exprOp,apdu.apduService,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// get the service choice
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,apdu.apduInvokeID,alInvokeID.intValue))
		throw "Service-choice (INVOKEID) mismatch";

	// expect the rest
	BACnetAPDUDecoder dec( apdu );
	ExpectALData( dec );

	// make sure all the data was matched
	if (dec.pktLength != 0)
		throw ExecError( "Additional application data not matched"
		, execPacket->baseLineStart
		);
}

//
//	ScriptExecutor::ExpectDevSegmentACK
//

void ScriptExecutor::ExpectDevSegmentACK( const BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pNegativeACK, pServer, pInvokeID, pSeqNumber, pWindowSize
	;
	BACnetBoolean			alNegativeACK, alServer
	;
	BACnetInteger			alInvokeID, alSeqNumber, alWindowSize
	;

	pNegativeACK = GetKeywordValue( kwNEGATIVEACK, alNegativeACK );
	if (pNegativeACK && !Match(pNegativeACK->exprOp,apdu.apduNak,alNegativeACK.boolValue))
		throw "Service-choice (NEGATIVEACK) mismatch";

	pServer = GetKeywordValue( kwSERVER, alServer );
	if (pServer && !Match(pServer->exprOp,apdu.apduSrv,alServer.boolValue))
		throw "Service-choice (SERVER) mismatch";

	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,apdu.apduInvokeID,alInvokeID.intValue))
		throw "Service-choice (INVOKEID) mismatch";

	pSeqNumber = GetKeywordValue( kwSEQUENCENR, alSeqNumber );
	if (pSeqNumber && !Match(pSeqNumber->exprOp,apdu.apduSeq,alSeqNumber.intValue))
		throw "Service-choice (SEQUENCENR) mismatch";

	pWindowSize = GetKeywordValue( kwWINDOWSIZE, alWindowSize );
	if (pWindowSize && !Match(pWindowSize->exprOp,apdu.apduWin,alWindowSize.intValue))
		throw "Service-choice (WINDOWSIZE) mismatch";
}

//
//	ScriptExecutor::ExpectDevError
//

void ScriptExecutor::ExpectDevError( const BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pService, pInvokeID
	;
	BACnetInteger			alService, alInvokeID
	;

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (pService && !Match(pService->exprOp,apdu.apduService,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// get the service choice
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,apdu.apduInvokeID,alInvokeID.intValue))
		throw "Service-choice (INVOKEID) mismatch";

	// expect the rest
	BACnetAPDUDecoder dec( apdu );
	ExpectALData( dec );

	// make sure all the data was matched
	if (dec.pktLength != 0)
		throw ExecError( "Additional application data not matched"
		, execPacket->baseLineStart
		);
}

//
//	ScriptExecutor::ExpectDevReject
//

void ScriptExecutor::ExpectDevReject( const BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pInvokeID, pReason
	;
	BACnetInteger			alInvokeID, alReason
	;

	// get the service choice
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID, ScriptALConfirmedServiceMap );
	if (pInvokeID && !Match(pInvokeID->exprOp,apdu.apduInvokeID,alInvokeID.intValue))
		throw "Service-choice (INVOKEID) mismatch";

	// get the service choice
	pReason = GetKeywordValue( kwREJECTREASON, alReason, ScriptALRejectReasonMap );
	if (pReason && !Match(pReason->exprOp,apdu.apduAbortRejectReason,alReason.intValue))
		throw "Service-choice (REJECTREASON) mismatch";
}

//
//	ScriptExecutor::ExpectDevAbort
//

void ScriptExecutor::ExpectDevAbort( const BACnetAPDU &apdu )
{
	ScriptPacketExprPtr		pServer, pInvokeID, pReason
	;
	BACnetBoolean			alServer
	;
	BACnetInteger			alInvokeID, alReason
	;

	// get the service choice
	pServer = GetKeywordValue( kwSERVER, alServer );
	if (pServer && !Match(pServer->exprOp,apdu.apduInvokeID,alServer.boolValue))
		throw "Service-choice (SERVER) mismatch";

	// get the service choice
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,apdu.apduInvokeID,alInvokeID.intValue))
		throw "Service-choice (INVOKEID) mismatch";

	// get the service choice
	pReason = GetKeywordValue( kwABORTREASON, alReason, ScriptALAbortReasonMap );
	if (pReason && !Match(pReason->exprOp,apdu.apduAbortRejectReason,alReason.intValue))
		throw "Service-choice (ABORTREASON) mismatch";
}

//
//	ScriptExecutor::ExpectConfirmedRequest
//

void ScriptExecutor::ExpectConfirmedRequest( BACnetAPDUDecoder &dec )
{
	int						header, valu
	;
	ScriptPacketExprPtr		pSegMsg, pMOR, pSegResp, pMaxResp, pInvokeID, pSeq, pWindow, pService
	;
	BACnetBoolean			alSegMsg, alMOR, alSegResp
	;
	BACnetInteger			alMaxResp, alInvokeID, alSeq, alWindow, alService
	;

	// check the header
	header = (dec.pktLength--,*dec.pktBuffer++);
	if ((header >> 4) != 0)
		throw "Confirmed request expected";

	// check segmented message
	pSegMsg = GetKeywordValue( kwSEGMSG, alSegMsg );
	if (!pSegMsg)
		pSegMsg = GetKeywordValue( kwSEGMENTEDMESSAGE, alSegMsg );
	if (pSegMsg && !Match(pSegMsg->exprOp,(header >> 3) & 0x01,alSegMsg.boolValue))
		throw "Segmented message mismatch";

	// check more follows
	pMOR = GetKeywordValue( kwMOREFOLLOWS, alMOR );
	if (pMOR && !Match(pMOR->exprOp,(header >> 2) & 0x01,alMOR.boolValue))
		throw "More-follows mismatch";

	// check segmented response
	pSegResp = GetKeywordValue( kwSEGRESP, alSegResp );
	if (!pSegResp)
		pSegResp = GetKeywordValue( kwSEGRESPACCEPTED, alSegResp );
	if (pSegResp && !Match(pSegResp->exprOp,(header >> 1) & 0x01,alSegResp.boolValue))
		throw "Segmented response accepted mismatch";

	// extract the max response size
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// lots of options for maximum response size
	pMaxResp = GetKeywordValue( kwMAXRESP, alMaxResp );
	if (!pMaxResp)
		pMaxResp = GetKeywordValue( kwMAXRESPONSE, alMaxResp );
	if (!pMaxResp)
		pMaxResp = GetKeywordValue( kwMAXSIZE, alMaxResp );
	if (pMaxResp) {
		// translate the max response size into the code
		if (alMaxResp.intValue < 16)
			;
		else
		if (alMaxResp.intValue <= 50)
			alMaxResp.intValue = 0;
		else
		if (alMaxResp.intValue <= 128)
			alMaxResp.intValue = 1;
		else
		if (alMaxResp.intValue <= 206)
			alMaxResp.intValue = 2;
		else
		if (alMaxResp.intValue <= 480)
			alMaxResp.intValue = 3;
		else
		if (alMaxResp.intValue <= 1024)
			alMaxResp.intValue = 4;
		else
		if (alMaxResp.intValue <= 1476)
			alMaxResp.intValue = 5;
		else
			alMaxResp.intValue = 6;

		// now check it
		if (pMaxResp && !Match(pMaxResp->exprOp,valu,alMaxResp.intValue))
			throw "Max response size mismatch";
	}

	// extract the invoke ID
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,valu,alInvokeID.intValue))
		throw ExecError( "Invoke ID mismatch", pInvokeID->exprLine );

	// might check these
	pSeq = GetKeywordValue( kwSEQUENCENR, alSeq );
	pWindow = GetKeywordValue( kwWINDOWSIZE, alWindow );

	// check segmented message stuff
	if ((header & 0x80) != 0) {
		// extract the sequence number and check it
		valu = (dec.pktLength--,*dec.pktBuffer++);
		if (pSeq && !Match(pSeq->exprOp,valu,alSeq.intValue))
			throw ExecError( "Invoke ID mismatch", pSeq->exprLine );

		// extract the proposed window size and check it
		valu = (dec.pktLength--,*dec.pktBuffer++);
		if (pWindow && !Match(pWindow->exprOp,valu,alWindow.intValue))
			throw ExecError( "Proposed window size mismatch", pWindow->exprLine );
	} else
	if (pSeq)
		throw "Sequence number not matched, message is not segmented";
	else
	if (pWindow)
		throw "Window size not matched, message is not segmented";

	// extract the service choice
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (pService && !Match(pService->exprOp,valu,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// expect the rest
	ExpectALData( dec );
}

//
//	ScriptExecutor::ExpectUnconfirmedRequest
//

void ScriptExecutor::ExpectUnconfirmedRequest( BACnetAPDUDecoder &dec )
{
	int						valu
	;
	ScriptPacketExprPtr		pService
	;
	BACnetInteger			alService
	;

	// check the header
	valu = (dec.pktLength--,*dec.pktBuffer++);
	if ((valu >> 4) != 1)
		throw "UnconfirmedRequest expected";

	// extract the service choice
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALUnconfirmedServiceMap );
	if (pService && !Match(pService->exprOp,valu,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// expect the rest
	ExpectALData( dec );
}

//
//	ScriptExecutor::ExpectSimpleACK
//

void ScriptExecutor::ExpectSimpleACK( BACnetAPDUDecoder &dec )
{
	int						valu
	;
	ScriptPacketExprPtr		pInvokeID, pService
	;
	BACnetInteger			alInvokeID, alService
	;

	// check the header
	valu = (dec.pktLength--,*dec.pktBuffer++);
	if ((valu >> 4) != 2)
		throw "SimpleACK expected";

	// extract the invoke ID
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,valu,alInvokeID.intValue))
		throw ExecError( "Invoke ID mismatch", pInvokeID->exprLine );

	// extract the service choice
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (pService && !Match(pService->exprOp,valu,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";
}

//
//	ScriptExecutor::ExpectComplexACK
//

void ScriptExecutor::ExpectComplexACK( BACnetAPDUDecoder &dec )
{
	int						header, valu
	;
	ScriptPacketExprPtr		pSegMsg, pMOR, pInvokeID, pSeq, pWindow, pService
	;
	BACnetBoolean			alSegMsg, alMOR
	;
	BACnetInteger			alInvokeID, alSeq, alWindow, alService
	;

	// check the header
	header = (dec.pktLength--,*dec.pktBuffer++);
	if ((header >> 4) != 3)
		throw "ComplexACK expected";

	// check segmented message
	pSegMsg = GetKeywordValue( kwSEGMSG, alSegMsg );
	if (!pSegMsg)
		pSegMsg = GetKeywordValue( kwSEGMENTEDMESSAGE, alSegMsg );
	if (pSegMsg && !Match(pSegMsg->exprOp,(header >> 3) & 0x01,alSegMsg.boolValue))
		throw "Segmented message mismatch";

	// check more follows
	pMOR = GetKeywordValue( kwMOREFOLLOWS, alMOR );
	if (pMOR && !Match(pMOR->exprOp,(header >> 2) & 0x01,alMOR.boolValue))
		throw "More-follows mismatch";

	// extract the invoke ID
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,valu,alInvokeID.intValue))
		throw ExecError( "Invoke ID mismatch", pInvokeID->exprLine );

	// might check these
	pSeq = GetKeywordValue( kwSEQUENCENR, alSeq );
	pWindow = GetKeywordValue( kwWINDOWSIZE, alWindow );

	// check segmented message stuff
	if ((header & 0x80) != 0) {
		// extract the sequence number and check it
		valu = (dec.pktLength--,*dec.pktBuffer++);
		if (pSeq && !Match(pSeq->exprOp,valu,alSeq.intValue))
			throw ExecError( "Invoke ID mismatch", pSeq->exprLine );

		// extract the proposed window size and check it
		valu = (dec.pktLength--,*dec.pktBuffer++);
		if (pWindow && !Match(pWindow->exprOp,valu,alWindow.intValue))
			throw ExecError( "Proposed window size mismatch", pWindow->exprLine );
	} else
	if (pSeq)
		throw "Sequence number not matched, message is not segmented";
	else
	if (pWindow)
		throw "Window size not matched, message is not segmented";

	// extract the service choice
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (pService && !Match(pService->exprOp,valu,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// expect the rest
	ExpectALData( dec );
}

//
//	ScriptExecutor::ExpectSegmentACK
//

void ScriptExecutor::ExpectSegmentACK( BACnetAPDUDecoder &dec )
{
	int						valu
	;
	ScriptPacketExprPtr		pNegativeACK, pServer, pInvokeID, pSeqNumber, pService, pWindowSize
	;
	BACnetBoolean			alNegativeACK, alServer
	;
	BACnetInteger			alInvokeID, alSeqNumber, alService, alWindowSize
	;

	// check the header
	valu = (dec.pktLength--,*dec.pktBuffer++);
	if ((valu >> 4) != 4)
		throw "SegmentACK expected";

	// check negative ACK
	pNegativeACK = GetKeywordValue( kwNEGATIVEACK, alNegativeACK );
	if (pNegativeACK && !Match(pNegativeACK->exprOp,(valu >> 1) & 0x01,alNegativeACK.boolValue))
		throw "Negative-ACK mismatch";

	// check the server bit
	pServer = GetKeywordValue( kwSERVER, alServer );
	if (pServer && !Match(pServer->exprOp,valu & 0x01,alServer.boolValue))
		throw "Server mismatch";

	// extract the invoke ID
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,valu,alInvokeID.intValue))
		throw ExecError( "Invoke ID mismatch", pInvokeID->exprLine );

	// extract the service choice
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the service choice
	pService = GetKeywordValue( kwSERVICE, alService, ScriptALConfirmedServiceMap );
	if (pService && !Match(pService->exprOp,valu,alService.intValue))
		throw "Service-choice (SERVICE) mismatch";

	// extract the sequence number
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the sequence number
	pSeqNumber = GetKeywordValue( kwSEQUENCENR, alSeqNumber );
	if (pSeqNumber && !Match(pSeqNumber->exprOp,valu,alSeqNumber.intValue))
		throw ExecError( "Sequence number mismatch", pSeqNumber->exprLine );

	// extract the actual window size
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the window size
	pWindowSize = GetKeywordValue( kwWINDOWSIZE, alWindowSize );
	if (pWindowSize && !Match(pWindowSize->exprOp,valu,alWindowSize.intValue))
		throw ExecError( "Window size mismatch", pWindowSize->exprLine );
}

//
//	ScriptExecutor::ExpectError
//
//Added by Liangping Xu, 2002-8-5********//

void ScriptExecutor::ExpectError( BACnetAPDUDecoder &dec )
{
    int			          valu;

	ScriptPacketExprPtr	  pInvokeID, pErrorService;
	BACnetInteger		  alInvokeID, alReason;

    CString errorService;
	
	// check the header
	valu = (dec.pktLength--,*dec.pktBuffer++);
	if ((valu >> 4) != 5)
		throw "Error expected";

	// extract the invoke ID
	valu = (dec.pktLength--,*dec.pktBuffer++);
	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,valu,alInvokeID.intValue))
		throw ExecError( "Invoke ID mismatch", pInvokeID->exprLine );

	// extract the Error Choice
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the Error Choice
	pErrorService = execPacket->packetExprList.Find( kwERRORCHOICE );
	errorService=pErrorService->exprValue;
	if(errorService.CompareNoCase(CString(NetworkSniffer::BACnetConfirmedServiceChoice[valu])))
	 		 throw ExecError( "Error Service mismatch", pErrorService->exprLine );

	//extract AL data,including the Error Class & Code Nmu
	ExpectALData( dec );
}


//
//	ScriptExecutor::ExpectReject
//

void ScriptExecutor::ExpectReject( BACnetAPDUDecoder &dec )
{
	int						valu
	;
	ScriptPacketExprPtr		pInvokeID, pReason
	;
	BACnetInteger			alInvokeID, alReason
	;

	// check the header
	valu = (dec.pktLength--,*dec.pktBuffer++);
	if ((valu >> 4) != 6)
		throw "Reject expected";

	// extract the invoke ID
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,valu,alInvokeID.intValue))
		throw ExecError( "Invoke ID mismatch", pInvokeID->exprLine );

	// extract the reject reason
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// check the reason
	pReason = GetKeywordValue( kwREJECTREASON, alReason, ScriptALRejectReasonMap );
	if (pReason && !Match(pReason->exprOp,valu,alReason.intValue))
		throw ExecError( "Reject reason mismatch", pReason->exprLine );
}

//
//	ScriptExecutor::ExpectAbort
//

void ScriptExecutor::ExpectAbort( BACnetAPDUDecoder &dec )
{
	int						valu
	;
	ScriptPacketExprPtr		pServer, pInvokeID, pReason
	;
	BACnetBoolean			alServer
	;
	BACnetInteger			alInvokeID, alReason
	;

	// check the header
	valu = (dec.pktLength--,*dec.pktBuffer++);
	if ((valu >> 4) != 7)
		throw "Abort expected";

	// check the server bit
	pServer = GetKeywordValue( kwSERVER, alServer );
	if (pServer && !Match(pServer->exprOp,valu & 0x01,alServer.boolValue))
		throw "Server mismatch";

	// extract the invoke ID
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the invoke ID
	pInvokeID = GetKeywordValue( kwINVOKEID, alInvokeID );
	if (pInvokeID && !Match(pInvokeID->exprOp,valu,alInvokeID.intValue))
		throw ExecError( "Invoke ID mismatch", pInvokeID->exprLine );

	// extract the abort reason
	valu = (dec.pktLength--,*dec.pktBuffer++);

	// get the abort reason
	pReason = GetKeywordValue( kwABORTREASON, alReason, ScriptALAbortReasonMap );
	if (pReason && !Match(pReason->exprOp,valu,alReason.intValue))
		throw ExecError( "Abort reason mismatch", pReason->exprLine );
}

//
//	ScriptExecutor::ExpectALData
//
//	There are two flavors of application layer variable encoding stuff.  One is with a keyword 
//	on the front of the type with a bunch of parameters, the other is ALDATA followed by a 
//	hex string.
//

// madanner 9/25/02 Method drastically altered
// see prior versions for comparision (sorry)

void ScriptExecutor::ExpectALData( BACnetAPDUDecoder &dec )
{
	int						indx, len;
	BACnetOctetString		ostr;
	CString strThrowMessage;

   //***Added by Liangping Xu, 2002-8-5***//
	nArrayIndx = -1;
   //***Ended by Liangping Xu, 2002-8-5***//

	// get the index of the first data
	indx = execPacket->packetExprList.FirstData();
	if (indx < 0)
		return;

	// get the length
	len = execPacket->packetExprList.Length();

	// loop through each of the packet expressions in the packet expression list

	for (; indx < len; indx++) {
		ScriptPacketExprPtr spep = execPacket->packetExprList.Child( indx );
		if (!spep)
			throw "Application variable element expected";

		// check for raw data
		if ((spep->exprKeyword == kwAL) || (spep->exprKeyword == kwALDATA)) {
			// translate the expression, resolve parameter names into values
			ScriptTokenList tlist;
			ResolveExpr( spep->exprValue, spep->exprLine, tlist );

			// just a little error checking
			if (tlist.Length() != 1)
				throw "ALDATA requires an octet string";

			// reference or data?
			if (tlist[0].tokenType == scriptReference)
			{
				BACnetAnyValue		bacnetEPICSProperty;

				GetEPICSProperty( tlist[0].tokenSymbol, &bacnetEPICSProperty, tlist[0].m_nIndex);

				// Will throw errors
				bacnetEPICSProperty.CompareToEncodedStream( dec, spep->exprOp, spep->exprValue );
			}
			else
			{
				// extract octet string
				if (!tlist[0].IsEncodeable( ostr ))
					throw "Octet string expected";

				ASSERT(0);		// not implemented...  
				//CompareStreamData(dec, spep->exprOp, ostr.strBuff, ostr.strLen, spep->exprValue ); 
			}

			continue;
		}

		// skip things that aren't data
		if (!spep->exprIsData)
			continue;

		// do the type specific encoding
		try {
			int rslt = ScriptToken::Lookup( spep->exprKeyword, ScriptALMap );
			if (rslt < 0)
				throw "Invalid keyword";

			switch (rslt) {
				case 1:								// NULL
					ExpectALNull( spep, dec );
					break;
				case 2:								// BOOLEAN
					ExpectALBoolean( spep, dec );
					break;
				case 3:								// UNSIGNED
					ExpectALUnsigned( spep, dec );
					break;
				case 4:								// INTEGER
					ExpectALInteger( spep, dec );
					break;
				case 5:								// REAL
					ExpectALReal( spep, dec );
					break;
				case 6:								// DOUBLE
					ExpectALDouble( spep, dec );
					break;
				case 7:								// OCTETSTRING
					ExpectALOctetString( spep, dec );
					break;
				case 8:								// CHARACTERSTRING
					ExpectALCharacterString( spep, dec );
					break;
				case 9:								// BITSTRING
					ExpectALBitString( spep, dec );
					break;
				case 10:							// ENUMERATED
					ExpectALEnumerated( spep, dec );
					break;
				case 11:							// DATE
					ExpectALDate( spep, dec );
					break;
				case 12:							// TIME
					ExpectALTime( spep, dec );
					break;
				case 13:							// OBJECTIDENTIFIER
					ExpectALObjectIdentifier( spep, dec );
					break;
				case 14:							// DEVICEIDENTIFIER
					ExpectALDeviceIdentifier( spep, dec );
					break;
				case 15:							// PROPERTYIDENTIFIER
					ExpectALPropertyIdentifier( spep, dec );
					break;
				case 16:							// OPENINGTAG
					ExpectALOpeningTag( spep, dec );
					break;
				case 17:							// CLOSINGTAG
					ExpectALClosingTag( spep, dec );
					break;
			}
		}
		catch (const char *errMsg) {
			throw ExecError( errMsg, spep->exprLine );
		}
	}

	// make sure all the data was matched
	if (dec.pktLength != 0)
		throw ExecError( "Additional application data not matched", execPacket->baseLineStart );
}


//
//	ScriptExecutor::ExpectALNull
//

void ScriptExecutor::ExpectALNull( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					context = kAppContext
	;
	BACnetNull			nullData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );
	
	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 0) {
		// check the type
		if (tag.tagClass)
			throw "Application tagged null expected";
		if (tag.tagNumber != nullAppTag)
			throw "Mismatched data type, null expected";
	} else
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";

		// check the type
		if (!tag.tagClass)
			throw "Context tagged null expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Too many keyword values";

	// decode it
	nullData.Decode( dec );
}

//
//	ScriptExecutor::ExpectALBoolean
//

void ScriptExecutor::ExpectALBoolean( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					indx, context = kAppContext
	;
	BACnetBoolean		boolData, scriptData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );
	
	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tag.tagClass)
			throw "Application tagged boolean expected";
		if (tag.tagNumber != booleanAppTag)
			throw "Mismatched data type, boolean expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
		if (!tag.tagClass)
			throw "Context tagged boolean expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Boolean keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue		bacnetEPICSProperty;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

		// verify the type
		if (bacnetEPICSProperty.GetType() != ebool)
			throw "Boolean property value expected in EPICS";

		scriptData.boolValue = ((BACnetBoolean *) bacnetEPICSProperty.GetObject())->boolValue;
	} else
	if ( tlist[indx].IsDontCare() )
		spep->exprOp = '?=';
	else if (!tlist[indx].IsEncodeable( scriptData ))
		throw "Boolean value expected";

	// decode it
	boolData.Decode( dec );
	CompareAndThrowError(boolData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILBOOL);

/*
	// verify the value
	if (!Match(spep->exprOp,boolData.boolValue,scriptData.boolValue))
		throw "Boolean value mismatch";
*/
}

//
//	ScriptExecutor::ExpectALUnsigned
//

void ScriptExecutor::ExpectALUnsigned( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					indx, context = kAppContext
	;
	BACnetUnsigned		uintData, scriptData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );
	
	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tag.tagClass)
			throw "Application tagged unsigned expected";
		if (tag.tagNumber != unsignedIntAppTag)
			throw "Mismatched data type, unsigned expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
		if (!tag.tagClass)
			throw "Context tagged unsigned expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Unsigned keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue		bacnetEPICSProperty;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

		// verify the type
		if ( !bacnetEPICSProperty.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetUnsigned)) )
			throw "Unsigned property value expected in EPICS";

		scriptData.uintValue = ((BACnetUnsigned *) bacnetEPICSProperty.GetObject())->uintValue;
	} else
	if ( tlist[indx].IsDontCare() )
		spep->exprOp = '?=';
	else if (!tlist[indx].IsEncodeable( scriptData ))
		throw "Unsigned value expected";

	// There may be a bug here... possibly we should always Decode the uintData regardless of
	// what the scriptData indicates as size.  But I don't know enough yet to tell what this would
	// break?  madanner 11/4/02

   //***Added by Liangping Xu, 2002-8-5***//
    nArrayIndx = scriptData.uintValue;
   //***Ended by Liangping Xu, 2002-8-5***//
	// check for wierd versions
	if (spep->exprKeyword == kwUNSIGNED8) {
//		if ((uintData.uintValue < 0) || (uintData.uintValue > 255))			// madanner 11/4/02, should be scriptData?
		if ((scriptData.uintValue < 0) || (scriptData.uintValue > 255))
			throw "Unsigned8 value out of range (0..255)";
		if (tlist.Length() == 2)
			throw "Unsigned8 keyword cannot be context encoded";

		uintData.uintValue = (dec.pktLength--,*dec.pktBuffer++);
	} else
	if (spep->exprKeyword == kwUNSIGNED16) {
//		if ((uintData.uintValue < 0) || (uintData.uintValue > 65535))		// madanner 11/4/02, should be scriptData?
		if ((scriptData.uintValue < 0) || (scriptData.uintValue > 65535))
			throw "Unsigned16 value out of range (0..65535)";
		if (tlist.Length() == 2)
			throw "Unsigned16 keyword cannot be context encoded";

		uintData.uintValue = (dec.pktLength--,*dec.pktBuffer++);
		uintData.uintValue = (uintData.uintValue << 8) + (dec.pktLength--,*dec.pktBuffer++);
	} else
	if (spep->exprKeyword == kwUNSIGNED32) {
		if (tlist.Length() == 2)
			throw "Unsigned32 keyword cannot be context encoded";

		uintData.uintValue = (dec.pktLength--,*dec.pktBuffer++);
		uintData.uintValue = (uintData.uintValue << 8) + (dec.pktLength--,*dec.pktBuffer++);
		uintData.uintValue = (uintData.uintValue << 8) + (dec.pktLength--,*dec.pktBuffer++);
		uintData.uintValue = (uintData.uintValue << 8) + (dec.pktLength--,*dec.pktBuffer++);
	} else {
		// decode it
		uintData.Decode( dec );
	}

	CompareAndThrowError(uintData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILUNSIGNED);
/*
	// verify the value
	if (!Match(spep->exprOp,uintData.uintValue,scriptData.uintValue))
		throw "Unsigned value mismatch";
*/
}

//
//	ScriptExecutor::ExpectALInteger
//

void ScriptExecutor::ExpectALInteger( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					indx, context = kAppContext
	;
	BACnetInteger		intData, scriptData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tag.tagClass)
			throw "Application tagged integer expected";
		if (tag.tagNumber != integerAppTag)
			throw "Mismatched data type, integer expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;

		// check the type
		if (!tag.tagClass)
			throw "Context tagged integer expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Integer keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue		bacnetEPICSProperty;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

		// verify the type
		if ( bacnetEPICSProperty.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetUnsigned)) )
			scriptData.intValue = ((BACnetUnsigned *) bacnetEPICSProperty.GetObject())->uintValue;
		else if ( bacnetEPICSProperty.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetInteger)) )
			scriptData.intValue = ((BACnetInteger *) bacnetEPICSProperty.GetObject())->intValue;
		else
			throw "Integer property value expected in EPICS";
	} else
	if ( tlist[indx].IsDontCare() )
		spep->exprOp = '?=';
	else if (!tlist[indx].IsEncodeable( scriptData ))
		throw "Integer value expected";

	// decode it
	intData.Decode( dec );
	CompareAndThrowError(intData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILINT);
/*
	// verify the value
	if (!Match(spep->exprOp,intData.intValue,scriptData.intValue))
		throw "Integer value mismatch";
*/
}

//
//	ScriptExecutor::ExpectALReal
//

void ScriptExecutor::ExpectALReal( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					indx, context = kAppContext
	;
	BACnetReal			realData, scriptData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tag.tagClass)
			throw "Application tagged real expected";
		if (tag.tagNumber != realAppTag)
			throw "Mismatched data type, real expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
		if (!tag.tagClass)
			throw "Context tagged real expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Real keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue		bacnetEPICSProperty;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

		if ( !bacnetEPICSProperty.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetReal)) )
			throw "Real property value expected in EPICS";

		scriptData.realValue = ((BACnetReal *) bacnetEPICSProperty.GetObject())->realValue;
	} else
	if ( tlist[indx].IsDontCare() )
		spep->exprOp = '?=';
	else if (!tlist[indx].IsEncodeable( scriptData ))
		throw "Single precision floating point value expected";

	// decode it
	realData.Decode( dec );
	CompareAndThrowError(realData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILREAL);
/*
	// verify the value
	if (!Match(spep->exprOp,realData.realValue,scriptData.realValue))
		throw "Real value mismatch";
*/
}

//
//	ScriptExecutor::ExpectALDouble
//

void ScriptExecutor::ExpectALDouble( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					indx, context = kAppContext
	;
	BACnetDouble		doubleData, scriptData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tag.tagClass)
			throw "Application tagged double expected";
		if (tag.tagNumber != doubleAppTag)
			throw "Mismatched data type, double expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;

		// check the type
		if (!tag.tagClass)
			throw "Context tagged double expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Double keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue		bacnetEPICSProperty;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

		if ( !bacnetEPICSProperty.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetReal)) )
			throw "Floating point property value expected in EPICS";

		scriptData.doubleValue = ((BACnetReal *) bacnetEPICSProperty.GetObject())->realValue;
	} else
	if ( tlist[indx].IsDontCare() )
		spep->exprOp = '?=';
	else if (!tlist[indx].IsEncodeable( scriptData ))
		throw "Double precision floating point expected";

	// decode it
	doubleData.Decode( dec );
	CompareAndThrowError(doubleData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILDOUBLE);

/*
	// verify the value
	if (!Match(spep->exprOp,doubleData.doubleValue,scriptData.doubleValue))
		throw "Real value mismatch";
*/
}

//
//	ScriptExecutor::ExpectALOctetString
//

void ScriptExecutor::ExpectALOctetString( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					indx, context = kAppContext
	;
	BACnetOctetString	ostrData, scriptData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tag.tagClass)
			throw "Application tagged octet string expected";
		if (tag.tagNumber != octetStringAppTag)
			throw "Mismatched data type, octet string expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;

		if (!tag.tagClass)
			throw "Context tagged double expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Octet string keyword requires 1 or 2 parameters";

	// no references
	if (tlist[indx].tokenType == scriptReference)
		throw "Octet string property references not supported";
	if ( tlist[indx].IsDontCare() )
		spep->exprOp = '?=';
	else if (!tlist[indx].IsEncodeable( scriptData ))
		throw "Octet string value expected";

	// decode it
	ostrData.Decode( dec );
	CompareAndThrowError(ostrData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILOCTSTRING);

/*
	// verify the value
	switch (spep->exprOp) {
		case '=':
			if (ostrData.strLen != scriptData.strLen)
				throw "Octet string mismatch";
			for (i = 0; i < ostrData.strLen; i++)
				if (ostrData.strBuff[i] != scriptData.strBuff[i])
					throw "Octet string mismatch";
			break;
		case '!=':
			if (ostrData.strLen != scriptData.strLen)
				break;
			for (i = 0; i < ostrData.strLen; i++)
				if (ostrData.strBuff[i] != scriptData.strBuff[i])
					break;
			if (i >= ostrData.strLen)
				throw "Octet string mismatch";
			break;
		default:
			throw "Equality and inequality comparisons only";
	}
*/
}

//
//	ScriptExecutor::ExpectALCharacterString
//
//	See SendALCharacterString for additional comments.
//

void ScriptExecutor::ExpectALCharacterString( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int						indx = -1, context = kAppContext;
//	unsigned int minLen; // for string len
//	unsigned int i; // counter 
	BACnetCharacterString	cstrData, scriptData
	;
	ScriptTokenList			tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (tlist[0].tokenType == scriptReference)
			indx = 0;
		else if ( tlist[0].IsDontCare() )
			spep->exprOp = '?=';
		else if (!tlist[0].IsEncodeable( scriptData ))
			throw "ASCII string value expected";

		if (tag.tagClass)
			throw "Application tagged character string expected";
		if (tag.tagNumber != characterStringAppTag)
			throw "Mismatched data type, character string expected";
	} else
	if (tlist.Length() == 2) {
		if (tlist[1].tokenType == scriptReference) {
			if (!tlist[0].IsInteger( context ))
				throw "Tag number expected";
			indx = 1;
		} else {
			if (ScriptToken::Lookup( tlist[0].tokenSymbol, ScriptCharacterTypeMap ) < 0)
				throw "Unknown encoding";

			if ( tlist[1].IsDontCare() )
				spep->exprOp = '?=';
			else
			{
				CString buff;
				buff.Format( "%s, %s", tlist[0].tokenValue, tlist[1].tokenValue );
				scriptData.Decode( buff );
			}

			if (tag.tagClass)
				throw "Application tagged character string expected";
			if (tag.tagNumber != characterStringAppTag)
				throw "Mismatched data type, character string expected";
		}
	} else
	if (tlist.Length() == 3) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		if (ScriptToken::Lookup( tlist[1].tokenSymbol, ScriptCharacterTypeMap ) < 0)
			throw "Unknown encoding";

		if ( tlist[2].IsDontCare() )
			spep->exprOp = '?=';
		else
		{
			CString buff;
			buff.Format( "%s, %s", tlist[1].tokenValue, tlist[2].tokenValue );
			scriptData.Decode( buff );
		}

		// check the type
		if (!tag.tagClass)
			throw "Context tagged double expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Missing requred parameters";

	// see if a reference was used
	if (indx >= 0) {
		BACnetAnyValue		bacnetEPICSProperty;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

		if ( !bacnetEPICSProperty.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetCharacterString)) )
			throw "Character string property value expected in EPICS";

		scriptData.SetValue( (char *) ((BACnetCharacterString *) bacnetEPICSProperty.GetObject())->strBuff );
	}

	// decode it
	cstrData.Decode( dec );
	CompareAndThrowError(cstrData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILSTRING);
}

//
//	ScriptExecutor::ExpectALBitString
//
//	Yet another custom extension, bit strings can be specified as a list of T/F values or 
//	B'1010100111'.
//

void ScriptExecutor::ExpectALBitString( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					context = kAppContext, bit, i
	;
	BACnetBitString		bstrData, scriptData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 0)
		throw "Bit string keyword values required";

	const ScriptToken &data = tlist[0];

	// must only be one bit or a bit string
	if (tlist.Length() == 1) {
		if (data.tokenType == scriptReference) {
			BACnetAnyValue		bacnetEPICSProperty;

			GetEPICSProperty( data.tokenSymbol, &bacnetEPICSProperty, data.m_nIndex);

			// verify the type
			if ( bacnetEPICSProperty.GetType() != bits)
				throw "Bit string property value expected in EPICS";

			scriptData = *((BACnetBitString *) bacnetEPICSProperty.GetObject());
		} else
		if ( data.IsDontCare() )
			spep->exprOp = '?=';
		else if (data.tokenEnc == scriptBinaryEnc) {
			if (!data.IsEncodeable( scriptData ))
				throw "Bit string value expected";
		} else if (data.tokenType == scriptKeyword) {
			if (data.IsInteger( bit, ScriptBooleanMap ))
				scriptData += bit;
		} else
			throw "Bit list expected";

		if (tag.tagClass)
			throw "Application tagged bit string expected";
		if (tag.tagNumber != bitStringAppTag)
			throw "Mismatched data type, bit string expected";
	} else {
		i = 0;
		if (data.tokenEnc != scriptBinaryEnc)
			if (data.IsInteger( context )) {
				i += 1;

				if (!tag.tagClass)
					throw "Context tagged bit string expected";
				if (tag.tagNumber != context)
					throw "Mismatched context tag value";
			}

		if (tlist[i].tokenType == scriptReference) {
			BACnetAnyValue		bacnetEPICSProperty;

			GetEPICSProperty( tlist[i].tokenSymbol, &bacnetEPICSProperty, tlist[i].m_nIndex);

			// verify the type
			if ( bacnetEPICSProperty.GetType() != bits)
				throw "Bit string property value expected in EPICS";

			scriptData = *((BACnetBitString *) bacnetEPICSProperty.GetObject());
		} else
		if ( tlist[i].IsDontCare() )
			spep->exprOp = '?=';
		else if (tlist[i].IsEncodeable( scriptData ))
			;
		else {
			int count = 0;
			while (i < tlist.Length())
				if (tlist[i++].IsInteger( bit, ScriptBooleanMap ))
					scriptData.SetBit( count++, bit );
				else
					throw "Bit value expected";
		}
	}

	// decode it
	bstrData.Decode( dec );
	CompareAndThrowError(bstrData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILBITSTRING);

/*	// verify the value
	switch (spep->exprOp) {
		case '=':
			if (!(bstrData == scriptData))
				throw "Bit string mismatch";
			break;
		case '!=':
			if (bstrData == scriptData)
				throw "Bit string mismatch";
			break;
		default:
			throw "Equality and inequality comparisons only";
	}
*/
}


void ScriptExecutor::CompareAndThrowError( BACnetEncodeable & rbacnet1, BACnetEncodeable & rbacnet2, int iOperator, unsigned int nError )
{
	CString strError;

	// Account for dont' care operator.  Operator usually filled in last minute... not specified
	// as real operator so we don't have to place test in ALL virtual matching methods !!

	if ( iOperator != '?=' && !rbacnet2.Match(rbacnet1, iOperator, &strError) )
	{
		CString strErrorPrefix;
		strErrorPrefix.LoadString(nError);
		throw CString(strErrorPrefix + strError);
	}
}


//
//	ScriptExecutor::ExpectALEnumerated
//

void ScriptExecutor::ExpectALEnumerated( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					indx, context = kAppContext
	;
	BACnetEnumerated	enumData, scriptData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tag.tagClass)
			throw "Application tagged enumerated expected";
		if (tag.tagNumber != enumeratedAppTag)
			throw "Mismatched data type, enumerated expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
		if (!tag.tagClass)
			throw "Context tagged enumerated expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Integer keyword requires 1 or 2 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue		bacnetEPICSProperty;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

		if ( !bacnetEPICSProperty.GetObject()->IsKindOf(RUNTIME_CLASS(BACnetEnumerated)) )
			throw "Enumerated property value expected in EPICS";

		scriptData.enumValue = ((BACnetEnumerated *) bacnetEPICSProperty.GetObject())->enumValue;
	} else {
		try {
			if ( tlist[indx].IsDontCare() )
				spep->exprOp = '?=';
			else
				scriptData.Decode( tlist[indx].tokenValue );
		}
		catch (...) {
			throw "Integer value expected";
		}
	}

	// decode it
	enumData.Decode( dec );
	CompareAndThrowError(enumData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILENUM);

/*
	// verify the value
	if (!Match(spep->exprOp,enumData.enumValue,scriptData.enumValue))
		throw "Enumeration value mismatch";
*/
}

//
//	ScriptExecutor::ExpectALDate
//

void ScriptExecutor::ExpectALDate( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int				context = kAppContext
	;
	BACnetDate		dateData, scriptData
	;
	ScriptScanner	scan( spep->exprValue )
	;
	ScriptToken		tok
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// get something from the front
	scan.Next( tok );

	// if it is a number, it must be a tag
	if ((tok.tokenType == scriptValue) && (tok.IsInteger( context ))) {
		scan.Peek( tok );
		if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == ','))
			scan.Next( tok );

		if (tok.tokenType == scriptReference) {
			BACnetAnyValue		bacnetEPICSProperty;

			GetEPICSProperty( tok.tokenSymbol, &bacnetEPICSProperty, tok.m_nIndex);

			if ( bacnetEPICSProperty.GetType() != ptDate )
				throw "Date property value expected in EPICS";

			scriptData = *((BACnetDate *) bacnetEPICSProperty.GetObject());
		} else
		{
			if ( tok.IsDontCare() )
				spep->exprOp = '?=';
			else 
				scriptData.Decode( scan.scanSrc );
		}

		if (!tag.tagClass)
			throw "Context tagged date expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else {

		if ( tok.IsDontCare() )
			spep->exprOp = '?=';
		else 
			scriptData.Decode( spep->exprValue );

		if (tag.tagClass)
			throw "Application tagged date expected";
		if (tag.tagNumber != dateAppTag)
			throw "Mismatched data type, date expected";
	}

	// decode it
	dateData.Decode( dec );
	CompareAndThrowError(dateData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILDATE);

}

//
//	ScriptExecutor::ExpectALTime
//

void ScriptExecutor::ExpectALTime( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int				holdContext, context = kAppContext
	;
	BACnetTime		timeData, scriptData
	;
	ScriptScanner	scan( spep->exprValue )
	;
	ScriptToken		tok
	;
	BACnetAPDUTag	tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// get something from the front
	scan.Next( tok );

	try {
		// if it is a number
		if ((tok.tokenType == scriptValue) && (tok.IsInteger( holdContext ))) {
			scan.Peek( tok );
			if ((tok.tokenType == scriptSymbol) && (tok.tokenSymbol == ',')) {
				context = holdContext;
				scan.Next( tok );
			   }                          //added by Liangping Xu  
			if (tok.tokenType == scriptReference) {
				BACnetAnyValue		bacnetEPICSProperty;

				GetEPICSProperty( tok.tokenSymbol, &bacnetEPICSProperty, tok.m_nIndex);

				if ( bacnetEPICSProperty.GetType() != ptTime )
					throw "Time property value expected in EPICS";

				scriptData = *((BACnetTime *) bacnetEPICSProperty.GetObject());
			} else
			{
				if ( tok.IsDontCare() )
					spep->exprOp = '?=';
				else
					scriptData.Decode( scan.scanSrc );	// do the rest as a time
			}

			if (!tag.tagClass)
				throw "Context tagged date expected";
			if (tag.tagNumber != context)
				throw "Mismatched context tag value";
		} else {
			if ( tok.IsDontCare() )
				spep->exprOp = '?=';
			else
				scriptData.Decode( spep->exprValue );	// do whole thing

			if (tag.tagClass)
				throw "Application tagged time expected";
			if (tag.tagNumber != timeAppTag)
				throw "Mismatched data type, time expected";
		}
             // deleted by Liangping Xu
			//} else
	//		throw "Time keyword parameter format invalid";
	}
	catch (...) {
		throw "Time keyword parameter format invalid";
	}

	// decode it
	timeData.Decode( dec );
	CompareAndThrowError(timeData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILTIME);
}

//
//	ScriptExecutor::ExpectALObjectIdentifier
//

void ScriptExecutor::ExpectALObjectIdentifier( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int						indx, context = kAppContext, objType, instanceNum
	;
	BACnetObjectIdentifier	objData, scriptData
	;
	ScriptTokenList			tlist
	;
	BACnetAPDUTag			tag
	;
	
	// extract the tag
	tag.Peek( dec );
	
	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		indx = 0;
		if (tlist[0].tokenType != scriptReference)
			throw "Object identifier keyword expects an EPICS reference";
	} else
	if (tlist.Length() == 2) {
		indx = 0;
		if (tag.tagClass)
			throw "Application tagged object identifier expected";
		if (tag.tagNumber != objectIdentifierAppTag)
			throw "Mismatched data type, object identifier expected";
	} else
	if (tlist.Length() == 3) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		indx = 1;
		if (!tag.tagClass)
			throw "Context tagged object identifier expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Object identifier keyword requires 1, 2 or 3 parameters";

	// reference or real data?
	if (tlist[indx].tokenType == scriptReference) {
		BACnetAnyValue		bacnetEPICSProperty;

		GetEPICSProperty( tlist[indx].tokenSymbol, &bacnetEPICSProperty, tlist[indx].m_nIndex);

		if ( bacnetEPICSProperty.GetType() != ob_id )
			throw "Object identifier property value expected in EPICS";

		scriptData = *((BACnetObjectIdentifier *) bacnetEPICSProperty.GetObject());
	} else {
		if (!tlist[indx].IsInteger( objType, ScriptObjectTypeMap ))
			throw "Object identifier type expected";
		if ( tlist[indx+1].IsDontCare() )
			spep->exprOp = '?=';
		else if (!tlist[indx+1].IsInteger( instanceNum ))
			throw "Object identifier instance expected";

//		scriptData = (objType << 22) + instanceNum;
		scriptData.SetValue((BACnetObjectType) objType, instanceNum);
	}
	
	// decode it
	objData.Decode( dec );
	CompareAndThrowError(objData, scriptData, spep->exprOp, IDS_SCREX_COMPFAILOBJID);

/*	// verify the value
	if (!Match(spep->exprOp,objData.objID,scriptData))
		throw "Object identifier mismatch";
*/
	// store the context for property references that may appear later
	execObjID = objData.objID;
}

//
//	ScriptExecutor::ExpectALDeviceIdentifier
//

void ScriptExecutor::ExpectALDeviceIdentifier( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int						context = kAppContext, instanceNum
	;
	BACnetObjectIdentifier	objData
	;
	ScriptTokenList			tlist
	;
	BACnetAPDUTag			tag
	;
	
	// extract the tag
	tag.Peek( dec );
	
	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if ( tlist[0].IsDontCare() )
			spep->exprOp = '?=';
		else if (!tlist[0].IsInteger( instanceNum ))
			throw "Device identifier instance value expected";
		if (tag.tagClass)
			throw "Application tagged object identifier expected";
		if (tag.tagNumber != objectIdentifierAppTag)
			throw "Mismatched data type, object identifier expected";
	} else if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		if ( tlist[1].IsDontCare() )
			spep->exprOp = '?=';
		else if (!tlist[1].IsInteger( instanceNum ))
			throw "Device identifier instance value expected";
		if (!tag.tagClass)
			throw "Context tagged object identifier expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Device identifier keyword requires 1 or 2 parameters";

	// decode it
	objData.Decode( dec );
	CompareAndThrowError(objData, BACnetObjectIdentifier(8, instanceNum), spep->exprOp, IDS_SCREX_COMPFAILDEVID);
/*
	// verify the value
	if (!Match(spep->exprOp,(unsigned long) objData.objID,(unsigned long)((8 << 22) + instanceNum)))
		throw "Device identifier mismatch";
*/
}

//
//	ScriptExecutor::ExpectALPropertyIdentifier
//

void ScriptExecutor::ExpectALPropertyIdentifier( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					context = kAppContext, valu
	;
	BACnetEnumerated	enumData
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( valu, ScriptPropertyMap ))
			throw "Property name expected";
		if (tag.tagClass)
			throw "Application tagged enumerated (property identifier) expected";
		if (tag.tagNumber != enumeratedAppTag)
			throw "Mismatched data type, enumerated (property identifier) expected";
	} else
	if (tlist.Length() == 2) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		if (!tlist[1].IsInteger( valu, ScriptPropertyMap ))
			throw "Property name expected";
		if (!tag.tagClass)
			throw "Context tagged enumerated expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Property identifier keyword requires 1 or 2 parameters";

	// decode it
	enumData.Decode( dec );
	CompareAndThrowError(enumData, BACnetEnumerated(valu), spep->exprOp, IDS_SCREX_COMPFAILPROP);
/*
	// verify the value
	if (!Match(spep->exprOp,enumData.enumValue,valu))
		throw "Enumeration value mismatch";
*/
}

//
//	ScriptExecutor::ExpectALOpeningTag
//

void ScriptExecutor::ExpectALOpeningTag( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					context
	;
	BACnetOpeningTag	openTag
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Opening tag requires tag number";

	// decode it
	openTag.Decode( dec );
}

//
//	ScriptExecutor::ExpectALClosingTag
//

void ScriptExecutor::ExpectALClosingTag( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec )
{
	int					context
	;
	BACnetClosingTag	closeTag
	;
	ScriptTokenList		tlist
	;
	BACnetAPDUTag		tag
	;
	
	// extract the tag
	tag.Peek( dec );

	// translate the expression, resolve parameter names into values
	ResolveExpr( spep->exprValue, spep->exprLine, tlist );

	// tag is optional
	if (tlist.Length() == 1) {
		if (!tlist[0].IsInteger( context ))
			throw "Tag number expected";
		if (tag.tagNumber != context)
			throw "Mismatched context tag value";
	} else
		throw "Closing tag requires tag number";

	// decode it
	closeTag.Decode( dec );
}

//
//	ScriptExecutor::GetKeywordValue
//

ScriptPacketExprPtr ScriptExecutor::GetKeywordValue( int keyword, BACnetEncodeable &enc, ScriptTranslateTablePtr tp )
{
	ScriptPacketExprPtr	pep = 0
	;

	// find the keyword
	pep = execPacket->packetExprList.Find( keyword );
	if (!pep)
		return 0;

	ScriptTokenList		tlist
	;

	// translate the expression, resolve parameter names into values
	ResolveExpr( pep->exprValue, pep->exprLine, tlist );
	if (tlist.Length() < 1)
		throw ExecError( "Keyword value expected", pep->exprLine );

	// get a reference to the first parameter
	const ScriptToken &t = tlist[0];

	// if it is a keyword, lookup the value
	if ((t.tokenType == scriptKeyword) && tp) {
		int rslt = t.Lookup( t.tokenSymbol, tp );
		if (rslt < 0)
			throw ExecError( "Invalid keyword", pep->exprLine );

		char buff[16];
		sprintf( buff, "%d", rslt );

		try {
			enc.Decode( buff );
		}
		catch (...) {
			throw ExecError( "Keyword translation format error", pep->exprLine );
		}
	} else
	if (!t.IsEncodeable( enc ))
		throw ExecError( "Value translation format error", pep->exprLine );

	// success
	return pep;
}

//
//	ScriptExecutor::ResetFamily
//

void ScriptExecutor::ResetFamily( ScriptBasePtr sbp )
{
	// set the object status
	sbp->baseStatus = 0;

	// tell the app to set the image list element to match
	SetImageStatus( sbp, 0 );

	// change the children
	for (POSITION xpos = sbp->GetHeadPosition(); xpos; )
		ResetFamily( (ScriptBasePtr)sbp->GetNext( xpos ) );
}

//
//	ScriptExecutor::ResetTest
//

void ScriptExecutor::ResetTest( ScriptTestPtr stp )
{
	// reset all non-dependancy elements
	for (int j = 0; j < stp->Length(); j++) {
		ScriptBasePtr sbp = (ScriptBasePtr)stp->Child( j );
		if (sbp->baseType != ScriptBase::scriptDependency)
			ResetFamily( sbp );
	}

	// change this test status (which can update the section)
	SetTestStatus( stp, 0 );
}

//
//	ScriptExecutor::SetTestStatus
//

void ScriptExecutor::SetTestStatus( ScriptTestPtr stp, int stat )
{
	bool	depChanged
	;

	ASSERT( stp->baseType == ScriptBase::scriptTest );

	// set the object status
	stp->baseStatus = stat;

	// tell the app to set the image list element to match
	SetImageStatus( stp, stat );

	// update the status of the dependencies
	// get the parent section
	ScriptSectionPtr ssp = (ScriptSectionPtr)stp->baseParent;

	// loop through the tests
	for (int i = 0; i < ssp->Length(); i++) {
		ScriptTestPtr tp = (ScriptTestPtr)ssp->Child( i );

		// reset the change flag
		depChanged = false;

		// check for a dependancy
		for (int j = 0; j < tp->Length(); j++) {
			ScriptDependencyPtr sdp = (ScriptDependencyPtr)tp->Child( j );
			if (sdp->baseType != ScriptBase::scriptDependency)
				continue;

			// check the name
			if (sdp->baseLabel == stp->baseLabel) {
				// set the object status
				sdp->baseStatus = stat;

				// tell the app to set the image list element to match
				SetImageStatus( sdp, stat );

				depChanged = true;
			}
		}

		// verify the test status to match the deps
		if (depChanged && (tp != stp)) {
			int newStatus = CalcTestStatus( tp );
			if (newStatus != tp->baseStatus)
				SetTestStatus( tp, newStatus );		// ### if A deps on B and B on A, recursive nightmare!
		}
	}

	// verify the section status
	VerifySectionStatus( ssp );
}

//
//	ScriptExecutor::SetPacketStatus
//

void ScriptExecutor::SetPacketStatus( ScriptPacketPtr spp, int stat )
{
	TRACE2( "Packet %08X status %d\n", spp, stat );

	// set the object status
	spp->baseStatus = stat;

	// tell the app to set the image list element to match
	SetImageStatus( spp, stat );
}

//
//	ScriptExecutor::CalcTestStatus
//

int ScriptExecutor::CalcTestStatus( ScriptTestPtr stp )
{
	int		newStatus = stp->baseStatus
	;

	// loop through the deps
	for (int i = 0; i < stp->Length(); i++) {
		ScriptDependencyPtr sdp = (ScriptDependencyPtr)stp->Child( i );
		if (sdp->baseType != ScriptBase::scriptDependency)
			continue;
		
		// test all the combinations
		switch ((newStatus << 4) + sdp->baseStatus) {
			case 0x00:		// no status for some
			case 0x10:
			case 0x01:
				newStatus = 0;
				break;

			case 0x11:		// everything OK so far, stay green
				newStatus = 1;
				break;

			case 0x02:		// found (or had) a warning
			case 0x12:
			case 0x20:
			case 0x21:
			case 0x22:
				newStatus = 2;
				break;

			case 0x03:		// found (or had) a failure
			case 0x13:
			case 0x23:
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
				newStatus = 3;
				break;
		}
	}

	// return the result
	return newStatus;
}

void ScriptExecutor::VerifySectionStatus( ScriptSectionPtr ssp )
{
	int			newStatus = 0
	,			len = ssp->Length()
	;

	// no tests in section leaves it at 0
	if (len == 0) {
		// set the object status
		ssp->baseStatus = newStatus;

		// tell the app to set the image list element to match
		SetImageStatus( ssp, newStatus );

		return;
	}

	// pick up the first child status
	newStatus = ssp->Child( 0 )->baseStatus;

	// loop through the deps
	for (int i = 1; i < ssp->Length(); i++) {
		ScriptTestPtr stp = (ScriptTestPtr)ssp->Child( i );
		
		// test all the combinations
		switch ((newStatus << 4) + stp->baseStatus) {
			case 0x00:		// no status for some
			case 0x10:
			case 0x01:
				newStatus = 0;
				break;

			case 0x11:		// everything OK so far, stay green
				newStatus = 1;
				break;

			case 0x02:		// found (or had) a warning
			case 0x12:
			case 0x20:
			case 0x21:
			case 0x22:
				newStatus = 2;
				break;

			case 0x03:		// found (or had) a failure
			case 0x13:
			case 0x23:
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
				newStatus = 3;
				break;
		}
	}

	// see if the status changed
	if (newStatus != ssp->baseStatus) {
		// set the object status
		ssp->baseStatus = newStatus;

		// tell the app to set the image list element to match
		SetImageStatus( ssp, newStatus );
	}
}

//
//	ScriptExecutor::SendNPDU
//
//	When a packet has been built it is sent to a specific filter for forwarding 
//	to the correct endpoint.  This function is provided as a compliment to 
//	ReceiveNPDU().
//

void ScriptExecutor::SendNPDU( ScriptNetFilterPtr fp, const BACnetNPDU &npdu )
{
	fp->Indication( npdu );
}

//
//	ScriptExecutor::ReceiveNPDU
//
//	This function is called by a filter when some message is coming up through 
//	the stack.  The executor will check to see if it passes whatever test is 
//	pending in the currently running script.
//

void ScriptExecutor::ReceiveNPDU( ScriptNetFilterPtr fp, const BACnetNPDU &npdu )
{
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// if were not running, just toss the message
	if (execState != execRunning)
		return;

	TRACE1( "Got an NPDU on %s\n", fp->filterName );

	// if we're not expecting something, toss it
	if (!execPending) {
		TRACE0( "(not expecting a packet)\n" );
		Msg( 3, 0, "Exector not expecting a packet" );
		return;
	}
	if (execPacket->packetType != ScriptPacket::expectPacket) {
		TRACE0( "(not pointing to an expect packet)\n" );
		Msg( 3, 0, "Exector not pointing to an EXPECT packet" );
		return;
	}

	// match against the pending tests
	for (ScriptPacketPtr pp = execPacket; pp; pp = pp->packetFail)
		if (pp->baseStatus == 2) {
			// this test is still pending, stash the execPacket
			ScriptPacketPtr savePacket = execPacket;
			execPacket = pp;
			if (ExpectPacket(fp,npdu)) {
				// test was successful, reset all pending packets to unprocessed
				for (ScriptPacketPtr pp1 = savePacket; pp1; pp1 = pp1->packetFail)
					if ((pp1->baseStatus == 2) && (pp1 != pp))
						SetPacketStatus( pp1, 0 );

				// move on to next statement
				NextPacket( true );
				break;
			}
			execPacket = savePacket;
		}

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::ReceiveAPDU
//
//	This function is called when a complete APDU is received from the device
//	object.
//

void ScriptExecutor::ReceiveAPDU( const BACnetAPDU &apdu )
{
	CSingleLock		lock( &execCS )
	;

	// lock to prevent multiple thread access
	lock.Lock();

	// if were not running, just toss the message
	if (execState != execRunning)
		return;

	TRACE0( "Got some APDU!\n" );

	// if we're not expecting something, toss it
	if (!execPending) {
		TRACE0( "(not expecting a packet)\n" );
		Msg( 3, 0, "Exector not expecting a packet" );
		return;
	}
	if (execPacket->packetType != ScriptPacket::expectPacket) {
		TRACE0( "(not pointing to an expect packet)\n" );
		Msg( 3, 0, "Exector not pointing to an EXPECT packet" );
		return;
	}

	// match against the pending tests
	for (ScriptPacketPtr pp = execPacket; pp; pp = pp->packetFail)
		if (pp->baseStatus == 2) {
			// this test is still pending, stash the execPacket
			ScriptPacketPtr savePacket = execPacket;
			execPacket = pp;
			if (ExpectDevPacket(apdu)) {
				// test was successful, reset all pending packets to unprocessed
				for (ScriptPacketPtr pp1 = savePacket; pp1; pp1 = pp1->packetFail)
					if ((pp1->baseStatus == 2) && (pp1 != pp))
						SetPacketStatus( pp1, 0 );

				// move on to next statement
				NextPacket( true );
				break;
			}
			execPacket = savePacket;
		}

	// unlock
	lock.Unlock();
}

//
//	ScriptExecutor::SetImageStatus
//

void ScriptExecutor::SetImageStatus( ScriptBasePtr sbp, int stat )
{
	ScriptExecMsgPtr	msg = new ScriptExecMsg()
	;

	// describe the change
	msg->msgDoc = execDoc;
	msg->msgBase = sbp;
	msg->msgStatus = stat;

	// queue it
	execMsgQueue.Write( msg );

	// tell the application
	::PostThreadMessage( AfxGetApp()->m_nThreadID
		, WM_VTS_EXECMSG, (WPARAM)0, (LPARAM)0
		);
}


LPCSTR OperatorToString(int iOperator)
{
	switch( iOperator )
	{
		case '?=':	return "?=";	// don't care case
		case '=': return "=";
		case '<': return "<";
		case '>': return ">";
		case '<=': return "<=";
		case '>=': return ">=";
		case '!=': return "!=";
	}

	ASSERT(0);
	return NULL;
}






//
//	Matching Functions
//


bool Match( int op, int a, int b )
{
	switch (op) {
		case '?=':	return true;	// don't care case
		case '=': return (a == b);
		case '<': return (a < b);
		case '>': return (a > b);
		case '<=': return (a <= b);
		case '>=': return (a >= b);
		case '!=': return (a != b);
	}

	return false;
}

bool Match( int op, unsigned long a, unsigned long b )
{
	switch (op) {
		case '?=':	return true;	// don't care case
		case '=': return (a == b);
		case '<': return (a < b);
		case '>': return (a > b);
		case '<=': return (a <= b);
		case '>=': return (a >= b);
		case '!=': return (a != b);
	}

	return false;
}

bool Match( int op, float a, float b )
{
	switch (op) {
		//Modified by Yajun Zhou, 2002-7-16
		//case "=": return (a == b);
		//case '<': return (a < b);
		//case '>': return (a > b);
		//case '<=': return (a <= b);
		//case '>=': return (a >= b);
		//case '!=': return (a != b);
		case '?=':	return true;	// don't care case
		case '=': return (fabs(a - b) < FLOAT_EPSINON);
		case '<': return (a < b && fabs(a - b) > FLOAT_EPSINON);
		case '>': return (a > b && fabs(a - b) > FLOAT_EPSINON);
		case '<=': return (a < b || fabs(a - b) < FLOAT_EPSINON);
		case '>=': return (a > b || fabs(a - b) < FLOAT_EPSINON);
		case '!=': return (fabs(a - b) > FLOAT_EPSINON);
		///////////////////////////////////
	}

	return false;
}

bool Match( int op, double a, double b )
{
	switch (op) {
		//Modified by Yajun Zhou, 2002-7-16
		//case "=": return (a == b);
		//case '<': return (a < b);
		//case '>': return (a > b);
		//case '<=': return (a <= b);
		//case '>=': return (a >= b);
		//case '!=': return (a != b);
		case '=': return (fabs(a - b) < DOUBLE_EPSINON);
		case '<': return (a < b && fabs(a - b) > DOUBLE_EPSINON);
		case '>': return (a > b && fabs(a - b) > DOUBLE_EPSINON);
		case '<=': return (a < b || fabs(a - b) < DOUBLE_EPSINON);
		case '>=': return (a > b || fabs(a - b) < DOUBLE_EPSINON);
		case '!=': return (fabs(a - b) > DOUBLE_EPSINON);
		///////////////////////////////////
	}

	return false;
}


bool Match( int op, CTime &timeThis, CTime &timeThat )
{
	switch(op)
	{
		case '?=':	return true;	// don't care case
		case '=':	return (timeThis == timeThat) != 0;		// stop crazy Microsoft BOOL warning
		case '<':	return (timeThis < timeThat) != 0;
		case '>':	return (timeThis > timeThat) != 0;
		case '<=':	return (timeThis <= timeThat) != 0;
		case '>=':	return (timeThis >= timeThat) != 0;
		case '!=':	return (timeThis != timeThat) != 0;
		default:
			ASSERT(0);
	}
	return false;
}




/*
void ScriptExecutor::CompareStreamData( BACnetAPDUDecoder & dec, int iOperator, const BACnetOctet * pData, int nLen, LPCSTR lpstrValueName )
{
	// Let's just compare byte by byte
	// If the comparison failed, we won't even get here due to throw condition

	CString strThrowMessage;

	// check the length
	if (dec.pktLength < nLen)
		strThrowMessage.Format(IDS_SCREX_COMPSHORTDATA, dec.pktLength, lpstrValueName, nLen );
	else
	{
		// Good ol' memcmp will do for this but we need to check to see if we're supposed to equal or not equal...

		int iCompResult = memcmp(dec.pktBuffer, (void *) pData, nLen);
		switch( iOperator )
		{
			case '=':

				if ( iCompResult != 0 )
					strThrowMessage.Format(IDS_SCREX_COMPSTREAMFAIL_NE, lpstrValueName );
				break;

			case '!=':

				if ( iCompResult == 0 )
					strThrowMessage.Format(IDS_SCREX_COMPSTREAM_E, lpstrValueName );
				break;

			default:		// just give up, eh?

				strThrowMessage.Format(IDS_SCREX_COMPEQREQ, lpstrValueName );
		}
	}

	if ( !strThrowMessage.IsEmpty() )
		throw CString(strThrowMessage);

	dec.pktLength -= nLen;
	dec.pktBuffer += nLen;
}
*/