// VTSAny.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"
#include "VTSCtrl.h"
#include "VTSAny.h"

#include "VTSObjectIdentifierDialog.h"

#include "VTSDateTimeDlg.h"
#include "VTSDateRangeDlg.h"
#include "VTSAddressBindingDlg.h"
#include "VTSCalendarEntryDlg.h"

#include "VTSActionDlg.h"
#include "VTSActionCommandDlg.h"
#include "VTSActionListDlg.h"
#include "VTSBinaryPVDlg.h"
#include "VTSClientCOVDlg.h"
#include "VTSCOVSubscriptionDlg.h"
#include "VTSDailyScheduleDlg.h"
#include "VTSDaysOfWeekDlg.h"
#include "VTSDestinationDlg.h"
#include "VTSDeviceStatusDlg.h"
#include "VTSDeviceObjectPropertyReferenceDlg.h"
#include "VTSDeviceObjectReferenceDlg.h"
#include "VTSEngineeringUnitsDlg.h"
#include "VTSEventParameterDlg.h"
#include "VTSEventStateDlg.h"
#include "VTSEventTransitionBitsDlg.h"
#include "VTSEventTypeDlg.h"
#include "VTSFileAccessMethodDlg.h"
#include "VTSLifeSafetyModeDlg.h"
#include "VTSLifeSafetyOperationDlg.h"
#include "VTSLifeSafetyStateDlg.h"
#include "VTSLimitEnableDlg.h"
#include "VTSLogRecordDlg.h"
#include "VTSLogStatusDlg.h"
#include "vtsmaintenancedlg.h"
#include "VTSNotifyTypeDlg.h"
#include "VTSObjectPropertyReferenceDlg.h"
#include "VTSObjectPropertyValueDlg.h"
#include "VTSObjectTypeDlg.h"
#include "VTSObjectTypesSupportedDlg.h"
#include "VTSPropertyValueDlg.h"
#include "VTSPolarityDlg.h"
#include "VTSPriorityArrayDlg.h"
#include "VTSPriorityValueDlg.h"
#include "VTSProgramErrorDlg.h"
#include "VTSProgramRequestDlg.h"
#include "VTSProgramStateDlg.h"
#include "VTSPropertyIdentifierDlg.h"
#include "VTSPropertyReferenceDlg.h"
#include "VTSPropertyStatesDlg.h"
#include "VTSRecipientDlg.h"
#include "VTSRecipientProcess.h"
#include "VTSReliabilityDlg.h"
#include "VTSResultFlasDlg.h"
#include "VTSSegmentationDlg.h"
#include "VTSServicesSupportedDlg.h"
#include "VTSSessionKeyDlg.h"
#include "VTSSetPointReferenceDlg.h"
#include "VTSSilenceStateDlg.h"
#include "VTSSpecialEventDlg.h"
#include "VTSStatusFlagsDlg.h"
#include "VTSTimeStampDlg.h"
#include "VTSTimeValueDlg.h"
#include "VTSVTClassDlg.h"
#include "VTSVTSessionDlg.h"
#include "VTSWeekNDayDlg.h"

#include "VTSListOfDestinationDlg.h"
#include "VTSListOfReadAccessResultDlg.h"
#include "VTSListOfReadAccessSpecDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	VTSAnyList::VTSAnyList
//

VTSAnyList::VTSAnyList( void )
{
}

//
//	VTSAnyList::~VTSAnyList
//

VTSAnyList::~VTSAnyList( void )
{
	for (POSITION pos = GetHeadPosition(); pos; )
		delete (VTSAnyElementPtr)GetNext( pos );
}

//
//	VTSAnyList::Add
//

void VTSAnyList::Add( void )
{
	VTSAnyElementPtr	cur
	;

	// create a new element
	cur = new VTSAnyElement;

	// initialize the type and context
	cur->elemType = 0;
	cur->elemContext = -1;

	// encode a null
	BACnetNull().Encode( cur->elemEncoder );

	// add it to our list of ports
	AddTail( cur );
}

//
//	VTSAnyList::Remove
//

void VTSAnyList::Remove( int i )
{
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	delete (VTSAnyElementPtr)GetAt( pos );
	RemoveAt( pos );
}

