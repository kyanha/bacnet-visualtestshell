
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
const char * GetSocketErrorMsg( int nSocketError );

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

	unsigned char	localbroadcastAddr[4] = { 0xFF, 0xFF, 0xFF, 0xFF};
	char szHostName[128];
	if( gethostname(szHostName, 128) == 0 )
	{
		// Get host adresses
		struct hostent * pHost;
		pHost = gethostbyname(szHostName);
		LPSTR lpAddr = pHost->h_addr_list[0];
		if (lpAddr) 
			memcpy (&localbroadcastAddr, lpAddr, 3);
	}	//To get localbroadcast address
		//Modified by Zhu Zhenhua, 2003-12-12

	BACnetOctet	*msg
	;

	// check for nothing to send
	if (pdu.pduLen == 0)
		return;

	// allocate a buffer big enough for the address and data
	msg = new BACnetOctet[ pdu.pduLen + 6 ];

	// set up the socket address for sending
	if (pdu.pduAddr.addrType == localBroadcastAddr) {
		*(unsigned long *)msg = localbroadcastAddr[0]+localbroadcastAddr[1]*0x100+localbroadcastAddr[2]*0x10000+localbroadcastAddr[3]*0x1000000;
		*(unsigned short *)(msg+4) = m_Port;
	} else
	if (pdu.pduAddr.addrType == globalBroadcastAddr) {
		*(unsigned long *)msg = 0xFF+0xFF*0x100+0xFF*0x10000+0xFF*0x1000000;
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
						BACnetIPAddr( ntohl( *(unsigned long *)&saUdpClient.sin_addr), ntohs(saUdpClient.sin_port) )
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
	char	errMsg[2048];
	const char * msg;

	int stat = GetLastError();
	if (stat == 0)
		return;

	msg = GetSocketErrorMsg(stat);

	sprintf( errMsg, "%s() error %d, %s", func, stat, msg );
	AfxMessageBox( errMsg );
}



