#ifndef _VTSCtrl
#define _VTSCtrl

#include "BACnet.hpp"
#include "BACnetIP.hpp"

//
//	VTSCtrl
//

class VTSCtrl {
	public:
		BOOL		ctrlNull;				// no value specified
		bool		ctrlEnabled;			// control enabled on page

		int			ctrlID;					// dialog control ID
		const CWnd*	ctrlWindow;				// owner window

		VTSCtrl( const CWnd* wp, int id );	// bind to window and control
		virtual ~VTSCtrl( void );

		virtual void Enable( void );		// enable object and control
		virtual void Disable( void );		// disable both

		virtual void CtrlToObj( void );		// interpret control contents, save in object
		virtual void ObjToCtrl( void );		// object value reflected in control

		virtual void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		virtual void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents

		void UpdateData( BOOL bCtrlToObj = true );	// call one of above functions.
	};

typedef VTSCtrl *VTSCtrlPtr;

//
//	VTSAddrCtrl
//

class VTSAddrCtrl : public VTSCtrl, public BACnetOctetString {
	public:
		bool	emptyIsNull;						// empty control value is null

		VTSAddrCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );						// interpret control contents, save in object
		void ObjToCtrl( void );						// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSAddrCtrl *VTSAddrCtrlPtr;

//
//	VTSEnetAddrCtrl
//

class VTSNameList;

class VTSEnetAddrCtrl : public VTSCtrl, public BACnetAddress {
	public:
		int				ctrlComboID;
		VTSNameList		*ctrlNameList;

		VTSEnetAddrCtrl( const CWnd* wp, int cid, int tid );	// bind to window and control

		void LoadCombo( VTSNameList *nameList, unsigned int portID );		// load the names
		void Selchange( void );							// combo selection changed
		void FindName( const char *name );				// initialize with a specific name (TD or IUT)

		virtual void Enable( void );					// enable object and control
		virtual void Disable( void );					// disable both

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSEnetAddrCtrl *VTSEnetAddrCtrlPtr;

//
//	VTSIPAddrCtrl
//

class VTSIPAddrCtrl : public VTSCtrl, public BACnetIPAddr {
	public:
		int				ctrlComboID;
		VTSNameList		*ctrlNameList;

		VTSIPAddrCtrl( const CWnd* wp, int cid, int tid );	// bind to window and controls

		void LoadCombo( VTSNameList *nameList, unsigned int portID );		// load the names
		void Selchange( void );							// combo selection changed
		void FindName( const char *name );				// initialize with a specific name (TD or IUT)

		virtual void Enable( void );					// enable object and control
		virtual void Disable( void );					// disable both

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSIPAddrCtrl *VTSIPAddrCtrlPtr;

//
//	VTSRemoteAddrCtrl
//

class VTSIntegerCtrl;

class VTSRemoteAddrCtrl : public VTSCtrl, public BACnetAddress {
	public:
		int				ctrlComboID;
		VTSNameList		*ctrlNameList;
		VTSIntegerCtrl	*ctrlNet;

		VTSRemoteAddrCtrl( const CWnd* wp, VTSIntegerCtrl *icp, int cid, int tid );	// bind to window and controls

		void LoadCombo( VTSNameList *nameList, unsigned int portID, bool okBroadcast );	// load the names

		void Selchange( void );							// combo selection changed
		void FindName( const char *name );				// initialize with a specific name (TD or IUT)

		virtual void Enable( void );					// enable object and control
		virtual void Disable( void );					// disable both

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSRemoteAddrCtrl *VTSRemoteAddrCtrlPtr;

//
//	VTSBooleanCtrl
//

class VTSBooleanCtrl : public VTSCtrl, public BACnetBoolean {
	public:
		bool	m_bCheckBox;					// true iff ctrl is a check box

		VTSBooleanCtrl( const CWnd* wp, int id, bool isCheckBox = false );	// bind to window and control

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control
		void SetCheck( bool fChecked );			// madanner 9/4/02
												// simplifies setting control from external 

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSBooleanCtrl *VTSBooleanCtrlPtr;

//
//	VTSEnumeratedCtrl
//

class VTSEnumeratedCtrl : public VTSCtrl, public BACnetEnumerated {
	public:
		char	**m_Table;						// ptr to list of char*
		int		m_TableSize;					// number of enumeration values
		bool	m_bCombo;						// true iff ctrl is a combo

		VTSEnumeratedCtrl( const CWnd* wp, int id, char **table, int tableSize, bool isCombo = false );	// bind to window and control

