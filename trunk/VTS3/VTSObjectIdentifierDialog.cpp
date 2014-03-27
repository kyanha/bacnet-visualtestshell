// VTSObjectIdentifierDialog.cpp : implementation file
//
#include "stdafx.h"
#include "VTS.h"
#include "VTSObjectIdentifierDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VTSObjectIdentifierDialog dialog

VTSObjectIdentifierDialog::VTSObjectIdentifierDialog(CWnd* pParent /*=NULL*/)
	: CDialog(VTSObjectIdentifierDialog::IDD, pParent)
	, objID(0), validObjID(false)
{
	//{{AFX_DATA_INIT(VTSObjectIdentifierDialog)
	m_Encoded = _T("");
	//}}AFX_DATA_INIT
}

void VTSObjectIdentifierDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSObjectIdentifierDialog)
	DDX_Control(pDX, IDC_VENDOR, m_Vendor);
	DDX_Control(pDX, IDC_RESERVED, m_Reserved);
	DDX_Control(pDX, IDC_INSTANCE, m_Instance);
	DDX_Control(pDX, IDC_OBJTYPECOMBO, m_ObjTypeCombo);
	DDX_Text(pDX, IDC_ENCODED, m_Encoded);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(VTSObjectIdentifierDialog, CDialog)
	//{{AFX_MSG_MAP(VTSObjectIdentifierDialog)
	ON_CBN_SELCHANGE(IDC_OBJTYPECOMBO, OnSelchangeObjTypeCombo)
	ON_EN_CHANGE(IDC_RESERVED, OnChangeReserved)
	ON_EN_CHANGE(IDC_VENDOR, OnChangeVendor)
	ON_EN_CHANGE(IDC_INSTANCE, OnChangeInstance)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSObjectIdentifierDialog message handlers

BOOL VTSObjectIdentifierDialog::OnInitDialog() 
{
	CString		txt
	;

	CDialog::OnInitDialog();


	m_ObjTypeCombo.ResetContent();
	NetworkSniffer::BAC_STRTAB_BACnetObjectType.FillCombo( m_ObjTypeCombo );
	m_ObjTypeCombo.AddString("Reserved");
	m_ObjTypeCombo.AddString("Vendor");
		
	// use value given to us
	objType = (objID >> 22);
	instance = (objID & ((1 << 22) - 1));

	// format the object type
	txt.Format( "%d", objType );

	// check for known values [MAX_DEFINED_OBJ is defined in VTS.h
   if (objType < MAX_DEFINED_OBJ)
   {
		// standard, known type
		m_ObjTypeCombo.SetCurSel( objType );

		m_Reserved.SetWindowText( txt );
		m_Reserved.EnableWindow( true );
		m_Vendor.SetWindowText( _T("") );
		m_Vendor.EnableWindow( false );
   }
   else if (objType < 128)
   {
		// reserved type
		m_ObjTypeCombo.SetCurSel( MAX_DEFINED_OBJ ); //was 18

		m_Reserved.SetWindowText( txt );
		m_Reserved.EnableWindow( true );
		m_Vendor.SetWindowText( _T("") );
		m_Vendor.EnableWindow( false );
   }
   else
   {
		// vendor type
		m_ObjTypeCombo.SetCurSel( MAX_DEFINED_OBJ +1 ); //was 19

		m_Reserved.SetWindowText( _T("") );
		m_Reserved.EnableWindow( false );
		m_Vendor.SetWindowText( txt );
		m_Vendor.EnableWindow( true );
	}

	// format the instance
	txt.Format( "%d", instance );
	m_Instance.SetWindowText( txt );

	// update the encoded description
	UpdateEncoding( true );

	// finished
	return TRUE;
}

