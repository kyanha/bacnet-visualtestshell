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
//					11)	Modified by Yajun Zhou, 2002-9-1
//						Fixed a bug that the LineNumCtrl's background is 
//						white and could not work when horizontally scrolling
//						under windows98.
//					12)	Modified by Yajun Zhou, 2003-5-19
//						Used memDC to reduce flicker in function OnPaint() and 
//						deleted some unused functions.
//					13)	Modified by Yajun Zhou, 2003-5-20
//						Added handlers of cut, paste, undo and contextmenu.
//******************************************************************
// ScriptEdit.cpp : implementation file
//
#include "stdafx.h"
#include "VTS.h"
#include "ScriptEdit.h"
#include "ScriptFrame.h"
#include "GoToLineDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace NetworkSniffer {
	extern char *BACnetPropertyIdentifier[];
}
//Added by Zhu Zhenhua, 2003-12-25, to help tester in inputing 

#define MARGIN_3			25
#define CHAR_HEIGHT			16
#define LIMITTEXT			0x000FFFFF
#define MAX_PROPID			174  //Modified by Zhu Zhenhua, 2004-5-11
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

	m_pframe = NULL; 
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
	ON_WM_HSCROLL()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(EN_HSCROLL, OnHscroll)
	ON_WM_VSCROLL()
	ON_CONTROL_REFLECT(EN_VSCROLL, OnVscroll)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_WM_CONTEXTMENU()
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

	m_pEdit->SetLimitText(LIMITTEXT);

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
	
	UpdateEditArea();

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
 	return FALSE;
}

void ScriptEdit::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
	
	m_pEdit->SetMargins(m_nMargin+1, 0);
	
	m_nFirstVisibleLn = 0;
	m_nCurrentLine = 0;
	m_nTempDigit = 3;
	m_nLineCount = m_pEdit->GetLineCount();

//Added by Zhu Zhenhua, 2003-12-25, to help tester in inputing 
	
	for(int i = 0; i < MAX_PROPID; i ++)
	{	
		CString strprop;
		strprop = NetworkSniffer::BACnetPropertyIdentifier[i];
		AddInputHelpString(strprop);
	}
	{
		AddInputHelpString("analog-INPUT");
		AddInputHelpString("analog-OUTPUT");
		AddInputHelpString("analog-VALUE");
		AddInputHelpString("binary-INPUT");
		AddInputHelpString("binary-OUTPUT");
		AddInputHelpString("binary-VALUE");
		AddInputHelpString("Calendar");
		AddInputHelpString("Command");
		AddInputHelpString("Device");					
		AddInputHelpString("event-ENROLLMENT");
		AddInputHelpString("File");
		AddInputHelpString("Group");
		AddInputHelpString("Loop");
		AddInputHelpString("multistate-INPUT");
		AddInputHelpString("multistate-OUTPUT");
		AddInputHelpString("notification-CLASS");
		AddInputHelpString("Program");
		AddInputHelpString("Schedule");
		AddInputHelpString("Averaging");
		AddInputHelpString("multistate-VALUE");
		AddInputHelpString("Trendlog");
		AddInputHelpString("Lifesafety-POINT");
		AddInputHelpString("Lifesafety-ZONE");								
	}

}

void ScriptEdit::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcClient;
    GetClientRect( &rcClient );
	
	CDC memDC;
    CBitmap bitmap;
    memDC.CreateCompatibleDC(&dc);
    bitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
    CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);
	
	m_nFirstVisibleLn = m_pEdit->GetFirstVisibleLine();
	int nCurrentY;
	nCurrentY = (m_nCurrentLine - m_nFirstVisibleLn) * CHAR_HEIGHT;
	TRACE("m_nCurrentLine = %d, m_nFirstVisibleLn = %d, nCurrentY = %d\n",m_nCurrentLine, m_nFirstVisibleLn, nCurrentY);
	
	memDC.FillSolidRect( rcClient, RGB(255,255,255) );
	
	if(nCurrentY+CHAR_HEIGHT-1 < m_nClientY)
	{
		//madanner 6/03, heap allocation not necessary
		CRect RLine(m_nMargin, nCurrentY, m_nClientX, nCurrentY + CHAR_HEIGHT);
		memDC.FillSolidRect( &RLine, RGB(255,255,0) );
//		CRect *pRLine = new CRect(m_nMargin, nCurrentY, m_nClientX, nCurrentY + CHAR_HEIGHT);
//		memDC.FillSolidRect( pRLine, RGB(255,255,0) );
//		delete pRLine;
	}
	
    CWnd::DefWindowProc(WM_PAINT,(WPARAM)memDC.m_hDC, 0 );

	DisplayLnNum();
	
    dc.BitBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), &memDC, 0,0, SRCCOPY);
    memDC.SelectObject(pOldBitmap);
}

