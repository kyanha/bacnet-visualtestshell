// ScriptExecutor.h: interface for the ScriptExecutor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTEXECUTOR_H__AC59C5B2_BAFF_11D4_BEF7_00A0C95A9812__INCLUDED_)
#define AFX_SCRIPTEXECUTOR_H__AC59C5B2_BAFF_11D4_BEF7_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>
#include <afxtempl.h>

#include "VTSDoc.h"
#include "ScriptBase.h"
#include "ScriptDocument.h"
#include "ScriptPacket.h"

#include "BACnet.hpp"
#include "BACnetTask.hpp"
#include "VTSQueue.h"

//
//	ScriptExecMsg
//

class ScriptExecMsg {
	public:
		ScriptDocumentPtr	msgDoc;			// current script document
		ScriptBasePtr		msgBase;		// section/test/deps/packet
		int					msgStatus;		// common status code
	};

typedef ScriptExecMsg *ScriptExecMsgPtr;
const int kScriptExecMsgSize = sizeof( ScriptExecMsg );

typedef VTSQueue<ScriptExecMsg> ScriptExecMsgQueue;

//
//	ScriptFilter
//

const int kMaxScriptFilterNameLen = 32;

enum ScriptFilterType
	{ scriptNPDUFilter
	, scriptAPDUFilter
	};

class ScriptFilter {
	public:
		ScriptFilter( ScriptFilterType typ, char *name );
		virtual ~ScriptFilter( void );

		ScriptFilterType	filterType;
		char				filterName[kMaxScriptFilterNameLen];		// filter name
	};

typedef ScriptFilter *ScriptFilterPtr;
const int kScriptFilterSize = sizeof( ScriptFilter );

//
//	ScriptFilterList
//

class ScriptFilterList : public CList<ScriptFilterPtr,ScriptFilterPtr> {
		friend class ScriptFilter;

	protected:
		void AddFilter( ScriptFilterPtr fp );					// add a filter
		void RemoveFilter( ScriptFilterPtr fp );				// remove a filter

	public:
		ScriptFilterList( void );
		~ScriptFilterList( void );

		ScriptFilterPtr FindFilter( const char *name );			// find a filter

		int Length( void );										// number of defined filters
		ScriptFilterPtr operator []( int i );					// index into filter list
	};

typedef ScriptFilterList *ScriptFilterListPtr;
const int kScriptFilterListSize = sizeof( ScriptFilterList );

extern ScriptFilterList	gMasterFilterList;						// global list of all filters

//
//	ScriptNetFilter
//

class ScriptNetFilter : public ScriptFilter, public BACnetNetClient, public BACnetNetServer {
	public:
		ScriptNetFilter( char *name );
		virtual ~ScriptNetFilter( void );

		virtual void Indication( const BACnetNPDU &npdu );
		virtual void Confirmation( const BACnetNPDU &npdu );
	};

typedef ScriptNetFilter *ScriptNetFilterPtr;
const int kScriptNetFilterSize = sizeof( ScriptNetFilter );

//
//	ScriptNetPacket
//

class ScriptNetPacket : public BACnetTask {
	protected:
		ScriptFilterPtr		packetFilter;		// where this came from
		FILETIME			packetTime;			// receive time
		BACnetOctet			*packetData;		// copy of the data
		BACnetNPDU			packetNPDU;			// other content

		void ProcessTask( void );				// deliver to executor
		
	public:
		ScriptNetPacket( ScriptFilterPtr fp, const BACnetNPDU &npdu );
		virtual ~ScriptNetPacket( void );

	private:
		ScriptNetPacket( const ScriptNetPacket &pkt );	// no copy ctor
		void operator =( const ScriptNetPacket &pkt );	// no assignment
	};

typedef ScriptNetPacket *ScriptNetPacketPtr;
const int kScriptNetPacketSize = sizeof( ScriptNetPacket );

//
//	ScriptDebugNetFilter
//

class ScriptDebugNetFilter : public ScriptFilter, public BACnetNetServer {
	public:
		ScriptDebugNetFilter	*filterPeer;

		ScriptDebugNetFilter( char *name );
		virtual ~ScriptDebugNetFilter( void );

		virtual void Indication( const BACnetNPDU &npdu );
	};

typedef ScriptDebugNetFilter *ScriptDebugNetFilterPtr;
const int kScriptDebugNetFilterSize = sizeof( ScriptDebugNetFilter );

void Bind( ScriptDebugNetFilterPtr fp1, ScriptDebugNetFilterPtr fp2 );

//
//	ScriptAppFilter
//

class ScriptAppFilter : public ScriptFilter, public BACnetAppClient, public BACnetAppServer {
	public:
		ScriptAppFilter( char *name );
		virtual ~ScriptAppFilter( void );

