// VTSDoc.h : interface of the VTSDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VTSDOC_H__BDE65088_B82F_11D3_BE52_00A0C95A9812__INCLUDED_)
#define AFX_VTSDOC_H__BDE65088_B82F_11D3_BE52_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>
#include <afxtempl.h>

#include "BACnet.hpp"
#include "BACnetRouter.hpp"

#include "BACnetBTR.hpp"
#include "BACnetBBMD.hpp"
#include "BACnetBIPSimple.hpp"
#include "BACnetBIPForeign.hpp"

#include "WinIP.hpp"
#include "WinWinPcap.hpp"

#include "JConfig.hpp"
#include "JDB.hpp"

#include "VTSDB.h"

class VTSStatisticsCollector;
typedef VTSStatisticsCollector *VTSStatisticsCollectorPtr;

class VTSStatisticsDlg;
typedef VTSStatisticsDlg *VTSStatisticsDlgPtr;
// forward declarations

class VTSDoc;
typedef VTSDoc *VTSDocPtr;

class CFrameContext;
typedef CFrameContext *CFrameContextPtr;

class VTSPort;
typedef VTSPort *VTSPortPtr;

class VTSWinWinPcapPort;
typedef VTSWinWinPcapPort *VTSWinWinPcapPortPtr;

class VTSWinIPPort;
typedef VTSWinIPPort *VTSWinIPPortPtr;

class VTSPortDlg;
typedef VTSPortDlg *VTSPortDlgPtr;

class VTSClient;
typedef VTSClient *VTSClientPtr;

class VTSServer;
typedef VTSServer *VTSServerPtr;

class VTSDevice;
typedef VTSDevice *VTSDevicePtr;

class VTSDevicePort;
typedef VTSDevicePort *VTSDevicePortPtr;

class VTSDeviceDlg;
typedef VTSDeviceDlg *VTSDeviceDlgPtr;

class VTSObjPropValueList;
typedef VTSObjPropValueList *VTSObjPropValueListPtr;

struct CSendGroup;
typedef CSendGroup *CSendGroupPtr;
typedef CSendGroupPtr *CSendGroupList;

class ScriptNetFilter;
typedef ScriptNetFilter *ScriptNetFilterPtr;

//
//	VTSPort
//
//	The VTSPort object sits between the VTSDoc and a derived class of BACnetPort (one 
//	of the VTSxPort objects).  The port that it connects to depends on the contents of 
//	the descriptor.
//

class VTSPort {
	protected:
		void AddToMasterList( void );
		void RemoveFromMasterList( void );

	public:
		int					portStatus;					// non-zero iff error
		char				*portStatusDesc;			// status description

		objId				portDescID;					// ID of descriptor
		VTSPortDesc			portDesc;					// port configuration info

		VTSDocPtr			portDoc;					// doc for packets
		CSendGroupList		portSendGroup;				// send group to form packets

		BACnetPortPtr		portEndpoint;				// endpoint to get them
		ScriptNetFilterPtr	portFilter;					// way to process them in scripts

		BACnetBTRPtr		portBTR;					// BTR object in stream
		BACnetBBMDPtr		portBBMD;					// BBMD obj
		BACnetBIPSimplePtr	portBIPSimple;				// BIP Simple endpoint
		BACnetBIPForeignPtr	portBIPForeign;				// BIP Foreign
		BACnetNetServerPtr	portBindPoint;				// points to one of the above

		VTSDevicePtr		portDevice;					// pointer to bound device

		VTSPort( VTSDocPtr dp, objId id );
		~VTSPort( void );

		void ReadDesc( void );							// read descriptor from database
		void WriteDesc( void );							// save changes to descriptor
		void Configure( CString *cp );					// request a configuration dialog

		void Refresh( void );							// reconnect to port

		void SendData( BACnetOctet *data, int len );	// pass data to endpoint
	};

typedef VTSPort *VTSPortPtr;

//
//	VTSPortList
//

class VTSPortList : public CList<VTSPortPtr,VTSPortPtr> {
		friend class VTSPort;

	protected:
		VTSDocPtr	m_pDoc;

	public:
		VTSPortList( void );
		~VTSPortList( void );

		void Load( VTSDocPtr docp );					// bind to database and load
		void Unload( void );							// toss the ports away

