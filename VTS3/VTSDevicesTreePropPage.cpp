// VTSDevicesTreePropPage.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "Propid.h"

#include "VTSPropValue.h"
#include "VTSDevicesTreePropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VTSDevicesTreePropPage property page

IMPLEMENT_DYNCREATE(VTSDevicesTreePropPage, VTSPropertyPage)

VTSDevicesTreePropPage::VTSDevicesTreePropPage( VTSPageCallbackInterface * pparent )
                      :VTSPropertyPage(VTSDevicesTreePropPage::IDD, pparent)
{
	//{{AFX_DATA_INIT(VTSDevicesTreePropPage)
	m_nPropIndex = -1;
	m_nPropID = 0;
	m_fIsArray = FALSE;
	//}}AFX_DATA_INIT

	m_pdevproperty = NULL;
}


VTSDevicesTreePropPage::VTSDevicesTreePropPage() : VTSPropertyPage()
{
	ASSERT(0);
}


VTSDevicesTreePropPage::~VTSDevicesTreePropPage()
{
}



void VTSDevicesTreePropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSDevicesTreePropPage)
	DDX_CBIndex(pDX, IDC_PROPCOMBO, m_nPropIndex);
	DDX_Text(pDX, IDC_PROPID, m_nPropID);
	DDV_MinMaxInt(pDX, m_nPropID, 0, MAX_PROP_ID);
	DDX_Check(pDX, IDC_PROPISARRAY, m_fIsArray);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSDevicesTreePropPage, VTSPropertyPage)
	//{{AFX_MSG_MAP(VTSDevicesTreePropPage)
	ON_CBN_SELCHANGE(IDC_PROPCOMBO, OnSelChangePropCombo)
	ON_EN_CHANGE(IDC_PROPID, OnChangePropID)
	ON_BN_CLICKED(IDC_PROPISARRAY, OnPropIsArray)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// VTSDevicesTreePropPage message handlers

BOOL VTSDevicesTreePropPage::OnKillActive() 
{
	VTSPropertyPage::OnKillActive();
	CtrlToProp(m_pdevproperty);

	return TRUE;
}

BOOL VTSDevicesTreePropPage::OnSetActive() 
{
	// load the drop-down
	for (int i = 0; i < MAX_PROP_ID; i++ )
		((CComboBox *) GetDlgItem(IDC_PROPCOMBO))->AddString( VTSDevProperty::GetDescription(i) );

	m_pdevproperty = (VTSDevProperty *) RetrieveCurrentData();
	if ( m_pdevproperty != NULL && !m_pdevproperty->IsKindOf(RUNTIME_CLASS(VTSDevProperty)) )
		m_pdevproperty = NULL;

	PropToCtrl(m_pdevproperty);

	// calls DDX
	VTSPropertyPage::OnSetActive();

	GetDlgItem(IDC_PROPISARRAY)->EnableWindow( m_pdevproperty != NULL && m_pdevproperty->GetValues()->GetSize() <= 1 );
	return m_pdevproperty != NULL;
}


void VTSDevicesTreePropPage::PropToCtrl( VTSDevProperty * pdevproperty )
{
	if ( pdevproperty == NULL )
		return;

	m_nPropIndex = pdevproperty->GetID();
	m_nPropID = m_nPropIndex;

	if ( pdevproperty->GetValues()->GetSize() > 1 )
		pdevproperty->SetIsArray(true);

	m_fIsArray = pdevproperty->IsArray();
}


void VTSDevicesTreePropPage::CtrlToProp( VTSDevProperty * pdevproperty )
{
	if ( pdevproperty == NULL )
		return;

	pdevproperty->SetID(m_nPropIndex);
	m_nPropID = m_nPropIndex;
	pdevproperty->SetIsArray(m_fIsArray == TRUE);
}


void VTSDevicesTreePropPage::OnSelChangePropCombo() 
{
	UpdateData(TRUE);
	CtrlToProp(m_pdevproperty);

	CDataExchange dx(this, FALSE);
	DDX_Text(&dx, IDC_PROPID, m_nPropID);

	NotifyOfDataChange();
}


void VTSDevicesTreePropPage::OnChangePropID() 
{
	CDataExchange dx(this, TRUE);
	DDX_Text(&dx, IDC_PROPID, m_nPropID);

	if ( m_nPropID < 0 )
		m_nPropID = 0;
	else if ( m_nPropID >= MAX_PROP_ID )
		m_nPropID = MAX_PROP_ID - 1;

	((CComboBox *) GetDlgItem(IDC_PROPCOMBO))->SetCurSel(m_nPropID);
	OnSelChangePropCombo();
}

void VTSDevicesTreePropPage::OnPropIsArray() 
{
	UpdateData(TRUE);
	CtrlToProp(m_pdevproperty);
	NotifyOfDataChange();
}
