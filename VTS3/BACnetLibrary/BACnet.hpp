#ifndef _BACnet
#define _BACnet

#define _TSMDebug	1

#if _TSMDebug
#include <iostream.h>
#endif

//
//	General Typedefs
//

typedef unsigned char	BACnetOctet, *BACnetOctetPtr;	// unsigned character

//
//	General Constants
//

const unsigned short	kVendorID = 15;					// Cornell vendor ID
const int				kMaxAddressLen = 8;				// longest address supported

//
//	BACnet Address
//
//	These address types are listed in increasing order of complexity
//

enum BACnetAddressType
	{ nullAddr
	, localBroadcastAddr
	, localStationAddr
	, remoteBroadcastAddr
	, remoteStationAddr
	, globalBroadcastAddr
	};

struct BACnetAddress {
	BACnetAddressType	addrType;
	unsigned short		addrNet;
	unsigned short		addrLen;
	unsigned char		addrAddr[kMaxAddressLen];
	
	BACnetAddress( const unsigned char *addr, const unsigned short len );
	BACnetAddress( const unsigned short net, const unsigned char *addr, const unsigned short len );
	BACnetAddress( const BACnetAddressType typ = nullAddr, const unsigned short net = 0, const unsigned char *addr = 0, const unsigned short len = 0 );

	// initializers (when constructor can't be used)
	void LocalStation( const unsigned char *addr, const unsigned short len );
	void RemoteStation( const unsigned short net, const unsigned char *addr, const unsigned short len );
	void LocalBroadcast( void );
	void RemoteBroadcast( const short net );
	void GlobalBroadcast( void );
	};

typedef BACnetAddress *BACnetAddressPtr;
const int kBACnetAddressSize = sizeof( BACnetAddress );

int operator ==( const BACnetAddress &addr1, const BACnetAddress &addr2 );
#if _TSMDebug
ostream &operator <<(ostream &strm,const BACnetAddress &addr);
#endif

//
//	BACnetContext
//

const int kAppContext = -1;

//	BACnetEncodeable

class BACnetAPDUEncoder;
class BACnetAPDUDecoder;

class BACnetEncodeable {
	public:
		virtual void Encode( BACnetAPDUEncoder& enc, int context = kAppContext ) = 0;
		virtual void Decode( BACnetAPDUDecoder& dec ) = 0;
		void Peek( BACnetAPDUDecoder& dec );

		virtual void Encode( char *enc );
		virtual void Decode( const char *dec );
	};

typedef BACnetEncodeable *BACnetEncodeablePtr;

//
//	BACnetPDU Atomic Types
//

class BACnetNull : public BACnetEncodeable {
	public:
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetBoolean : public BACnetEncodeable {
	public:
		enum eBACnetBoolean { bFalse = 0, bTrue = 1 };
		eBACnetBoolean		boolValue;
		
		BACnetBoolean( int bvalu = 0 );
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetEnumerated : public BACnetEncodeable {
	public:
		int		enumValue;
		
		BACnetEnumerated( int evalu = 0 );
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );

		void Encode( char *enc, const char **table, int tsize );
		void Decode( const char *dec, const char **table, int tsize );
	};

class BACnetUnsigned : public BACnetEncodeable {
	public:
		unsigned int	uintValue;
		
