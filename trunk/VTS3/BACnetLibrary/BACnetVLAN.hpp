#ifndef _BACnetVLAN
#define _BACnetVLAN

#include "BACnet.hpp"
#include "BACnetTask.hpp"

//
//	BACnetVLAN
//
//	A BACnet Virtual LAN consists of two different objects, one that represents the 
//	LAN and one for nodes on the LAN.  The BACnetVLAN object is a factory that creates 
//	VLAN nodes, so it can use whatever mechanism it wants to assign addresses and keep 
//	them LAN specific.
//

class BACnetVLAN;
typedef BACnetVLAN *BACnetVLANPtr;

class BACnetVLANNode;
typedef BACnetVLANNode *BACnetVLANNodePtr;

//
//	BACnetVLANMsg
//

class BACnetVLANMsg : public BACnetTask {
		friend class BACnetVLAN;
		
	protected:
		BACnetVLANPtr		msgLAN;
		BACnetAddress		msgSource;
		BACnetAddress		msgDestination;
		BACnetOctet			*msgData;
		int					msgLen;
	
	public:
		BACnetVLANMsg( BACnetVLANPtr lan, const BACnetAddress &source, const BACnetNPDU &pdu );
		~BACnetVLANMsg( void );
		
		void ProcessTask( void );
	};

typedef BACnetVLANMsg *BACnetVLANMsgPtr;

//
//	BACnetVLANNode
//

class BACnetVLANNode : public BACnetNetServer {
		friend class BACnetVLAN;
		
	public:
		BACnetAddress	nodeAddr;
		
	protected:
		BACnetVLANPtr	nodeLAN;
		
		BACnetVLANNode( BACnetAddress addr, BACnetVLANPtr lan )
			: nodeAddr(addr), nodeLAN(lan)
		{
		}
		
		virtual void Indication( const BACnetNPDU &pdu );
	};

//
//	BACnetVLAN
//

const int	kBACnetVLANNodeListSize = 8;
		
class BACnetVLAN {
		friend class BACnetVLANMsg;
		
	protected:
		int					vlanID;
		BACnetVLANNodePtr	nodeList[kBACnetVLANNodeListSize];
		int					nodeListSize;
		
		void ProcessMessage( BACnetVLANMsgPtr msg );
		
	public:
		BACnetVLAN( int id );
		
		BACnetVLANNodePtr	NewNode( void );
		void DeleteNode( BACnetVLANNodePtr np );
	};

#endif
