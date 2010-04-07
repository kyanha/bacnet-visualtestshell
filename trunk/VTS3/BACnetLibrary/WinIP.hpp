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
		u_short					m_udpPort;
		bool					m_Continue;
		SOCKET					m_socket;						// the real socket
		CWinThread				*m_Thread;
		int						m_ReceiveError;
		
		void InitPort( const char *pInterface, u_short udpPort );

	public:
		// Fill a combo with the list of available IP interfaces
		static void FillInterfaceCombo( CComboBox &theCombo );
		
		WinIP( const char *pInterface, u_short udpPort );
		WinIP( const char *pPortAndInterface );
		~WinIP( void );

		void Indication( const BACnetNPDU &pdu );
		void SendData( BACnetOctet *data, int len );		// raw data request
	};

typedef WinIP *WinIPPtr;

#endif