		virtual void Indication( const BACnetAPDU &apdu );
		virtual void Confirmation( const BACnetAPDU &apdu );
	};

typedef ScriptAppFilter *ScriptAppFilterPtr;
const int kScriptAppFilterSize = sizeof( ScriptAppFilter );

//
//	ScriptAppPacket
//

class ScriptAppPacket : public BACnetTask {
	protected:
		FILETIME		packetTime;				// receive time
		BACnetOctet		*packetData;			// copy of the data
		BACnetAPDU		packetAPDU;				// other content

		void ProcessTask( void );				// deliver to executor
		
	public:
		ScriptAppPacket( const BACnetNPDU &npdu );

	private:
		ScriptAppPacket( const ScriptAppPacket &pkt );	// no copy ctor
		void operator =( const ScriptAppPacket &pkt );	// no assignment
	};

typedef ScriptAppPacket *ScriptAppPacketPtr;
const int kScriptAppPacketSize = sizeof( ScriptAppPacket );

//
//	ScriptExecutor
//

enum ScriptExecutorState
		{ execIdle
		, execRunning
		, execStopped
		};

class ScriptExecutor : public BACnetTask {
	private:
		class ExecError {
			public:
				const char		*errMsg;
				int				errLineNo;

				ExecError( const char *msg, int lineNo = -1 );
			};

		ScriptExecMsgQueue		execMsgQueue;	// messages to application

		CCriticalSection		execCS;			// app/task control

		ScriptExecutorState		execState;		// execution state
		int						execRootTime;	// time group started
		bool					execPending;	// timer set during execution

		VTSDocPtr				execDB;			// database to receive test information
		ScriptDocumentPtr		execDoc;		// current script document
		ScriptTestPtr			execTest;		// current test
		ScriptPacketPtr			execPacket;		// current packet
		int						execStepForced;	// packet forced to pass (=1) or fail (=2)
		unsigned long			execObjID;		// object context for property references

		void ProcessTask( void );				// timer went off
		void NextPacket( bool okPacket );		// move to next in sequence

		void ResolveExpr( const char *expr, int exprLine, ScriptTokenList &lst );
		void* GetReferenceData( int prop, int *typ, BACnetAPDUDecoder *dp = 0 );

		bool SendPacket( void );				// send execPacket, return true iff success

		void SendBVLCResult( ScriptTokenList &tlist, CByteArray &packet );
		void SendWriteBDT( ScriptTokenList &tlist, CByteArray &packet );
		void SendReadBDTAck( ScriptTokenList &tlist, CByteArray &packet );
		void SendForwardedNPDU( ScriptTokenList &tlist, CByteArray &packet );
		void SendRegisterFD( ScriptTokenList &tlist, CByteArray &packet );
		void SendReadFDTAck( ScriptTokenList &tlist, CByteArray &packet );
		void SendDeleteFDTEntry( ScriptTokenList &tlist, CByteArray &packet );

		void SendWhoIsRouterToNetwork( ScriptTokenList &tlist, CByteArray &packet );
		void SendIAmRouterToNetwork( ScriptTokenList &tlist, CByteArray &packet );
		void SendICouldBeRouterToNetwork( ScriptTokenList &tlist, CByteArray &packet );
		void SendRejectMessageToNetwork( ScriptTokenList &tlist, CByteArray &packet );
		void SendRouterBusyToNetwork( ScriptTokenList &tlist, CByteArray &packet );
		void SendRouterAvailableToNetwork( ScriptTokenList &tlist, CByteArray &packet );
		void SendInitializeRoutingTable( ScriptTokenList &tlist, CByteArray &packet );
		void SendInitializeRoutingTableAck( ScriptTokenList &tlist, CByteArray &packet );
		void SendEstablishConnectionToNetwork( ScriptTokenList &tlist, CByteArray &packet );
		void SendDisconnectConnectionToNetwork( ScriptTokenList &tlist, CByteArray &packet );

		void SendConfirmedRequest( CByteArray &packet );
		void SendUnconfirmedRequest( CByteArray &packet );
		void SendSimpleACK( CByteArray &packet );
		void SendComplexACK( CByteArray &packet );
		void SendSegmentACK( CByteArray &packet );
		void SendError( CByteArray &packet );
		void SendReject( CByteArray &packet );
		void SendAbort( CByteArray &packet );