		void Add( void );								// add a new port
		void Remove( int i );							// remove a port
		VTSPortPtr FindPort( const char *name );		// find a port with a given name

		int Length( void );								// number of defined ports
		VTSPortPtr operator []( int i );				// index into port list
	};

typedef VTSPortList *VTSPortListPtr;

extern VTSPortList	gMasterPortList;					// global list of all ports

//
//	VTSNameList
//

class VTSNameList {
	protected:
		VTSDocPtr	m_pDoc;
		
		static VTSNameDesc searchName;
		static int TDSearch( const VTSNameDescPtr, const VTSNameDescPtr );

	public:
		VTSNameList( void );
		~VTSNameList( void );

		void Load( VTSDocPtr docp );					// bind to database

		void Add( void );								// add a new name
		void Remove( int i );							// remove a name

		int Length( void );								// number of defined names
		const char* AddrToName( const BACnetAddress &addr, objId portID );	// translate to a name

		void ReadName( int i, VTSNameDescPtr ndp );
		void WriteName( int i, VTSNameDescPtr ndp );

		void DefineTD( objId port, const BACnetOctet *addr, int len );
		const BACnetAddress *FindTD( objId port );		// what is the TD address for a port
	};

typedef VTSNameList *VTSNameListPtr;

namespace NetworkSniffer {

extern JDBListPtr	gNameListSearchList;
extern VTSNameDesc	gNameListSearchName;

void SetLookupContext( objId port, JDBListPtr list );
const char* LookupName( int net, const BACnetOctet *addr, int len );
int NameSearch( const VTSNameDescPtr, const VTSNameDescPtr );

}

//
//	VTSClient
//

class VTSClient : public BACnetClient {
	public:
		VTSDevicePtr		clientDev;

		VTSClient( VTSDevicePtr dp );

		void Confirmation( const BACnetAPDU &apdu );

		void IAm( void );	// initiate a global-broadcast I-Am
	};

typedef VTSClient *VTSClientPtr;

//
//	VTSServer
//

class VTSServer : public BACnetServer {
	public:
		VTSDevicePtr		serverDev;

		VTSServer( VTSDevicePtr dp );

		void Indication( const BACnetAPDU &apdu );
		void Response( const BACnetAPDU &pdu );

		void WhoIs( const BACnetAPDU &apdu );
		void IAm( const BACnetAPDU &apdu );

		void ReadProperty( const BACnetAPDU &apdu );
		void WriteProperty( const BACnetAPDU &apdu );
	};

typedef VTSServer *VTSServerPtr;

//
//	VTSDevice
//
//	The VTSDevice object wraps around a BACnetDevice, BACnetRouter, VTSServer and 
//	VTSClient.
//

class VTSDevice {
		friend class VTSClient;
		friend class VTSServer;

	protected:
		void AddToMasterList( void );
		void RemoveFromMasterList( void );

		BACnetDevice		devDevice;
		BACnetRouter		devRouter;
		
		VTSClient			devClient;
		VTSServer			devServer;

		VTSPortPtr			devPort;
		VTSDevicePortPtr	devPortEndpoint;

	public:
		objId				devDescID;					// ID of descriptor
		VTSDeviceDesc		devDesc;					// device configuration info
		VTSDocPtr			devDoc;						// doc for packets
		VTSObjPropValueListPtr	devObjPropValueList;	// list of objects, properties and values

		VTSDevice( VTSDocPtr dp, objId id );
		~VTSDevice( void );

		void ReadDesc( void );							// read descriptor from database
		void WriteDesc( void );							// save changes to descriptor

		void Bind( VTSPortPtr pp, int net );			// associate with a port and network
		void Unbind( VTSPortPtr pp );					// disassociate

		void SendAPDU( const BACnetAPDU &apdu );		// message from a script

		void IAm( void );								// ask the client to send out an I-Am
	};

typedef VTSDevice *VTSDevicePtr;

//
//	VTSDeviceList
//

class VTSDeviceList : public CList<VTSDevicePtr,VTSDevicePtr> {
	protected:
		VTSDocPtr	m_pDoc;
		
		static VTSDeviceDesc searchDevice;
		static int TDSearch( const VTSDeviceDescPtr, const VTSDeviceDescPtr );

	public:
		VTSDeviceList( void );
		~VTSDeviceList( void );