void ScriptEdit::OnEditGotoline() 
{
	int nLine;
	CGoToLineDlg dlg;
	if(dlg.DoModal()==IDOK)
	{
		nLine = dlg.m_nLineIndex;
		GotoLine(nLine);
	}	
}

void ScriptEdit::OnSetfocus() 
{
	CString str;
	str.Format(" Ln: %d",m_nCurrentLine+1);

//madanner 5/03	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
//	pFrm->SetLnPaneText(str);	
	if ( m_pframe != NULL )
		m_pframe->SetLnPaneText(str);
}

void ScriptEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_ptCurPoint = m_pEdit->GetCaretPos();

	CEditView::OnLButtonDown(nFlags, point);

	UpdateEditArea();
	GetCurLineIndex();
}

void ScriptEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CEditView::OnLButtonUp(nFlags, point);

	UpdateEditArea();
	GetCurLineIndex();
}

void ScriptEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_UP || nChar == VK_DOWN )
		ScrollCurLnVisible(nChar);

	CEditView::OnKeyDown(nChar, nRepCnt, nFlags);
	
	UpdateEditArea();
	GetCurLineIndex();
}

void ScriptEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CEditView::OnChar(nChar, nRepCnt, nFlags);
	if((nChar >= 65 && nChar <= 90) || (nChar>= 97 && nChar <= 122) || (nChar == 95 || nChar == 45))
	{
		OnHelpInput(nChar, nRepCnt, nFlags);
//Added by Zhu Zhenhua, 2003-12-25, to help tester in inputing 
	}
		
	if(nChar == VK_RETURN)
	{
		UpdateEditArea();
		GetCurLineIndex();
	}

}

void ScriptEdit::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CEditView::OnVScroll(nSBCode, nPos, pScrollBar);

	UpdateEditArea();
}

void ScriptEdit::OnVscroll() 
{
	UpdateEditArea();
}

void ScriptEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CEditView::OnHScroll(nSBCode, nPos, pScrollBar);

	UpdateEditArea();
}

void ScriptEdit::OnHscroll() 
{
	UpdateEditArea();
}

void ScriptEdit::OnEditCut() 
{
	CEditView::OnEditCut();
	GetCurLineIndex();
	UpdateEditArea();	
}

void ScriptEdit::OnEditPaste() 
{
	CEditView::OnEditPaste();
	GetCurLineIndex();
	UpdateEditArea();	
}

void ScriptEdit::OnEditUndo() 
{
	CEditView::OnEditUndo();
	GetCurLineIndex();
	UpdateEditArea();	
}

void ScriptEdit::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CEditView::OnContextMenu(pWnd, point);
	GetCurLineIndex();
	UpdateEditArea();		
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
	str.Format(" Ln: %d",nLineIndex);

