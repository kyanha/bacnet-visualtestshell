
#include "stdafx.h"

#include <iostream>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "VTSCtrl.h"
#include "VTSDoc.h"

#include "ScriptBase.h"
#include "ScriptKeywords.h"

#include "SendPage.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

//
//	VTSCtrl::VTSCtrl
//

VTSCtrl::VTSCtrl( const CWnd* wp, int id )
	: ctrlNull(true), ctrlEnabled(true)
	, ctrlWindow(wp), ctrlID(id)
{
}

//
//	VTSCtrl::~VTSCtrl
//

VTSCtrl::~VTSCtrl( void )
{
}

//
//	VTSCtrl::Enabled
//

void VTSCtrl::Enable( void )
{
	if (ctrlEnabled)
		return;

	ctrlEnabled = true;
	ctrlWindow->GetDlgItem( ctrlID )->EnableWindow( true );
}

//
//	VTSCtrl::Disable
//

void VTSCtrl::Disable( void )
{
	if (!ctrlEnabled)
		return;

	ctrlEnabled = false;
	ctrlWindow->GetDlgItem( ctrlID )->EnableWindow( false );
}

//
//	VTSCtrl::CtrlToObj
//

void VTSCtrl::CtrlToObj( void )
{
	ASSERT( 0 );
}

//
//	VTSCtrl::ObjToCtrl
//

void VTSCtrl::ObjToCtrl( void )
{
	ASSERT( 0 );
}

//
//	VTSCtrl::SaveCtrl
//

void VTSCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
	ASSERT( 0 );
}

//
//	VTSCtrl::RestoreCtrl
//

void VTSCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	ASSERT( 0 );
}

//
//	VTSCtrl::UpdateData
//

