#ifndef _WinIP
#define _WinIP

#include <winsock.h>

#include "BACnet.hpp"

//
//	WinIP
//

const u_short	kDefaultPort = 0xBAC0;
const u_short	kDynamicPort = 0x0000;

class WinIP : public BACnetPort {
		friend UINT WinIPThreadFunc( LPVOID pParam );

	protected:
		short					m_Port;
		bool					m_Continue;
		SOCKET					sock;						// the real socket
		CWinThread				*m_Thread;
		int						m_ReceiveError;
		
		void InitPort( short socket );

	public:
		static void StringToHostPort( char *str, u_long *hostp, u_long *maskp, u_short *portp );
		
		WinIP( short port = kDefaultPort );
		WinIP( const char *port );
		~WinIP( void );

		void Indication( const BACnetNPDU &pdu );
		void SendData( BACnetOctet *data, int len );		// raw data request
	};

typedef WinIP *WinIPPtr;

#endif
