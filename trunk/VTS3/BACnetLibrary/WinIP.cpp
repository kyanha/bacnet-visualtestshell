
#include "stdafx.h"

#include "BACnetIP.hpp"
#include "WinIP.hpp"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT WinIPThreadFunc( LPVOID pParam );
void CheckSocketError( char *func );

//
//	WinIP::WinIP
//

WinIP::WinIP( short port )
	: m_ReceiveError(0)
{
	InitPort( port );
}

//
//	WinIP::WinIP
//

WinIP::WinIP( const char *port )
	: m_ReceiveError(0)
{
	int		socket
	;

	if ((port[0] == '0') && (toupper(port[1]) == 'X')) {
		port += 2;
		for (socket = 0; isxdigit(*port); port++)
			socket = (socket << 4) + (isdigit(*port) ? (*port - '0') : (toupper(*port) - 'A' + 10));
	} else
		socket = atoi( port );

	InitPort( socket );
}

//
//	WinIP::InitPort
//

void WinIP::InitPort( short port )
{
	int				err
	;
	char			sName[256]
	;
	HOSTENT			*pHostInfo
	;
	BOOL			fBroadcast = TRUE
	; 
	SOCKADDR_IN		saUdpServer
	;
	SOCKADDR		saName
	;
	int				saNameLen = sizeof(saName)
	;

	// get the host name
	err = gethostname( sName, sizeof(sName) );
	if (err == SOCKET_ERROR) {
		CheckSocketError( "gethostname" );
		portStatus = -1;
		return;
	}

	// get the host information
	pHostInfo = gethostbyname( sName );
	if (!pHostInfo) {
		CheckSocketError( "gethostbyname" );
		portStatus = -1;
		return;
	}

	// initialize the socket descriptor
	sock = socket ( AF_INET, SOCK_DGRAM, 0 );
    if (sock == INVALID_SOCKET) {
		CheckSocketError( "socket" );
		portStatus = -1;
		return;
	}

	// bind to the port
	saUdpServer.sin_family = AF_INET;
	saUdpServer.sin_addr.s_addr = htonl( INADDR_ANY );
	saUdpServer.sin_port = htons( port );
	
	err = bind( sock, (SOCKADDR FAR *)&saUdpServer, sizeof(SOCKADDR_IN) );
	if (err == SOCKET_ERROR) {
		CheckSocketError( "bind" );
		portStatus = -1;
		return;
	}

	// get the name
	err = getsockname( sock, &saName, &saNameLen );
	if (err == SOCKET_ERROR) {
		CheckSocketError( "getsockname" );
		portStatus = -1;
		return;
	}

	// get a copy of the port returned from getsockname.  If the request was for a 
	// dynamic port, this is the port that was actually bound.  In theory this 
	// information may not be available until after there is IO on the socket, who 
	// the hell knows?
	m_Port = *(unsigned short *)saName.sa_data;

	// pack the host address and port into the portLocalAddr
	portLocalAddr.addrType = localStationAddr;
	portLocalAddr.addrLen = sizeof(unsigned long) + sizeof(unsigned short);
	*(unsigned long *)portLocalAddr.addrAddr = *(unsigned long *)*pHostInfo->h_addr_list;
	*(unsigned short *)(portLocalAddr.addrAddr + 4) = m_Port;
	
	// enable broadcast
	err = setsockopt( sock, SOL_SOCKET, SO_BROADCAST, (CHAR *)&fBroadcast, sizeof(BOOL) ); 
	if (err == SOCKET_ERROR)
		CheckSocketError( "setsockopt" );

	// pack the host address and port into the portBroadcastAddr
	portBroadcastAddr.addrType = localStationAddr;
	portBroadcastAddr.addrLen = sizeof(unsigned long) + sizeof(unsigned short);
	*(unsigned long *)portBroadcastAddr.addrAddr = 0xFFFFFFFF;
	*(unsigned short *)(portBroadcastAddr.addrAddr + 4) = m_Port;
	
	// allow it to run once it gets started
	m_Continue = true;

	// start it suspended
	m_Thread =
		AfxBeginThread( WinIPThreadFunc, this
			, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED
			);

	// dont let windows delete the thread object
	m_Thread->m_bAutoDelete = FALSE;

	// now let it run
	m_Thread->ResumeThread();

	// clear the port status, all is well
	portStatus = 0;
}

//
//	WinIP::~WinIP
//

WinIP::~WinIP()
{
	int		loopCount
	;
	DWORD	dwExitCode
	;

	// check for a startup error
	if (portStatus != 0)
		return;

	// see if our thread is still around
	::GetExitCodeThread( m_Thread->m_hThread, &dwExitCode );
	if (dwExitCode == STILL_ACTIVE) {
		// the thread is still running
		m_Continue = false;
		
		// closing the socket forces the recvfrom() to fail
		closesocket( sock );
		
		// wait for it to go away
		for (loopCount = 0; loopCount < 5; loopCount++)
			if (::WaitForSingleObject( m_Thread->m_hThread, 1000 ) == WAIT_OBJECT_0)
				break;
		if (loopCount == 5) {
			AfxMessageBox( "WARNING: terminating WinIP thread" );
			::TerminateThread( m_Thread->m_hThread, 0 );
		}
	} else {
		// close the socket, thread left it open
		closesocket( sock );
	}
	
	// The thread has terminated. Delete the CWinThread object.
	delete m_Thread;
}