void VTSCtrl::UpdateData( BOOL bCtrlToObj )
{
	if (bCtrlToObj)
		CtrlToObj();
	else
		ObjToCtrl();
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSEnetAddrCtrl
//

VTSEnetAddrCtrl::VTSEnetAddrCtrl( const CWnd* wp, int cid, int tid )
	: VTSCtrl( wp, tid ), ctrlComboID(cid), ctrlNameList(0)
{
}

void VTSEnetAddrCtrl::LoadCombo( VTSNameList *nameList, unsigned int portID )
{
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	int				indx
	;
	VTSNameDesc		name
	;

	// already initialized?
	if (ctrlNameList)
		return;

	// save a pointer to the name list
	ctrlNameList = nameList;

	// scan through the names
	for (int i = 0; i < nameList->Length(); i++) {
		// read the description
		ctrlNameList->ReadName( i, &name );

		// if it matches our port, save the name and index
		if ((name.namePort == portID) || (name.namePort == 0))
			if (((name.nameAddr.addrType == localStationAddr) && (name.nameAddr.addrLen == 6))
				|| (name.nameAddr.addrType == localBroadcastAddr)) {
					indx = cbp->AddString( name.nameName );
					if ((indx == CB_ERR) || (indx == CB_ERRSPACE))
						break;
					cbp->SetItemData( indx, i );
				}
	}
}

void VTSEnetAddrCtrl::Selchange( void )
{
	unsigned char	broadcastAddr[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
	;
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		name
	;
	int				i = cbp->GetItemData( cbp->GetCurSel() )
	;

	ctrlNameList->ReadName( i, &name );

	addrLen = 6;
	if (name.nameAddr.addrType == localBroadcastAddr)
		memcpy( addrAddr, broadcastAddr, 6 );
	else
		memcpy( addrAddr, name.nameAddr.addrAddr, 6 );

	ctrlNull = false;

	ObjToCtrl();
}

//
//	VTSEnetAddrCtrl::FindName
//

void VTSEnetAddrCtrl::FindName( const char *name )
{
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		nameDesc
	;

	// scan through the names
	for (int i = 0; i < cbp->GetCount(); i++) {
		// read the description
		ctrlNameList->ReadName( cbp->GetItemData( i ), &nameDesc );

		// if it matches the name, set the selection
		if (strcmp(nameDesc.nameName,name) == 0) {
			cbp->SetCurSel( i );
			Selchange();
			break;
		}
	}
}


//
//	VTSEnetAddrCtrl::Enable
//

void VTSEnetAddrCtrl::Enable( void )
{
	VTSCtrl::Enable();
	ctrlWindow->GetDlgItem( ctrlComboID )->EnableWindow( true );
}

//
//	VTSEnetAddrCtrl::Disable
//

void VTSEnetAddrCtrl::Disable( void )
{
	VTSCtrl::Disable();
	ctrlWindow->GetDlgItem( ctrlComboID )->EnableWindow( false );
}

//
//	VTSEnetAddrCtrl::CtrlToObj
//

void VTSEnetAddrCtrl::CtrlToObj( void )
{
	int			upperNibble, lowerNibble
	;
	char		c
	;
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// remove contents
	addrLen = 0;

	// translate the text into octets
	for (;;) {
		// look for a hex digit
		while ((c = toupper(*s++)) && !isxdigit(c))
			;
		if (!c) break;
		upperNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

		// look for another hex digit
		while ((c = toupper(*s++)) && !isxdigit(c))
			;
		if (!c) break;
		lowerNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

		// add the byte
		if (addrLen < sizeof(addrAddr))
			addrAddr[ addrLen++ ] = (upperNibble << 4) + lowerNibble;
	}

	// if nothing specified, set value to null
	ctrlNull = (addrLen == 0);
	if (ctrlNull)
		return;

	// get the combo box
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		name
	;

	// scan through the names
	bool found = false;
	for (int i = 0; i < cbp->GetCount(); i++) {
		// read the description
		ctrlNameList->ReadName( cbp->GetItemData( i ), &name );

		// if it matches our port, save the name and index
		if ((name.nameAddr.addrLen == addrLen) && (memcmp(name.nameAddr.addrAddr,addrAddr,addrLen) == 0)) {
			cbp->SetCurSel( i );
			found = true;
			break;
		}
	}

	// clear if no matching name found
	if (!found)
		cbp->SetWindowText( _T("") );
}

//
//	VTSEnetAddrCtrl::ObjToCtrl
//

void VTSEnetAddrCtrl::ObjToCtrl( void )
{
	char	buff[kMaxAddressLen * 3], *s = buff
	;

	// clear the buffer
	buff[0] = 0;

	// encode the address
	if (!ctrlNull)
		for (int i = 0; i < addrLen; i++) {
			if (s != buff)
				*s++ = '-';
			sprintf( s, "%02X", addrAddr[i] );
			s += 2;
		}

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( buff );
}

//
//	VTSEnetAddrCtrl::SaveCtrl
//

void VTSEnetAddrCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSEnetAddrCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else {
		BACnetInteger( addrType ).Encode( enc );
		BACnetInteger( addrNet ).Encode( enc );
		BACnetOctetString( addrAddr, addrLen ).Encode( enc );
	}
}

//
//	VTSEnetAddrCtrl::RestoreCtrl
//

void VTSEnetAddrCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSEnetAddrCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		BACnetInteger	type, net
		;
		BACnetOctetString	addr( 6 )
		;

		ctrlNull = false;

		type.Decode( dec );
		addrType = (BACnetAddressType)type.intValue;
		net.Decode( dec );
		addrNet = net.intValue;
		addr.Decode( dec );
		addrLen = addr.strLen;
		memcpy( addrAddr, addr.strBuff, addrLen );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSIPAddrCtrl
//

VTSIPAddrCtrl::VTSIPAddrCtrl( const CWnd* wp, int cid, int tid )
	: VTSCtrl( wp, tid ), ctrlComboID(cid), ctrlNameList(0)
{
}


void VTSIPAddrCtrl::LoadCombo( VTSNameList *nameList, unsigned int portID )
{
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	int				indx
	;
	VTSNameDesc		name
	;

	// already initialized?
	if (ctrlNameList)
		return;

	// save a pointer to the name list
	ctrlNameList = nameList;

	// scan through the names
	for (int i = 0; i < nameList->Length(); i++) {
		// read the description
		ctrlNameList->ReadName( i, &name );

		// if it matches our port, save the name and index
		if ((name.namePort == portID) || (name.namePort == 0))
			if (((name.nameAddr.addrType == localStationAddr) && (name.nameAddr.addrLen == 6))
				|| (name.nameAddr.addrType == localBroadcastAddr)) {
					indx = cbp->AddString( name.nameName );
					if ((indx == CB_ERR) || (indx == CB_ERRSPACE))
						break;
					cbp->SetItemData( indx, i );
				}
	}
}

void VTSIPAddrCtrl::Selchange( void )
{
	unsigned char	broadcastAddr[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xBA, 0xC0 }
	;
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		name
	;
	int				i = cbp->GetItemData( cbp->GetCurSel() )
	;

	ctrlNameList->ReadName( i, &name );

	addrLen = 6;
	if (name.nameAddr.addrType == localBroadcastAddr)
		memcpy( addrAddr, broadcastAddr, 6 );
	else
		memcpy( addrAddr, name.nameAddr.addrAddr, 6 );

	ctrlNull = false;

	ObjToCtrl();
}

//
//	VTSIPAddrCtrl::FindName
//

void VTSIPAddrCtrl::FindName( const char *name )
{
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		nameDesc
	;

	// scan through the names
	for (int i = 0; i < cbp->GetCount(); i++) {
		// read the description
		ctrlNameList->ReadName( cbp->GetItemData( i ), &nameDesc );

		// if it matches the name, set the selection
		if (strcmp(nameDesc.nameName,name) == 0) {
			cbp->SetCurSel( i );
			Selchange();
			break;
		}
	}
}

//
//	VTSIPAddrCtrl::Enable
//

void VTSIPAddrCtrl::Enable( void )
{
	VTSCtrl::Enable();
	ctrlWindow->GetDlgItem( ctrlComboID )->EnableWindow( true );
}

//
//	VTSIPAddrCtrl::Disable
//

void VTSIPAddrCtrl::Disable( void )
{
	VTSCtrl::Disable();
	ctrlWindow->GetDlgItem( ctrlComboID )->EnableWindow( false );
}

//
//	VTSIPAddrCtrl::CtrlToObj
//

void VTSIPAddrCtrl::CtrlToObj( void )
{
	CString			str
	;
	unsigned long	host
	;
	unsigned short	port
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );

	ctrlNull = str.IsEmpty();
	if (ctrlNull)
		return;

	// interpret the string
	StringToHostPort( (char *)str.operator LPCTSTR(), &host, 0, &port );
	Pack( host, port );

	// get the combo box
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		name
	;

	// scan through the names
	bool found = false;
	for (int i = 0; i < cbp->GetCount(); i++) {
		// read the description
		ctrlNameList->ReadName( cbp->GetItemData( i ), &name );

		// if it matches our port, save the name and index
		if ((name.nameAddr.addrLen == addrLen) && (memcmp(name.nameAddr.addrAddr,addrAddr,addrLen) == 0)) {
			cbp->SetCurSel( i );
			found = true;
			break;
		}
	}

	// clear if no matching name found
	if (!found)
		cbp->SetWindowText( _T("") );
}

//
//	VTSIPAddrCtrl::ObjToCtrl
//