//
//	VTSAnyList::Length
//

int VTSAnyList::Length( void )
{
	return CList<VTSAnyElementPtr,VTSAnyElementPtr>::GetCount();
}

//
//	VTSAnyList::operator []
//

VTSAnyElementPtr VTSAnyList::operator []( int i )
{
	POSITION	pos = FindIndex( i )
	;

	ASSERT( pos != NULL );
	return (VTSAnyElementPtr)GetAt( pos );
}

/////////////////////////////////////////////////////////////////////////////
// VTSAny dialog

VTSAny::VTSAny(CWnd* pParent /*=NULL*/)
	: CDialog(VTSAny::IDD, pParent)
{
	//{{AFX_DATA_INIT(VTSAny)
	//}}AFX_DATA_INIT
}

//Xiao Shiyuan 2002-12-3
VTSAny::VTSAny(const VTSAny& any)
{   
}

//Xiao Shiyuan 2002-12-3
VTSAny& VTSAny::operator = (const VTSAny& any)
{
   if(this != &any)
   {
	   for (POSITION pos = m_anyList.GetHeadPosition(); pos; )
		delete (VTSAnyElementPtr)m_anyList.GetNext( pos );
 
	   m_anyList.RemoveAll();	   

	   for(int index = 0; index < any.m_anyList.GetCount(); index++)
	   {		    
			VTSAnyElementPtr old_elemPtr = any.m_anyList.GetAt(any.m_anyList.FindIndex(index));
			VTSAnyElementPtr elemPtr = new VTSAnyElement();
			
            elemPtr->elemContext = old_elemPtr->elemContext;
			elemPtr->elemType = old_elemPtr->elemType;	
			elemPtr->elemEncoder = old_elemPtr->elemEncoder;
			
			m_anyList.AddTail(elemPtr);
		}
	    
		m_hWndOwner = any.m_hWndOwner;
		m_iSelectedElem = any.m_iSelectedElem;
   }   

   return *this;
}

