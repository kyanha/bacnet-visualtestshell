// SendReadRange.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"

#include "Send.h"
#include "SendReadRange.h"

#include "VTSObjectIdentifierDialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace NetworkSniffer 
{
	extern char *BACnetPropertyIdentifier[];
}

BACnetAPDUEncoder SendReadRange::pageContents;

/////////////////////////////////////////////////////////////////////////////
// SendReadRange property page

IMPLEMENT_DYNCREATE(SendReadRange, CPropertyPage)

SendReadRange::SendReadRange(void)
 : CSendPage(SendReadRange::IDD)
	, m_ObjectID( this, IDC_OBJECTID )
	, m_PropCombo( this, IDC_PROPCOMBO, NetworkSniffer::BACnetPropertyIdentifier, MAX_PROP_ID, true )
	, m_ArrayIndex( this, IDC_ARRAYINDEX )
    , m_ReadRangeStartDate( this, IDC_STARTDATUM )
	, m_ReadRangeEndDate( this, IDC_ENDDATUM )
    , m_ReadRangeStartTime( this, IDC_TIMESTART )
	, m_ReadRangeEndTime( this, IDC_ENDTIME )
    , m_ReadRangeCount( this, IDC_COUNT )
  	, m_ReadRangePosRef( this, IDC_REFINDEX )
	, m_RadioChoice(0)
{
	//{{AFX_DATA_INIT(SendReadRange)
//	m_RadioChoice = -1;
	//}}AFX_DATA_INIT
}

void SendReadRange::DoDataExchange(CDataExchange* pDX)
{
	TRACE1( "SendReadRange::DoDataExchange(%d)\n", pDX->m_bSaveAndValidate );

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SendReadRange)
	DDX_Radio(pDX, IDC_RADIONONE, m_RadioChoice);
	//}}AFX_DATA_MAP

	m_ObjectID.UpdateData( pDX->m_bSaveAndValidate );
	m_PropCombo.UpdateData( pDX->m_bSaveAndValidate );
	m_ArrayIndex.UpdateData( pDX->m_bSaveAndValidate );

}

BEGIN_MESSAGE_MAP(SendReadRange, CPropertyPage)
	//{{AFX_MSG_MAP(SendReadRange)
	ON_EN_CHANGE(IDC_ARRAYINDEX, OnChangeArrayindex)
	ON_EN_CHANGE(IDC_ENDDATUM, OnChangeEnddatum)
	ON_BN_CLICKED(IDC_OBJECTIDBTN, OnObjectidbtn)
	ON_CBN_SELCHANGE(IDC_PROPCOMBO, OnSelchangePropcombo)
	ON_BN_CLICKED(IDC_RADIONONE, OnRadionone)
	ON_BN_CLICKED(IDC_RADIOPOSITION, OnRadioposition)
	ON_BN_CLICKED(IDC_RADIOTIME, OnRadiotime)
	ON_BN_CLICKED(IDC_RADIOTIMERANGE, OnRadiotimerange)
	ON_EN_CHANGE(IDC_OBJECTID, OnChangeObjectid)
	ON_EN_SETFOCUS(IDC_ENDDATUM, OnSetfocusEnddatum)
	ON_EN_CHANGE(IDC_STARTDATUM, OnChangeStartdatum)
	ON_EN_SETFOCUS(IDC_STARTDATUM, OnSetfocusStartdatum)
	ON_EN_CHANGE(IDC_TIMESTART, OnChangeTimestart)
	ON_EN_CHANGE(IDC_ENDTIME, OnChangeEndtime)
	ON_EN_CHANGE(IDC_REFINDEX, OnChangePosRef)
	ON_EN_CHANGE(IDC_COUNT, OnChangeCount)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SendReadRange message handlers

BOOL SendReadRange::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// load the enumeration table
	m_PropCombo.LoadCombo();

    m_ReadRangeStartDate.Disable();
	m_ReadRangeEndDate.Disable();
    m_ReadRangeStartTime.Disable();
	m_ReadRangeEndTime.Disable();
	m_ReadRangeCount.Disable();
	m_ReadRangePosRef.Disable();

	
	UpdateData();
//    SynchronizeControls();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//
//	SendReadRange::InitPage
//