void VTSIPAddrCtrl::ObjToCtrl( void )
{
	char	*txt = (ctrlNull ? "" : AddressToString())
	;

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( txt );
}

//
//	VTSIPAddrCtrl::SaveCtrl
//

void VTSIPAddrCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSIPAddrCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else {
		BACnetInteger( addrType ).Encode( enc );
		BACnetInteger( addrNet ).Encode( enc );
		BACnetOctetString( addrAddr, addrLen ).Encode( enc );
	}
}

//
//	VTSIPAddrCtrl::RestoreCtrl
//

void VTSIPAddrCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSIPAddrCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		BACnetInteger	type, net
		;
		BACnetOctetString	addr( 6 )
		;

		ctrlNull = false;

		type.Decode( dec );
		addrType = (BACnetAddressType)type.intValue;
		net.Decode( dec );
		addrNet = net.intValue;
		addr.Decode( dec );
		addrLen = addr.strLen;
		memcpy( addrAddr, addr.strBuff, addrLen );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSRemoteAddrCtrl
//

VTSRemoteAddrCtrl::VTSRemoteAddrCtrl( const CWnd* wp, VTSIntegerCtrl *icp, int cid, int tid )
	: VTSCtrl( wp, tid ), ctrlNet(icp), ctrlComboID(cid), ctrlNameList(0)
{
}

//
//	VTSRemoteAddrCtrl::LoadCombo
//

void VTSRemoteAddrCtrl::LoadCombo( VTSNameList *nameList, unsigned int portID, bool okBroadcast )
{
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	int				indx
	;
	VTSNameDesc		name
	;

	// already initialized?
	if (ctrlNameList)
		return;

	// save a pointer to the name list
	ctrlNameList = nameList;

	// scan through the names
	for (int i = 0; i < nameList->Length(); i++) {
		// read the description
		ctrlNameList->ReadName( i, &name );

		// if it matches our port, save the name and index
		if ((name.namePort == portID) || (name.namePort == 0))
			if ((name.nameAddr.addrType == remoteStationAddr) 
				|| (okBroadcast && (name.nameAddr.addrType == remoteBroadcastAddr))
				|| (okBroadcast && (name.nameAddr.addrType == globalBroadcastAddr)) ) {
					indx = cbp->AddString( name.nameName );
					if ((indx == CB_ERR) || (indx == CB_ERRSPACE))
						break;
					cbp->SetItemData( indx, i );
				}
	}
}

//
//	VTSRemoteAddrCtrl::Selchange
//

void VTSRemoteAddrCtrl::Selchange( void )
{
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		name
	;
	int				i = cbp->GetItemData( cbp->GetCurSel() )
	;

	ctrlNameList->ReadName( i, &name );

	if (name.nameAddr.addrType == remoteBroadcastAddr) {
		ctrlNet->ctrlNull = false;
		ctrlNet->intValue = name.nameAddr.addrNet;
		ctrlNet->ObjToCtrl();
		addrLen = 0;
	} else
	if (name.nameAddr.addrType == globalBroadcastAddr) {
		ctrlNet->ctrlNull = false;
		ctrlNet->intValue = 65535;
		ctrlNet->ObjToCtrl();
		addrLen = 0;
	} else {
		ctrlNet->ctrlNull = false;
		ctrlNet->intValue = name.nameAddr.addrNet;
		ctrlNet->ObjToCtrl();

		addrLen = name.nameAddr.addrLen;
		memcpy( addrAddr, name.nameAddr.addrAddr, addrLen );
	}

	ctrlNull = false;

	ObjToCtrl();
}

//
//	VTSRemoteAddrCtrl::FindName
//

void VTSRemoteAddrCtrl::FindName( const char *name )
{
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		nameDesc
	;

	// scan through the names
	for (int i = 0; i < cbp->GetCount(); i++) {
		// read the description
		ctrlNameList->ReadName( cbp->GetItemData( i ), &nameDesc );

		// if it matches the name, set the selection
		if (strcmp(nameDesc.nameName,name) == 0) {
			cbp->SetCurSel( i );
			Selchange();
			break;
		}
	}
}

//
//	VTSRemoteAddrCtrl::Enable
//

void VTSRemoteAddrCtrl::Enable( void )
{
	VTSCtrl::Enable();
	ctrlWindow->GetDlgItem( ctrlComboID )->EnableWindow( true );
}

//
//	VTSRemoteAddrCtrl::Disable
//

void VTSRemoteAddrCtrl::Disable( void )
{
	VTSCtrl::Disable();
	ctrlWindow->GetDlgItem( ctrlComboID )->EnableWindow( false );
}

//
//	VTSRemoteAddrCtrl::CtrlToObj
//

