// EPICSViewNode.h: interface for the CEPICSViewNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EPICSVIEWNODE_H__368688EE_2735_4BD5_AEE5_91CDC77F77B6__INCLUDED_)
#define AFX_EPICSVIEWNODE_H__368688EE_2735_4BD5_AEE5_91CDC77F77B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEPICSTreeView;
class CEPICSViewInfoPanel;

class CEPICSViewNode : public CObject
{
protected:
	CEPICSTreeView * m_ptreeview;

public:
	CEPICSViewNode( CEPICSTreeView * ptreeview );
	virtual ~CEPICSViewNode();

	virtual void LoadInfoPanel() = 0;
};



class CEPICSViewNodeRoot : public CEPICSViewNode
{
private:
	int m_nSyntaxE;
	int m_nConE;

	void LoadErrorFile( CEPICSViewInfoPanel * ppanel );

public:
	CEPICSViewNodeRoot( CEPICSTreeView * ptreeview, int nSyntaxErrors, int nConErrors );
	virtual ~CEPICSViewNodeRoot();

	virtual void LoadInfoPanel();
};




#endif // !defined(AFX_EPICSVIEWNODE_H__368688EE_2735_4BD5_AEE5_91CDC77F77B6__INCLUDED_)
