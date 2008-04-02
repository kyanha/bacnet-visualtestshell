// VTSDevicesTreeObjPage.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"

#include "VTSPropValue.h"
#include "VTSDevicesTreeObjPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace NetworkSniffer {
	extern char *BACnetObjectType[];
}

/////////////////////////////////////////////////////////////////////////////
// VTSDevicesTreeObjPage property page

IMPLEMENT_DYNCREATE(VTSDevicesTreeObjPage, VTSPropertyPage)

VTSDevicesTreeObjPage::VTSDevicesTreeObjPage( VTSPageCallbackInterface * pparent )
                      :VTSPropertyPage(VTSDevicesTreeObjPage::IDD, pparent)
{
	//{{AFX_DATA_INIT(VTSDevicesTreeObjPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	ClearData();
	m_pdevobject = NULL;
}



VTSDevicesTreeObjPage::VTSDevicesTreeObjPage() : VTSPropertyPage()
{
	ASSERT(0);
}


VTSDevicesTreeObjPage::~VTSDevicesTreeObjPage()
{
}


void VTSDevicesTreeObjPage::ClearData(void)
{
	m_nVendor = 0;
	m_nReserved = 0;
	m_nInstance = 0;
	m_nObjType = 0;
	ValidateTypeCode();
}


void VTSDevicesTreeObjPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSDevicesTreeObjPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Text(pDX, IDC_VENDOR, m_nVendor);
	DDX_Text(pDX, IDC_RESERVED, m_nReserved);
	DDX_Text(pDX, IDC_INSTANCE, m_nInstance);
	DDX_Control(pDX, IDC_OBJTYPECOMBO, m_ObjTypeCombo);
//	DDX_CBIndex(pDX, IDC_OBJTYPECOMBO, m_nObjType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSDevicesTreeObjPage, VTSPropertyPage)
	//{{AFX_MSG_MAP(VTSDevicesTreeObjPage)
		// NOTE: the ClassWizard will add message map macros here
	ON_CBN_SELCHANGE(IDC_OBJTYPECOMBO, OnSaveChange)
	ON_EN_CHANGE(IDC_RESERVED, OnSaveChange)
	ON_EN_CHANGE(IDC_VENDOR, OnSaveChange)
	ON_EN_CHANGE(IDC_INSTANCE, OnSaveChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



BOOL VTSDevicesTreeObjPage::OnSetActive() 
{
	m_ObjTypeCombo.ResetContent();
	for( int i = 0; i < MAX_DEFINED_OBJ; i++)
	{
		m_ObjTypeCombo.AddString( NetworkSniffer::BACnetObjectType[i] );
	}
	m_ObjTypeCombo.AddString("Reserved");
	m_ObjTypeCombo.AddString("Vendor");

	
	m_pdevobject = (VTSDevObject *) RetrieveCurrentData();
	if ( m_pdevobject != NULL &&  !m_pdevobject->IsKindOf(RUNTIME_CLASS(VTSDevObject)) )
		m_pdevobject = NULL;

	ObjToCtrl(m_pdevobject);

	// calls DDX
	VTSPropertyPage::OnSetActive();
	EnableControls( m_pdevobject != NULL );	

	NotifyOfDataChange();
	return m_pdevobject != NULL;
}


BOOL VTSDevicesTreeObjPage::OnKillActive() 
{
	// calls DDX
	VTSPropertyPage::OnKillActive();
	CtrlToObj(m_pdevobject);
	EnableControls(false);	

	return TRUE;
}



void VTSDevicesTreeObjPage::ObjToCtrl( VTSDevObject * pdevobject )
{
	if ( pdevobject == NULL )
	{
		ClearData();
		return;
	}

	m_nReserved = m_nVendor = 0;
	m_nObjType = pdevobject->GetType();
	m_nInstance = pdevobject->GetInstance();

	// check for known values [MAX_DEFINED_OBJ is defined in VTS.h
	if ( (m_nObjType >= MAX_DEFINED_OBJ) && (m_nObjType < 128))
	{
		m_nReserved = m_nObjType;
		m_nObjType = MAX_DEFINED_OBJ;
	}
	else if ( m_nObjType >= 128 )
	{
		m_nVendor = m_nObjType;
		m_nObjType = MAX_DEFINED_OBJ + 1; //was 19, 		// vendor type
	}
	m_ObjTypeCombo.SelectString( 0, NetworkSniffer::BACnetObjectType[m_nObjType] );
}


void VTSDevicesTreeObjPage::CtrlToObj( VTSDevObject * pdevobject )
{
	if ( pdevobject == NULL )
		return;

	// put member data back into device object
	// Validate all control variables... stuff later.

	ValidateTypeCode();

	m_nObjType = m_ObjTypeCombo.GetCurSel();

	int nObjID = m_nObjType;
	switch(m_nObjType)
	{
		case MAX_DEFINED_OBJ:	nObjID = m_nReserved; break;
		case MAX_DEFINED_OBJ + 1:	nObjID = m_nVendor;
	}

	pdevobject->SetID(nObjID, m_nInstance);
}


void VTSDevicesTreeObjPage::ValidateTypeCode(void)
{
	if ( m_nInstance < 0 )
		m_nInstance = 0;
	else if ( m_nInstance >= (1 << 22) )
		m_nInstance = 1 << 22;

	if ( m_nReserved <= MAX_DEFINED_OBJ )
		m_nReserved = MAX_DEFINED_OBJ + 1;
	else if ( m_nReserved >= 128 )
		m_nReserved = 127;

	if ( m_nVendor < 128 )
		m_nVendor = 128;
	else if ( m_nVendor >= 1024 )
		m_nVendor = 1023;
}


void VTSDevicesTreeObjPage::EnableControls( bool fEnable )
{
	GetDlgItem(IDC_INSTANCE)->EnableWindow( fEnable );
	GetDlgItem(IDC_OBJTYPECOMBO)->EnableWindow( fEnable );
	GetDlgItem(IDC_RESERVED)->EnableWindow(fEnable && m_nObjType == MAX_DEFINED_OBJ);
	GetDlgItem(IDC_VENDOR)->EnableWindow(fEnable && m_nObjType == MAX_DEFINED_OBJ + 1);
}



void VTSDevicesTreeObjPage::OnSaveChange( void )
{
	UpdateData(TRUE);
	CtrlToObj(m_pdevobject);
	EnableControls(true);
	NotifyOfDataChange();
}