void SendReadRange::InitPage( void )
{
	// flush the data
	m_ObjectID.ctrlNull = true;
	m_PropCombo.ctrlNull = true;
	m_ArrayIndex.ctrlNull = true;

}
void SendReadRange::SynchronizeControls()
{
	GetDlgItem( IDC_STARTDATUM )->EnableWindow( !m_RadioChoice );
	GetDlgItem( IDC_ENDDATUM )->EnableWindow( !m_RadioChoice );
	GetDlgItem( IDC_TIMESTART )->EnableWindow( !m_RadioChoice );
	GetDlgItem( IDC_ENDTIME )->EnableWindow( !m_RadioChoice );
	GetDlgItem( IDC_COUNT )->EnableWindow( !m_RadioChoice );
	GetDlgItem( IDC_REFINDEX )->EnableWindow( !m_RadioChoice );
}
//
//	SendReadRange::EncodePage
//

void SendReadRange::EncodePage( CByteArray* contents )
{
	CByteArray			header
	;
	BACnetAPDUEncoder	enc
	;

	// encode the service choice
	header.Add( 0x1A );

	// encode the object ID
	if (m_ObjectID.ctrlNull)
		throw "Object ID required";
	m_ObjectID.Encode( enc, 0 );

	// encode the property
	if (m_PropCombo.ctrlNull)
		throw "Property ID required";
	m_PropCombo.Encode( enc, 1 );

	// encode the (optional) array index
	if (!m_ArrayIndex.ctrlNull)
		m_ArrayIndex.Encode( enc, 2 );
	
		
	switch (m_RadioChoice)  // The current selection of the radio button
	{
	case 0:  // nothing selected
		break;

	case 1:
		BACnetOpeningTag().Encode( enc, 3 );

		m_ReadRangePosRef.Encode( enc);
        m_ReadRangeCount.Encode( enc );

		BACnetClosingTag().Encode( enc, 3 );
        break;

	case 2:
		BACnetOpeningTag().Encode( enc, 4 );

        m_ReadRangeStartDate.Encode( enc );
        m_ReadRangeStartTime.Encode( enc );
        m_ReadRangeCount.Encode( enc );

		BACnetClosingTag().Encode( enc, 4 );
		break;

	case 3:
		BACnetOpeningTag().Encode( enc, 5 );

        m_ReadRangeStartDate.Encode( enc );
        m_ReadRangeStartTime.Encode( enc );
	    m_ReadRangeEndDate.Encode( enc );
	    m_ReadRangeEndTime.Encode( enc );

		BACnetClosingTag().Encode( enc, 5 );
		break;
	}

	// copy the encoding into the byte array
	for (int i = 0; i < enc.pktLength; i++)
		header.Add( enc.pktBuffer[i] );

	// stuff the header on the front
	contents->InsertAt( 0, &header );
}

//
//	CSendReadProp::SavePage
//

void SendReadRange::SavePage( void )
{
	TRACE0( "CSendReadProp::SavePage\n" );

	pageContents.Flush();

	m_ObjectID.SaveCtrl( pageContents );
	m_PropCombo.SaveCtrl( pageContents );
	m_ArrayIndex.SaveCtrl( pageContents );
    m_ReadRangeStartDate.SaveCtrl( pageContents );
	m_ReadRangeEndDate.SaveCtrl( pageContents );
    m_ReadRangeStartTime.SaveCtrl( pageContents );
	m_ReadRangeEndTime.SaveCtrl( pageContents );
    m_ReadRangeCount.SaveCtrl( pageContents );
  	m_ReadRangePosRef.SaveCtrl( pageContents );
}

//
//	CSendReadProp::RestorePage
//

void SendReadRange::RestorePage( void )
{
	BACnetAPDUDecoder	dec( pageContents )
	;
//	BACnetInteger		xType
	;
	TRACE0( "CSendReadProp::RestorePage\n" );

	if (dec.pktLength == 0)
		return;

	m_ObjectID.RestoreCtrl( dec );
	m_PropCombo.RestoreCtrl( dec );
	m_ArrayIndex.RestoreCtrl( dec );
/*
	m_ReadRangeStartDate.SaveCtrl( dec );
	m_ReadRangeEndDate.SaveCtrl( dec );
    m_ReadRangeStartTime.SaveCtrl( dec );
	m_ReadRangeEndTime.SaveCtrl( dec );
    m_ReadRangeCount.SaveCtrl( dec );
  	m_ReadRangePosRef.SaveCtrl( dec );
*/
}