		BACnetUnsigned( unsigned int uivalu = 0 );
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetInteger : public BACnetEncodeable {
	public:
		int				intValue;
		
		BACnetInteger( int ivalu = 0 );
		inline int getValue(void) { return intValue; }
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetReal : public BACnetEncodeable {
	public:
		float			realValue;
	
		BACnetReal( float rvalu = 0.0 );
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetDouble : public BACnetEncodeable {
	public:
		double			doubleValue;
		
		BACnetDouble( double dvalu = 0.0 );
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetCharacterString : public BACnetEncodeable {
	public:
		int				strEncoding;	// encoding
		unsigned		strLen;			// number of octets
		BACnetOctet		*strBuff;		// pointer to data
				
		BACnetCharacterString( char *svalu = 0 );
		virtual ~BACnetCharacterString( void );
		
		void SetValue( char *svalu, int enc = 0 );

		bool Equals( const char *valu );	// true iff matches, must be ASCII encoding

		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetOctetString : public BACnetEncodeable {
	public:
		int				strLen;			// number of octets
		int				strBuffLen;		// non-zero if owned
		BACnetOctet		*strBuff;		// pointer to data
		
		void PrepBuffer( int size );	// make buffer owned and at least (size) octets

		BACnetOctetString( void );
		BACnetOctetString( int len );
		BACnetOctetString( BACnetOctet *bytes, int len );
		BACnetOctetString( const BACnetOctetString &cpy );
		virtual ~BACnetOctetString( void );
		
		void Flush( void );
		void Append( BACnetOctet byte );
		void Insert( BACnetOctet *bytes, int len, int pos );
		void Insert( const BACnetOctetString &cpy, int pos );
		BACnetOctet &operator [](const int indx);
		void Reference( BACnetOctet *bytes, int len );
		int Length( void );

		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetBitString : public BACnetEncodeable {
	protected:
		int				bitLen;
		int				bitBuffLen;
		unsigned long	*bitBuff;
		
		void	SetSize( int siz );
		
	public:
		BACnetBitString( void );
		BACnetBitString( int siz );
		BACnetBitString( const BACnetBitString &cpy );
		virtual ~BACnetBitString( void );
		
		void	SetBit( int bit, int valu = 1 );
		void	ResetBit( int bit );
		int		GetBit( int bit );
		
		const int operator [](int bit);
		BACnetBitString &operator +=( const int bit );
		BACnetBitString &operator -=( const int bit );
		
		BACnetBitString &operator =( const BACnetBitString &arg );
		BACnetBitString &operator |=( const BACnetBitString &arg );
		BACnetBitString &operator &=( const BACnetBitString &arg );

		bool operator ==( const BACnetBitString &arg );

		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetDate : public BACnetEncodeable {
	public:
		int			year;						// year - 1900, 255 = don't care
		int			month;						// 1..12
		int			day;						// 1..31
		int			dayOfWeek;					// 1 = Monday
		
		BACnetDate( void );
		BACnetDate( int y, int m, int d );
		
		void	CalcDayOfWeek( void );					// compute dayOfWeek from date
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

class BACnetTime : public BACnetEncodeable {
	public:
		int			hour;						// 0..23, 255 = don't care
		int			minute;						// 0..59
		int			second;						// 0..59
		int			hundredths;					// 0..99
		
		BACnetTime( void );
		BACnetTime( int hr, int mn = 0, int sc = 0, int hun = 0 );
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

enum BACnetObjectType {
		analogInput						= 0,
		analogOutput					= 1,
		analogValue						= 2,
		binaryInput						= 3,
		binaryOutput					= 4,
		binaryValue						= 5,
		calendar						= 6,
		command							= 7,
		device							= 8,
		eventEnrollment					= 9,
		file							= 10,
		group							= 11,
		loop							= 12,
		multistateInput					= 13,
		multistateOutput				= 14,
		notificationClass				= 15,
		program							= 16,
		schedule						= 17
		};

class BACnetObjectIdentifier : public BACnetEncodeable {
	public:
		unsigned int	objID;
		
		BACnetObjectIdentifier( int objType = 0, int instanceNum = 0 );

		void SetValue( BACnetObjectType objType, int instanceNum );
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
		void Encode( char *enc );
		void Decode( const char *dec );
	};

//
//	Opening and Closing Tags
//
//	These are not native BACnet object types per se, they are just used to open and close 
//	a known context.  There are no attributes to keep track of, so there's no constructor 
//	and the object really doesn't take up any size.
//
//	There is no application encoding for an opening tag, so the context parameter is 
//	required.  During decoding the application can peek at the tag and see what kind of 
//	thing the next piece is, along with the context if it's context encoded.
//

class BACnetOpeningTag : public BACnetEncodeable {
	public:
		void Encode( BACnetAPDUEncoder& enc, int context );		// encode, context required
		void Decode( BACnetAPDUDecoder& dec );					// decode
	};

class BACnetClosingTag : public BACnetEncodeable {
	public:
		void Encode( BACnetAPDUEncoder& enc, int context );		// encode, context required
		void Decode( BACnetAPDUDecoder& dec );					// decode
	};

//
//	BACnetAPDUTag
//
//	A BACnetAPDUTag is a description of content that sits in front of each object in 
//	the stream of data.  According to the standard, the tag 'class' specifies that the 
//	content should be interpreted as 'application' (which describes the type) and 
//	'context' (which really means you have to look it up in the standard to figure 
//	out the datatype, you can't tell just by looking at the tag).
//
//	To help various applications peek into the datastream before decoding it, the 
//	BACnetTagClass defines opening and closing tag classes.  They are not described as 
//	such in the standard, but it really helps simplify the decoding code.
//

enum BACnetTagClass {
		applicationTagClass				= 0,
		contextTagClass					= 1,
		openingTagClass					= 2,
		closingTagClass					= 3
		};

enum BACnetApplicationTag {
		unusedAppTag					= -1,
		nullAppTag						= 0,
		booleanAppTag					= 1,
		unsignedIntAppTag				= 2,
		integerAppTag					= 3,
		realAppTag						= 4,
		doubleAppTag					= 5,
		octetStringAppTag				= 6,
		characterStringAppTag			= 7,
		bitStringAppTag					= 8,
		enumeratedAppTag				= 9,
		dateAppTag						= 10,
		timeAppTag						= 11,
		objectIdentifierAppTag			= 12,
		reservedAppTag13				= 13,
		reservedAppTag14				= 14,
		reservedAppTag15				= 15
		};
		
class BACnetAPDUTag : public BACnetEncodeable {
	public:
		BACnetTagClass			tagClass;			// class
		BACnetApplicationTag	tagNumber;			// tag number
		int						tagLVT;				// length/value/type
		
		BACnetAPDUTag( BACnetApplicationTag tnum = nullAppTag, int tlen = 0 );	// application
		BACnetAPDUTag( int context, int tlen = 0 );								// context
		BACnetAPDUTag( BACnetTagClass tclass, BACnetApplicationTag tnum, int tlen = 0 );
		
		void Encode( BACnetAPDUEncoder& enc, int context = kAppContext );	// encode
		void Decode( BACnetAPDUDecoder& dec );								// decode
	};

//
//	BACnetNPDU
//
//	A BACnetNPDU is the object that gets passed between a client and an endpoint
//	(a server).  It encapsulates the address (source or destination depending on
//	the context) and a stream of octets.  It also has some extra information that 
//	is also encoded in the NPCI (a reply is expected and the network priority) to 
//	make it simpler to access (rather than sucking the same information out of the
//	encoded NPCI).
//

struct BACnetNPDU {
	BACnetAddress		pduAddr;
	const BACnetOctet	*pduData;
	int					pduLen;
	int					pduExpectingReply;		// see 6.2.2 (1 or 0)
	int					pduNetworkPriority;		// see 6.2.2 (0, 1, 2, or 3)
	
	BACnetNPDU( const BACnetAddress &addr, const BACnetOctet *data, const int len
		, const int reply = 0, const int priority = 0
		);
	};

typedef BACnetNPDU *BACnetNPDUPtr;

//
//	BACnetAPDUEncoder
//
//	An encoder is an object passed to the various types to store the encoded version 
//	of themselves.
//

const int kDefaultBufferSize = 1024;

class BACnetAPDUEncoder {
	protected:
		int					pktBuffSize;			// allocated size
		
	public:
		BACnetAPDUEncoder( int initBuffSize = kDefaultBufferSize );
		BACnetAPDUEncoder( BACnetOctet *buffPtr, int buffLen = 0 );		// already have a buffer
		~BACnetAPDUEncoder( void );
		
		BACnetOctet			*pktBuffer;				// pointer to start of buffer
		int					pktLength;				// number of encoded octets
		
		void SetBuffer( const BACnetOctet *buffer, int len );
		void NewBuffer( int len );					// allocate a new buffer

		void CheckSpace( int len );					// resize iff necessary
		void Append( BACnetOctet ch );				// simple copy, should be inline!
		void Append( BACnetOctet *buff, int len );	// raw copy into buffer
		void Flush( void );							// remove all contents
	};

typedef BACnetAPDUEncoder *BACnetAPDUEncoderPtr;
const int kBACnetAPDUEncoderSize = sizeof( BACnetAPDUEncoder );

//
//	BACnetAPDUDecoder
//
//	A decoder sets up a context that is used to decode objects.  It is passed to the 
//	objects to decode themselves.
//

class BACnetAPDUDecoder {
	public:
		const BACnetOctet	*pktBuffer;				// pointer to buffer
		int					pktLength;				// number of encoded octets
		
		BACnetAPDUDecoder( const BACnetOctet *buffer = 0, int len = 0 );
		BACnetAPDUDecoder( const BACnetAPDUEncoder &enc );
		BACnetAPDUDecoder( const BACnetNPDU &npdu );
		
		void SetBuffer( const BACnetOctet *buffer, int len );

		void ExamineTag( BACnetAPDUTag &t );			// just peek at the next tag
		
		void CopyOctets( BACnetOctet *buff, int len );	// raw copy into buffer
		int ExtractData( BACnetOctet *buffer );		// skip the tag and extract the data
	};

typedef BACnetAPDUDecoder *BACnetAPDUDecoderPtr;
const int kBACnetAPDUDecoderSize = sizeof( BACnetAPDUDecoder );

ostream &operator <<(ostream &strm, const BACnetAPDUDecoder &dec );

//
//	BACnetNetClient
//	BACnetNetServer
//

enum BACnetNetworkMessageType
		{	WhoIsRouterToNetwork			= 0x00
		,	IAmRouterToNetwork				= 0x01
		,	ICouldBeRouterToNetwork			= 0x02
		,	RejectMessageToNetwork			= 0x03
		,	RouterBusyToNetwork				= 0x04
		,	RouterAvailableToNetwork		= 0x05
		,	InitializeRoutingTable			= 0x06
		,	InitializeRoutingTableAck		= 0x07
		,	EstablishConnectionToNetwork	= 0x08
		,	DisconnectConnectionToNetwork	= 0x09
		};

class BACnetNetClient;
typedef BACnetNetClient *BACnetNetClientPtr;

class BACnetNetServer;
typedef BACnetNetServer *BACnetNetServerPtr;

class BACnetNetClient {
		friend void Bind( BACnetNetClientPtr, BACnetNetServerPtr );
		friend void Unbind( BACnetNetClientPtr, BACnetNetServerPtr );
		friend bool IsBound( BACnetNetClientPtr, BACnetNetServerPtr );
		
	protected:
		BACnetNetServerPtr		clientPeer;
		
	public:
		BACnetNetClient( void );
		virtual ~BACnetNetClient( void );
		
		void Request( const BACnetNPDU &pdu );
		virtual void Confirmation( const BACnetNPDU &pdu ) = 0;
	};

class BACnetNetServer {
		friend void Bind( BACnetNetClientPtr, BACnetNetServerPtr );
		friend void Unbind( BACnetNetClientPtr, BACnetNetServerPtr );
		friend bool IsBound( BACnetNetClientPtr, BACnetNetServerPtr );
		
	protected:
		BACnetNetClientPtr		serverPeer;
		
	public:
		BACnetNetServer( void );
		virtual ~BACnetNetServer( void );
		
		virtual void Indication( const BACnetNPDU &pdu ) = 0;
		void Response( const BACnetNPDU &pdu );
	};

#if _TSMDebug
class BACnetDebugNPDU : public BACnetNetClient, public BACnetNetServer {
	protected:
		const char			*label;
		
	public:
		BACnetDebugNPDU( const char *lbl );
		BACnetDebugNPDU( BACnetNetServerPtr sp, const char *lbl );
		
		virtual void Indication( const BACnetNPDU &pdu );
		virtual void Confirmation( const BACnetNPDU &pdu );
	};
#endif

//
//	BACnetTask
//
//	A task is something that needs to be done.  There is a global list of all
//	of the tasks that need to be processed, the task manager takes care of making
//	sure all of the tasks get processed.
//
//	The taskTime is the time in clock ticks that the task should be processed.  If 
//	it's less than the current time (including zero) the task will fire.  An active
//	task is one that has been installed and will fire, NOT that a task is currently
//	running.
//
//	The taskType describes one three types of tasks: one-shot, one-shot and deleted, 
//	and recurring.  The taskInterval specifies how often a recurring task should 
//	execute.  If the taskInterval is zero for these recurring tasks, the task will 
//	be processed once each time the TaskManager function is called.
//
//	The suspend and resume methods are virtual so other functions can be tied to them
//	(like activating/deactivating a text field).  The ProcessTask function is pure 
//	virtual so useful objects must supply a method.
//
//	The TaskManager function is called by the main event loop.  It could be called 
//	by anybody that might tie up the machine for a while, like inside a TrackControl
//	loop.  The TaskControlProc is a glue routine that should be assigned to controls
//	where no other function would normally be provided.
//

class BACnetTask {
	public:
		enum BACnetTaskType
				{ oneShotTask		= 0
				, oneShotDeleteTask	= 1
				, recurringTask		= 2
				};

		BACnetTaskType	taskType;				// how to process
		long			taskInterval;			// how often to reschedule (ms)
		int				isActive;				// task is in queue to fire
		
		BACnetTask( BACnetTaskType typ = oneShotTask, long delay = 0 );
		virtual ~BACnetTask(void);
		
		void InstallTask(void);			 		// install into queue
		void SuspendTask(void);					// remove from execution queue
		void ResumeTask(void);					// put back in
		
		virtual void ProcessTask(void) = 0;		// do something
	};

typedef BACnetTask *BACnetTaskPtr;

//
//	BACnetTaskManager
//

class BACnetTaskManager {
	public:
		BACnetTaskManager( void ) {};
		virtual ~BACnetTaskManager( void ) {};

		virtual void InstallTask( BACnetTaskPtr tp ) = 0;
		virtual void SuspendTask( BACnetTaskPtr tp ) = 0;
		virtual void ResumeTask( BACnetTaskPtr tp ) = 0;
		
		virtual void ProcessTasks( void ) = 0;
	};

typedef BACnetTaskManager *BACnetTaskManagerPtr;

extern BACnetTaskManagerPtr gTaskManager;

//
//	BACnetPort
//
//	A port is a kind of server that is the lowest connecting point to 
//	the operating system.  Normally the req/conf interface is used, but 
//	if the application has a reason to send raw data out the port it 
//	can use this interface.  If it needs to track the raw data that was 
//	sent and received, it can override the FilterData member function.
//

class BACnetPort : public BACnetNetServer {
    public:
		enum BACnetPortDirection { portSending, portReceiving };
	
		BACnetPort( void );
		virtual ~BACnetPort( void );

		BACnetAddress			portLocalAddr;		// port has this address
		BACnetAddress			portBroadcastAddr;	// use this to broadcast (could be null)

		int			portStatus;						// non-zero iff something is wrong
		virtual void PortStatusChange( void );		// notification that something changed

		virtual void SendData( BACnetOctet *, int len ) = 0;
		virtual void FilterData( BACnetOctet *, int len, BACnetPortDirection dir );
	};

typedef BACnetPort *BACnetPortPtr;

//
//	BACnetAPDU
//

enum BACnetAPDUType
	{ confirmedRequestPDU		= 0x00
	, unconfirmedRequestPDU		= 0x01
	, simpleAckPDU				= 0x02
	, complexAckPDU				= 0x03
	, segmentAckPDU				= 0x04
	, errorPDU					= 0x05
	, rejectPDU					= 0x06
	, abortPDU					= 0x07
	};

class BACnetAPDU : public BACnetAPDUEncoder  {
	public:
		BACnetAddress		apduAddr;				// source/destination address
		BACnetAPDUType		apduType;
		bool				apduSeg;				// segmented
		bool				apduMor;				// more follows
		bool				apduSA;					// segmented response accepted
		bool				apduSrv;				// sent by server
		bool				apduNak;				// negative acknowledgement
		int					apduSeq;				// sequence number
		int					apduWin;				// actual/proposed window size
		int					apduMaxResp;			// max response accepted (decoded)
		int					apduService;
		int					apduInvokeID;
		int					apduAbortRejectReason;
		int					apduExpectingReply;		// see 6.2.2 (1 or 0)
		int					apduNetworkPriority;	// see 6.2.2 (0, 1, 2, or 3)

		BACnetAPDU( int initBuffSize = kDefaultBufferSize );
		BACnetAPDU( BACnetOctet *buffPtr, int buffLen = 0 );		// already have a buffer

		void Encode( BACnetAPDUEncoder& enc ) const;				// encode
		void Decode( const BACnetAPDUDecoder& dec );				// decode
	};

typedef BACnetAPDU *BACnetAPDUPtr;

//
//	BACnetConfirmedServiceAPDU
//

enum BACnetConfirmedServiceChoice {
// Alarm and Event Services
		acknowledgeAlarm				= 0,
		confirmedCOVNotification		= 1,
		confirmedEventNotification		= 2,
		getAlarmSummary					= 3,
		getEnrollmentSummary			= 4,
		subscribeCOV					= 5,

// File Access Services
		atomicReadFile					= 6,
		atomicWriteFile					= 7,

// Object Access Services
		addListElement					= 8,
		removeListElement				= 9,
		createObject					= 10,
		deleteObject					= 11,
		readProperty					= 12,
		readPropertyConditional			= 13,
		readPropertyMultiple			= 14,
		writeProperty					= 15,
		writePropertyMultiple			= 16,

// Remote Device Management Services
		deviceCommunicationControl		= 17,
		confirmedPrivateTransfer		= 18,
		confirmedTextMessage			= 19,
		reinitializeDevice				= 20,

// Virtual Terminal Services
		vtOpen							= 21,
		vtClose							= 22,
		vtData							= 23,

// Security Services
		authenticate					= 24,
		requestKey						= 25
		};

class BACnetConfirmedServiceAPDU : public BACnetAPDU {
	public:
		BACnetConfirmedServiceAPDU( BACnetConfirmedServiceChoice ch );
	};

//
//	BACnetUnconfirmedServiceAPDU
//

enum BACnetUnconfirmedServiceChoice {
		iAm								= 0,
		iHave							= 1,
		unconfirmedCOVNotification		= 2,
		unconfirmedEventNotification	= 3,
		unconfirmedPrivateTransfer		= 4,
		unconfirmedTextMessage			= 5,
		timeSynchronization				= 6,
		whoHas							= 7,
		whoIs							= 8
		};

class BACnetUnconfirmedServiceAPDU : public BACnetAPDU {
	public:
		BACnetUnconfirmedServiceAPDU( BACnetUnconfirmedServiceChoice ch );
	};

//
//	BACnetSimpleAckAPDU
//

class BACnetSimpleAckAPDU : public BACnetAPDU {
	public:
		BACnetSimpleAckAPDU( BACnetConfirmedServiceChoice ch, BACnetOctet invID );
	};

//
//	BACnetComplexAckAPDU
//

class BACnetComplexAckAPDU : public BACnetAPDU {
	public:
		BACnetComplexAckAPDU( BACnetConfirmedServiceChoice ch, BACnetOctet invID = 0 );
		
		void SetInvokeID( BACnetOctet id );
	};

//
//	BACnetSegmentAckAPDU
//

class BACnetSegmentAckAPDU : public BACnetAPDU {
	public:
		BACnetSegmentAckAPDU( const BACnetAddress &dest, BACnetOctet invID, int nak, int srv, BACnetOctet seg, BACnetOctet win );
	};

//
//	BACnetErrorAPDU
//

class BACnetErrorAPDU : public BACnetAPDU {
	public:
		BACnetErrorAPDU( BACnetConfirmedServiceChoice ch, BACnetOctet invID );
	};

//
//	BACnetRejectAPDU
//

enum BACnetRejectReason {
		otherReject						= 0,
		bufferOverflowReject			= 1,
		inconsistentParamtersReject		= 2,
		invalidParameterDataTypeReject	= 3,
		invalidTagReject				= 4,
		missingRequiredParameterReject	= 5,
		parameterOutOfRangeReject		= 6,
		tooManyArgumentsReject			= 7,
		undefinedEnumerationReject		= 8,
		unrecognizedService				= 9
		};

class BACnetRejectAPDU : public BACnetAPDU {
	public:
		BACnetRejectAPDU( BACnetOctet invID, BACnetRejectReason reason );
	};

//
//	BACnetAbortAPDU
//

enum BACnetAbortReason {
		otherAbort							= 0,
		bufferOverflowAbort					= 1,
		invalidAPDUInThisStateAbort			= 2,
		preemptedByHigherPriorityTaskAbort	= 3,
		segmentationNotSupportedAbort		= 4,
		
		// custom abort codes
		apduTimeoutAbort					= 63
		};

class BACnetAbortAPDU : public BACnetAPDU {
	public:
		BACnetAbortAPDU( const BACnetAddress &dest, int srv, BACnetOctet invID, BACnetAbortReason reason );
	};

//
//	BACnetAppClient
//	BACnetAppServer
//

class BACnetAppClient;
typedef BACnetAppClient *BACnetAppClientPtr;

class BACnetAppServer;
typedef BACnetAppServer *BACnetAppServerPtr;

class BACnetAppClient {
		friend void Bind( BACnetAppClientPtr, BACnetAppServerPtr );
		friend void Unbind( BACnetAppClientPtr, BACnetAppServerPtr );
		friend bool IsBound( BACnetAppClientPtr, BACnetAppServerPtr );
		
	protected:
		BACnetAppServerPtr		clientPeer;
		
	public:
		BACnetAppClient( void );
		virtual ~BACnetAppClient( void );
		
		void Request( const BACnetAPDU &pdu );
		virtual void Confirmation( const BACnetAPDU &pdu ) = 0;
	};

class BACnetAppServer {
		friend void Bind( BACnetAppClientPtr, BACnetAppServerPtr );
		friend void Unbind( BACnetAppClientPtr, BACnetAppServerPtr );
		friend bool IsBound( BACnetAppClientPtr, BACnetAppServerPtr );
		
	protected:
		BACnetAppClientPtr		serverPeer;
		
	public:
		BACnetAppServer( void );
		virtual ~BACnetAppServer( void );
		
		virtual void Indication( const BACnetAPDU &pdu ) = 0;
		void Response( const BACnetAPDU &pdu );
	};

//
//	BACnetDeviceInfo
//

enum BACnetSegmentation {
		segmentedBoth		= 0,
		segmentedTransmit	= 1,
		segmentedReceive	= 2,
		noSegmentation		= 3
		};

class BACnetDeviceInfo {
	public:
		BACnetObjectIdentifier	deviceID;
		BACnetAddress			deviceAddr;
		BACnetSegmentation		deviceSegmentation;			// supports segments requests
		int						deviceSegmentSize;			// how to divide up chunks
		int						deviceWindowSize;			// how many to send
		int						deviceMaxAPDUSize;			// maximum APDU size
		int						deviceNextInvokeID;			// next invoke ID for client
		int						deviceVendorID;				// which vendor is this
		
		BACnetDeviceInfo		*deviceNext;				// list of information blocks
	};

typedef BACnetDeviceInfo *BACnetDeviceInfoPtr;

//
//	BACnetDevice
//

class BACnetClient;
typedef BACnetClient *BACnetClientPtr;

class BACnetServer;
typedef BACnetServer *BACnetServerPtr;

class BACnetDevice : public BACnetNetClient, public BACnetDeviceInfo {
	public:
		int						deviceAPDUTimeout;			// how long to wait for ack
		int						deviceAPDUSegmentTimeout;	// how long to wait between segments
		int						deviceAPDURetries;			// how many retries are acceptable
		
		BACnetDevice( void );
		
		BACnetClientPtr			deviceClients;
		BACnetServerPtr			deviceServers;
		
		void Bind( BACnetClientPtr cp );
		void Unbind( BACnetClientPtr cp );
		void Bind( BACnetServerPtr sp );
		void Unbind( BACnetServerPtr sp );
		
		BACnetDeviceInfoPtr GetInfo( const BACnetObjectIdentifier &id );
		BACnetDeviceInfoPtr GetInfo( const BACnetAddress &addr );
		
		void Indication( const BACnetAPDU &apdu );		// outgoing packet
		void Confirmation( const BACnetNPDU &apdu );	// incoming packet
		
		int GetInvokeID( void );						// new invoke ID
	};

typedef BACnetDevice *BACnetDevicePtr;

BACnetOctet MaxAPDUEncode( int siz );
int MaxAPDUDecode( BACnetOctet siz );

//
//	BACnetAPDUSegment
//

class BACnetTSM;
typedef BACnetTSM *BACnetTSMPtr;

class BACnetAPDUSegment {
	public:
		BACnetTSMPtr			segTSMPtr;				// invoke ID, seg size, window stuff
		
		BACnetAPDU				segAPDU;				// returned PDU
		BACnetOctet				*segBuff;				// pointer to buffer
		int						segBuffSize;			// allocated size
		int						segLen;					// current length
		
		BACnetAPDUSegment( const BACnetAPDU &apdu, BACnetTSMPtr tp );
		BACnetAPDUSegment( int ssize, BACnetTSMPtr tp );
		~BACnetAPDUSegment( void );
		
		const BACnetAPDU &operator[](const int indx);	// get a segment
		int AddSegment( const BACnetAPDU &apdu );		// add on end
		
		const BACnetAPDU &ResultAPDU( void );			// return complete message
	};

typedef BACnetAPDUSegment *BACnetAPDUSegmentPtr;

//
//	BACnetTSM
//

enum BACnetTSMState
		{ tsmIdle
		, tsmSegmentedRequest
		, tsmAwaitConfirmation
		, tsmSegmentedConfirmation
		, tsmAwaitResponse
		, tsmSegmentedResponse
		, tsmBusy
		};

class BACnetTSM : public BACnetTask {
		friend class BACnetDevice;
		friend class BACnetAPDUSegment;
		
	protected:
		BACnetTSMState			tsmState;
		BACnetAPDUSegmentPtr	tsmSeg;
		
		BACnetAddress			tsmAddr;
		int						tsmInvokeID;
		
		int						tsmRetryCount;
		
		BACnetSegmentation		tsmSegmentation;
		int						tsmSegmentSize;				// how big are chunks
		int						tsmSegmentRetryCount;
		int						tsmSegmentCount;
		int						tsmSegmentsSent;
		
		int						tsmInitialSequenceNumber;	// first sequence number
		int						tsmLastSequenceNumber;		// last valid seq number received
		
		int						tsmActualWindowSize;
		int						tsmProposedWindowSize;
		
	public:
		BACnetDevicePtr			tsmDevice;					// bound device
		
		BACnetTSM( BACnetDevicePtr dp );
		virtual ~BACnetTSM( void );
		
		void StartTimer( int msecs );
		void StopTimer( void );
		void RestartTimer( int msecs );
		
		void FillWindow( int seqNum );
		int InWindow( int seqNum, int initSeqNum );
		
#if _TSMDebug
	protected:
		inline void SetState( BACnetTSMState newState )
		{
			cout << (unsigned long)this
				<< " from " << tsmState << " to " << newState
				<< endl;
			tsmState = newState;
		}
#else
	protected:
		inline void SetState( BACnetTSMState newState )
		{
			tsmState = newState;
		}
#endif
	};

//
//	BACnetClientTSM
//

class BACnetClientTSM : public BACnetTSM, public BACnetAppServer {
	public:
		BACnetClientTSM( BACnetDevicePtr dp );
		virtual ~BACnetClientTSM( void );
		
		void Request( const BACnetAPDU &pdu );			// msg to device
		
		void Indication( const BACnetAPDU &pdu );		// msg from client
		void Confirmation( const BACnetAPDU &pdu );		// msg from device
		
		void ProcessTask( void );
		
		void SegmentedRequest( const BACnetAPDU &apdu );
		void SegmentedRequestTimeout( void );
		void AwaitConfirmation( const BACnetAPDU &apdu );
		void AwaitConfirmationTimeout( void );
		void SegmentedConfirmation( const BACnetAPDU &apdu );
		void SegmentedConfirmationTimeout( void );
	};

//
//	BACnetServerTSM
//

class BACnetServerTSM : public BACnetTSM, public BACnetAppClient {
	public:
		BACnetServerTSM( BACnetDevicePtr dp );
		virtual ~BACnetServerTSM( void );
		
		void Response( const BACnetAPDU &apdu );		// msg to device
		
		void Indication( const BACnetAPDU &apdu );		// msg from device
		void Confirmation( const BACnetAPDU &apdu );	// msg from server
		
		void ProcessTask( void );
		
		void Idle( const BACnetAPDU &apdu );
		void SegmentedRequest( const BACnetAPDU &apdu );
		void SegmentedRequestTimeout( void );
		void AwaitResponse( const BACnetAPDU &apdu );
		void SegmentedResponse( const BACnetAPDU &apdu );
		void SegmentedResponseTimeout( void );
	};

//
//	BACnetClient
//

class BACnetClient : public BACnetAppClient {
		friend class BACnetDevice;
		
	private:
		BACnetClientTSM		clientTSM;
		BACnetClientPtr		clientNext;
		
	public:
		BACnetClient( BACnetDevicePtr dp );
		virtual ~BACnetClient( void );
	};

//
//	BACnetServer
//

class BACnetServer : public BACnetAppServer {
		friend class BACnetDevice;
		
	private:
		BACnetServerTSM		serverTSM;
		BACnetServerPtr		serverNext;
		
	public:
		BACnetServer( BACnetDevicePtr dp );
		virtual ~BACnetServer( void );
	};

#endif
// vinculum - a unifying bond, link, or tie