void VTSRemoteAddrCtrl::CtrlToObj( void )
{
	int			upperNibble, lowerNibble
	;
	char		c
	;
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// remove contents
	addrLen = 0;

	// translate the text into octets
	for (;;) {
		// look for a hex digit
		while ((c = toupper(*s++)) && !isxdigit(c))
			;
		if (!c) break;
		upperNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

		// look for another hex digit
		while ((c = toupper(*s++)) && !isxdigit(c))
			;
		if (!c) break;
		lowerNibble = (isdigit(c) ? (c - '0') : (c - 'A' + 10));

		// add the byte
		if (addrLen < sizeof(addrAddr))
			addrAddr[ addrLen++ ] = (upperNibble << 4) + lowerNibble;
	}

	// if nothing specified, set value to null
	ctrlNull = (addrLen == 0);
	if (ctrlNull)
		return;

	// get the combo box
	CComboBox		*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlComboID )
	;
	VTSNameDesc		name
	;

	// scan through the names
	bool found = false;
	for (int i = 0; i < cbp->GetCount(); i++) {
		// read the description
		ctrlNameList->ReadName( cbp->GetItemData( i ), &name );

		// if it matches our stuff, set it to the current selection
		if ((name.nameAddr.addrType == remoteStationAddr) 
				&& (name.nameAddr.addrNet == ctrlNet->intValue)
				&& (name.nameAddr.addrLen == addrLen)
				&& (memcmp(name.nameAddr.addrAddr,addrAddr,addrLen) == 0) )
			found = true;
		else
		if ((name.nameAddr.addrType == remoteBroadcastAddr) 
				&& (name.nameAddr.addrNet == ctrlNet->intValue)
				&& (addrLen == 0) )
			found = true;
		else
		if ((name.nameAddr.addrType == globalBroadcastAddr) 
				&& (ctrlNet->intValue == 65535)
				&& (addrLen == 0) )
			found = true;
		else
			;
		if (found) {
			cbp->SetCurSel( i );
			break;
		}
	}

	// clear if no matching name found
	if (!found)
		cbp->SetWindowText( _T("") );
}

//
//	VTSRemoteAddrCtrl::ObjToCtrl
//

void VTSRemoteAddrCtrl::ObjToCtrl( void )
{
	char	buff[kMaxAddressLen * 3], *s = buff
	;

	// clear the buffer
	buff[0] = 0;

	// encode the address
	if (!ctrlNull)
		for (int i = 0; i < addrLen; i++) {
			if (s != buff)
				*s++ = '-';
			sprintf( s, "%02X", addrAddr[i] );
			s += 2;
		}

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( buff );
}

//
//	VTSRemoteAddrCtrl::SaveCtrl
//

void VTSRemoteAddrCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSRemoteAddrCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else {
		BACnetInteger( addrType ).Encode( enc );
		BACnetInteger( addrNet ).Encode( enc );
		BACnetOctetString( addrAddr, addrLen ).Encode( enc );
	}
}

//
//	VTSRemoteAddrCtrl::RestoreCtrl
//

void VTSRemoteAddrCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSRemoteAddrCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		BACnetInteger	type, net
		;
		BACnetOctetString	addr( 8 )
		;

		ctrlNull = false;

		type.Decode( dec );
		addrType = (BACnetAddressType)type.intValue;
		net.Decode( dec );
		addrNet = net.intValue;
		addr.Decode( dec );
		addrLen = addr.strLen;
		memcpy( addrAddr, addr.strBuff, addrLen );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSBooleanCtrl
//

VTSBooleanCtrl::VTSBooleanCtrl( const CWnd* wp, int id, bool isCheckBox )
	: VTSCtrl( wp, id )
	, m_bCheckBox(isCheckBox)
{
}

//
//	VTSBooleanCtrl::CtrlToObj
//

void VTSBooleanCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	if (m_bCheckBox) {
		ctrlNull = false;
		boolValue = (eBACnetBoolean)(((CButton *)ctrlWindow->GetDlgItem( ctrlID ))->GetState() & 0x0003);
		return;
	}

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSBooleanCtrl::ObjToCtrl
//

void VTSBooleanCtrl::ObjToCtrl( void )
{
	CString		str
	;

	if (m_bCheckBox)
		((CButton *)ctrlWindow->GetDlgItem( ctrlID ))->SetCheck( boolValue ? 1 : 0 );
	else {
		// values are normalized
		if (!ctrlNull)
			Encode( str.GetBuffer( 16 ) );

		// set the text
		((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
	}
}

//
//	VTSBooleanCtrl::SaveCtrl
//

void VTSBooleanCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSBooleanCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSBooleanCtrl::RestoreCtrl
//

void VTSBooleanCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSBooleanCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSEnumeratedCtrl
//

VTSEnumeratedCtrl::VTSEnumeratedCtrl( const CWnd* wp, int id, char **table, int tableSize, bool isCombo )
	: VTSCtrl( wp, id )
	, m_Table(table), m_TableSize(tableSize), m_bCombo(isCombo)
{
}

//
//	VTSEnumeratedCtrl::LoadCombo
//

void VTSEnumeratedCtrl::LoadCombo( void )
{
	if (!m_bCombo)
		return;
	
	CComboBox	*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlID )
	;

	if (cbp->GetCount() == 0) {
		for (int i = 0; i < m_TableSize; i++)
			cbp->AddString( m_Table[i] );

		// make sure at least eight lines are visible
		SetDropDownSize( 8 );
	}

	// set up the first value
	if (!ctrlNull)
		cbp->SetCurSel( enumValue );
}

//
//	VTSEnumeratedCtrl::SetDropDownSize
//
//	This code was found on the web from:
//
//		http://mulvey.dyndns.com/Katy/VisualC/code/cbresize.html
//
//	Thank you Katy!
//

void VTSEnumeratedCtrl::SetDropDownSize( UINT lines )
{
	CComboBox	*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlID )
	;

	::SetDropDownSize( *cbp, lines );
}

void SetDropDownSize( CComboBox& box, UINT lines )
{
	ASSERT( IsWindow(box) );

	CRect		cbSize;
	int			height;

	box.GetClientRect( cbSize );

	// start with the size of the edit box
	height = box.GetItemHeight(-1);

	// add enough for the lines
	height += box.GetItemHeight(0) * lines;

	// now add on the height of the border of the edit box
	height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

	// now add the height of the border of the dropdown box
	height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

	// set the size of the window
	box.SetWindowPos( NULL,			// not relative to any other windows
		0, 0,							// TopLeft corner doesn't change
		cbSize.right, height,			// existing width, new height
		SWP_NOMOVE | SWP_NOZORDER
		);
}

//
//	VTSEnumeratedCtrl::CtrlToObj
//