		void LoadCombo( void );					// initialize combo list
		void SetDropDownSize( UINT lines );		// change the number of lines to display

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSEnumeratedCtrl *VTSEnumeratedCtrlPtr;

void SetDropDownSize( CComboBox& box, UINT lines );

//
//	VTSUnsignedCtrl
//

class VTSUnsignedCtrl : public VTSCtrl, public BACnetUnsigned {
	public:
		VTSUnsignedCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );							// interpret control contents, save in object
		void ObjToCtrl( void );							// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSUnsignedCtrl *VTSUnsignedCtrlPtr;

//
//	VTSIntegerCtrl
//

class VTSIntegerCtrl : public VTSCtrl, public BACnetInteger {
	public:
		VTSIntegerCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSIntegerCtrl *VTSIntegerCtrlPtr;

//
//	VTSRealCtrl
//

class VTSRealCtrl : public VTSCtrl, public BACnetReal {
	public:
		VTSRealCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSRealCtrl *VTSRealCtrlPtr;

//
//	VTSDoubleCtrl
//

class VTSDoubleCtrl : public VTSCtrl, public BACnetDouble {
	public:
		VTSDoubleCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSDoubleCtrl *VTSDoubleCtrlPtr;

//
//	VTSCharacterStringCtrl
//

class VTSCharacterStringCtrl : public VTSCtrl, public BACnetCharacterString {
	public:
		bool	emptyIsNull;						// empty control value is null

		VTSCharacterStringCtrl( const CWnd* wp, int id );	// bind to window and control

		void CtrlToObj( void );						// interpret control contents, save in object
		void ObjToCtrl( void );						// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSCharacterStringCtrl *VTSCharacterStringCtrlPtr;

//
//	VTSOctetStringCtrl
//

class VTSOctetStringCtrl : public VTSCtrl, public BACnetOctetString {
	public:
		bool	emptyIsNull;						// empty control value is null

		VTSOctetStringCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );						// interpret control contents, save in object
		void ObjToCtrl( void );						// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSOctetStringCtrl *VTSOctetStringCtrlPtr;

//
//	VTSBitStringCtrl
//

class VTSBitStringCtrl : public VTSCtrl, public BACnetBitString {
	public:
		bool	emptyIsNull;						// empty control value is null

		VTSBitStringCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );						// interpret control contents, save in object
		void ObjToCtrl( void );						// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSBitStringCtrl *VTSBitStringCtrlPtr;

//
//	VTSDateCtrl
//

class VTSDateCtrl : public VTSCtrl, public BACnetDate {
	public:
		VTSDateCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSDateCtrl *VTSDateCtrlPtr;

//
//	VTSTimeCtrl
//

class VTSTimeCtrl : public VTSCtrl, public BACnetTime {
	public:
		VTSTimeCtrl( const CWnd* wp, int id );		// bind to window and control

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSTimeCtrl *VTSTimeCtrlPtr;

//
//	VTSObjectIdentifierCtrl
//

class VTSObjectIdentifierCtrl : public VTSCtrl, public BACnetObjectIdentifier {
	public:
		VTSObjectIdentifierCtrl( const CWnd* wp, int id );	// bind to window and control

		void CtrlToObj( void );					// interpret control contents, save in object
		void ObjToCtrl( void );					// object value reflected in control

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSObjectIdentifierCtrl *VTSObjectIdentifierCtrlPtr;

//
//	VTSListCtrl
//

struct VTSListColDesc {
	char*	colName;					// column name
	int		colFormat;					// LVCFMT_LEFT, LVCFMT_RIGHT, or LVCFMT_CENTER
	int		colWidth;					// width
	int		colCtrlID;					// ID of matching control
	};

typedef VTSListColDesc *VTSListColDescPtr;

class CSendPage;

class VTSListCtrl {
	private:
		bool			listAddInProgress;	// iff new item being added

	public:
		VTSListCtrl( void );
		virtual ~VTSListCtrl( void );

		int				listSelectedRow;	// selected row, -1 iff nothing selected

		CSendPage		*listWindow;		// window containing controls
		CListCtrl		*listCtrl;			// control to oversee
		VTSListColDesc	*listItems;			// columns

		void Init( CSendPage* wp, CListCtrl *cp, VTSListColDesc *dp  );	// start empty

		int	GetItemCount( void );						// from the list
		const char *GetItemText( int row, int col );	//

		virtual const char *GetCtrlText( int ctrlID );
		virtual void SetCtrlText( int ctrlID, const char *text );
		virtual void EnableCtrl( int ctrlID, int enable );

		void AddButtonClick( void );		// add button clicked
		void RemoveButtonClick( void );		// remove button clicked

		void OnItemChanging( NMHDR *pNMHDR, LRESULT *pResult );

		void OnChangeItem( int ctrlID );	// text in control changed
	};

typedef VTSListCtrl *VTSListCtrlPtr;

//
//	VTSStatusFlags
//

class VTSStatusFlags : public BACnetBitString {
	public:
		VTSStatusFlags( const CWnd* wp );			// bind to window and control

		bool		ctrlEnabled;					// controls enabled on page

		const CWnd*	ctrlWindow;						// owner window

		void Enable( void );						// enable object and controls
		void Disable( void );						// disable both

		void InAlarmClick( void );					// In-alarm button clicked
		void FaultClick( void );					// Fault button clicked
		void OverriddenClick( void );				// Overridden button clicked
		void OutOfServiceClick( void );				// Out-of-service button clicked

		void CtrlToObj( void );						// interpret control contents, save in object
		void ObjToCtrl( void );						// object value reflected in control
		void UpdateData( BOOL bCtrlToObj );			// call one of the above

		void SaveCtrl( BACnetAPDUEncoder& enc );		// save the contents
		void RestoreCtrl( BACnetAPDUDecoder& dec );		// restore the contents
	};

typedef VTSStatusFlags *VTSStatusFlagsPtr;

#endif
