// WPRPList.h: interface for the WPRPList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WPRPLIST_H__AF524C1A_9CD4_4C9B_9CD5_C66546E641EE__INCLUDED_)
#define AFX_WPRPLIST_H__AF524C1A_9CD4_4C9B_9CD5_C66546E641EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BACnet.hpp"
#include "BACnetIP.hpp"
#include "VTSAny.h"

//Xiao Shiyuan 2002-12-2

class WPRPList  
{
public:
};

class WPMRPMElem 
{
public:
	BACnetEnumerated		m_prop;
	BACnetUnsigned			m_array;
	VTSAny				    m_value;
	BACnetUnsigned			m_priority;	
	BACnetEnumerated		m_class;
	BACnetEnumerated		m_code;
};

typedef WPMRPMElem *WPMRPMElemPtr;

class WPMRPMList : public CList<WPMRPMElemPtr, WPMRPMElemPtr>
{
public:
	CString m_ObjIDStr;
	BACnetObjectIdentifier	m_ObjID;
};

typedef WPMRPMList *WPMRPMListPtr;

class WPMRPMListList : public CList<WPMRPMListPtr, WPMRPMListPtr>
{
};

#endif // !defined(AFX_WPRPLIST_H__AF524C1A_9CD4_4C9B_9CD5_C66546E641EE__INCLUDED_)