void VTSEnumeratedCtrl::CtrlToObj( void )
{
	if (m_bCombo) {
		CComboBox	*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlID )
		;

		ctrlNull = false;
		enumValue = cbp->GetCurSel();

		return;
	}

	CEdit		*ctrl = (CEdit *)ctrlWindow->GetDlgItem( ctrlID )
	;
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	ctrl->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		// use the built-in decoder
		Decode( s, (const char **)m_Table, m_TableSize );
	}
	catch (...) {
	}

#if 0
	// assume it hasn't been found
	enumValue = -1;

	// try and find it
	if (!m_Table)
		enumValue = atoi( s );
	else
		for (int i = 0; i < m_TableSize; i++)
			if (strncmp(s,m_Table[i],strlen(s)) == 0) {
				enumValue = i;
				break;
			}
#endif
#if 0
	// This interesting code changes the control text to the complete 
	// enumerated value and sets the selection to what wasn't entered 
	// by the user.  Would be better if the rest of the text was gray
	// but that would take time I don't have right now.
	// NOTE: the control is disabled to prevent message loops.
	if (enumValue >= 0) {
		int len = strlen(s);
		str = m_Table[enumValue];

		// set the text
		ctrlEnabled = false;
		ctrl->SetWindowText( str );
		ctrl->SetSel( len, str.GetLength() );
		ctrlEnabled = true;
	}
#endif
}

//
//	VTSEnumeratedCtrl::ObjToCtrl
//

