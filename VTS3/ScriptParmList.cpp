// ScriptParmList.cpp : implementation file
//

#include "stdafx.h"
#include "VTS.h"

#include "ScriptBase.h"
#include "ScriptParmList.h"
#include "ScriptParmUpdate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ScriptParmList

IMPLEMENT_DYNCREATE(ScriptParmList, CListView)

//
//	ScriptParmList::ScriptParmList
//

ScriptParmList::ScriptParmList()
	: m_iSelectedElem(-1)
{
}

//
//	ScriptParmList::~ScriptParmList
//

ScriptParmList::~ScriptParmList()
{
}

BEGIN_MESSAGE_MAP(ScriptParmList, CListView)
	//{{AFX_MSG_MAP(ScriptParmList)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGING, OnItemChanging)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	ScriptParm::ScriptParm
//

ScriptParm::ScriptParm( const char *name )
	: parmName(name), parmValue(""), parmScriptValue(""), parmDesc("")
	, parmLine(-1)
{
	parmSymbol = ScriptToken::HashCode( name );
}

//
//	ScriptParm::~ScriptParm
//

ScriptParm::~ScriptParm( void )
{
}

//
//	ScriptParmList::Add
//

void ScriptParmList::Add( CString &name, CString &valu, CString &desc, int line )
{
	int				indx
	,				code = ScriptToken::HashCode( name )
	;
	ScriptParmPtr	pp
	;

	// look for the var
	indx = LookupIndex( code );

	// if found, update the script value and line
	if (indx >= 0) {
		// the data is tucked behind the item element
		pp = (ScriptParmPtr)m_pListCtrl->GetItemData( indx );

		// if the current value matches the script update it
		if (pp->parmValue == pp->parmScriptValue) {
			pp->parmValue = valu;
			m_pListCtrl->SetItemText( indx, 1, (LPCTSTR)pp->parmValue );
		}

		// set the script value, line, and description
		pp->parmScriptValue = valu;
		pp->parmLine = line;
		pp->parmDesc = desc;

		// reset the mark
		pp->parmMark = false;

		// make sure the list gets the new description
		m_pListCtrl->SetItemText( indx, 2, (LPCTSTR)pp->parmDesc );

		// set the image status
		m_pListCtrl->SetItem( indx, 0, TVIF_IMAGE, 0
			, ((pp->parmValue != pp->parmScriptValue) ? 1 : 0)
			, 0, 0, 0
			);

		return;
	}

	// create a new parameter record
	pp = new ScriptParm( name );

	// save the values and line number
	pp->parmValue = pp->parmScriptValue = valu;
	pp->parmLine = line;
	pp->parmDesc = desc;

	// reset the mark
	pp->parmMark = false;

	// add the item to the list
	indx = m_pListCtrl->InsertItem( 0x7FFFFFFF, (LPCTSTR)pp->parmName );
	m_pListCtrl->SetItemText( indx, 1, (LPCTSTR)pp->parmValue );
	m_pListCtrl->SetItemText( indx, 2, (LPCTSTR)pp->parmDesc );
	m_pListCtrl->SetItemData( indx, (DWORD)pp );

	// set the image for the list to match the status of the parm
	m_pListCtrl->SetItem( indx, 0, TVIF_IMAGE, 0, ((pp->parmValue != pp->parmScriptValue) ? 1 : 0), 0, 0, 0 );
}

//
//	ScriptParmList::Mark
//

void ScriptParmList::Mark( void )
{
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++) {
		ScriptParmPtr pp = (ScriptParmPtr)m_pListCtrl->GetItemData( i );
		pp->parmMark = true;
	}
}

//
//	ScriptParmList::Release
//

void ScriptParmList::Release( void )
{
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++) {
		ScriptParmPtr pp = (ScriptParmPtr)m_pListCtrl->GetItemData( i );

		// if it's marked, delete it
		if (pp->parmMark) {
			// remove from list
			m_pListCtrl->DeleteItem( i );

			// delete the parameter
			delete pp;

			// reset to check this spot again
			i -= 1;
		}
	}
}

//
//	ScriptParmList::Length
//

int ScriptParmList::Length( void )
{
	return m_pListCtrl->GetItemCount();
}

//
//	ScriptParmList::operator []
//

ScriptParmPtr ScriptParmList::operator []( int indx )
{
	ASSERT( (indx >= 0) && (indx < Length()) );
	return (ScriptParmPtr)m_pListCtrl->GetItemData( indx );
}

//
//	ScriptParmList::Reset
//

