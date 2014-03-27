// VTSDevicesTreeObjPage.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"

#include "VTSPropValue.h"
#include "VTSDevicesTreeObjPage.h"

#include "bacnet.hpp"
#include "StringTables.h"

//namespace PICS {
//#include "db.h" 
//#include "service.h"
//#include "vtsapi.h"
//#include "stdobj.h"
//#include "props.h"
//#include "bacprim.h"
//#include "dudapi.h"
//#include "propid.h"

//extern "C" void CreatePropertyFromEPICS( PICS::generic_object * pObj, int PropId, BACnetAnyValue * pbacnetAnyValue );

//}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	NetworkSniffer::BAC_STRTAB_BACnetObjectType.FillCombo( m_ObjTypeCombo );
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

	// The combo was populated with NetworkSniffer::BAC_STRTAB_BACnetObjectType.m_nStrings
	// strings from the enum table, plus "Reserved" and "Vendor"
	const char *pString;
	int nStrings = NetworkSniffer::BAC_STRTAB_BACnetObjectType.m_nStrings;
	if (m_nObjType < nStrings)
	{
		pString = NetworkSniffer::BAC_STRTAB_BACnetObjectType.m_pStrings[m_nObjType];
	}
	else if (m_nObjType < 128)
	{
		m_nReserved = m_nObjType;
		m_nObjType = nStrings;
		pString = "Reserved";
	}
	else
	{
		m_nVendor = m_nObjType;
		m_nObjType = nStrings + 1;
		pString = "Vendor";
	}

	m_ObjTypeCombo.SelectString( 0, pString );
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
		case MAX_DEFINED_OBJ:
			nObjID = m_nReserved; 
			break;
		case MAX_DEFINED_OBJ + 1:	
			nObjID = m_nVendor;
			break;
	}

	unsigned oldID = pdevobject->GetID();
	pdevobject->SetID(nObjID, m_nInstance);

	// 1) if no current properties, 
	CObArray * pobarray = (CObArray *)pdevobject->GetProperties();
	if ( pobarray != NULL && pobarray->GetSize() == 0 )
	{
	// 3) Create all required properties with default values
		if ( m_nObjType < MAX_DEFINED_OBJ )
		{
			// TODO: LJT add all required properties for selected object type
			//       Create file format to read defaults from?? Suggest use same format as
			//       chosen for the import/export functionality
			// standard object type
			VTSDevProperty * devprop = new VTSDevProperty();
			pobarray->Add(devprop);
			devprop->SetID(77);   // Object_Name
			// now create value for this property
			CObArray * pobvalarray = (CObArray *)devprop->GetValues();
			VTSDevValue * pdevvalue = new VTSDevValue();
			pobvalarray->Add(pdevvalue);
			pdevvalue->m_nType = 7;
			pdevvalue->m_nContext = -1;

			BACnetAPDUEncoder	compEncoder;
			BACnetCharacterString xx;
			CString str;
			str.Format( "object %u-%u", m_nObjType, m_nInstance );
			xx.SetValue( str, 0);
			xx.Encode( compEncoder, pdevvalue->m_nContext);
//			VTSCharacterStringCtrl m_CharStr(this, 0);
//			m_CharStr.SetValue( "ObjectName", 0 );
//			m_CharStr.Encode( compEncoder, pdevvalue->m_nContext );
			memcpy( pdevvalue->m_abContent, compEncoder.pktBuffer, compEncoder.pktLength );
		}
	}
/*
	// Load Default values for all objects
	PICS::PICSdb * pd = new PICS::PICSdb;

  // need to open file and load objects into db
	File ifile = fopen( "myfile", "r" );
	char data[500];

	readline(data, sizeof(data));

	// keep reading until find 'objects in db'
	PICS::ReadObjects(pd);

	// now repeat for each object until we find the matching type

	PICS::generic_object * pObj = (PICS::generic_object *) pd->Database;
	DWORD dwPropID;
	char szPropName[100];
	BACnetAnyValue bacnetAnyValue;
	// Load default values for object need to find object type
	for (int i = 0; i < sizeof(pObj->propflags); i++ )
	{
		if ( PICS::GetPropNameSupported(szPropName, i, pObj->object_type, pObj->propflags, &dwPropID, NULL) > 0 )
		{
			// got number add Property using dwPropID;
			VTSDevProperty * devprop = new VTSDevProperty();
			pobarray->Add(devprop);
			devprop->SetID(dwProp);

			// try to determine value
			if ( (pObj->propflags[i] & ValueUnknown) != 0 )
			{
				// property not specified don't do anything
			}
			else
			{
				try
				{
					PICS::CreatePropertyFromEPICS( pObj, (PICS::BACnetPropertyIdentifier) dwPropID, &bacnetAnyValue );
				}
				catch(...)
				{
					bacnetAnyValue.SetObject(NULL);
				}
				// now set value
				CObArray * pobvalarray = (CObArray *)devprop->GetValues();
				VTSDevValue * pdevvalue = new VTSDevValue();
				pobvalarray->Add(pdevvalue);
				pdevvalue->m_nContext = -1;
				
				pdevvalue->m_nType = bacnetAnyValue.DataType();

				BACnetAPDUEncoder	compEncoder;
				bacnetAnyValue.Encode( compEncoder, -1 );
				memcpy( pdevvalue->m_abContent, compEncoder.pktBuffer, compEncoder.pktLength );
			}
		}
	}
*/

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


