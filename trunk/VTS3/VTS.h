// VTS.h : main header file for the VTS application
//

#if !defined(AFX_VTS_H__BDE65080_B82F_11D3_BE52_00A0C95A9812__INCLUDED_)
#define AFX_VTS_H__BDE65080_B82F_11D3_BE52_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#define MAX_DEFINED_OBJ	  51	// This is one more than defined BACnetObjectType :Modifyed Shiyuan Xiao 7/14/2005
#define MAX_PROP_ID	      345	// Currently IS_UTC, +1
#define MAX_SERVS_SUPP     50        

#define	DEVICE_LOOPBACK		1		// set to zero to disable "looped" messages

#define MAX_FAIL_TIMES      7        //		add by GJB, 29/12/2003
#define MAX_BIBBS           100      //  msd 9/1/04 - added. Provides a little growing room.
#define MAX_DATALINK_OPTIONS  30     //  msd 9/1/04 - added. Provide a little growing room.

// Just the name.  Each usage must use GetTempPath for the path
#define FILE_CHECK_EPICS_CONS "EPICSConsChk.txt"

//
//	VTSApp
//

class VTSApp : public CWinApp {

	private:
		CDocTemplate * m_pdoctempConfig;
		CRecentFileList* m_pRecentWorkspaceList;

	public:
		VTSApp(void);
		virtual ~VTSApp(void);

		// Get the full path of theDirectory relative to the executable
		void GetRelativeToExe( CString &thePath, LPCTSTR theDirectory ) const;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(VTSApp)
		public:
		virtual BOOL InitInstance();
		virtual int ExitInstance();
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		//}}AFX_VIRTUAL

//		void CheckBACMACNTVersion( void );
		void LoadWorkspaceMRU(UINT nMaxMRU);
		void AddToRecentWorkspaceList(LPCTSTR lpszPathName);
		CRecentFileList* GetRecentFileList();

		CDocument * GetWorkspace(void);
		void CheckWinPcapVersion( void );

	// Implementation
		//{{AFX_MSG(VTSApp)
		afx_msg void OnAppAbout();
		afx_msg void OnFileWksNew();
		afx_msg void OnFileWksSwitch();
		afx_msg void OnUpdateRecentWorkspaceMenu(CCmdUI* pCmdUI);
		afx_msg BOOL OnOpenRecentWorkspace(UINT nID);

			// NOTE - the ClassWizard will add and remove member functions here.
			//    DO NOT EDIT what you see in these blocks of generated code !
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

typedef VTSApp *VTSAppPtr;

//
//	Application specific windows messages
//

#define WM_VTS_RCOUNT		(WM_APP+1)			// new record count
#define WM_VTS_PORTSTATUS	(WM_APP+2)			// new port status
#define WM_VTS_EXECMSG		(WM_APP+3)			// new executor message
#define WM_VTS_MAXPACKETS	(WM_APP+4)			// reached max packets


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VTS_H__BDE65080_B82F_11D3_BE52_00A0C95A9812__INCLUDED_)