void VTSAny::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSAny)
	DDX_Control(pDX, IDC_ELEMLIST, m_ElemList);
	DDX_Control(pDX, IDC_TYPECOMBO, m_TypeCombo);
	DDX_Control(pDX, IDC_CONTEXT, m_Context);
	DDX_Control(pDX, IDC_VALUE, m_Value);
	DDX_Control(pDX, IDC_VALUEIDBTN, m_ValueIDButton);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(VTSAny, CDialog)
	//{{AFX_MSG_MAP(VTSAny)
	ON_BN_CLICKED(IDC_ADDELEM, OnAddElem)
	ON_BN_CLICKED(IDC_REMOVEELEM, OnRemoveElem)
	ON_CBN_SELCHANGE(IDC_TYPECOMBO, OnSelchangeTypeCombo)
	ON_EN_CHANGE(IDC_CONTEXT, OnChangeContext)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_ELEMLIST, OnItemchangingElemList)
	ON_BN_CLICKED(IDC_VALUEIDBTN, OnValueIDButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSAny message handlers

BOOL VTSAny::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int		elemType = m_TypeCombo.GetCurSel();	
	// initialize the port list
	m_ElemList.m_nFlags |= LVS_SINGLESEL;
	m_ElemList.InsertColumn( 0, _T("Type"), LVCFMT_LEFT, 96 );
	m_ElemList.InsertColumn( 1, _T("Context"), LVCFMT_LEFT, 48 );
	m_ElemList.InsertColumn( 2, _T("Data"), LVCFMT_LEFT, 96 );

	// fill out the control with stuff that may already be encoded
	for (int i = 0; i < m_anyList.Length(); i++) {
		m_ElemList.InsertItem( i, (LPCTSTR)"(placeholder)" );
		ElemToList( i );
	}

	// make sure nothing is selected
	ResetSelection();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//
//	VTSAny::SetSelection
//

void VTSAny::SetSelection( int indx )
{
	char				buff[16]
	;
	VTSAnyElementPtr	curElem
	;

	// if something is selected and its not this one, reset
	if ((m_iSelectedElem >= 0) && (indx != m_iSelectedElem))
		ResetSelection();

	// get a pointer to the port
	curElem = m_anyList[indx];

	// set the type
	m_TypeCombo.SetCurSel( curElem->elemType );
	// enable/disable the ID button
	m_ValueIDButton.EnableWindow( curElem->elemType == 12 );

	// set the context
	buff[0] = 0;
	if (curElem->elemContext >= 0)
		sprintf( buff, "%d", curElem->elemContext );
	m_Context.SetWindowText( buff );

	// decode into m_Value based on the type
	DecodeValue( indx );

	// set the local variables to the port information
	m_iSelectedElem = indx;

	// make sure the controls are properly enabled
	SynchronizeControls();

	// let the CDialog sync the controls with the local vars
	UpdateData( false );
}

//
//	VTSAny::ResetSelection
//

void VTSAny::ResetSelection( void )
{
	// nothing selected, just return
	if (m_iSelectedElem < 0)
		return;

	// clear out the contents of the local vars
	m_iSelectedElem = -1;

	m_TypeCombo.SetCurSel( -1 );
	m_Context.SetWindowText( _T("") );
	m_Value.SetWindowText( _T("") );

	// disable the ID button
	m_ValueIDButton.EnableWindow( false );
	

	// let the CDialog sync the controls with the local vars
	UpdateData( false );

	// make sure the controls are properly disabled
	SynchronizeControls();
}

//
//	VTSAny::SynchronizeControls
//

void VTSAny::SynchronizeControls( void )
{
	// disable if nothing selected
	int elemType ;
	if(m_iSelectedElem >= 0)
		elemType= m_anyList[m_iSelectedElem]->elemType;
	GetDlgItem( IDC_TYPECOMBO )->EnableWindow( m_iSelectedElem >= 0 );
	GetDlgItem( IDC_CONTEXT )->EnableWindow( m_iSelectedElem >= 0 && elemType<15);
	if (m_iSelectedElem < 0)
		GetDlgItem( IDC_VALUE )->EnableWindow( false );
	else {
		GetDlgItem( IDC_VALUE )->EnableWindow( (elemType != 0) && (elemType != 13) && (elemType != 14)&&(elemType<15));
		m_ValueIDButton.EnableWindow( elemType == 12 
									  ||elemType>=15);
	}
}

void VTSAny::OnAddElem() 
{
	int					i
	;

	// add an element to the list
	m_anyList.Add();

	// get its index, it will always be the last one
	i = m_anyList.Length() - 1;

	// add an item to the list and sync it
	m_ElemList.InsertItem( i, (LPCTSTR)"" );
	ElemToList( i );

	// make sure the record is visible and selected
	m_ElemList.EnsureVisible( i, false );
	m_ElemList.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );

	// make sure it's selected
	SetSelection( i );
}

void VTSAny::OnRemoveElem() 
{
	// if nothing selected, return
	if (m_iSelectedElem < 0)
		return;

	// remove the selected element
	m_anyList.Remove( m_iSelectedElem );

	// remove it from the control
	m_ElemList.DeleteItem( m_iSelectedElem );
	// reset the selection
	m_ElemList.SetItemState( m_iSelectedElem - 1 < 0 ? 0 : m_iSelectedElem - 1, LVIS_SELECTED, LVIS_SELECTED );
}

void VTSAny::OnSelchangeTypeCombo() 
{
	int		elemType = m_TypeCombo.GetCurSel()
	;

	// skip changes when there is no selection
	if (m_iSelectedElem < 0)
		return;
	// set the list type
	m_anyList[m_iSelectedElem]->elemType = elemType;

	// synchronize the enable/disable state of the value and id button
	SynchronizeControls();

	// if the element type doesn't have a value, clear the field
	if ((elemType == 0) || (elemType == 13) || (elemType == 15))
		m_Value.SetWindowText( _T("") );

	// update the encoding of m_Value in the new context
	EncodeValue();

	// sync the control
	ElemToList( m_iSelectedElem );
}

