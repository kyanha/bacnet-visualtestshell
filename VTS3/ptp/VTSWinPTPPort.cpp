// VTSWinPTPPort.cpp: implementation of the VTSWinPTPPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTS.h"
#include "VTSWinPTPPort.h"
#include "PI.h"
#include "send.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VTSWinPTPPort::VTSWinPTPPort(VTSPortPtr pp)
			:WinPTP(pp->portDesc.portConfig)
			,m_pPort( pp )
{
	// let the port know which send group to use
	m_pPort->portSendGroup = gPTPGroupList;
}

VTSWinPTPPort::~VTSWinPTPPort(void)
{
}

void VTSWinPTPPort::FilterData( BACnetOctet *data, int len, BACnetPortDirection dir )
{
	VTSPacket	pkt	;
	unsigned char address[8];
	
	// fill in the packet header
	pkt.packetHdr.packetPortID = m_pPort->portDescID;
	pkt.packetHdr.packetProtocolID = (int)BACnetPIInfo::ProtocolType::ptpProtocol;
	pkt.packetHdr.packetFlags = 0;
	pkt.packetHdr.packetType = (dir == portSending) ? txData : rxData;
	
	// ignore the source or destination
	ZeroMemory(address,sizeof(address));
	if (dir == portSending)
		pkt.packetHdr.packetDestination.LocalStation( address, sizeof(address) );
	else
	if (dir == portReceiving) 
	{
		pkt.packetHdr.packetSource.LocalStation( address,sizeof(address) );
	}	

	// let the packet refer to the pdu contents, cast away const
	pkt.NewDataRef( data, len );
	
	// save it in the database;
	m_pPort->portDoc->m_pDB->WritePacket( -1, pkt );
	
	// tell the application there is a new packet count
	if (m_pPort->portDoc->m_postMessages)
		::PostThreadMessage( AfxGetApp()->m_nThreadID
			, WM_VTS_RCOUNT, (WPARAM)0, (LPARAM)m_pPort->portDoc
			);
}

void VTSWinPTPPort::PortStatusChange( void )
{
}