//
//	WinIP::Indication
//
//	This function is called when the client wants to send some data out the
//	port.  The address must be a localStation or a localBroadcast.
//

void WinIP::Indication( const BACnetNPDU &pdu )
{
	BACnetOctet	*msg
	;

	// check for nothing to send
	if (pdu.pduLen == 0)
		return;

	// allocate a buffer big enough for the address and data
	msg = new BACnetOctet[ pdu.pduLen + 6 ];

	// set up the socket address for sending
	if (pdu.pduAddr.addrType == localBroadcastAddr) {
		*(unsigned long *)msg = htonl( INADDR_BROADCAST );
		*(unsigned short *)(msg+4) = m_Port;
	} else
	if (pdu.pduAddr.addrType == localStationAddr) {
		memcpy( msg, pdu.pduAddr.addrAddr, 6 );
	} else {
		delete[] msg;
		return;
	}

	memcpy( msg + 6, pdu.pduData, pdu.pduLen );
	SendData( msg, pdu.pduLen + 6 );

	delete[] msg;
}

//
//	WinIP::SendData
//
//	This function is called with the data to send out the port.
//

void WinIP::SendData( BACnetOctet *data, int len )
{
	int			err
	;
	SOCKADDR_IN	saUdpServer
	;

	// let the application filter it
	FilterData( data, len, portSending );

	// check for nothing to send
	if (len <= 6)
		return;

	// set up the socket address for sending
	saUdpServer.sin_family = AF_INET;
	saUdpServer.sin_addr.s_addr = *(unsigned long *)data;
	saUdpServer.sin_port = *(unsigned short *)(data + 4); 

	// send it out
	err = sendto( sock, (const char *)(data + 6), len - 6, 0
		, (SOCKADDR *)&saUdpServer, sizeof(SOCKADDR_IN)
		); 
	if (err == SOCKET_ERROR)
		CheckSocketError( "sendto" );
}

//
//	WinIPThreadFunc
//

#define MAX_MSGLEN	65536

UINT WinIPThreadFunc( LPVOID pParam )
{
	int				nBytes, clientSize
	;
	WinIPPtr		pServer = (WinIPPtr)pParam
	;
	SOCKADDR_IN		saUdpClient
	;
	char			achBuffer[MAX_MSGLEN]
	;
	BACnetOctet		*msg, *dst
	;

	pServer->m_ReceiveError = 0;
	while (pServer->m_Continue) {
		// receive a datagram on the bound port
		clientSize = sizeof( SOCKADDR_IN ); 
		nBytes = recvfrom( pServer->sock, achBuffer, MAX_MSGLEN, 0
			, (SOCKADDR FAR *)&saUdpClient, &clientSize
			);
		if (nBytes == SOCKET_ERROR) {
			pServer->m_ReceiveError = GetLastError();
			break;
		} else {
			// allocate a buffer just for the filter
			msg = dst = new BACnetOctet[ nBytes + 6 ];

			// copy in the source address and data
			*(unsigned long *)dst = *(unsigned long *)&saUdpClient.sin_addr;
			dst += 4;
			*(unsigned short *)dst = saUdpClient.sin_port;
			dst += 2;
			memcpy( dst, achBuffer, nBytes );

			// let the application filter it
			pServer->FilterData( (unsigned char *)msg, nBytes + 6, BACnetPort::portReceiving );

			// toss the buffer
			delete[] msg;

			// pass the contents up to the client
			pServer->Response(
				BACnetNPDU(
						BACnetIPAddr( *(unsigned long *)&saUdpClient.sin_addr, saUdpClient.sin_port )
					,	(unsigned char *)achBuffer, nBytes
					)
				);
		}
	}

	// finished
    return 0;
}

//
//	CheckSocketError
//

void CheckSocketError( char *func )
{
	char	errMsg[100], *msg
	;

	int stat = GetLastError();
	if (stat == 0)
		return;
	
	switch (stat) {
		case WSANOTINITIALISED:
			msg = "not inialized";
			break;
		case WSAENETDOWN:
			msg = "network down";
			break;
		case WSAEACCES:
			msg = "access";
			break;
		case WSAEINPROGRESS:
			msg = "in progress";
			break;
		case WSAEFAULT:
			msg = "fault";
			break;
		case WSAEINVAL :
			msg = "host name invalid";
			break;
		case WSAENETRESET:
			msg = "network reset";
			break;
		case WSAENOBUFS:
			msg = "no buffers";
			break;
		case WSAENOTCONN:
			msg = "not connected";
			break;
		case WSAENOTSOCK:
			msg = "not a socket";
			break;
		case WSAEOPNOTSUPP:
			msg = "option not supported";
			break;
		case WSAESHUTDOWN:
			msg = "shutdown";
			break;
		case WSAEWOULDBLOCK:
			msg = "would block";
			break;
		case WSAEMSGSIZE:
			msg = "message size (too big)";
			break;
		case WSAECONNABORTED:
			msg = "connection aborted";
			break;
		case WSAECONNRESET:
			msg = "connection reset";
			break;
		case WSAEADDRNOTAVAIL:
			msg = "address not available";
			break;
		case WSAEAFNOSUPPORT:
			msg = "address family no support";
			break;
		case WSAEDESTADDRREQ:
			msg = "destination address required";
			break;
		case WSAENETUNREACH:
			msg = "network unreachable";
			break;
		default:
			msg = "unknown error";
			break;
	}

	sprintf( errMsg, "%s() error %d, %s", func, stat, msg );
	AfxMessageBox( errMsg );
}
