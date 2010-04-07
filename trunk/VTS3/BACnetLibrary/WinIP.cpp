
#include "stdafx.h"

#include "BACnetIP.hpp"
#include "WinIP.hpp"

#include "Iphlpapi.h"	// for adapter enumeration

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT WinIPThreadFunc( LPVOID pParam );
void CheckSocketError( char *func );
const char * GetSocketErrorMsg( int nSocketError );

//=============================================================================
// Class to enumerate available IP adapters
class WinIP_EnumerateAdapters
{
public:
	IP_ADAPTER_INFO *m_pInfo;
	
	WinIP_EnumerateAdapters()
	: m_pInfo( NULL )
	{
	}

	~WinIP_EnumerateAdapters()
	{
		if (m_pInfo)
			free(m_pInfo);
	}

	// Get a list of available interfaces
	bool Enumerate()
	{
		if (m_pInfo == NULL)
		{
			// Allow several attempts if initial buffer is too small
			ULONG ret;
			ULONG size = 16384;
			for (int ix = 0; ix < 3; ix++)
			{
				m_pInfo = (IP_ADAPTER_INFO*)malloc(size);
		        ret = GetAdaptersInfo( m_pInfo, &size );
				if (ret != ERROR_BUFFER_OVERFLOW)
				{
					break;
				}
				free( m_pInfo );
				m_pInfo = NULL;
			}
			
			if (ret != ERROR_SUCCESS)
			{
				CString str;
				str.Format( "Adapter enumeration failed: %s", GetSocketErrorMsg(ret) );
				AfxMessageBox( str );
				if (m_pInfo) {
					free(m_pInfo);
					m_pInfo = NULL;
				}
			}
		}

		return (m_pInfo != NULL);
	}