void VTSEnumeratedCtrl::ObjToCtrl( void )
{
	if (m_bCombo) {
		CComboBox	*cbp = (CComboBox *)ctrlWindow->GetDlgItem( ctrlID )
		;

		cbp->SetCurSel( enumValue );

		return;
	}

	CString		str
	;

	// values are normalized to decimal
	if (!ctrlNull)
		Encode( str.GetBuffer( 256 ), (const char **)m_Table, m_TableSize );

#if 0
	// values are from the table
	if (!ctrlNull)
		if (!m_Table)
			str.Format( "%d", enumValue );
		else
		if ((enumValue < 0) || (enumValue >= m_TableSize))
			str = "(out of bounds)";
		else
			str = m_Table[enumValue];
#endif

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSEnumeratedCtrl::SaveCtrl
//

void VTSEnumeratedCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSEnumeratedCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSEnumeratedCtrl::RestoreCtrl
//

void VTSEnumeratedCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSEnumeratedCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSUnsignedCtrl
//

VTSUnsignedCtrl::VTSUnsignedCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
{
}

//
//	VTSUnsignedCtrl::CtrlToObj
//

void VTSUnsignedCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		// use the built-in decoder
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSUnsignedCtrl::ObjToCtrl
//

void VTSUnsignedCtrl::ObjToCtrl( void )
{
	CString		str
	;

	// values are normalized to decimal
	if (!ctrlNull)
		Encode( str.GetBuffer( 16 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSUnsignedCtrl::SaveCtrl
//

void VTSUnsignedCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSUnsignedCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSUnsignedCtrl::RestoreCtrl
//

void VTSUnsignedCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSUnsignedCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSIntegerCtrl
//

VTSIntegerCtrl::VTSIntegerCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
{
}

//
//	VTSIntegerCtrl::CtrlToObj
//

void VTSIntegerCtrl::CtrlToObj( void )
{
	bool		negative = false
	;
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		// use the built-in decoder
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSIntegerCtrl::ObjToCtrl
//

void VTSIntegerCtrl::ObjToCtrl( void )
{
	CString		str
	;

	// values are normalized to decimal
	if (!ctrlNull)
		Encode( str.GetBuffer( 16 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSIntegerCtrl::SaveCtrl
//

void VTSIntegerCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSIntegerCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSIntegerCtrl::RestoreCtrl
//

void VTSIntegerCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSIntegerCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSRealCtrl
//

VTSRealCtrl::VTSRealCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
{
}

//
//	VTSRealCtrl::CtrlToObj
//

void VTSRealCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		// use the built-in decoder
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSRealCtrl::ObjToCtrl
//

void VTSRealCtrl::ObjToCtrl( void )
{
	CString		str
	;

	// values are normalized to decimal
	if (!ctrlNull)
		Encode( str.GetBuffer( 16 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSRealCtrl::SaveCtrl
//

void VTSRealCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSRealCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSRealCtrl::RestoreCtrl
//

void VTSRealCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSRealCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSDoubleCtrl
//

VTSDoubleCtrl::VTSDoubleCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
{
}

//
//	VTSDoubleCtrl::CtrlToObj
//

void VTSDoubleCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		// use the built-in decoder
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSDoubleCtrl::ObjToCtrl
//

void VTSDoubleCtrl::ObjToCtrl( void )
{
	CString		str
	;

	// values are normalized to decimal
	if (!ctrlNull)
		Encode( str.GetBuffer( 16 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSDoubleCtrl::SaveCtrl
//

void VTSDoubleCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSDoubleCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSDoubleCtrl::RestoreCtrl
//

void VTSDoubleCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSDoubleCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSCharacterStringCtrl
//

VTSCharacterStringCtrl::VTSCharacterStringCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
	, emptyIsNull( true )
{
}

//
//	VTSCharacterStringCtrl::CtrlToObj
//

void VTSCharacterStringCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// no data might mean empty is null
	ctrlNull = (!*s && emptyIsNull);
	if (ctrlNull)
		return;

	try {
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSCharacterStringCtrl::ObjToCtrl
//

void VTSCharacterStringCtrl::ObjToCtrl( void )
{
	CString		str
	;

	// values are normalized
	// ### this could be trouble depending on the encoding and number of chars that need escaping
	if (!ctrlNull)
		Encode( str.GetBuffer( strLen + 16 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSCharacterStringCtrl::SaveCtrl
//

void VTSCharacterStringCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSCharacterStringCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSCharacterStringCtrl::RestoreCtrl
//

void VTSCharacterStringCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSCharacterStringCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSOctetStringCtrl
//

VTSOctetStringCtrl::VTSOctetStringCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
	, emptyIsNull( true )
{
}

//
//	VTSOctetStringCtrl::CtrlToObj
//

void VTSOctetStringCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// no data might mean empty is null
	ctrlNull = (!*s && emptyIsNull);

	try {
		// use the built-in decoder
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSOctetStringCtrl::ObjToCtrl
//

void VTSOctetStringCtrl::ObjToCtrl( void )
{
	CString		str
	;

	if (!ctrlNull)
		Encode( str.GetBuffer( strLen * 2 + 4 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSOctetStringCtrl::SaveCtrl
//

void VTSOctetStringCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSOctetStringCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSOctetStringCtrl::RestoreCtrl
//

void VTSOctetStringCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSOctetStringCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSBitStringCtrl
//

VTSBitStringCtrl::VTSBitStringCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
	, emptyIsNull( true )
{
}

//
//	VTSBitStringCtrl::CtrlToObj
//

void VTSBitStringCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// no data might mean empty is null
	ctrlNull = (!*s && emptyIsNull);
	if (ctrlNull)
		return;

	try {
		// use the built-in decoder
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSBitStringCtrl::ObjToCtrl
//

void VTSBitStringCtrl::ObjToCtrl( void )
{
	CString		str
	;

	// loop through the bits
	if (!ctrlNull)
		Encode( str.GetBuffer( bitLen + 4 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSBitStringCtrl::SaveCtrl
//

void VTSBitStringCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSBitStringCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSBitStringCtrl::RestoreCtrl
//

void VTSBitStringCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSBitStringCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSDateCtrl
//

VTSDateCtrl::VTSDateCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
{
}

//
//	VTSDateCtrl::CtrlToObj
//

void VTSDateCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		// use the built-in decoder
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSDateCtrl::ObjToCtrl
//

void VTSDateCtrl::ObjToCtrl( void )
{
	CString		str
	;

	if (!ctrlNull)
		Encode( str.GetBuffer( 32 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSDateCtrl::SaveCtrl
//

void VTSDateCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSDateCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSDateCtrl::RestoreCtrl
//

void VTSDateCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSDateCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSTimeCtrl
//

VTSTimeCtrl::VTSTimeCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
{
}

//
//	VTSTimeCtrl::CtrlToObj
//

void VTSTimeCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSTimeCtrl::ObjToCtrl
//

void VTSTimeCtrl::ObjToCtrl( void )
{
	CString		str
	;

	if (!ctrlNull)
		Encode( str.GetBuffer( 32 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSTimeCtrl::SaveCtrl
//

void VTSTimeCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSTimeCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSTimeCtrl::RestoreCtrl
//

void VTSTimeCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSTimeCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSObjectIdentifierCtrl
//

VTSObjectIdentifierCtrl::VTSObjectIdentifierCtrl( const CWnd* wp, int id )
	: VTSCtrl( wp, id )
{
}

//
//	VTSObjectIdentifierCtrl::CtrlToObj
//

void VTSObjectIdentifierCtrl::CtrlToObj( void )
{
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set ctrlNull and return
	ctrlNull = (!*s);
	if (ctrlNull)
		return;

	try {
		Decode( s );
	}
	catch (...) {
	}
}

//
//	VTSObjectIdentifierCtrl::ObjToCtrl
//

void VTSObjectIdentifierCtrl::ObjToCtrl( void )
{
	CString		str
	;

	if (!ctrlNull)
		Encode( str.GetBuffer( 32 ) );

	// set the text
	((CEdit *)ctrlWindow->GetDlgItem( ctrlID ))->SetWindowText( str );
}

//
//	VTSObjectIdentifierCtrl::SaveCtrl
//

void VTSObjectIdentifierCtrl::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSObjectIdentifierCtrl::SaveCtrl\n" );

	if (ctrlNull)
		BACnetNull().Encode( enc );
	else
		Encode( enc );
}

//
//	VTSObjectIdentifierCtrl::RestoreCtrl
//

void VTSObjectIdentifierCtrl::RestoreCtrl( BACnetAPDUDecoder& dec )
{
	BACnetAPDUTag	tag
	;

//	TRACE0( "VTSObjectIdentifierCtrl::RestoreCtrl\n" );

	dec.ExamineTag( tag );
	if (tag.tagNumber == nullAppTag) {
		ctrlNull = true;
		BACnetNull().Decode( dec );
	} else {
		ctrlNull = false;
		Decode( dec );
	}
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSListCtrl::VTSListCtrl
//

VTSListCtrl::VTSListCtrl( void )
	: listSelectedRow(-1), listWindow(0), listCtrl(0), listItems(0)
{
}

//
//	VTSListCtrl::~VTSListCtrl
//
//	The list control doesn't maintain any data outside of the list contents, 
//	so there's nothing to do.  The class has virtual functions, so this is 
//	a virtual dtor.
//

VTSListCtrl::~VTSListCtrl( void )
{
}

//
//	VTSListCtrl::Init
//
//	When the CSendPage is initialized the OnInitDialog function is called, so all 
//	of the controls for the window have been created.  That function calls this 
//	one to initialize the list.
//

void VTSListCtrl::Init( CSendPage* wp, CListCtrl *cp, VTSListColDesc *dp  )
{
	// nothing is selected
	listSelectedRow = -1;
	listAddInProgress = false;

	// keep copies of the parameters
	listWindow = wp;
	listCtrl = cp;
	listItems = dp;

	// create the columns and disable the item controls
	for (int i = 0; listItems[i].colCtrlID; i++) {
		listCtrl->InsertColumn( i
			, listItems[i].colName, listItems[i].colFormat, listItems[i].colWidth
			);
		SetCtrlText( listItems[i].colCtrlID, "" );
		EnableCtrl( listItems[i].colCtrlID, false );
	}
}

//
//	VTSListCtrl::GetItemCount
//
//	In theory it would be simpler to get the item count directly from the list control, 
//	but it is better to be balanced.
//

int	VTSListCtrl::GetItemCount( void )
{
	return (listCtrl ? listCtrl->GetItemCount() : 0);
}

//
//	VTSListCtrl::EnableCtrl
//
//	The list will request that a control be enabled or disabled.  Normally this 
//	simply enables or disables the contorl, but in case there is some more 
//	processing needed, this is passed along to the derived class.
//

void VTSListCtrl::EnableCtrl( int ctrlID, int enable )
{
	TRACE2( "EnableCtrl( %d, %d )\n", ctrlID, enable );

	listWindow->GetDlgItem( ctrlID )->EnableWindow( enable );
}

//
//	VTSListCtrl::GetItemText
//
//	This function extracts the text from the list.  It is usually used in the 
//	encoding routines.  Because this uses a local static buffer, it is not 
//	thread safe.
//
//	WARNING: Do not use CListCtrl::GetItemText(r,c), it doesn't work, don't ask 
//	me why.
//

const char *VTSListCtrl::GetItemText( int row, int col )
{
	static char		buff[512]
	;
	LVITEM			itm
	;

	itm.mask = LVIF_TEXT;
	itm.iItem = row;
	itm.iSubItem = col;
	itm.pszText = buff;
	itm.cchTextMax = sizeof(buff);

	listCtrl->GetItem( &itm );

	return buff;
}

//
//	VTSListCtrl::SetCtrlText
//

void VTSListCtrl::SetCtrlText( int ctrlID, const char *text )
{
	TRACE2( "SetCtrlText( %d, '%s' )\n", ctrlID, text );
	listWindow->GetDlgItem( ctrlID )->SetWindowText( text );
}

//
//	VTSListCtrl::GetCtrlText
//

const char *VTSListCtrl::GetCtrlText( int ctrlID )
{
	static CString		rString
	;

	listWindow->GetDlgItem( ctrlID )->GetWindowText( rString );
	TRACE2( "GetCtrlText( %d ) = '%s'\n", ctrlID, rString );

	return rString.operator LPCTSTR();
}

//
//	VTSListCtrl::AddButtonClick
//

void VTSListCtrl::AddButtonClick( void )
{
	int		listLen = listCtrl->GetItemCount()
	;

	// deselect if something was selected
	POSITION selPos = listCtrl->GetFirstSelectedItemPosition();
	if (selPos != NULL) {
		int nItem = listCtrl->GetNextSelectedItem( selPos );
		listCtrl->SetItemState( nItem, 0, LVIS_SELECTED );
	}

	// create a new item
	listAddInProgress = true;
	listCtrl->InsertItem( listLen, "?2" );

	// enable the controls and empty them
	for (int i = 0; listItems[i].colCtrlID; i++) {
		// enable the control
		EnableCtrl( listItems[i].colCtrlID, true );

		// initialize it (which may not end up being an empty string)
		SetCtrlText( listItems[i].colCtrlID, "" );

		// let the virtual function return the string (it might be translated)
		listCtrl->SetItemText( listLen, i, GetCtrlText( listItems[i].colCtrlID ) );
	}

	// make sure it is visible and selected
	listCtrl->EnsureVisible( listLen, false );
	listCtrl->SetItemState( listLen
		, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED
		);

	// give the control the focus
	listCtrl->SetFocus();

	// store the item number
	listSelectedRow = listLen;
	listAddInProgress = false;

	// build the encoding
	listWindow->UpdateEncoded();
}

//
//	VTSListCtrl::RemoveButtonClick
//

void VTSListCtrl::RemoveButtonClick( void )
{
	int		listLen = listCtrl->GetItemCount()
	,		curRow = listSelectedRow
	;

	// must have a selected row
	if (curRow < 0)
		return;

	// deselect the row
	listCtrl->SetItemState( curRow, 0, LVIS_SELECTED | LVIS_FOCUSED );

	// delete the row
	listLen -= 1;
	listCtrl->DeleteItem( curRow );

	// see if the next item can be selected
	if (curRow >= listLen)
		curRow -= 1;

	if (curRow >= 0) {
		// make sure it is visible and selected
		listCtrl->EnsureVisible( curRow, false );
		listCtrl->SetItemState( curRow
			, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED
			);

		// give it the focus
		listCtrl->SetFocus();
	}

	// build the encoding
	listWindow->UpdateEncoded();}

//
//	VTSListCtrl::OnItemChanging
//

void VTSListCtrl::OnItemChanging( NMHDR *pNMHDR, LRESULT *pResult )
{
	NM_LISTVIEW*	pNMListView = (NM_LISTVIEW*)pNMHDR
	;

	// forget messages that don't change the state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	// skip messages during new item initialization
	if (listAddInProgress)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0) {
		// item becoming selected
		listSelectedRow = pNMListView->iItem;

		// enable the controls and copy the contents
		for (int i = 0; listItems[i].colCtrlID; i++) {
			EnableCtrl( listItems[i].colCtrlID, true );
			SetCtrlText( listItems[i].colCtrlID, GetItemText( listSelectedRow, i ) );
		}
	} else {
		// item no longer selected
		if (pNMListView->iItem == listSelectedRow) {
			listSelectedRow = -1;

			for (int j = 0; listItems[j].colCtrlID; j++) {
				SetCtrlText( listItems[j].colCtrlID, "" );
				EnableCtrl( listItems[j].colCtrlID, false );
			}
		}
	}
}

//
//	VTSListCtrl::OnChangeItem
//

void VTSListCtrl::OnChangeItem( int ctrlID )
{
	int			col
	;
	const char	*txt
	;

	// must have a selected row
	if (listSelectedRow < 0)
		return;

	// find the column number
	for (col = 0; listItems[col].colCtrlID != ctrlID; col++)
		;

	// control must be owned by the list
	ASSERT( listItems[col].colCtrlID != 0 );

	// get the new control text
	txt = GetCtrlText( ctrlID );

	// transfer it to the column of the current row
	listCtrl->SetItemText( listSelectedRow, col, txt );

	// build the encoding
	listWindow->UpdateEncoded();
}

/////////////////////////////////////////////////////////////////////////////

//
//	VTSStatusFlags::VTSStatusFlags
//

#define XIDC_INALARM		1130
#define XIDC_FAULT			1131
#define XIDC_OVERRIDDEN		1132
#define XIDC_OUTOFSERVICE	1133

VTSStatusFlags::VTSStatusFlags( const CWnd* wp )
	: BACnetBitString( 4 )
	, ctrlEnabled(true)
	, ctrlWindow(wp)
{
}

//
//	VTSStatusFlags::Enable
//
//	Enabling the flags means enabling all of the buttons and making their status reflect 
//	the value in the bit string.
//

void VTSStatusFlags::Enable( void )
{
	CButton	*inAlarmButton
	,		*faultButton
	,		*overriddenButton
	,		*outOfServiceButton
	;

	inAlarmButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_INALARM );
	faultButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_FAULT );
	overriddenButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_OVERRIDDEN );
	outOfServiceButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_OUTOFSERVICE );

	ctrlEnabled = true;
	inAlarmButton->EnableWindow( true );
	inAlarmButton->SetCheck( GetBit(0) );
	faultButton->EnableWindow( true );
	faultButton->SetCheck( GetBit(1) );
	overriddenButton->EnableWindow( true );
	overriddenButton->SetCheck( GetBit(2) );
	outOfServiceButton->EnableWindow( true );
	outOfServiceButton->SetCheck( GetBit(3) );
}

//
//	VTSStatusFlags::Disable
//
//	Disabling the status flags means disabling all of the buttons.  I also clear them out
//	(make sure they are unchecked) to make the display look nice.
//

void VTSStatusFlags::Disable( void )
{
	CButton	*inAlarmButton
	,		*faultButton
	,		*overriddenButton
	,		*outOfServiceButton
	;

	inAlarmButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_INALARM );
	faultButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_FAULT );
	overriddenButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_OVERRIDDEN );
	outOfServiceButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_OUTOFSERVICE );

	ctrlEnabled = false;
	inAlarmButton->SetCheck( 0 );
	inAlarmButton->EnableWindow( false );
	faultButton->SetCheck( 0 );
	faultButton->EnableWindow( false );
	overriddenButton->SetCheck( 0 );
	overriddenButton->EnableWindow( false );
	outOfServiceButton->SetCheck( 0 );
	outOfServiceButton->EnableWindow( false );
}

//
//	VTSStatusFlags::InAlarmClick
//

void VTSStatusFlags::InAlarmClick( void )
{
	CButton	*inAlarmButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_INALARM )
	;

	SetBit( 0, (inAlarmButton->GetState() & 0x0001) );
}

//
//	VTSStatusFlags::FaultClick
//

void VTSStatusFlags::FaultClick( void )
{
	CButton	*faultButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_FAULT )
	;

	SetBit( 1, (faultButton->GetState() & 0x0001) );
}

//
//	VTSStatusFlags::OverriddenClick
//

void VTSStatusFlags::OverriddenClick( void )
{
	CButton	*overriddenButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_OVERRIDDEN )
	;

	SetBit( 2, (overriddenButton->GetState() & 0x0001) );
}

//
//	VTSStatusFlags::OutOfServiceClick
//

void VTSStatusFlags::OutOfServiceClick( void )
{
	CButton	*outOfServiceButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_OUTOFSERVICE )
	;

	SetBit( 3, (outOfServiceButton->GetState() & 0x0001) );
}

//
//	VTSStatusFlags::CtrlToObj
//

void VTSStatusFlags::CtrlToObj( void )
{
	InAlarmClick();
	FaultClick();
	OverriddenClick();
	OutOfServiceClick();
}

//
//	VTSStatusFlags::ObjToCtrl
//

void VTSStatusFlags::ObjToCtrl( void )
{
	CButton	*inAlarmButton
	,		*faultButton
	,		*overriddenButton
	,		*outOfServiceButton
	;

	inAlarmButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_INALARM );
	faultButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_FAULT );
	overriddenButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_OVERRIDDEN );
	outOfServiceButton = (CButton *)ctrlWindow->GetDlgItem( XIDC_OUTOFSERVICE );

	inAlarmButton->SetCheck( GetBit(0) );
	faultButton->SetCheck( GetBit(1) );
	overriddenButton->SetCheck( GetBit(2) );
	outOfServiceButton->SetCheck( GetBit(3) );
}

//
//	VTSStatusFlags::UpdateData
//

void VTSStatusFlags::UpdateData( BOOL bCtrlToObj )
{
	if (bCtrlToObj)
		CtrlToObj();
	else
		ObjToCtrl();
}