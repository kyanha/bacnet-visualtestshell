//******************************************************************
// 	Author:			Yajun Zhou
//	Date:			2002-4-22
//	Description:	Accept the message of mouse and keyboard of
//					the edit area; Get the current line index; 
//					Move the caret to the specified line; Highlight
//					the current line.
//	Modify Log:		1)	Modified by Yajun Zhou, 2002-4-27 
//						Add a ctrl to display the line number.
//					2)	Modified by Yajun Zhou, 2002-5-8
//						Add a new feature to highlight the current 
//						line;
//						Fix BUG_1 that couldn't display the correct
//						line number on the statusbar when the current
//						line is the first visible line;
//					3)	Modified by Yajun Zhou, 2002-5-10
//						Fix BUG_2 that display a wrong index in the
//						statusbar and in the LineNumCtrl when typing
//						ENTER;
//					4)	Modified by Yajun Zhou, 2002-5-10
//						Reduce flicker of LineNumCtrl when refresh.
//					5)	Modified by Yajun Zhou, 2002-5-22
//						Reduce flicker when refresh and fix some bugs
//						that highlight a wrong line.
//					6)	Modified by Yajun Zhou, 2002-6-20
//						Delete the method, OnKillfocus().  So that when
//						the EditView lost its focus the window on the 
//						statusbar display current line number.
//					7)	Modified by Yajun Zhou, 2002-6-24
//						Add a method, SetDefaultFont(), to change the 
//						default font. 
//					8)	Modified by Yajun Zhou, 2002-7-8
//						Fixed a bug that display wrong line numbers in 
//						the LineNumCtrl when mouse wheeling. 
//					9)	Modified by Yajun Zhou, 2002-7-12
//						Fixed a bug that sometimes when the current line 
//						is the last visible line, it doesn't be highlighted.
//					10)	Modified by Yajun Zhou, 2002-8-5
//						Modified the function, DisplayLnNum(), so that it 
//						could display numbers larger than 9999 correctly.
//******************************************************************
// ScriptEdit.cpp : implementation file
//
#include "stdafx.h"
#include "VTS.h"
#include "ScriptEdit.h"
#include "MainFrm.h"
#include "GoToLineDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MARGIN_3			25
#define MARGIN_4			30
#define CHAR_HEIGHT			16
/////////////////////////////////////////////////////////////////////////////
// ScriptEdit

IMPLEMENT_DYNCREATE(ScriptEdit, CEditView)

ScriptEdit::ScriptEdit()
{
	m_pEdit	= &GetEditCtrl();

	m_nMargin		= MARGIN_3;
	m_nClientX		= 0;
	m_nClientY		= 0;
	m_ptCurPoint.x	= MARGIN_3;
	m_ptCurPoint.y	= 0;

	m_bDelete		= false;
	m_bInput		= false;
}

ScriptEdit::~ScriptEdit()
{
}


BEGIN_MESSAGE_MAP(ScriptEdit, CEditView)
	//{{AFX_MSG_MAP(ScriptEdit)
	ON_CONTROL_REFLECT(EN_SETFOCUS, OnSetfocus)
	ON_COMMAND(IDC_EDIT_GOTOLINE, OnEditGotoline)
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_MOUSEMOVE()
	ON_WM_HSCROLL()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(EN_HSCROLL, OnHscroll)
	ON_WM_VSCROLL()
	ON_CONTROL_REFLECT(EN_VSCROLL, OnVscroll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScriptEdit drawing

void ScriptEdit::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// ScriptEdit diagnostics

#ifdef _DEBUG
void ScriptEdit::AssertValid() const
{
	CEditView::AssertValid();
}

void ScriptEdit::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// ScriptEdit message handlers
int ScriptEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEditView::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetDefaultFont();
	// TODO: Add your specialized creation code here
	DWORD dwStyle = ES_MULTILINE | ES_RIGHT
					| WS_CHILD | WS_VISIBLE | WS_TABSTOP ;

	if(!m_LineNumCtrl.Create(dwStyle,
		CRect(0, 0, 0, 0), &GetEditCtrl(), IDC_LINENUMBER))
	{
		TRACE0("Failed to create LineNumCtrl\n");
		return -1;      // fail to create
	}

	m_LineNumCtrl.SetDefaultFont();
	m_LineNumCtrl.SetReadOnly(true);
	m_LineNumCtrl.EnableWindow(false);

	return 0;
}

void ScriptEdit::OnSize(UINT nType, int cx, int cy) 
{
	CEditView::OnSize(nType, cx, cy);

	m_nClientX = cx;
	m_nClientY = cy;

	if (::IsWindow(m_LineNumCtrl.m_hWnd))
		m_LineNumCtrl.MoveWindow(1, 1, m_nMargin, m_nClientY, TRUE);

	m_nVisibleLnCount = cy/CHAR_HEIGHT;
	
	UpdateWholeEdit();
	DisplayLnNum();

	TRACE("cx = %d, cy = %d\n", cx,cy);
}