void VTSObjectIdentifierDialog::OnSelchangeObjTypeCombo() 
{
	CString		txt
	;

	// extract the value from the control
	objType = m_ObjTypeCombo.GetCurSel();

	// check for known values
	if (objType < MAX_DEFINED_OBJ)  //was 18
	{
		// standard, known type
		txt.Format( "%d", objType );

		m_Reserved.SetWindowText( txt );
		m_Reserved.EnableWindow( true );
		m_Vendor.SetWindowText( _T("") );
		m_Vendor.EnableWindow( false );
   }
   else if (objType == MAX_DEFINED_OBJ)
	{
		// reserved type
		txt.Format( "%d", objType );

		m_Reserved.SetWindowText( txt );
		m_Reserved.EnableWindow( true );
		m_Vendor.SetWindowText( _T("") );
		m_Vendor.EnableWindow( false );
   }
   else
	{
		// vendor type
		objType = 128;
		txt.Format( "%d", objType );

		m_Reserved.SetWindowText( _T("") );
		m_Reserved.EnableWindow( false );
		m_Vendor.SetWindowText( txt );
		m_Vendor.EnableWindow( true );
   }

	// update the encoded id
	UpdateEncoding( true );
}

void VTSObjectIdentifierDialog::OnChangeReserved() 
{
	int			valu
	;
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	m_Reserved.GetWindowText( str );
	s = str.operator LPCTSTR();

	// make sure there's data
	if (!*s) {
		UpdateEncoding( false );
		return;
	}

	// extract the value, disallow out of range values
	valu = atoi( s );
	if ((valu < 0) || (valu >= 128)) {
		UpdateEncoding( false );
		return;
	}

	// save the new value
	objType = valu;

	// update the control to reflect the new value
	m_ObjTypeCombo.SetCurSel( objType < MAX_DEFINED_OBJ ? objType : MAX_DEFINED_OBJ ); //was 18 replaced by MAX_DEFINED_OBJ

	// update the encoded id
	UpdateEncoding( true );
}

void VTSObjectIdentifierDialog::OnChangeVendor() 
{
	int			valu
	;
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	m_Vendor.GetWindowText( str );
	s = str.operator LPCTSTR();

	// make sure there's data
	if (!*s) {
		UpdateEncoding( false );
		return;
	}

	// extract the value, disallow out of range values
	valu = atoi( s );
	if ((valu < 128) || (valu >= 1024)) {
		UpdateEncoding( false );
		return;
	}

	// save the new value
	objType = valu;

	// update the encoded id
	UpdateEncoding( true );
}

void VTSObjectIdentifierDialog::OnChangeInstance() 
{
	int			valu
	;
	CString		str
	;
	LPCTSTR		s
	;

	// get the text from the control
	m_Instance.GetWindowText( str );
	s = str.operator LPCTSTR();

	// make sure there's data
	if (!*s) {
		UpdateEncoding( false );
		return;
	}

	// extract the value, disallow out of range values
	valu = atoi( s );
	if ((valu < 0) || (valu >= (1 << 22))) {
		UpdateEncoding( false );
		return;
	}

	// save the new value
	instance = valu;

	// update the encoded id
	UpdateEncoding( true );
}

//
//	VTSObjectIdentifierDialog::UpdateEncoding
//

void VTSObjectIdentifierDialog::UpdateEncoding( bool valid )
{
	if (!valid) {
		m_Encoded = _T("Error");

		validObjID = false;
		objID = 0;
	} else {
		char	typeBuff[32];
		const char *s;

		if (objType < NetworkSniffer::BAC_STRTAB_BACnetObjectType.m_nStrings)
			s = NetworkSniffer::BAC_STRTAB_BACnetObjectType.m_pStrings[objType];
		else
		{
			s = typeBuff;
			if (objType < 128)
				sprintf( typeBuff, "RESERVED %d", objType );
			else
				sprintf( typeBuff, "VENDOR %d", objType );
		}

		m_Encoded.Format( "%s, %d", s, instance );

		validObjID = true;
		objID = (objType << 22) + instance;
	}

	// let the controls get synced
	UpdateData( false );
}