void VTSAny::OnChangeContext() 
{
	// skip changes when there is no selection
	if (m_iSelectedElem < 0)
		return;

	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	m_Context.GetWindowText( str );
	s = str.operator LPCTSTR();

	// if no data available, set to no context
	if (!*s)
		m_anyList[m_iSelectedElem]->elemContext = -1;
	else {
		int		intValue
		;

		// check for a sign to ignore
		if (*s == '+')
			s += 1;

		// check for hex notation
		if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) {
			s += 2;
			for (intValue = 0; isxdigit(*s); s++)
				intValue = (intValue << 4) + (isdigit(*s) ? (*s - '0') : (toupper(*s) - 'A' + 10));
		} else {
			for (intValue = 0; isdigit(*s); s++)
				intValue = (intValue *10) + (*s - '0');
		}

		// set the context
		m_anyList[m_iSelectedElem]->elemContext = intValue;
	}

	// update the encoding of m_Value in the new context
	EncodeValue();

	// sync the control
	ElemToList( m_iSelectedElem );
}

void VTSAny::OnChangeValue() 
{
	// skip changes when there is no selection
	if (m_iSelectedElem < 0)
		return;

	// update the encoding of the new m_Value in the current context
	EncodeValue();

	// sync the control
	ElemToList( m_iSelectedElem );
}

