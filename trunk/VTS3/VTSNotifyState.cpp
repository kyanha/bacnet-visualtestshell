// VTSNotifyState.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"

#include "VTSNotifyState.h"
#include "VTSNotificationParameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace NetworkSniffer {
	extern char *BACnetPropertyIdentifier[];

	extern char *BACnetBinaryPV[];
	extern char *BACnetEventType[];
	extern char *BACnetPolarity[];
	extern char *BACnetProgramRequest[];
	extern char *BACnetProgramState[];
	extern char *BACnetProgramError[];
	extern char *BACnetReliability[];
	extern char *BACnetEventState[];
	extern char *BACnetDeviceStatus[];
	extern char *BACnetEngineeringUnits[];
}

char *gBACnetBoolean[] =
	{ "False"
	, "True"
	};

VTSPropertyState gPropertyState[] =
	{ { "Boolean-value",		gBACnetBoolean,							2 }
	, { "Binary-value",			NetworkSniffer::BACnetBinaryPV,			2 }
	, { "Event-Type",			NetworkSniffer::BACnetEventType,		6 }
	, { "Polarity",				NetworkSniffer::BACnetPolarity,			2 }
	, { "Program-request",		NetworkSniffer::BACnetProgramRequest,	6 }
	, { "Program-state",		NetworkSniffer::BACnetProgramState,		6 }
	, { "Program-error",		NetworkSniffer::BACnetProgramError,		5 }
	, { "Reliabilty",			NetworkSniffer::BACnetReliability,		9 }
	, { "Event-state",			NetworkSniffer::BACnetEventState,		5 }
	, { "Device-status",		NetworkSniffer::BACnetDeviceStatus,		5 }
	, { "Engineering-units",	NetworkSniffer::BACnetEngineeringUnits,	142 }
	};

/////////////////////////////////////////////////////////////////////////////
// VTSNotifyState property page

IMPLEMENT_DYNCREATE(VTSNotifyState, CPropertyPage)

#pragma warning( disable : 4355 )
VTSNotifyState::VTSNotifyState()
	: CPropertyPage(VTSNotifyState::IDD)
	, m_StatusFlags( this )
{
	//{{AFX_DATA_INIT(VTSNotifyState)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// init the values, "wizard" locked up controls
	m_Type = 0;
	m_Value = 0;
}
#pragma warning( default : 4355 )

VTSNotifyState::~VTSNotifyState()
{
}

void VTSNotifyState::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSNotifyState)
	DDX_Control(pDX, IDC_VALUECOMBO, m_ValueCombo);
	DDX_Control(pDX, IDC_TYPECOMBO, m_TypeCombo);
	//}}AFX_DATA_MAP

	m_StatusFlags.UpdateData( pDX->m_bSaveAndValidate );
}

BEGIN_MESSAGE_MAP(VTSNotifyState, CPropertyPage)
	//{{AFX_MSG_MAP(VTSNotifyState)
	ON_BN_CLICKED(IDC_INALARM, OnInAlarm)
	ON_BN_CLICKED(IDC_FAULT, OnFault)
	ON_BN_CLICKED(IDC_OVERRIDDEN, OnOverridden)
	ON_BN_CLICKED(IDC_OUTOFSERVICE, OnOutOfService)
	ON_CBN_SELCHANGE(IDC_TYPECOMBO, OnSelchangeTypeCombo)
	ON_CBN_SELCHANGE(IDC_VALUECOMBO, OnSelchangeValueCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	VTSNotifyState::OnInitDialog
//

void SetDropDownSize( CComboBox& box, UINT lines );

BOOL VTSNotifyState::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// initialize the type combo
	for (int i = 0; i < sizeof(gPropertyState)/sizeof(VTSPropertyState); i++)
		m_TypeCombo.AddString( gPropertyState[i].psName );
	m_TypeCombo.SetCurSel( m_Type );
	SetDropDownSize( m_TypeCombo, 8 );

	// initialize the value combo
	for (int j = 0; j < gPropertyState[m_Type].psTableLen; j++)
		m_ValueCombo.AddString( gPropertyState[m_Type].psTable[j] );
	m_ValueCombo.SetCurSel( m_Value );
	SetDropDownSize( m_ValueCombo, 8 );

	return TRUE;
}

//
//	VTSNotifyState::Encode
//

void VTSNotifyState::Encode( BACnetAPDUEncoder& enc, int context )
{
	BACnetOpeningTag().Encode( enc, 1 );

	// open the property state context
	BACnetOpeningTag().Encode( enc, 0 );

	// encode the value based on the type
	BACnetEnumerated( m_Value ).Encode( enc, m_Type );

	// close off that context
	BACnetClosingTag().Encode( enc, 0 );

	// status flags are simple
	m_StatusFlags.Encode( enc, 1 );

	BACnetClosingTag().Encode( enc, 1 );
}

//
//	VTSNotifyState::OnSetActive
//

BOOL VTSNotifyState::OnSetActive() 
{
	pageParent->m_EventType = 1;
	return CPropertyPage::OnSetActive();
}

//
//	VTSNotifyState::OnChangeX
//
//	The following set of functions are called when one of the interface elements
//	has changed.
//

void VTSNotifyState::OnSelchangeTypeCombo() 
{
	int		newSel = m_TypeCombo.GetCurSel()
	,		curLen = m_ValueCombo.GetCount()
	;

	// remove the old values
	while (m_ValueCombo.GetCount() != 0)
		m_ValueCombo.DeleteString( 0 );

	// make the new values available
	for (int j = 0; j < gPropertyState[newSel].psTableLen; j++)
		m_ValueCombo.AddString( gPropertyState[newSel].psTable[j] );

	// make the first one the current selection
	m_Value = 0;
	m_ValueCombo.SetCurSel( 0 );

	// save the new selection
	m_Type = newSel;
}

void VTSNotifyState::OnSelchangeValueCombo() 
{
	// save the new selection
	m_Value = m_ValueCombo.GetCurSel();
}

void VTSNotifyState::OnInAlarm() 
{
	m_StatusFlags.InAlarmClick();
}

void VTSNotifyState::OnFault() 
{
	m_StatusFlags.FaultClick();
}

void VTSNotifyState::OnOverridden() 
{
	m_StatusFlags.OverriddenClick();
}

void VTSNotifyState::OnOutOfService() 
{
	m_StatusFlags.OutOfServiceClick();
}