HBRUSH ScriptEdit::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	if(nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetBkMode(TRANSPARENT); 
		return HBRUSH(GetStockObject(HOLLOW_BRUSH));
	}
	
	// TODO: Return a non-NULL brush if the parent's handler should not be called
	return NULL;
}

BOOL ScriptEdit::OnEraseBkgnd(CDC* pDC) 
{
	m_nFirstVisibleLn = m_pEdit->GetFirstVisibleLine();
	int nCurrentY;
	nCurrentY = (m_nCurrentLine - m_nFirstVisibleLn) * CHAR_HEIGHT;
	TRACE("m_nCurrentLine = %d, m_nFirstVisibleLn = %d, nCurrentY = %d\n",m_nCurrentLine, m_nFirstVisibleLn, nCurrentY);

	CRect rClient; 
	GetClientRect(&rClient);
	pDC->FillSolidRect( rClient, RGB(255,255,255) );

	if(nCurrentY+CHAR_HEIGHT-1 < m_nClientY)
	{
		CRect rLine = new CRect(m_nMargin, nCurrentY, m_nClientX, nCurrentY + CHAR_HEIGHT);
		pDC->FillSolidRect( rLine, RGB(255,255,0) );
	}

 	return TRUE;
}

void ScriptEdit::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
	
	m_pEdit->SetMargins(m_nMargin+1, 0);
	
	m_nFirstVisibleLn = 0;
	m_nCurrentLine = 0;
	m_nTempDigit = 3;
	m_nLineCount = m_pEdit->GetLineCount();

	DisplayLnNum();
}

void ScriptEdit::OnEditGotoline() 
{
	int nLine;
	CGoToLineDlg dlg;
	if(dlg.DoModal()==IDOK)
	{
		nLine = dlg.m_nLineIndex;
		SetLine(nLine);
	}	
}

void ScriptEdit::OnSetfocus() 
{
	CString str;
	str.Format(" Ln: %d",m_nCurrentLine+1);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->SetLnPaneText(str);	
}

void ScriptEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_ptCurPoint = m_pEdit->GetCaretPos();

	CEditView::OnLButtonDown(nFlags, point);

	UpdateTwoLine();
	GetCurLineIndex();
	DisplayLnNum();
}

void ScriptEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CEditView::OnLButtonUp(nFlags, point);

	UpdateTwoLine();
	GetCurLineIndex();
	DisplayLnNum();
}

void ScriptEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	CEditView::OnMouseMove(nFlags, point);

	if(nFlags == MK_LBUTTON)
		DisplayLnNum();
}

BOOL ScriptEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	BOOL bResult = CEditView::OnMouseWheel(nFlags, zDelta, pt);

	UpdateWholeEdit();
	DisplayLnNum();

	return bResult;
}

void ScriptEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_UP || nChar == VK_DOWN )
		ScrollCurLnVisible(nChar);

	CEditView::OnKeyDown(nChar, nRepCnt, nFlags);
	
	if(nChar == VK_RETURN 
		|| nChar == VK_SCROLL || nChar == VK_NUMLOCK || nChar == VK_CAPITAL
		|| nChar == VK_CONTROL || nChar == VK_SHIFT
		|| nChar == VK_ESCAPE
		|| nChar == VK_INSERT
		)
	{
		return;
	}

	if(nChar == VK_NEXT || nChar == VK_PRIOR)
	{
		if(m_nCurrentLine == m_pEdit->LineFromChar(-1))
		{
			GetCurLineIndex();
			return;
		}

		UpdateWholeEdit();
		GetCurLineIndex();
		DisplayLnNum();
		return;
	}

	if(nChar == VK_UP || nChar == VK_DOWN)
	{
		UpdateTwoLine();
		GetCurLineIndex();
		DisplayLnNum();
		return;
	}

	if(nChar == VK_LEFT || nChar == VK_RIGHT
		|| nChar == VK_HOME || nChar == VK_END)
	{
		if(!IsCurLnVisible())
		{
			UpdateWholeEdit();
			GetCurLineIndex();
			DisplayLnNum();
			return;
		}

		if(m_nCurrentLine == m_pEdit->LineFromChar(-1))
		{
//			UpdateOneLine(m_ptCurPoint.y);
			return;
		}
		else
		{
			UpdateTwoLine();
			GetCurLineIndex();
			DisplayLnNum();
			return;
		}
	}

	if(nChar == VK_DELETE)
	{
		m_bDelete = true;

		if(!IsCurLnVisible())
		{
			UpdateWholeEdit();
			GetCurLineIndex();
			DisplayLnNum();
			return;
		}
		if(m_nLineCount == m_pEdit->GetLineCount())
		{
			UpdateOneLine(m_ptCurPoint.y);
			GetCurLineIndex();
			DisplayLnNum();
			return;
		}
		else
		{
			UpdateRect(m_ptCurPoint.y);
			GetCurLineIndex();
			DisplayLnNum();
			return;
		}
	}

	if(nChar == VK_BACK || nChar == VK_TAB || nChar == VK_SPACE
		|| (nChar >= 48 && nChar <= 127))
	{
		return;
	}

	UpdateWholeEdit();
	GetCurLineIndex();
	DisplayLnNum();
}

void ScriptEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_bInput = true;

	CEditView::OnChar(nChar, nRepCnt, nFlags);

	if(nChar == VK_RETURN)
	{
		UpdateWholeEdit();
		GetCurLineIndex();
		DisplayLnNum();
		return;
	}
	else
	{
		if(!IsCurLnVisible())
		{
			UpdateWholeEdit();
			GetCurLineIndex();
			DisplayLnNum();
			return;
		}

		if(m_nCurrentLine == m_pEdit->LineFromChar(-1))
		{
			UpdateOneLine(m_ptCurPoint.y);
			DisplayLnNum();
			return;
		}
		else
		{
			UpdateRect(m_ptCurPoint.y);
			GetCurLineIndex();
			DisplayLnNum();
			return;
		}
	}
}

void ScriptEdit::OnChange() 
{
	if(m_bInput)
	{
		m_bInput = false;
		return;
	}
	
	if(m_bDelete)
	{
		m_bDelete = false;
		return;
	}
	
	UpdateWholeEdit();
	GetCurLineIndex();
	DisplayLnNum();
}

void ScriptEdit::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CEditView::OnVScroll(nSBCode, nPos, pScrollBar);

	if(m_nFirstVisibleLn == m_pEdit->GetFirstVisibleLine())
	{
		DisplayLnNum();
		return;
	}
	
	UpdateWholeEdit();
	DisplayLnNum();
}

void ScriptEdit::OnVscroll() 
{
	UpdateWholeEdit();
}

void ScriptEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CEditView::OnHScroll(nSBCode, nPos, pScrollBar);

	UpdateWholeEdit();
}

