// VTSPreferences.h: interface for the VTSPreferences class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VTSPREFERENCES_H__CF611A55_1BB3_45C9_BC9A_58F8C5A69A29__INCLUDED_)
#define AFX_VTSPREFERENCES_H__CF611A55_1BB3_45C9_BC9A_58F8C5A69A29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//
//	VTSPreferences
//

#define SUMMARY_VIEW_COLUMN_COUNT		5
#define SUMMARY_VIEW_MAXCACHESLOTS		32000

class VTSPreferences : public CObject
{
	private:
		int		m_nSummaryFields;				// bit mask of display options
		int		m_nSummaryColumnWidth[SUMMARY_VIEW_COLUMN_COUNT];
		int		m_nInvokeID;
		int		m_nTimeFormat;					// 0=local time, 1=UTC

		int		m_nCachePacketCount;			// number of packet slots allocated for list cache
		int		m_nAutoscrollTimeout;			// seconds of inactivity for going back to autoscroll
		bool	m_fRelativePacketFile;			// true if packet file should be relative
		bool	m_fVerifyDelete;				// should we ask 'are you sure' when deleting packets?
		bool	m_fLoadEPICS;					// should we load the EPICS file upon startup?

		CString m_strLastEPICS;					// filename of last EPICS file loaded

	public:
		VTSPreferences();
		virtual ~VTSPreferences() {}

		bool SView_IsColumnOn(int nColumn);
		void SView_SetColumnOn(int nColumn, bool fOn = true );
		int SView_GetColumnWidth(int nColumn);
		void SView_SetColumnWidth(int nColumn, int nWidth);

		int Send_GetInvokeID() { return m_nInvokeID; }
		void Send_SetInvokeID(int nInvokeID) { m_nInvokeID = nInvokeID; }

		int Setting_GetCachePacketCount() { return m_nCachePacketCount; }
		void Setting_SetCachePacketCount( int nCount ) { m_nCachePacketCount = nCount > SUMMARY_VIEW_MAXCACHESLOTS ? SUMMARY_VIEW_MAXCACHESLOTS : (nCount < 1 ? 1 : nCount); }
		int Setting_GetAutoscrollTimeout() { return m_nAutoscrollTimeout; }
		void Setting_SetAutoscrollTimeout(int nTimeout) { m_nAutoscrollTimeout = nTimeout; }
		bool Setting_IsPacketFileRelative() { return m_fRelativePacketFile; }
		void Setting_SetPacketFileRelative(bool fRelative) { m_fRelativePacketFile = fRelative; }
		bool Setting_IsVerifyDelete() { return m_fVerifyDelete; }
		void Setting_SetVerifyDelete(bool fVerify) { m_fVerifyDelete = fVerify; }
		bool Setting_IsLoadEPICS() { return m_fLoadEPICS; }
		void Setting_SetLoadEPICS(bool fLoad) { m_fLoadEPICS = fLoad; }
		LPCSTR Setting_GetLastEPICS() { return m_strLastEPICS; };
		void Setting_SetLastEPICS(LPCSTR lpszEPICSFile);

		void Load( void );
		void Save( void );
		void DoPrefsDlg(void);
};

typedef VTSPreferences *VTSPreferencesPtr;

extern VTSPreferences	gVTSPreferences;


#endif // !defined(AFX_VTSPREFERENCES_H__CF611A55_1BB3_45C9_BC9A_58F8C5A69A29__INCLUDED_)