		void Load( VTSDocPtr docp );					// bind to database
		void Unload( void );

		void Add( void );								// add a new name
		void Remove( int i );							// remove a name
		VTSDevicePtr FindDevice( const char *name );	// find a device with a given name
		VTSDevicePtr FindDevice( objId id );			// find a device with a given objId

		int Length( void );								// number of defined ports
		VTSDevicePtr operator []( int i );				// index into device list
	};

typedef VTSDeviceList *VTSDeviceListPtr;

extern VTSDeviceList gMasterDeviceList;					// global list of all devices

//
//	VTSDoc
//

class VTSDoc : public CDocument
{
protected: // create from serialization only
	VTSDoc();
	DECLARE_DYNCREATE(VTSDoc)

// Attributes
public:
	enum Signal
		{ eInitialUpdate = 0
		, eNewFrameCount = 1
		};

	VTSDBPtr				m_pDB;
	int						m_PacketCount;		// packets in document

	VTSPortList				m_Ports;
	VTSNameList				m_Names;
	VTSDeviceList			m_Devices;

	bool					m_postMessages;		// OK to post messages about new packets
	
	VTSPortDlgPtr	m_pPortDlg;
	VTSStatisticsDlgPtr m_pStatitiscsDlg;// a pointer to the statistics dialog
	bool m_bStatisticsDlgInUse;// indicates whether the statistics dialog is shown
    VTSStatisticsCollectorPtr m_pStatisticsCollector;

// Operations
public:

	void BindFrameContext( CFrameContext *pfc );
	void UnbindFrameContext( CFrameContext *pfc );

	void SetPacketCount( int count );
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VTSDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

// Implementation
public:

	void DeletePackets( void );
	void DoPortsDialog( void );
	void PortStatusChange( void );
	void DoNamesDialog( void );
	void DoPreferencesDialog( void );
	void DoDevicesDialog( void );

	void DoSendWindow( int iGroup, int iItem );

	void NewPacketCount(void);

	virtual ~VTSDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CCriticalSection		m_FrameContextsCS;
	CFrameContextPtr		m_FrameContexts;



// Generated message map functions
protected:
	//{{AFX_MSG(VTSDoc)
	afx_msg void OnViewStatistics();
	afx_msg void OnFileSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

typedef VTSDoc *VTSDocPtr;

//
//	VTSDocList
//

class VTSDocList : public CList<VTSDocPtr,VTSDocPtr> {
	public:
		VTSDocList( void );
		~VTSDocList( void );
	
		// list operations
		void Append( VTSDocPtr vdp );				// add a child at the end
		void Remove( VTSDocPtr vdp );				// remove a child

		int Length( void );							// number of children
		VTSDocPtr Child( int indx );				// child by index
	};

typedef VTSDocList *VTSDocListPtr;
const int kVTSDocListSize = sizeof( VTSDocList );

extern VTSDocList gDocList;							// list of all documents

//
//	VTSWinWinPcapPort
//

class VTSWinWinPcapPort : public WinWinPcap {
	protected:
		VTSPortPtr			m_pPort;

	public:
		VTSWinWinPcapPort( VTSPortPtr pp );
		virtual ~VTSWinWinPcapPort( void );

		void FilterData( BACnetOctet *, int len, BACnetPortDirection dir );

		void PortStatusChange( void );
	};

//
//	VTSWinIPPort
//

class VTSWinIPPort : public WinIP {
	protected:
		VTSPortPtr			m_pPort;

	public:
		VTSWinIPPort( VTSPortPtr pp );
		virtual ~VTSWinIPPort( void );

		void FilterData( BACnetOctet *, int len, BACnetPortDirection dir );

		void PortStatusChange( void );
	};

//
//	VTSDevicePort
//

class VTSDevicePort : public BACnetPort {
	protected:
		VTSPortPtr			m_pPort;
		VTSDevicePtr		m_pDevice;

	public:
		VTSDevicePort( VTSPortPtr pp, VTSDevicePtr dp );
		virtual ~VTSDevicePort( void );

		void Indication( const BACnetNPDU &pdu );
		void SendData( BACnetOctet *data, int len );		// raw data request
	};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTSDOC_H__BDE65088_B82F_11D3_BE52_00A0C95A9812__INCLUDED_)
