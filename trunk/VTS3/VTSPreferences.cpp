// VTSPreferences.cpp: implementation of the VTSPreferences class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vts.h"
#include "VTSPreferences.h"
#include "VTSPreferencesDlg.h"

#include "VTSDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VTSPreferences::VTSPreferences()
{
	// Initialize all fields on
	m_nSummaryFields = 0xFF;				// bit mask of display options
	m_nSummaryColumnWidth[0] = 30;			// Number
	m_nSummaryColumnWidth[1] = 85;			// timestamp
	m_nSummaryColumnWidth[2] = 60;			// port
	m_nSummaryColumnWidth[3] = 120;			//dest/source
	m_nSummaryColumnWidth[4] = 150;			// service type
	m_nInvokeID = 0;
	m_nTimeFormat = 0;
	m_nCachePacketCount = 5000;				// start with about 1M of memory
	m_nAutoscrollTimeout = 300;	// seconds
	m_fRelativePacketFile = true;
}


bool VTSPreferences::SView_IsColumnOn(int nColumn)
{
	ASSERT(nColumn < SUMMARY_VIEW_COLUMN_COUNT && nColumn >= 0);
	return (m_nSummaryFields & (1 << nColumn)) != 0;
}

void VTSPreferences::SView_SetColumnOn(int nColumn, bool fOn /* = true */ )
{
	ASSERT(nColumn < SUMMARY_VIEW_COLUMN_COUNT && nColumn >= 0);
	m_nSummaryFields = (m_nSummaryFields & ~(1<<nColumn)) | ((fOn ? 1 : 0) << nColumn);
}


int VTSPreferences::SView_GetColumnWidth(int nColumn)
{
	ASSERT(nColumn < SUMMARY_VIEW_COLUMN_COUNT && nColumn >= 0);
	return m_nSummaryColumnWidth[nColumn];
}


void VTSPreferences::SView_SetColumnWidth(int nColumn, int nWidth)
{
	ASSERT(nColumn < SUMMARY_VIEW_COLUMN_COUNT && nColumn >= 0);
	m_nSummaryColumnWidth[nColumn] = nWidth;
}


//
//	VTSPreferences::Load
//

void VTSPreferences::Load( void )
{
	CWinApp* pApp = AfxGetApp();

	m_nSummaryFields = pApp->GetProfileInt( "SummaryView", "Fields", 0xFF);

	char szBuffer[15];
	for ( int i = 0; i < SUMMARY_VIEW_COLUMN_COUNT; i++ )
	{
		sprintf(szBuffer, "CWidth%d", i);
		m_nSummaryColumnWidth[i] = pApp->GetProfileInt( "SummaryView", szBuffer, m_nSummaryColumnWidth[i] );
	}

	m_nTimeFormat = pApp->GetProfileInt( "SummaryView", "TimeFormat", m_nTimeFormat);
	m_nInvokeID = pApp->GetProfileInt( "Send", "InvokeID", m_nInvokeID);
	m_nCachePacketCount = pApp->GetProfileInt( "Settings", "CachePacketCount", m_nCachePacketCount);
	m_nAutoscrollTimeout = pApp->GetProfileInt( "Settings", "AutoscrollTimeout", m_nAutoscrollTimeout);

	int nRelative = pApp->GetProfileInt( "Settings", "RelativePacketFile", m_fRelativePacketFile ? 1 : 0);
	m_fRelativePacketFile = nRelative != 0;
}

//
//	VTSPreferences::Save
//

void VTSPreferences::Save( void )
{
	CWinApp* pApp = AfxGetApp();

	pApp->WriteProfileInt( "SummaryView", "Fields", m_nSummaryFields );

	char szBuffer[15];
	for ( int i = 0; i < SUMMARY_VIEW_COLUMN_COUNT; i++ )
	{
		sprintf(szBuffer, "CWidth%d", i);
		pApp->WriteProfileInt( "SummaryView", szBuffer, m_nSummaryColumnWidth[i] );
	}

	pApp->WriteProfileInt( "SummaryView", "TimeFormat", m_nTimeFormat );
	pApp->WriteProfileInt( "Send", "InvokeID", m_nInvokeID );

	pApp->WriteProfileInt( "Settings", "CachePacketCount", m_nCachePacketCount);
	pApp->WriteProfileInt( "Settings", "AutoscrollTimeout", m_nAutoscrollTimeout);
	pApp->WriteProfileInt( "Settings", "RelativePacketFile", m_fRelativePacketFile ? 1 : 0);
}


void VTSPreferences::DoPrefsDlg()
{
	VTSPreferencesDlg	dlg(AfxGetMainWnd());

	dlg.m_nAutoscrollTimeout = Setting_GetAutoscrollTimeout();
	dlg.m_nPacketCount = Setting_GetCachePacketCount();
	dlg.m_nRelative = Setting_IsPacketFileRelative() ? 0 : 1;

	if ( dlg.DoModal() == IDOK )
	{
		Setting_SetAutoscrollTimeout(dlg.m_nAutoscrollTimeout);
		Setting_SetPacketFileRelative(dlg.m_nRelative != 1);

		if ( dlg.m_nPacketCount != Setting_GetCachePacketCount() )
		{
			Setting_SetCachePacketCount(dlg.m_nPacketCount);
			((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->SetNewCacheSize();
		}
	}
}