void ScriptParmList::Reset( void )
{
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++) {
		ScriptParmPtr pp = (ScriptParmPtr)m_pListCtrl->GetItemData( i );

		// if they're the same, skip it
		if (pp->parmValue == pp->parmScriptValue)
			continue;

		// set to the script value
		pp->parmValue = pp->parmScriptValue;

		// clean up the display
		m_pListCtrl->SetItemText( i, 1, (LPCTSTR)pp->parmValue );
		m_pListCtrl->SetItem( i, 0, TVIF_IMAGE, 0, 0, 0, 0, 0 );
	}
}

//
//	ScriptParmList::LookupIndex
//

int ScriptParmList::LookupIndex( int code )
{
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++) {
		ScriptParmPtr pp = (ScriptParmPtr)m_pListCtrl->GetItemData( i );
		if (pp->parmSymbol == code)
			return i;
	}

	// no success
	return -1;
}

//
//	ScriptParmList::LookupParm
//

ScriptParmPtr ScriptParmList::LookupParm( int code )
{
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++) {
		ScriptParmPtr pp = (ScriptParmPtr)m_pListCtrl->GetItemData( i );
		if (pp->parmSymbol == code)
			return pp;
	}

	// no success
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// ScriptParmList drawing

void ScriptParmList::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// ScriptParmList diagnostics

#ifdef _DEBUG
void ScriptParmList::AssertValid() const
{
	CListView::AssertValid();
}

void ScriptParmList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

//
//	ScriptParmList::PreCreateWindow
//

BOOL ScriptParmList::PreCreateWindow(CREATESTRUCT& cs) 
{
	// change to a report list
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_REPORT;

	return CListView::PreCreateWindow(cs);
}

//
//	ScriptParmList::OnCreate
//

int ScriptParmList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// let the list view create the control
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// get a pointer to the control
	m_pListCtrl = &GetListCtrl();

	// add the columns
	m_pListCtrl->InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 100 );
	m_pListCtrl->InsertColumn( 1, _T("Value"), LVCFMT_LEFT, 100 );
	m_pListCtrl->InsertColumn( 2, _T("Description"), LVCFMT_LEFT, 200 );

	// initialize the status image list
	m_ilStatus.Create( IDB_PARMSTATUS, 12, 1, RGB(255,0,255) );
	m_pListCtrl->SetImageList( &m_ilStatus, LVSIL_SMALL );

	// success
	return 0;
}

//
//	ScriptParmList::OnItemChanging
//

void ScriptParmList::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// forget messages that dont change the state
	if (pNMListView->uOldState == pNMListView->uNewState)
		return;

	if ((pNMListView->uNewState & LVIS_SELECTED) != 0) {
		TRACE1( "New selection %d\n", pNMListView->iItem );
		m_iSelectedElem = pNMListView->iItem;
	} else
	if (pNMListView->iItem == m_iSelectedElem) {
		TRACE0( "No more selection\n" );
		m_iSelectedElem = -1;
	}

	*pResult = 0;
}

//
//	ScriptParmList::OnDoubleClick
//

void ScriptParmList::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// tell the caller we handled it
	*pResult = 0;

	// toss if nothing selected
	if (m_iSelectedElem < 0)
		return;

	// get a pointer to the element
	ScriptParmPtr pp = (ScriptParmPtr)m_pListCtrl->GetItemData( m_iSelectedElem );

	// make sure it's valid
	ASSERT( pp );

	// construct the dialog box
	ScriptParmUpdate	dlg( pp )
	;

	// execute the modal
	if (dlg.DoModal() && dlg.m_ParmValueOK) {
		// update the parameter value from the scanned results
		pp->parmValue = dlg.m_ParmValueNormalized;

		// update the list to the new value and status
		m_pListCtrl->SetItemText( m_iSelectedElem, 1, (LPCTSTR)pp->parmValue );
		m_pListCtrl->SetItem( m_iSelectedElem, 0, TVIF_IMAGE, 0
			, ((pp->parmValue != pp->parmScriptValue) ? 1 : 0)
			, 0, 0, 0
			);
	}
}

//
//	ScriptParmList::OnColumnClick
//

void ScriptParmList::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	TRACE1( "Column %d clicked\n", pNMListView->iSubItem );

	*pResult = 0;
}

//
//	ScriptParmList::OnDestroy
//

void ScriptParmList::OnDestroy() 
{
	// delete the parameters
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)
		delete (ScriptParmPtr)m_pListCtrl->GetItemData( i );

	// continue with destruction
	CListView::OnDestroy();
}