		void SendALNull( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALBoolean( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALUnsigned( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALInteger( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALReal( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALDouble( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALOctetString( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALCharacterString( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALBitString( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALEnumerated( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALDate( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALTime( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALObjectIdentifier( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALDeviceIdentifier( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALPropertyIdentifier( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALOpeningTag( ScriptPacketExprPtr spep, CByteArray &packet );
		void SendALClosingTag( ScriptPacketExprPtr spep, CByteArray &packet );

		void SendALData( CByteArray &packet );

		bool ExpectPacket( ScriptNetFilterPtr fp, const BACnetNPDU &npdu );	// match execPacket, return true iff success

		void ExpectBVLCResult( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectWriteBDT( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectReadBDTAck( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectForwardedNPDU( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectRegisterFD( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectReadFDTAck( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectDeleteFDTEntry( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );

		void ExpectWhoIsRouterToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectIAmRouterToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectICouldBeRouterToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectRejectMessageToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectRouterBusyToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectRouterAvailableToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectInitializeRoutingTable( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectEstablishConnectionToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );
		void ExpectDisconnectConnectionToNetwork( ScriptTokenList &tlist, BACnetAPDUDecoder &dec );

		void ExpectConfirmedRequest( BACnetAPDUDecoder &dec );
		void ExpectUnconfirmedRequest( BACnetAPDUDecoder &dec );
		void ExpectSimpleACK( BACnetAPDUDecoder &dec );
		void ExpectComplexACK( BACnetAPDUDecoder &dec );
		void ExpectSegmentACK( BACnetAPDUDecoder &dec );
		void ExpectError( BACnetAPDUDecoder &dec );
		void ExpectReject( BACnetAPDUDecoder &dec );
		void ExpectAbort( BACnetAPDUDecoder &dec );

		void ExpectALData( BACnetAPDUDecoder &dec );

		void ExpectALNull( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALBoolean( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALUnsigned( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALInteger( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALReal( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALDouble( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALOctetString( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALCharacterString( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALBitString( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALEnumerated( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALDate( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALTime( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALObjectIdentifier( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALDeviceIdentifier( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALPropertyIdentifier( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALOpeningTag( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );
		void ExpectALClosingTag( ScriptPacketExprPtr spep, BACnetAPDUDecoder &dec );

		ScriptPacketExprPtr GetKeywordValue( int keyword, BACnetEncodeable &enc, ScriptTranslateTablePtr tp = 0 );

		void ResetFamily( ScriptBasePtr sbp );
		void ResetTest( ScriptTestPtr stp );
		void SetTestStatus( ScriptTestPtr stp, int stat );		// change the test status
		void SetPacketStatus( ScriptPacketPtr spp, int stat );	// change the packet status
		int CalcTestStatus( ScriptTestPtr stp );				// check deps and return status
		void VerifySectionStatus( ScriptSectionPtr ssp );

		void SetImageStatus( ScriptBasePtr sbp, int stat );		// change section/test/deps/packet status

	public:
		ScriptExecutor( void );
		virtual ~ScriptExecutor();

		void Setup( VTSDocPtr vdp, ScriptDocumentPtr sdp, ScriptTestPtr stp );
		void Cleanup( void );

		bool					execAllTests;		// true iff all tests
		bool					execSingleStep;		// break after current packet
		bool					execFailContinue;	// iff checking all tests and one failed, keep going anyway

		void Msg( int sc, int line, const char *msg );	// save a message in the database

		void Run( void );						// continue running
		void Halt( void );						// set to not running
		void Step( void );						// exec current packet, set up next one
		void Step( bool pass );					// act like packet being processed
		void Resume( void );					// go back to running
		void Kill( void );						// fail test and exit

		ScriptExecMsgPtr ReadMsg( void );		// read a message from the queue

		inline bool IsIdle( void ) { return (execState == execIdle); }
		inline bool IsRunning( void ) { return (execState == execRunning); }
		inline bool IsStopped( void ) { return (execState == execStopped); }

		bool IsBound( VTSDocPtr sdp );
		bool IsBound( ScriptDocumentPtr sdp );

		void SendNPDU( ScriptNetFilterPtr fp, const BACnetNPDU &npdu );
		void ReceiveNPDU( ScriptNetFilterPtr fp, const BACnetNPDU &npdu );

		void SendAPDU( ScriptAppFilterPtr fp, const BACnetAPDU &npdu );
		void ReceiveAPDU( ScriptAppFilterPtr fp, const BACnetAPDU &npdu );
	};

typedef ScriptExecutor *ScriptExecutorPtr;
const int kScriptExecutorSize = sizeof( ScriptExecutor );

extern ScriptExecutor	gExecutor;

#endif // !defined(AFX_SCRIPTEXECUTOR_H__AC59C5B2_BAFF_11D4_BEF7_00A0C95A9812__INCLUDED_)