//madanner, 5/03
//	CMainFrame* pFrm = (CMainFrame*) AfxGetMainWnd();
//	pFrm->SetLnPaneText(str);
	if ( m_pframe != NULL )
		m_pframe->SetLnPaneText(str);

	return m_nCurrentLine;
}

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-4-22
//	Purpose:	Go to the specified line
//	In:			int nLineIndex: The specified line index
//	Out:		void
//******************************************************************
void ScriptEdit::GotoLine(int nLineIndex)
{
	if(nLineIndex > m_nLineCount)
			nLineIndex = m_nLineCount;

	int nCharIndex = m_pEdit->LineIndex(nLineIndex-1);

	//madanner 6/03, make goto line visible... wouldn't do this if goto was upward
	m_pEdit->LineScroll((nLineIndex-1) - m_pEdit->GetFirstVisibleLine());
//	m_pEdit->LineScroll(m_nLineCount-m_nCurrentLine);
	m_pEdit->SetSel(nCharIndex,nCharIndex,false);
	
	//madanner 6/03, Calling GetCurLineIndex() without setting the caret highlights the wrong
	//line... must set caret too
	m_pEdit->SetCaretPos(m_pEdit->PosFromChar(nCharIndex));

	UpdateEditArea();
	GetCurLineIndex();
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
		m_nMargin = MARGIN_3 + (nDigit - 3) * 7;
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
//	Purpose:	Refresh the edit area.
//	In:			void
//	Out:		void
//******************************************************************
void ScriptEdit::UpdateEditArea()
{
	//madanner 6/03, heap allocation not necessary
	CRect Rect(m_nMargin, 0, m_nClientX, m_nClientY);
	InvalidateRect(&Rect);

//	CRect* pRect = new CRect(m_nMargin, 0, m_nClientX, m_nClientY);
//	InvalidateRect(pRect);
//	delete pRect;
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

//******************************************************************
//	Author:		Yajun Zhou
//	Date:		2002-11-1
//	Purpose:	Set the text of the specified line.
//	In:			int nLineIndex: The specified line index
//				LPTSTR lpszString: The text of new line
//	Out:		void
//******************************************************************
void ScriptEdit::SetLine(int nLineIndex, LPTSTR lpszString)
{
	int nChar;

	ASSERT(nLineIndex <= m_pEdit->GetLineCount() && nLineIndex >= 0);

	if(nLineIndex < m_pEdit->GetLineCount())
	{
		nChar = m_pEdit->LineIndex(nLineIndex);
		m_pEdit->SetSel(nChar, nChar);
		m_pEdit->ReplaceSel(lpszString);
	}
	else if(nLineIndex == m_pEdit->GetLineCount())
	{
		CString strBuffer;
		m_pEdit->GetLine(nLineIndex-1, strBuffer.GetBuffer(0));
		nChar = m_pEdit->LineIndex(nLineIndex-1) + strBuffer.GetLength();
		
		CString strLineFeed = "\r\n";
		m_pEdit->SetSel(nChar, nChar);
		m_pEdit->ReplaceSel(strLineFeed.GetBuffer(3));

		nChar = m_pEdit->LineIndex(nLineIndex);
		m_pEdit->SetSel(nChar, nChar);
		m_pEdit->ReplaceSel(lpszString);
	}		
}

//Added by Zhu Zhenhua, 2003-12-25
// Add help string in stringList 
bool ScriptEdit::AddInputHelpString(CString sString)
{
	bool bRet;		
	if( m_strList.IsEmpty() ) {
		// if list is empty add first string
		try {
			m_strList.AddHead( sString );
			bRet = true;
		} catch( CMemoryException *e ) {
			e->Delete();
			bRet = false;
		}
	} else {
		if( !m_strList.Find( sString ) ) {
			// insert into sorted list searching for a valid position
			int nCount = m_strList.GetCount();
			POSITION pos = m_strList.GetHeadPosition();
			for( int f=0; f<nCount; f++ ) {
				// ascending order
				if( sString < m_strList.GetAt( pos ) ) {
					break;
				} else {
					m_strList.GetNext( pos );
				}
			}
			try {
				if( f == nCount )
					m_strList.AddTail( sString );
				else
					m_strList.InsertBefore( pos, sString );
				bRet = true;
			} catch( CMemoryException *e ) {
				e->Delete();
				bRet = false;
			}
		} else
			bRet = true;
	}

	return bRet;
}


//Added by Zhu Zhenhua, 2003-12-25, to help tester in inputing 
void ScriptEdit::OnHelpInput(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	POSITION pos;
	CString sBuffer,sWord, sBufferLine;
	char pBuffer[1024];
	int nLen, nBegin, nstar, nOver, nEnd,nLastSpace, nWordLen;
	int nCurlin = GetCurLineIndex(); 
	int ncount = m_pEdit->LineLength(nCurlin);
	nBegin=m_pEdit->LineIndex(nCurlin);
	nEnd = nBegin + m_pEdit->LineLength(nCurlin);
	int len = m_pEdit->GetLine(nCurlin, pBuffer, 1024);
	pBuffer[len] = 0;
	sBufferLine = pBuffer;
//	delete pBuffer;
	
	m_pEdit->GetSel(nstar,nOver);
	sBuffer = sBufferLine.Left((nstar - nBegin));
	nLen = sBuffer.GetLength();
	for( int x=sBuffer.GetLength(); x>0; x-- )
		if( isspace( sBuffer.GetAt(x-1)))
			break;
	nLastSpace = x-1;
	if( nLastSpace >= 0 )
		sWord = sBuffer.Right( nLen - nLastSpace - 1 );
	else
		sWord = sBuffer;
	nWordLen = sWord.GetLength();
	if( !sWord.IsEmpty() ) 
	{
		for( int f=0; f<m_strList.GetCount(); f++ ) 
		{
			pos = m_strList.FindIndex( f );
			if( sWord == m_strList.GetAt( pos ).Left( nWordLen )) 
			{
				sWord = m_strList.GetAt( pos ).Right( m_strList.GetAt( pos ).GetLength() - nWordLen );
				m_pEdit->SetSel(nstar, nstar);
				m_pEdit->ReplaceSel(sWord.GetBuffer(0));
				m_pEdit->SetSel(nstar, (nstar+sWord.GetLength()) );
				break;
			}
		}
	}
}