const char * GetSocketErrorMsg( int nSocketError )
{
	const char * msg = NULL;

	switch (nSocketError) {
		case WSAEINTR:
			msg = "Interrupted function call.\n"
					"A blocking operation was interrupted by a call to WSACancelBlockingCall.";
			break;
		case WSAEACCES:
			msg = "Permission denied.\n"
					"An attempt was made to access a socket in a way forbidden by its access "
					"permissions. An example is using a broadcast address for sendto without "
					"broadcast permission being set using setsockopt(SO_BROADCAST).\n"
					"Another possible reason for the WSAEACCES error is that when the bind "
					"function is called (on Windows NT 4 SP4 or later), another application, "
					"service, or kernel mode driver is bound to the same address with exclusive "
					"access. Such exclusive access is a new feature of Windows NT 4 SP4 and "
					"later, and is implemented by using the SO_EXCLUSIVEADDRUSE option.\n";
			break;
		case WSAEFAULT:
			msg = "Bad address.\n"
					"The system detected an invalid pointer address in attempting to use a "
					"pointer argument of a call. This error occurs if an application passes an "
					"invalid pointer value, or if the length of the buffer is too small. For "
					"instance, if the length of an argument, which is a SOCKADDR structure, is "
					"smaller than the sizeof(SOCKADDR).\n";
			break;
		case WSAEINVAL:
			msg = "Invalid argument.\n"
					"Some invalid argument was supplied (for example, specifying an invalid level "
					"to the setsockopt function). In some instances, it also refers to the current "
					"state of the socket for instance, calling accept on a socket that is not listening.\n";
			break;
		case WSAEMFILE:
			msg = "Too many open files.\n"
					"Too many open sockets. Each implementation may have a maximum number of socket "
					"handles available, either globally, per process, or per thread.\n";
			break;
		case WSAEWOULDBLOCK:
			msg = "Resource temporarily unavailable.\n"
					"This error is returned from operations on nonblocking sockets that cannot be "
					"completed immediately, for example recv when no data is queued to be read from "
					"the socket. It is a nonfatal error, and the operation should be retried later. "
					"It is normal for WSAEWOULDBLOCK to be reported as the result from calling "
					"connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the "
					"connection to be established.\n";
			break;
		case WSAEINPROGRESS:
			msg = "Operation now in progress.\n"
					"A blocking operation is currently executing. Windows Sockets only allows a single "
					"blocking operation per- task or thread to be outstanding, and if any other "
					"function call is made (whether or not it references that or any other socket) "
					"the function fails with the WSAEINPROGRESS error.\n";
			break;
		case WSAEALREADY:
			msg = "Operation already in progress.\n"
					"An operation was attempted on a nonblocking socket with an operation already in "
					"progress - that is, calling connect a second time on a nonblocking socket that "
					"is already connecting, or canceling an asynchronous request (WSAAsyncGetXbyY) "
					"that has already been canceled or completed.\n";
			break;
		case WSAENOTSOCK:
			msg = "Socket operation on nonsocket.\n"
					"An operation was attempted on something that is not a socket. Either the socket "
					"handle parameter did not reference a valid socket, or for select, a member of an "
					"fd_set was not valid.\n";
			break;
		case WSAEDESTADDRREQ:
			msg = "Destination address required.\n"
					"A required address was omitted from an operation on a socket. For example, this "
					"error is returned if sendto is called with the remote address of ADDR_ANY.\n";
			break;
		case WSAEMSGSIZE:
			msg = "Message too long.\n"
					"A message sent on a datagram socket was larger than the internal message buffer "
					"or some other network limit, or the buffer used to receive a datagram was smaller "
					"than the datagram itself.\n";
			break;
		case WSAEPROTOTYPE:
			msg = "Protocol wrong type for socket.\n"
					"A protocol was specified in the socket function call that does not support the "
					"semantics of the socket type requested. For example, the ARPA Internet UDP protocol "
					"cannot be specified with a socket type of SOCK_STREAM.\n";
			break;
		case WSAENOPROTOOPT:
			msg = "Bad protocol option.\n"
					"An unknown, invalid or unsupported option or level was specified in a getsockopt "
					"or setsockopt call.\n";
			break;
		case WSAEPROTONOSUPPORT:
			msg = "Protocol not supported.\n"
					"The requested protocol has not been configured into the system, or no "
					"implementation for it exists. For example, a socket call requests a SOCK_DGRAM "
					"socket, but specifies a stream protocol.\n";
			break;
		case WSAESOCKTNOSUPPORT:
			msg = "Socket type not supported.\n"
					"The support for the specified socket type does not exist in this address family. "
					"For example, the optional type SOCK_RAW might be selected in a socket call, and "
					"the implementation does not support SOCK_RAW sockets at all.\n";
			break;
		case WSAEOPNOTSUPP:
			msg = "Operation not supported.\n"
					"The attempted operation is not supported for the type of object referenced. Usually "
					"this occurs when a socket descriptor to a socket that cannot support this operation "
					"is trying to accept a connection on a datagram socket.\n";
		case WSAEPFNOSUPPORT:
			msg = "Protocol family not supported.\n"
					"The protocol family has not been configured into the system or no implementation for "
					"it exists. This message has a slightly different meaning from WSAEAFNOSUPPORT. "
					"However, it is interchangeable in most cases, and all Windows Sockets functions that "
					"return one of these messages also specify WSAEAFNOSUPPORT.\n";
			break;
		case WSAEAFNOSUPPORT:
			msg = "Address family not supported by protocol family.\n"
					"An address incompatible with the requested protocol was used. All sockets are created "
					"with an associated address family (that is, AF_INET for Internet Protocols) and a "
					"generic protocol type (that is, SOCK_STREAM). This error is returned if an incorrect "
					"protocol is explicitly requested in the socket call, or if an address of the wrong "
					"family is used for a socket, for example, in sendto.\n";
			break;
		case WSAEADDRINUSE:
			msg = "Address already in use.\n"
					"Typically, only one usage of each socket address (protocol/IP address/port) is "
					"permitted. This error occurs if an application attempts to bind a socket to an IP "
					"address/port that has already been used for an existing socket, or a socket that "
					"was not closed properly, or one that is still in the process of closing. For "
					"server applications that need to bind multiple sockets to the same port number, "
					"consider using setsockopt(SO_REUSEADDR). Client applications usually need not call "
					"bind at all - connect chooses an unused port automatically. When bind is called "
					"with a wildcard address (involving ADDR_ANY), a WSAEADDRINUSE error could be delayed "
					"until the specific address is committed. This could happen with a call to another "
					"function later, including connect, listen, WSAConnect, or WSAJoinLeaf.\n";
			break;
		case WSAEADDRNOTAVAIL:
			msg = "Cannot assign requested address.\n"
					"The requested address is not valid in its context. This normally results from an "
					"attempt to bind to an address that is not valid for the local machine. This can also "
					"result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote "
					"address or port is not valid for a remote machine (for example, address or port 0).\n";
			break;
		case WSAENETDOWN:
			msg = "Network is down.\n"
					"A socket operation encountered a dead network. This could indicate a serious failure "
					"of the network system (that is, the protocol stack that the Windows Sockets DLL runs "
					"over), the network interface, or the local network itself.\n";
			break;
		case WSAENETUNREACH:
			msg = "Network is unreachable.\n"
					"A socket operation was attempted to an unreachable network. This usually means the "
					"local software knows no route to reach the remote host.\n";
			break;
		case WSAENETRESET:
			msg = "Network dropped connection on reset.\n"
					"The connection has been broken due to keep-alive activity detecting a failure while "
					"the operation was in progress. It can also be returned by setsockopt if an attempt "
					"is made to set SO_KEEPALIVE on a connection that has already failed.\n";
			break;
		case WSAECONNABORTED:
			msg = "Software caused connection abort.\n"
					"An established connection was aborted by the software in your host machine, possibly "
					"due to a data transmission time-out or protocol error.\n";
			break;
		case WSAECONNRESET:
			msg = "Connection reset by peer.\n"
					"An existing connection was forcibly closed by the remote host. This normally results "
					"if the peer application on the remote host is suddenly stopped, the host is "
					"rebooted, or the remote host uses a hard close (see setsockopt for more information "
					"on the SO_LINGER option on the remote socket.) This error may also result if a "
					"connection was broken due to keep-alive activity detecting a failure while one or "
					"more operations are in progress. Operations that were in progress fail with "
					"WSAENETRESET. Subsequent operations fail with WSAECONNRESET.\n";
			break;
		case WSAENOBUFS:
			msg = "No buffer space available.\n"
					"An operation on a socket could not be performed because the system lacked sufficient "
					"buffer space or because a queue was full.\n";
			break;
		case WSAEISCONN:
			msg = "Socket is already connected.\n"
					"A connect request was made on an already-connected socket. Some implementations also "
					"return this error if sendto is called on a connected SOCK_DGRAM socket (for SOCK_STREAM "
					"sockets, the to parameter in sendto is ignored) although other implementations treat "
					"this as a legal occurrence.\n";
			break;
		case WSAENOTCONN:
			msg = "Socket is not connected.\n"
					"A request to send or receive data was disallowed because the socket is not connected and "
					"(when sending on a datagram socket using sendto) no address was supplied. Any other type "
					"of operation might also return this error - for example, setsockopt setting SO_KEEPALIVE "
					"if the connection has been reset.\n";
			break;
		case WSAESHUTDOWN:
			msg = "Cannot send after socket shutdown.\n"
					"A request to send or receive data was disallowed because the socket had already been "
					"shut down in that direction with a previous shutdown call. By calling shutdown a "
					"partial close of a socket is requested, which is a signal that sending or receiving, "
					"or both have been discontinued.\n";
			break;
		case WSAETIMEDOUT:
			msg = "Connection timed out.\n"
					"A connection attempt failed because the connected party did not properly respond after "
					"a period of time, or the established connection failed because the connected host has "
					"failed to respond.\n";
			break;
		case WSAECONNREFUSED:
			msg = "Connection refused.\n"
					"No connection could be made because the target machine actively refused it. This usually "
					"results from trying to connect to a service that is inactive on the foreign host - "
					"that is, one with no server application running.\n";
			break;
		case WSAEHOSTDOWN:
			msg = "Host is down.\n"
					"A socket operation failed because the destination host is down. A socket operation "
					"encountered a dead host. Networking activity on the local host has not been initiated. "
					"These conditions are more likely to be indicated by the error WSAETIMEDOUT.\n";
			break;
		case WSAEHOSTUNREACH:
			msg = "No route to host.\n"
					"A socket operation was attempted to an unreachable host. See WSAENETUNREACH.\n";
			break;
		case WSAEPROCLIM:
			msg = "Too many processes.\n"
					"A Windows Sockets implementation may have a limit on the number of applications that "
					"can use it simultaneously. WSAStartup may fail with this error if the limit has been "
					"reached.\n";
			break;
		case WSASYSNOTREADY:
			msg = "Network subsystem is unavailable.\n"
					"This error is returned by WSAStartup if the Windows Sockets implementation cannot "
					"function at this time because the underlying system it uses to provide network "
					"services is currently unavailable. Users should check:\n"
					"- That the appropriate Windows Sockets DLL file is in the current path.\n"
					"- That they are not trying to use more than one Windows Sockets implementation "
					"simultaneously. If there is more than one Winsock DLL on your system, be sure the "
					"first one in the path is appropriate for the network subsystem currently loaded.\n"
					"- The Windows Sockets implementation documentation to be sure all necessary components "
					"are currently installed and configured correctly.\n";
			break;
		case WSAVERNOTSUPPORTED:
			msg = "Winsock.dll version out of range.\n"
					"The current Windows Sockets implementation does not support the Windows Sockets "
					"specification version requested by the application. Check that no old Windows "
					"Sockets DLL files are being accessed.\n";
			break;
		case WSANOTINITIALISED:
			msg = "Successful WSAStartup not yet performed.\n"
					"Either the application has not called WSAStartup or WSAStartup failed. The "
					"application may be accessing a socket that the current active task does not "
					"own (that is, trying to share a socket between tasks), or WSACleanup has been "
					"called too many times.\n";
			break;
		case WSAEDISCON:
			msg = "Graceful shutdown in progress.\n"
					"Returned by WSARecv and WSARecvFrom to indicate that the remote party has "
					"initiated a graceful shutdown sequence.\n";
			break;
#if 0
		case WSATYPE_NOT_FOUND:
			msg = "Class type not found.\n"
					"The specified class was not found.\n";
			break;
#endif
		case WSAHOST_NOT_FOUND:
			msg = "Host not found.\n"
					"No such host is known. The name is not an official host name or alias, or it "
					"cannot be found in the database(s) being queried. This error may also be returned "
					"for protocol and service queries, and means that the specified name could not be "
					"found in the relevant database.\n";
			break;
		case WSATRY_AGAIN:
			msg = "Nonauthoritative host not found.\n"
					"This is usually a temporary error during host name resolution and means that the "
					"local server did not receive a response from an authoritative server. A retry at "
					"some time later may be successful.\n";
			break;
		case WSANO_RECOVERY:
			msg = "This is a nonrecoverable error.\n"
					"This indicates some sort of nonrecoverable error occurred during a database lookup. "
					"This may be because the database files (for example, BSD-compatible HOSTS, SERVICES, "
					"or PROTOCOLS files) could not be found, or a DNS request was returned by the "
					"server with a severe error.\n";
			break;
		case WSANO_DATA:
			msg = "Valid name, no data record of requested type.\n"
					"The requested name is valid and was found in the database, but it does not have the "
					"correct associated data being resolved for. The usual example for this is a host "
					"name-to-address translation attempt (using gethostbyname or WSAAsyncGetHostByName) "
					"which uses the DNS (Domain Name Server). An MX record is returned but no A record "
					"- indicating the host itself exists, but is not directly reachable.\n";
			break;
#if 0
		case WSA_INVALID_HANDLE:
			msg = "Specified event object handle is invalid.\n"
					"An application attempts to use an event object, but the specified handle is not valid.\n";
			break;
		case WSA_INVALID_PARAMETER:
			msg = "One or more parameters are invalid.\n"
					"An application used a Windows Sockets function which directly maps to a Win32 function. "
					"The Win32 function is indicating a problem with one or more parameters.\n";
			break;
		case WSA_IO_INCOMPLETE:
			msg = "Overlapped I/O event object not in signaled state.\n"
					"The application has tried to determine the status of an overlapped operation which "
					"is not yet completed. Applications that use WSAGetOverlappedResult (with the fWait "
					"flag set to FALSE) in a polling mode to determine when an overlapped operation has "
					"completed, get this error code until the operation is complete.\n";
			break;
		case WSA_IO_PENDING:
			msg = "Overlapped operations will complete later.\n"
					"The application has initiated an overlapped operation that cannot be completed "
					"immediately. A completion indication will be given later when the operation has "
					"been completed.\n";
			break;
		case WSA_NOT_ENOUGH_MEMORY:
			msg = "Insufficient memory available.\n"
					"An application used a Windows Sockets function that directly maps to a Win32 "
					"function. The Win32 function is indicating a lack of required memory resources.\n";
			break;
		case WSA_OPERATION_ABORTED:
			msg = "Overlapped operation aborted.\n"
					"An overlapped operation was canceled due to the closure of the socket, or the "
					"execution of the SIO_FLUSH command in WSAIoctl.\n";
			break;
		case WSAINVALIDPROCTABLE:
			msg = "Invalid procedure table from service provider.\n"
					"A service provider returned a bogus procedure table to Ws2_32.dll. (Usually "
					"caused by one or more of the function pointers being null.)\n";
			break;
		case WSAINVALIDPROVIDER:
			msg = "Invalid service provider version number.\n"
					"A service provider returned a version number other than 2.0.\n";
			break;
		case WSAPROVIDERFAILEDINIT:
			msg = "Unable to initialize a service provider.\n"
					"Either a service provider's DLL could not be loaded (LoadLibrary failed) or the "
					"provider's WSPStartup/NSPStartup function failed.\n";
			break;
		case WSASYSCALLFAILURE:
			msg = "System call failure.\n"
					"Returned when a system call that should never fail does. For example, if a call "
					"to WaitForMultipleObjects fails or one of the registry functions fails trying to "
					"manipulate the protocol/name space catalogs.\n";
			break;
#endif
		default:
			msg = "unknown error";
			break;
	}

	return msg;
}