void ScriptEdit::OnHscroll() 
{
	UpdateWholeEdit();
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-4-22
//	Purpose:	Get the current line index
//	In:			void
//	Out:		void
//******************************************************************
int ScriptEdit::GetCurLineIndex()
{
	m_ptCurPoint = m_pEdit->GetCaretPos();
	
	int n = m_pEdit->CharFromPos(m_ptCurPoint);
	m_nCurrentLine = HIWORD(n);

	m_nFirstVisibleLn = m_pEdit->GetFirstVisibleLine();

	int nLineIndex = m_nCurrentLine + 1;

	//Start for BUG_1
	if(nLineIndex == m_nFirstVisibleLn)
	{
		nLineIndex++;
		m_nCurrentLine++;
	}
	//End for BUG_1

	CString str;
	if(nLineIndex == 65536)
		str = "";
	else
		str.Format(" Ln: %d",nLineIndex);

	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
	pFrm->SetLnPaneText(str);

	return m_nCurrentLine;
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-4-22
//	Purpose:	Go to the specified line
//	In:			int nLineIndex: The specified line index
//	Out:		void
//******************************************************************
void ScriptEdit::SetLine(int nLineIndex)
{
	if(nLineIndex > m_nLineCount)
			nLineIndex = m_nLineCount;

	int nCharIndex = m_pEdit->LineIndex(nLineIndex-1);

	m_pEdit->LineScroll(m_nLineCount-m_nCurrentLine);
	m_pEdit->SetSel(nCharIndex,nCharIndex,false);

	UpdateWholeEdit();
	GetCurLineIndex();
	DisplayLnNum();
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-4-27
//	Purpose:	Display line number in the CLineNumCtrl control.
//	In:			void
//	Out:		void
//******************************************************************
void ScriptEdit::DisplayLnNum()
{
	m_nLineCount = m_pEdit->GetLineCount();

	int nFirstVisibleLn = m_pEdit->GetFirstVisibleLine();
	int nLastVisibleLn = nFirstVisibleLn + m_nVisibleLnCount;

	m_LineNumCtrl.m_nLineCount = m_nLineCount;
	
	char buf[10];
	itoa(nLastVisibleLn, buf, 10);
	CString strLineNum = buf;
	int nDigit = strLineNum.GetLength();
	
	if(nDigit > 3 && nDigit != m_nTempDigit)
	{
		m_nTempDigit = nDigit;
		m_nMargin = MARGIN_3 + (nDigit - 3) * 6;
		m_pEdit->SetMargins(m_nMargin+1, 0);
		if (::IsWindow(m_LineNumCtrl.m_hWnd))
			m_LineNumCtrl.MoveWindow(1, 1, m_nMargin, m_nClientY, TRUE);

	}

	if(nDigit <= 3 && nDigit != m_nTempDigit)
	{
		m_nTempDigit = nDigit;
		m_nMargin = MARGIN_3;
		m_pEdit->SetMargins(m_nMargin+1, 0);
		if (::IsWindow(m_LineNumCtrl.m_hWnd))
			m_LineNumCtrl.MoveWindow(1, 1, m_nMargin, m_nClientY, TRUE);

	}

	m_LineNumCtrl.Display(nFirstVisibleLn+1, nLastVisibleLn);
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-5-10
//	Purpose:	Only refresh the edit area which changed.
//	In:			void
//	Out:		void
//******************************************************************
void ScriptEdit::UpdateWholeEdit()
{
	CRect* pRect = new CRect(m_nMargin, 0, m_nClientX, m_nClientY);
	InvalidateRect(pRect);
	delete pRect;
}

void ScriptEdit::UpdateRect(int nCurrentY)
{
	CRect* pRect = new CRect(m_nMargin, nCurrentY - CHAR_HEIGHT, m_nClientX, m_nClientY);
	InvalidateRect(pRect);
	delete pRect;
}

void ScriptEdit::UpdateOneLine(int nCurrentY)
{
	CRect* pLine = new CRect(m_nMargin, nCurrentY, m_nClientX, nCurrentY + CHAR_HEIGHT);
	InvalidateRect(pLine);
	delete pLine;
}

void ScriptEdit::UpdateTwoLine()
{
	if(m_nFirstVisibleLn != m_pEdit->GetFirstVisibleLine())
	{
		UpdateWholeEdit();
		return;
	}

	int nCurrentY = (m_nCurrentLine - m_nFirstVisibleLn) * CHAR_HEIGHT;
	UpdateOneLine(nCurrentY);

	m_ptCurPoint = m_pEdit->GetCaretPos();

	UpdateOneLine(m_ptCurPoint.y);
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-5-21
//	Purpose:	Judge whether the current line is visible or not.
//	In:			void
//	Out:		TRUE:	The current line is visible;
//				FALSE:	The current line is invisible;
//******************************************************************
BOOL ScriptEdit::IsCurLnVisible()
{
	if(m_nCurrentLine < m_nFirstVisibleLn ||
		m_nCurrentLine > (m_nFirstVisibleLn+m_nVisibleLnCount-1))
		return FALSE;
	else
		return TRUE;
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-5-22
//	Purpose:	When user type "Up" or "Down" and the current line is
//				invisible, it should scroll to be visible.
//	In:			void
//	Out:		void
//******************************************************************
void ScriptEdit::ScrollCurLnVisible(UINT nChar)
{
	if(m_nCurrentLine < m_nFirstVisibleLn)
	{
		if(nChar == VK_UP)
			m_pEdit->LineScroll(m_nCurrentLine-m_nFirstVisibleLn);
		if(nChar == VK_DOWN)
			m_pEdit->LineScroll(m_nCurrentLine-m_nFirstVisibleLn+1);
	}
	if(m_nCurrentLine > (m_nFirstVisibleLn+m_nVisibleLnCount-1))
	{
		if(nChar == VK_UP)
			m_pEdit->LineScroll(m_nCurrentLine-(m_nFirstVisibleLn+m_nVisibleLnCount));
		if(nChar == VK_DOWN)
			m_pEdit->LineScroll(m_nCurrentLine-(m_nFirstVisibleLn+m_nVisibleLnCount)+1);
	}
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-6-24
//	Purpose:	Set font.
//	In:			void
//	Out:		void
//******************************************************************
void ScriptEdit::SetDefaultFont()
{
	// instantiate a CFont pointer
    CFont* pFont  = new CFont();
    	
   	LOGFONT lf ;
   	memset(&lf, 0, sizeof(LOGFONT));       
    
   	lf.lfHeight = 16;
   	lf.lfWeight = 0;
   	lf.lfEscapement = 0;
   	lf.lfOrientation = 0;
   	lf.lfWeight = FW_NORMAL;
   	lf.lfItalic = FALSE;
   	lf.lfUnderline = FALSE;
   	lf.lfStrikeOut = 0;
   	lf.lfCharSet = ANSI_CHARSET;
   	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
   	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
   	lf.lfQuality = DEFAULT_QUALITY;
   	lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
   	strcpy(lf.lfFaceName, "Courier New");        
   		
    // give it a font using the LOGFONT structure
    pFont->CreateFontIndirect(&lf);
    
	// set the window's font, and force a repaint
    SetFont(pFont, TRUE); 
}