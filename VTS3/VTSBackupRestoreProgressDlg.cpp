// VTSBackupRestoreProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "VTSBackupRestoreProgressDlg.h"
#include "BakRestoreExecutor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VTSBackupRestoreProgressDlg dialog


VTSBackupRestoreProgressDlg::VTSBackupRestoreProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(VTSBackupRestoreProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(VTSBackupRestoreProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void VTSBackupRestoreProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSBackupRestoreProgressDlg)
	DDX_Control(pDX, IDOK, m_okCtrl);
	DDX_Control(pDX, IDC_BACKUP_RESTORE_KILL, m_killCtrl);
	DDX_Control(pDX, IDC_BACKUP_RESTORE_OUTPUT, m_editOutput);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSBackupRestoreProgressDlg, CDialog)
	//{{AFX_MSG_MAP(VTSBackupRestoreProgressDlg)
	ON_BN_CLICKED(IDC_BACKUP_RESTORE_KILL, OnBackupRestoreKill)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSBackupRestoreProgressDlg message handlers


void VTSBackupRestoreProgressDlg::OutMessage(const CString& msg, BOOL bnewLine /*= TRUE*/)
{
	CHARFORMAT cf;
	m_editOutput.GetDefaultCharFormat(cf);
	cf.cbSize = 10;
	m_editOutput.SetDefaultCharFormat(cf);	
	
	int currLine = m_editOutput.GetLineCount() - 1;
  m_editOutput.SetSel(-1,-1);
	m_editOutput.ReplaceSel(msg);
  m_editOutput.SetSel(-1, -1);
	if (bnewLine)
	{
   
		m_editOutput.ReplaceSel("\r\n");
	}
	else
	{
 		m_editOutput.ReplaceSel("\t");
	}
	
}

void VTSBackupRestoreProgressDlg::OnOK()
{
	DestroyWindow ();
}

void VTSBackupRestoreProgressDlg::OnCancel()
{
	DestroyWindow ();
}

extern BakRestoreExecutor gBakRestoreExecutor;

void VTSBackupRestoreProgressDlg::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
	gBakRestoreExecutor.DestoryOutputDlg();
    delete this;
}


void VTSBackupRestoreProgressDlg::OnBackupRestoreKill() 
{
	// TODO: Add your control notification handler code here
	gBakRestoreExecutor.Kill();
	m_killCtrl.EnableWindow(FALSE);
	m_okCtrl.EnableWindow(TRUE);
	CMenu* pMenu = GetSystemMenu(FALSE);	
	pMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND|MF_ENABLED);
}


void VTSBackupRestoreProgressDlg::BeginTestProcess()
{
	OutMessage("Beginning test process...");
	m_killCtrl.EnableWindow(TRUE);
	m_okCtrl.EnableWindow(FALSE);
	CMenu* pMenu = GetSystemMenu(FALSE);	
	pMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND|MF_GRAYED);
}


void VTSBackupRestoreProgressDlg::EndTestProcess()
{
	m_killCtrl.EnableWindow(FALSE);
	m_okCtrl.EnableWindow(TRUE);
	CMenu* pMenu = GetSystemMenu(FALSE);	
	pMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND|MF_ENABLED);
}

BOOL VTSBackupRestoreProgressDlg::OnInitDialog() 
{
  AfxInitRichEdit();
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_killCtrl.EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void VTSBackupRestoreProgressDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if (m_killCtrl || m_okCtrl || m_editOutput)
	{ 
		// Get size of the dialog
		void* p = &m_killCtrl;
		CRect rect;
		GetClientRect(&rect);
		// get size of the kill button
		CRect rectKill;
		m_killCtrl.GetClientRect(&rectKill);
		// get size of the ok button
		CRect rectOk;
		m_okCtrl.GetClientRect(&rectOk);
		// get size of the output window
		CRect rectOutput;
		m_editOutput.GetClientRect(&rectOutput);
		// move the position and resize output
		m_editOutput.MoveWindow(15, 15, rect.Width()-30, rect.Height()-15-(rectOk.Height())-30);
		// move and resize kill button
		m_killCtrl.MoveWindow((int)(0.339*rect.Width()-0.5*rectKill.Width()), rect.Height()-15-(rectKill.Height()), 
			rectKill.Width(), rectKill.Height());
		// move and resize ok button
		m_okCtrl.MoveWindow((int)(0.661*rect.Width()-0.5*rectOk.Width()), rect.Height()-15-(rectOk.Height()), 
			rectOk.Width(), rectOk.Height());
	}
	
}