void VTSAny::OnValueIDButton() 
{
	// make sure there is a selected element
	if (m_iSelectedElem < 0)
		return;

		VTSAnyElementPtr	curElem = m_anyList[m_iSelectedElem];

	BACnetAPDUDecoder	dec( curElem->elemEncoder.pktBuffer, curElem->elemEncoder.pktLength );
	curElem->elemEncoder.Flush();
	
	// depending on the type, encode the value
	switch (curElem->elemType) 
	{
		case 12:
		{
			// bind an object identifier to the value
			VTSObjectIdentifierCtrl m_ObjID( this, IDC_VALUE );

			// translate the current contents
			m_ObjID.CtrlToObj();

			VTSObjectIdentifierDialog	dlg;

			dlg.objID = m_ObjID.objID;
			if (dlg.DoModal() && dlg.validObjID) {
				// value change successful
				m_ObjID.ctrlNull = false;
				m_ObjID.objID = dlg.objID;
				m_ObjID.ObjToCtrl();
				
				//Deleted by Yajun Zhou, 2002-9-3
				//m_ObjID.Encode( m_anyList[m_iSelectedElem]->elemEncoder
				//	, m_anyList[m_iSelectedElem]->elemContext
				//	);
				////////////////////////////////////
			}	
			break;
		}
		case 15://date time
		{
			VTSDateTimeDlg dlg(this);
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
		
			break;
		}
		case 16://date range
		{
			VTSDateRangeDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 17://address binding
		{
			VTSAddressBindingDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 18: //calendar entry
		{
			VTSCalendarEntryDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, dlg.m_nChoice );
			}
			break;
		}
	//MOdified by Zhu Zhenhua , 2004-6-14
case 19: //action
		{
			VTSActionDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
case 20: //actionCommand
		{
			VTSActionCommandDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
case 21: //actionList
		{
			VTSActionListDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
case 22: //BinaryPV
		{
			VTSBinaryPVDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 23: //Client COV
		{
			VTSClientCOVDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 24: //COVSubscription
		{
			VTSCOVSubscriptionDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
case 25: //dailySchedule
		{
			VTSDailyScheduleDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
case 26: //daysofWeek
		{
			VTSDaysOfWeekDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
case 27: //destination
		{
			VTSDestinationDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 28://Device Status
		{	
			VTSDeviceStatusDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 29://DeviceObjectPropertyReference
		{	
			VTSDeviceObjectPropertyReferenceDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 30://DeviceObjectReference
		{	
			VTSDeviceObjectReferenceDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
	case 31://EngineeringUnits
		{	
			VTSEngineeringUnitsDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 32://EventParameter
		{	

			VTSEventParameterDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			
			break;
		}
	case 33://EventState
		{	
			VTSEventStateDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 34://EventTransitionBits
		{	
			VTSEventTransitionBitsDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 35://EventType
		{	
			VTSEventTypeDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 36://FileAccessMethod
		{	
			VTSFileAccessMethodDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 37://LifeSafetyMode
		{	
			VTSLifeSafetyModeDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 38://LifeSafetyOperation
		{	
			VTSLifeSafetyOperationDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 39://LifeSafetyState
		{	
			VTSLifeSafetyStateDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 40://LimitEnable
		{	
			VTSLimitEnableDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 41://LogRecord
		{	
			VTSLogRecordDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 42://LogStatus
		{	
			VTSLogStatusDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
	case 43://Maintenance
		{	
			VTSMaintenanceDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 44://NotifyType
		{	
			VTSNotifyTypeDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 45://ObjectPropertyReference
		{	
			VTSObjectPropertyReferenceDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
	case 46://ObjectPropertyValue
		{	
			VTSObjectPropertyValueDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 47://ObjecTtype
		{	
			VTSObjectTypeDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 48://ObectTypesSupported
		{	
			VTSObjectTypesSupportedDlg dlg(this);
			try{
				dlg.Decode(dec);
				}
			catch(...){
				MessageBox("Decode Error!");
				break;
				}
			if(dlg.DoModal())
				{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
				}		
			break;
		}
	case 49://Polarity 
		{	
			VTSPolarityDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 50://PriorityArray
		{	
			VTSPriorityArrayDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 51://PriorityValue
		{	
			VTSPriorityValueDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 52://ProgramError   
		{	
			VTSProgramErrorDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 53://ProgramRequest 
		{	
			VTSProgramRequestDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 54://ProgramState
		{	
			VTSProgramStateDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 55://PropertyIdentifier  
		{	
			VTSPropertyIdentifierDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 56://PropertyReference
		{	
			VTSPropertyReferenceDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 57://PropertyStates 
		{	
			VTSPropertyStatesDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 58://PropertyValue
		{	
			VTSPropertyValueDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 59://Recipient
		{	
			VTSRecipientDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
	case 60://RecipientProcess
		{	
			VTSRecipientProcess dlg(this);
			try{
				dlg.DecodeValue(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				try{
					dlg.EncodeValue( curElem->elemEncoder, curElem->elemContext );
				}
				catch(...){
					MessageBox("Encode Error!");
					break;
				}	
			}
			break;
		}
		case 61://Reliability 
		{	
			VTSReliabilityDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
	case 62://ResultFlags
		{	
			VTSResultFlasDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
		case 63://Segmentation 
		{	
			VTSSegmentationDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 64://ServicesSupported 
		{	
			VTSServicesSupportedDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			
			break;
		}
	case 65://SessionKey
		{	
			VTSSessionKeyDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
		case 66://SetPointReference
		{	
			VTSSetPointReferenceDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}		
		case 67://SilenceState  
		{	
			VTSSilenceStateDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 68://SpecialEvent   
		{				
			VTSSpecialEventDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}			
			break;
		}
		case 69://StatusFlags
		{	
			VTSStatusFlagsDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}		
		case 70: //time stamp
		{
			VTSTimeStampDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, dlg.m_nChoice );
			}
			break;
		}
		case 71: //time Value
		{
			VTSTimeValueDlg dlg(this);
			
			try{
			dlg.Decode(dec);
			}
			catch (...) {
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder,  curElem->elemContext);
			}
			break;
		}
	case 72://VTClass 
		{	
			VTSVTClassDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
	case 73://VTSession
		{	
			VTSVTSessionDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 74://WeekNDay
		{	
			VTSWeekNDayDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
	case 75://list of Destination
		{	
			VTSListOfDestinationDlg dlg(0, this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}
		case 76://List Of ReadAccessResult
			{	
			VTSListOfReadAccessResultDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}	
		case 77://List Of ReadAccessSpecification
			{	
			VTSListOfReadAccessSpecDlg dlg(this);
			try{
				dlg.Decode(dec);
			}
			catch(...){
				MessageBox("Decode Error!");
				break;
			}
			if(dlg.DoModal())
			{
				dlg.Encode( curElem->elemEncoder, curElem->elemContext );
			}
			break;
		}						
		default:
			break;
	}
	ElemToList( m_iSelectedElem );
}

void VTSAny::OnItemchangingElemList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// forget messages that dont change the state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0)
		SetSelection( pNMListView->iItem );	
	else
	if (pNMListView->iItem == m_iSelectedElem)
		ResetSelection();

	*pResult = 0;
}

//
//	VTSAny::ElemToList
//

void VTSAny::ElemToList( int indx )
{
	VTSAnyElementPtr	curElem = m_anyList[indx]
	;
	CString				typeText
	;

	m_TypeCombo.GetLBText( curElem->elemType, typeText );
	m_ElemList.SetItemText( indx, 0, typeText );

	if (curElem->elemContext < 0)
		m_ElemList.SetItemText( indx, 1, (LPCTSTR)"" );
	else {
		typeText.Format( "%d", curElem->elemContext );
		m_ElemList.SetItemText( indx, 1, typeText );
	}

	EncoderToHex( curElem->elemEncoder, typeText );
	m_ElemList.SetItemText( indx, 2, typeText );
}

//
//	VTSAny::Encode
//
//	A simplified way of copying the contents of the list elements into 
//	the encoder.  This simplifies EncodePage() functions.
//

void VTSAny::Encode( BACnetAPDUEncoder& enc, int context )
{
	if (context != kAppContext)
		BACnetOpeningTag().Encode( enc, context );

	for (int i = 0; i < m_anyList.Length(); i++) {
		BACnetAPDUEncoderPtr ep = &m_anyList[i]->elemEncoder;
		enc.Append( ep->pktBuffer, ep->pktLength );
	}

	if (context != kAppContext)
		BACnetClosingTag().Encode( enc, context );
}

//
//	EncoderToHex
//

void EncoderToHex( const BACnetAPDUEncoder &enc, CString &str )
{
	char	buff[4]
	;

	// flush what's there
	str.Empty();

	// translate the data
	for (int i = 0; i < enc.pktLength; i++) {
		sprintf( buff, "%02X", enc.pktBuffer[i] );
		str.Insert( i * 2, buff );
	}
}

//
//	VTSAny::EncodeValue
//

void VTSAny::EncodeValue( void )
{
	VTSAnyElementPtr	curElem = m_anyList[m_iSelectedElem]
	;

	// dump the current contents
	curElem->elemEncoder.Flush();

	// depending on the type, encode the value
	switch (curElem->elemType) {
		case 0: {
					BACnetNull().Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 1: {
					VTSBooleanCtrl m_Boolean( this, IDC_VALUE )
					;

					m_Boolean.CtrlToObj();
					m_Boolean.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 2: {
					VTSUnsignedCtrl m_Unsigned( this, IDC_VALUE )
					;

					m_Unsigned.CtrlToObj();
					m_Unsigned.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 3: {
					VTSIntegerCtrl m_Integer( this, IDC_VALUE )
					;

					m_Integer.CtrlToObj();
					m_Integer.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 4: {
					VTSRealCtrl m_Real( this, IDC_VALUE )
					;

					m_Real.CtrlToObj();
					m_Real.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 5: {
					VTSDoubleCtrl m_Double( this, IDC_VALUE )
					;

					m_Double.CtrlToObj();
					m_Double.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 6: {
					VTSOctetStringCtrl m_OctetStr( this, IDC_VALUE )
					;

					m_OctetStr.CtrlToObj();
					m_OctetStr.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 7: {
					VTSCharacterStringCtrl m_CharStr( this, IDC_VALUE )
					;

					m_CharStr.CtrlToObj();
					m_CharStr.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 8: {
					VTSBitStringCtrl m_BitStr( this, IDC_VALUE )
					;

					m_BitStr.CtrlToObj();
					m_BitStr.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 9: {
					VTSEnumeratedCtrl m_Enumerated( this, IDC_VALUE, 0, 0 )
					;

					m_Enumerated.CtrlToObj();
					m_Enumerated.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 10: {
					VTSDateCtrl m_Date( this, IDC_VALUE )
					;

					m_Date.CtrlToObj();
					m_Date.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 11: {
					VTSTimeCtrl m_Time( this, IDC_VALUE )
					;

					m_Time.CtrlToObj();
					m_Time.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 12: {
					VTSObjectIdentifierCtrl m_ObjID( this, IDC_VALUE )
					;

					m_ObjID.CtrlToObj();
					m_ObjID.Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 13: {
					BACnetOpeningTag().Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
		case 14: {
					BACnetClosingTag().Encode( curElem->elemEncoder, curElem->elemContext );
					break;
				}
	}
}

//
//	VTSAny::DecodeValue
//

void VTSAny::DecodeValue( int indx )
{
	VTSAnyElementPtr	curElem = m_anyList[indx]
	;
	BACnetAPDUDecoder	dec( curElem->elemEncoder.pktBuffer, curElem->elemEncoder.pktLength )
	;

	// depending on the type, decode the value
	switch (curElem->elemType) {
		case 0: {
					m_Value.SetWindowText( _T("") );
					break;
				}
		case 1: {
					VTSBooleanCtrl m_Boolean( this, IDC_VALUE )
					;

					m_Boolean.Decode( dec );
					m_Boolean.ctrlNull = false;
					m_Boolean.ObjToCtrl();
					break;
				}
		case 2: {
					VTSUnsignedCtrl m_Unsigned( this, IDC_VALUE )
					;

					m_Unsigned.Decode( dec );
					m_Unsigned.ctrlNull = false;
					m_Unsigned.ObjToCtrl();
					break;
				}
		case 3: {
					VTSIntegerCtrl m_Integer( this, IDC_VALUE )
					;

					m_Integer.Decode( dec );
					m_Integer.ctrlNull = false;
					m_Integer.ObjToCtrl();
					break;
				}
		case 4: {
					VTSRealCtrl m_Real( this, IDC_VALUE )
					;

					m_Real.Decode( dec );
					m_Real.ctrlNull = false;
					m_Real.ObjToCtrl();
					break;
				}
		case 5: {
					VTSDoubleCtrl m_Double( this, IDC_VALUE )
					;

					m_Double.Decode( dec );
					m_Double.ctrlNull = false;
					m_Double.ObjToCtrl();
					break;
				}
		case 6: {
					VTSOctetStringCtrl m_OctetStr( this, IDC_VALUE )
					;

					m_OctetStr.Decode( dec );
					m_OctetStr.ctrlNull = false;
					m_OctetStr.ObjToCtrl();
					break;
				}
		case 7: {
					VTSCharacterStringCtrl m_CharStr( this, IDC_VALUE )
					;

					m_CharStr.Decode( dec );
					m_CharStr.ctrlNull = false;
					m_CharStr.ObjToCtrl();
					break;
				}
		case 8: {
					VTSBitStringCtrl m_BitStr( this, IDC_VALUE )
					;

					m_BitStr.Decode( dec );
					m_BitStr.ctrlNull = false;
					m_BitStr.ObjToCtrl();
					break;
				}
		case 9: {
					VTSEnumeratedCtrl m_Enumerated( this, IDC_VALUE, 0, 0 )
					;

					m_Enumerated.Decode( dec );
					m_Enumerated.ctrlNull = false;
					m_Enumerated.ObjToCtrl();
					break;
				}
		case 10: {
					VTSDateCtrl m_Date( this, IDC_VALUE )
					;

					m_Date.Decode( dec );
					m_Date.ctrlNull = false;
					m_Date.ObjToCtrl();
					break;
				}
		case 11: {
					VTSTimeCtrl m_Time( this, IDC_VALUE )
					;

					m_Time.Decode( dec );
					m_Time.ctrlNull = false;
					m_Time.ObjToCtrl();
					break;
				}
		case 12: {
					VTSObjectIdentifierCtrl m_ObjID( this, IDC_VALUE )
					;

					m_ObjID.Decode( dec );
					m_ObjID.ctrlNull = false;
					m_ObjID.ObjToCtrl();
					break;
				}
		case 13: {
					m_Value.SetWindowText( _T("") );
					break;
				}
		case 14: {
					m_Value.SetWindowText( _T("") );
					break;
				}
	}
}

//
//	VTSAny::SaveCtrl
//
//	### This should generate an integer count of the number of elements in the list, followed 
//	by the components of each element.  Or a more elegant solution is to put in an opening tag, 
//	each of the elements, and a closing tag.
//

void VTSAny::SaveCtrl( BACnetAPDUEncoder& enc )
{
//	TRACE0( "VTSAny::SaveCtrl\n" );

	BACnetNull().Encode( enc );
}

//
//	VTSAny::RestoreCtrl
//

void VTSAny::RestoreCtrl( BACnetAPDUDecoder& dec )
{
//	TRACE0( "VTSAny::RestoreCtrl\n" );

	BACnetNull().Decode( dec );
}