////////////////////////////////////////////////////////////////////////
// SendReadRange::OnChangeArrayindex
//
void SendReadRange::OnChangeArrayindex() 
{
	
	m_ArrayIndex.UpdateData();
	SavePage();
	UpdateEncoded();
	
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnSelchangePropcombo
//
void SendReadRange::OnSelchangePropcombo() 
{
	m_PropCombo.UpdateData();
	SavePage();
	UpdateEncoded();
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnObjectidbtn
//
void SendReadRange::OnObjectidbtn() 
{
	VTSObjectIdentifierDialog	dlg
	;

	dlg.objID = m_ObjectID.objID;
	if (dlg.DoModal() && dlg.validObjID)
	{
		m_ObjectID.ctrlNull = false;
		m_ObjectID.objID = dlg.objID;
		m_ObjectID.ObjToCtrl();

		SavePage();
		UpdateEncoded();
	}
}

////////////////////////////////////////////////////////////////////////
// SendReadRange::OnChangeStartdatum
//
void SendReadRange::OnChangeStartdatum() 
{
    m_ReadRangeStartDate.UpdateData();
	SavePage();
	UpdateEncoded();
}
////////////////////////////////////////////////////////////
// SendReadRange::OnChangeEnddatum
//

void SendReadRange::OnChangeEnddatum() 
{
    m_ReadRangeEndDate.UpdateData();
	SavePage();
	UpdateEncoded();
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnRadionone
//
void SendReadRange::OnRadionone() 
{
	UpdateData();

    m_ReadRangeStartDate.Disable();
	m_ReadRangeEndDate.Disable();
    m_ReadRangeStartTime.Disable();
	m_ReadRangeEndTime.Disable();
	m_ReadRangePosRef.Disable();
	m_ReadRangeCount.Disable();
	SavePage();
	UpdateEncoded();
	
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnRadioposition
//
void SendReadRange::OnRadioposition() 
{
	UpdateData();

	m_ReadRangeStartDate.Disable();
	m_ReadRangeEndDate.Disable();
    m_ReadRangeStartTime.Disable();
	m_ReadRangeEndTime.Disable();
	m_ReadRangePosRef.Enable();
	m_ReadRangeCount.Enable();
	SavePage();
	UpdateEncoded();
	
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnRadiotime
//
void SendReadRange::OnRadiotime() 
{
	UpdateData();

	m_ReadRangeStartDate.Enable();
	m_ReadRangeEndDate.Disable();
    m_ReadRangeStartTime.Enable();
	m_ReadRangeEndTime.Disable();
	m_ReadRangePosRef.Disable();
	m_ReadRangeCount.Enable();
	SetDlgItemText( IDC_BEGDATECAPTION, "Reference Date and Time");
	SavePage();
	UpdateEncoded();
	
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnRadiotimerange
//
void SendReadRange::OnRadiotimerange() 
{
	UpdateData();

	m_ReadRangeStartDate.Enable();
	m_ReadRangeEndDate.Enable();
    m_ReadRangeStartTime.Enable();
	m_ReadRangeEndTime.Enable();
	m_ReadRangePosRef.Disable();
	m_ReadRangeCount.Disable();
	SetDlgItemText( IDC_BEGDATECAPTION, "Beginning Date and Time");
	SavePage();
	UpdateEncoded();
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnChangeObjectid
//

void SendReadRange::OnChangeObjectid() 
{
	
	m_ObjectID.UpdateData();
	SavePage();
	UpdateEncoded();
	
}
////////////////////////////////////////////////////////////////////////
//
//
void SendReadRange::OnSetfocusEnddatum() 
{
	// TODO: Add your control notification handler code here
	
}

void SendReadRange::OnSetfocusStartdatum() 
{
	// TODO: Add your control notification handler code here
	
}
////////////////////////////////////////////////////////////////////////
//SendReadRange::OnChangeTimestart
//
void SendReadRange::OnChangeTimestart() 
{
    m_ReadRangeStartTime.UpdateData();
	SavePage();
	UpdateEncoded();
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnChangeEndtime
//
void SendReadRange::OnChangeEndtime() 
{
    m_ReadRangeEndTime.UpdateData();
	SavePage();
	UpdateEncoded();
	
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnChangePosRef
//
void SendReadRange::OnChangePosRef() 
{
    m_ReadRangePosRef.UpdateData();
	SavePage();
	UpdateEncoded();
	
}
////////////////////////////////////////////////////////////////////////
// SendReadRange::OnChangeCount
//
void SendReadRange::OnChangeCount() 
{
    m_ReadRangeCount.UpdateData();
	SavePage();
	UpdateEncoded();
	
}
