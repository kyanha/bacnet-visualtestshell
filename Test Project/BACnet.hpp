#ifndef _BACnet
#define _BACnet

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
		void Encode( char *enc, const char **table = 0, int tsize = 0 );
		void Decode( const char *dec, const char **table = 0, int tsize = 0 );
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
		int				strLen;			// number of octets
		BACnetOctet		*strBuff;		// pointer to data
				
		BACnetCharacterString( char *svalu = 0 );
		~BACnetCharacterString( void );
		
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
		~BACnetOctetString( void );
		
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
		~BACnetBitString( void );
		
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

class BACnetObjectIdentifier : public BACnetEncodeable {
	public:
		unsigned int	objID;
		
		BACnetObjectIdentifier( int objType = 0, int instanceNum = 0 );
		
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
//	BACnetAPDUEncoder
//
//	An encoder is an object passed to the various types to store the encoded version 
//	of themselves.  The correct way to handle encoding is to build a buffer of data 
//	in your routine and pass it to a method that will copy it into the correct place 
//	in the buffer and update the length.  With a nod to speed, it's faster to stuff 
//	the data right in the buffer (after checking to make sure there's enough space 
//	of course!).
//

const int kDefaultBufferSize = 1024;

class BACnetAPDUEncoder {
	protected:
		int					pktBuffSize;			// allocated size
		
	public:
		BACnetAPDUEncoder( BACnetOctet *buffPtr );
		BACnetAPDUEncoder( int initBuffSize = kDefaultBufferSize );
		~BACnetAPDUEncoder( void );
		
		BACnetOctet			*pktBuffer;				// pointer to start of buffer
		int					pktLength;				// number of encoded octets
		
		void CheckSpace( int len );						// resize iff necessary
		void CopyOctets( BACnetOctet *buff, int len );	// raw copy into buffer
		void Flush( void );								// remove all contents
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
		
		BACnetAPDUDecoder( const BACnetOctet *buffer, int len );
		BACnetAPDUDecoder( const BACnetAPDUEncoder &enc );
		
		void SetBuffer( const BACnetOctet *buffer, int len );

		void ExamineTag(BACnetAPDUTag &t);			// just peek at the next tag
		
		void CopyOctets( BACnetOctet *buff, int len );	// raw copy into buffer
		int ExtractData( BACnetOctet *buffer );		// skip the tag and extract the data
	};

typedef BACnetAPDUDecoder *BACnetAPDUDecoderPtr;
const int kBACnetAPDUDecoderSize = sizeof( BACnetAPDUDecoder );

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
		
	protected:
		BACnetNetClientPtr		serverPeer;
		
	public:
		BACnetNetServer( void );
		virtual ~BACnetNetServer( void );
		
		virtual void Indication( const BACnetNPDU &pdu ) = 0;
		void Response( const BACnetNPDU &pdu );
	};

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
//	A BACnetAPDU is an object that gets passed between the 'top' of a router and an application 
//	client.  The apduExpectingReply and apduNetworkPriority are passed down to the network layer 
//	for proper processing.  The first octet of the service data will contain the pduType in the 
//	upper nibble, it is available in the header to make processing easier.
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

struct BACnetAPDU {
	BACnetAddress		apduAddr;
	BACnetAPDUType		apduType;
	int					apduInvokeID;
	int					apduAbortRejectReason;
	BACnetAPDUDecoder	apduData;
	int					apduPriority;

	BACnetAPDU( const BACnetAddress &addr, const BACnetAPDUType type
		, const int invokeID = 0, const int abortRejectReason = 0
		, const BACnetOctet *data = 0, const int len = 0
		, const int priority = 0
		);
	BACnetAPDU( const BACnetAddress &addr, const BACnetAPDUType type
		, const int invokeID, const BACnetAPDUDecoder &data
		, const int priority = 0
		);
	};

typedef BACnetAPDU *BACnetAPDUPtr;

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
		
	protected:
		BACnetAppClientPtr		serverPeer;
		
	public:
		BACnetAppServer( void );
		virtual ~BACnetAppServer( void );
		
		virtual void Indication( const BACnetAPDU &pdu ) = 0;
		void Response( const BACnetAPDU &pdu );
	};

#endif