	// Fill a ComboBoz with the available interfaces
	void FillCombo( CComboBox &theCombo )
	{
		theCombo.ResetContent();
		if (Enumerate())
		{
			IP_ADAPTER_INFO *pCur = m_pInfo;
			while (pCur)
			{
				// We find that
                // Name = {45355057-0882-430A-A542-99EA72F36353} 
				// Description = Intel(R) 82567LM Gigabit Network Connection - Packet Scheduler Miniport 
				// FriendlyName = Local Area Connection
				// So "Description" is the best to show a human
				if (strcmp(pCur->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
				{
					// IP address looks nice in the combo, but we DON'T want it in the saved configuration
					// string.
//					CString str;
//					str.Format( "%s  %s", pCur->IpAddressList.IpAddress.String, pCur->Description );
					theCombo.AddString( pCur->Description );
				}
				pCur = pCur->Next;
			}
		}
	}

	// Get the addresses best matching the specified adapter
	bool GetAddresses( const char	*pTheAdapter, 
					   in_addr		&theNodeAddress, 
					   in_addr		&theBroadcastAddress )
	{
		bool retval = false;
		if (Enumerate())
		{
			IP_ADAPTER_INFO *pCur = m_pInfo;
			IP_ADAPTER_INFO *pDefault = NULL;
			while (pCur)
			{
				if (strcmp(pCur->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
				{
					// Adapter has an address
					if (pDefault == NULL)
					{
						// Remember this as the default adapter
						pDefault = pCur;
					}

					if (strcmp(pCur->Description, pTheAdapter) == 0)
					{
						// Found
						break;
					}
				}

				pCur = pCur->Next;
			}

			if (pCur == NULL)
			{
				CString str;
				if (pDefault == NULL)
				{
					str.Format( "No IP adapters are available" );
					AfxMessageBox( str );
				}
				else if (*pTheAdapter != 0)
				{
					str.Format( "Selected IP adapter \"%s\" is not available.\n"
								"Using adapter \"%s\"", pTheAdapter, pDefault->Description );
					pCur = pDefault;
					AfxMessageBox( str );
				}
				else
				{
					// No adapter selected, as from an old-style configuration.
					// Just quietly use the default
					pCur = pDefault;
				}
			}

			if (pCur != NULL)
			{
				theNodeAddress.S_un.S_addr = inet_addr( pCur->IpAddressList.IpAddress.String );
				theBroadcastAddress.S_un.S_addr =
					theNodeAddress.S_un.S_addr | ~inet_addr( pCur->IpAddressList.IpMask.String );

				retval = true;
			}
		}

		return retval;
	}
};

//=============================================================================
// Fill a combo with the list of available IP interfaces
// static 
void WinIP::FillInterfaceCombo( CComboBox &theCombo )
{
	WinIP_EnumerateAdapters adapters;
	adapters.FillCombo( theCombo );
}

//=============================================================================
//
//	WinIP::WinIP
//

WinIP::WinIP( const char *pInterface, u_short udpPort )
	: m_ReceiveError(0)
{
	InitPort( pInterface, udpPort );
}

//
//	WinIP::WinIP
//

WinIP::WinIP( const char *pPortAndInterface )
	: m_ReceiveError(0)
{
	unsigned int udpPort;
	char *pOut;
	udpPort = strtoul( pPortAndInterface, &pOut, 0 );	// handles decimal, or hex with 0x prefix
	pPortAndInterface = pOut;

	// Ideally, we would separate IP-interface and port with a comma.  But that would
	// affect backward compatibility with settings stored in versions of VTS
	// before we introduced IP-interface.
	// Thus, we put port first, and optional IP-interface after it, separated
	// by a semi-colon.  
	// TODO: we get 47808,3 etc.
	// - How do we cope with a comma in an adapter name?
	// - Probably substitute '?' in the dialog
	// - Parse here to the comma
	if (*pPortAndInterface == ';')
		pPortAndInterface += 1;
	int len = strcspn( pPortAndInterface, "," );
	CString iface( pPortAndInterface, len );

	// Empty string selects the first available interface.
	InitPort( iface, udpPort );
}


//
//	WinIP::InitPort
//
void WinIP::InitPort( const char *pInterface, u_short udpPort )
{
	// Until we are happy
	portStatus = -1;

	// initialize the socket descriptor
	m_socket = socket( AF_INET, SOCK_DGRAM, 0 );
    if (m_socket == INVALID_SOCKET) {
		CheckSocketError( "socket" );
		return;
	}

	// http://blogs.msdn.com/zhengpei/Default.aspx?p=2 has info on
	// Windows multi-home operation.  Says
	// - Windows implements "weak end model": an address refers to a host,
	//   not an interface.
	// - bind to IPADDR_ANY will "pick" an address.
	//   - outgoing packets will be routed based on IP address
	//   - incoming packets that match ANY interface will be accepped
	// Thus, we prefer to bind to the address of a particular interface

	WinIP_EnumerateAdapters adapters;
	in_addr	nodeAddress, broadcastAddress;
	adapters.GetAddresses( pInterface, nodeAddress, broadcastAddress );

	// Bind to the adapter
	SOCKADDR_IN  saUdpServer;
	saUdpServer.sin_family = AF_INET;
	saUdpServer.sin_addr = nodeAddress;
	saUdpServer.sin_port = htons( udpPort );
	int err = bind( m_socket, (SOCKADDR*)&saUdpServer, sizeof(saUdpServer) );
	if (err == SOCKET_ERROR) {
		CheckSocketError( "bind" );
		return;
	}

	// Get the UDP port.  If the request was for a 
	// dynamic port, this is the port that was actually bound.  In theory this 
	// information may not be available until after there is IO on the socket, who 
	// the hell knows?
	SOCKADDR  saName;
	int		  saNameLen = sizeof(saName);
	err = getsockname( m_socket, &saName, &saNameLen );
	if (err == SOCKET_ERROR) {
		CheckSocketError( "getsockname" );
		return;
	}
	m_udpPort = *(unsigned short *)saName.sa_data;

	// Pack the host address and port into the portLocalAddr
	portLocalAddr.addrType = localStationAddr;
	portLocalAddr.addrLen = sizeof(unsigned long) + sizeof(unsigned short);
	memcpy( portLocalAddr.addrAddr, &nodeAddress.S_un, 4 );
	*(unsigned short *)(portLocalAddr.addrAddr + 4) = m_udpPort;
	
	// Get the broadcast address.
	portBroadcastAddr.addrType = localStationAddr;
	portBroadcastAddr.addrLen = sizeof(unsigned long) + sizeof(unsigned short);
	memcpy( portBroadcastAddr.addrAddr, &broadcastAddress.S_un, 4 );
	*(unsigned short *)(portBroadcastAddr.addrAddr + 4) = m_udpPort;
	
	// Enable broadcast
	BOOL fBroadcast = TRUE; 
	err = setsockopt( m_socket, SOL_SOCKET, SO_BROADCAST, (CHAR *)&fBroadcast, sizeof(fBroadcast) ); 
	if (err == SOCKET_ERROR)
		CheckSocketError( "setsockopt" );

	// Allow it to run once it gets started
	m_Continue = true;

	// Start it suspended
	m_Thread =
		AfxBeginThread( WinIPThreadFunc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );

	// Don't let windows delete the thread object
	m_Thread->m_bAutoDelete = FALSE;

	// Let it run
	m_Thread->ResumeThread();

	// Clear the port status, all is well
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
		closesocket( m_socket );
		
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
		closesocket( m_socket );
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
	BACnetOctet	*msg;

	// check for nothing to send
	if (pdu.pduLen == 0)
		return;

	// allocate a buffer big enough for the address and data
	msg = new BACnetOctet[ pdu.pduLen + 6 ];

	// set up the socket address for sending
	if (pdu.pduAddr.addrType == localBroadcastAddr) {
		memcpy( msg, portBroadcastAddr.addrAddr, 4 );
		*(unsigned short *)(msg+4) = m_udpPort;
	} else
	if (pdu.pduAddr.addrType == globalBroadcastAddr) {
		*(unsigned long *)msg = 0xFFFFFFFF;
		*(unsigned short *)(msg+4) = m_udpPort;
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
	err = sendto( m_socket, (const char *)(data + 6), len - 6, 0
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
		nBytes = recvfrom( pServer->m_socket, achBuffer, MAX_MSGLEN, 0
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

// Also called for MS/TP
const char* GetSocketErrorMsg( int nSocketError )
{
	// We assume this is called only from the UI thread so a static buffer is OK
	static CString s_buffer;

	LPTSTR pResult = NULL;
	DWORD retval = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								  FORMAT_MESSAGE_FROM_SYSTEM |
								  FORMAT_MESSAGE_IGNORE_INSERTS,
								  NULL,
								  nSocketError,
								  0,				// Any language
								  (LPTSTR)&pResult,	// gets OS-allocated buffer
								  0,				// no minimum size
								  NULL );
	s_buffer = pResult;
	LocalFree( pResult );
	return (LPCTSTR)s_buffer;
}
