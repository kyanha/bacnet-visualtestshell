// CheckEPICSCons.cpp : implementation file
//Added by Liangping Xu,2002-11

#include "stdafx.h"
#include "vts.h"
#include "CheckEPICSCons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCheckEPICSCons dialog


CCheckEPICSCons::CCheckEPICSCons(CWnd* pParent /*=NULL*/)
	: CDialog(CCheckEPICSCons::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCheckEPICSCons)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCheckEPICSCons::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCheckEPICSCons)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCheckEPICSCons, CDialog)
	//{{AFX_MSG_MAP(CCheckEPICSCons)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCheckEPICSCons message handlers

BOOL CCheckEPICSCons::OnInitDialog() 
{    FILE *mfile;
	 char *fp,*fp1;
	 BOOL nLine=0;
     char fbuf[256];
	 char outMsg[200];
	//Load c:\EPICSConsChk.txt,then used listbox to show EPICS Cons Check Results
	// TODO: Add extra initialization here
	CListBox *pLB=(CListBox *)GetDlgItem(IDC_EPICSCONSRESULT);
	pLB->SetHorizontalExtent(1500);
	mfile=fopen("c:\\EPICSConsChk.txt","r");
	if(mfile==NULL)
    	pLB->InsertString(-1,"EPICS Cons Check Report File  open error!");
	else{
		fgets(fbuf,sizeof(fbuf),mfile);
		while(feof(mfile)==0)
		{ fp=fp1=fbuf;
		  fp1=strchr(fbuf,0);       //delete '\n' from the end of line  
		  fp1--;
		  *fp1=0;
		  nLine++;
		  sprintf(outMsg,"%d. %s",nLine,fp);
		  pLB->InsertString(-1,outMsg);
          fgets(fbuf,sizeof(fbuf),mfile);
		}

	    fclose(mfile);
	}
	// madanner 6/03: moved close to inside successful condition

	return CDialog::OnInitDialog();  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
