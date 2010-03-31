// SelectTemplateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "ScriptEdit.h"
#include "SelectTemplateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=============================================================================
//=============================================================================
SelectTemplateDlg::SelectTemplateDlg(const ScriptTemplateCollection &theCollection, CWnd* pParent)
: CDialog(SelectTemplateDlg::IDD, pParent)
, m_collection(theCollection)
, m_templateIndex(-1)
{
	//{{AFX_DATA_INIT(SelectTemplateDlg)
	//}}AFX_DATA_INIT
}

//=============================================================================
void SelectTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectTemplateDlg)
	DDX_Control(pDX, IDC_LIST1, m_templateList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(SelectTemplateDlg, CDialog)
	//{{AFX_MSG_MAP(SelectTemplateDlg)
	ON_LBN_DBLCLK(IDC_LIST1, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=============================================================================
BOOL SelectTemplateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (int ix = 0; ix < m_collection.Size(); ix++)
	{
		int jx = m_templateList.AddString( m_collection.TemplateName( ix ) );
		m_templateList.SetItemData( jx, (DWORD)ix );
	}
	
	return TRUE;  
}


//=============================================================================
void SelectTemplateDlg::OnOK() 
{
	int ix = m_templateList.GetCurSel();
	if (ix >= 0)
	{
		m_templateIndex = m_templateList.GetItemData( ix );
	}
	
	CDialog::OnOK();
}
