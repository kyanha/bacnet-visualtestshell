
#ifndef _BACnetRouter
#define _BACnetRouter

//
//	BACnet Router
//

class BACnetRouter;
typedef BACnetRouter *BACnetRouterPtr;

class BACnetRouterAdapter;
typedef BACnetRouterAdapter *BACnetRouterAdapterPtr;

//
//	BACnetRouterAdapter
//

class BACnetRouterAdapter : public BACnetNetClient {
	public:
		int					adapterNet;
		BACnetRouterPtr		adapterRouter;
		
		BACnetRouterAdapter( int net, BACnetRouterPtr router )
			: adapterNet(net), adapterRouter(router)
		{
		}
		
		virtual void Confirmation( const BACnetNPDU &pdu );
	};

//
//	BACnetRouterList
//

struct BACnetRouterList {
	int						routerNet;			// -1 == local network
	BACnetAddress			routerAddr;			// LAN specific address of router
	BACnetRouterAdapterPtr	routerAdapter;		// which adapter to use
	};

//
//	BACnetRouter
//

const int kBACnetRouterMaxAdapters = 8;			// how many connected adapters
const int kBACnetRouterMaxRouterListLen = 50;	// how many networks
const int kBACnetRouterLocalNetwork = -1;		// local network, pass to BindToNetwork

class BACnetRouter : public BACnetNetServer {
		friend class BACnetRouterAdapter;
		
	protected:
		BACnetRouterAdapterPtr	adapterList[kBACnetRouterMaxAdapters];
		int						adapterListLen;
		BACnetRouterList		routerList[kBACnetRouterMaxRouterListLen];
		int						routerListLen;
		
		void ProcessNPDU( BACnetRouterAdapterPtr adapter, const BACnetNPDU &npdu );
		
		virtual void ProcessVendorNetMessage(	BACnetRouterAdapterPtr adapter
			,									const int msgType
			,									const BACnetAddress &srcAddr
			,									const BACnetAddress &destAddr
			,									const BACnetOctet *mptr
			,									const int len
			);
		
		void ProcessNetMessage(	BACnetRouterAdapterPtr adapter
			,					const int msgType
			,					const BACnetAddress &srcAddr
			,					const BACnetAddress &destAddr
			,					const BACnetOctet *mptr
			,					const int len
			);
		
		void BroadcastRoutingTable( BACnetRouterAdapterPtr adapter );
		
		void Indication( const BACnetNPDU &pdu );				// message from app
		
	public:
		BACnetRouter( void )
			: adapterListLen(0), routerListLen(0)
		{
		}
		
		void BindToEndpoint( BACnetNetServerPtr endp, int net );
		void UnbindFromEndpoint( BACnetNetServerPtr endp );

		void BroadcastRoutingTables( void );
	};

#endif
