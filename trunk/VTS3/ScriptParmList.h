#if !defined(AFX_SCRIPTPARMLIST_H__CBE193B5_B97C_11D4_BEF2_00A0C95A9812__INCLUDED_)
#define AFX_SCRIPTPARMLIST_H__CBE193B5_B97C_11D4_BEF2_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxcview.h>

//
//	ScriptParm
//

class ScriptParm {
		friend class ScriptParmList;

	protected:
		bool		parmMark;								// marked for deletion

	public:
		CString		parmName;								// parameter name
		int			parmSymbol;								// hash code
		CString		parmValue;								// current value (editable)
		CString		parmScriptValue;						// value as defined in the script
		CString		parmDesc;								// description
		int			parmLine;								// where parm is defined in script

		ScriptParm( const char *name );
		~ScriptParm( void );
	};

typedef ScriptParm *ScriptParmPtr;
const int kScriptParmSize = sizeof( ScriptParm );

//
// ScriptParmList
//

class ScriptParmList : public CListView
{
protected:
	ScriptParmList();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(ScriptParmList)

// Attributes
public:
	int				m_iSelectedElem;
	CImageList		m_ilStatus;							// status images

	CListCtrl		*m_pListCtrl;						// handy pointer to list control

// Operations
public:
	// list operations
	void Add( CString &name, CString &valu, CString &desc, int line );		// add a child at the end

	void Mark( void );									// set up for cleanup
	void Release( void );								// delete those still marked

	int Length( void );									// number of children
	ScriptParmPtr operator [](int indx);				// get a child

	void Reset( void );									// change all values to script defined

	int LookupIndex( int code );						// find by code, return index in list
	ScriptParmPtr LookupParm( int code );				// find by code, return pointer to parm

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ScriptParmList)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~ScriptParmList();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(ScriptParmList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTPARMLIST_H__CBE193B5_B97C_11D4_BEF2_00A0C95A9812__INCLUDED_)
