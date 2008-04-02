// VTSDevicesTreeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vts.h"
#include "VTSDevicesTreeDlg.h"

//#include "VTSValue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VTSDevicesTreeDlg dialog


IMPLEMENT_DYNAMIC(VTSDevicesTreeDlg, CDialog)

#pragma warning(disable:4355)

VTSDevicesTreeDlg::VTSDevicesTreeDlg(VTSDevices * pdevices, CWnd* pParent /*=NULL*/)
	: CDialog(VTSDevicesTreeDlg::IDD, pParent),
	  m_pageDev(this),
	  m_pageObj(this),
	  m_pageProp(this),
	  m_pageValue(this),
	  m_pageNull(IDD_DEVTREENULL)
{
	//{{AFX_DATA_INIT(VTSDevicesTreeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pdevices = pdevices;
	m_devicesLocal.DeepCopy(m_pdevices);

	m_htreeitemSelected = NULL;
	m_pnodeSelected = NULL;
}



void VTSDevicesTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VTSDevicesTreeDlg)
	DDX_Control(pDX, IDC_DEVICETREE, m_treeDevices);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VTSDevicesTreeDlg, CDialog)
	//{{AFX_MSG_MAP(VTSDevicesTreeDlg)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_NEWDEVICE, OnUpdateNewDevice)
	ON_COMMAND(ID_NEWDEVICE, OnNewDevice)
	ON_COMMAND(ID_NEWOBJECT, OnNewObject)
	ON_UPDATE_COMMAND_UI(ID_NEWOBJECT, OnUpdateNewObject)
	ON_COMMAND(ID_NEWPROPERTY, OnNewProperty)
	ON_UPDATE_COMMAND_UI(ID_NEWPROPERTY, OnUpdateNewProperty)
	ON_COMMAND(ID_NEWVALUE, OnNewValue)
	ON_UPDATE_COMMAND_UI(ID_NEWVALUE, OnUpdateNewValue)
	ON_COMMAND(ID_NEWCOMPONENT, OnNewValueComponent)
	ON_UPDATE_COMMAND_UI(ID_NEWCOMPONENT, OnUpdateNewValueComponent)
	ON_NOTIFY(TVN_SELCHANGED, IDC_DEVICETREE, OnSelchangedDeviceTree)
	ON_COMMAND(ID_DELETE, OnDelete)
	ON_UPDATE_COMMAND_UI(ID_DELETE, OnUpdateDelete)
	ON_NOTIFY(TVN_KEYDOWN, IDC_DEVICETREE, OnKeydownDevicetree)
	//}}AFX_MSG_MAP
	ON_COMMAND( IDC_DEVEXPORT, OnExport)
	ON_COMMAND( IDC_DEVIMPORT, OnImport)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VTSDevicesTreeDlg message handlers

void VTSDevicesTreeDlg::OnCancel() 
{
	// remove allocated temp elements...  destructor will kill array
	m_devicesLocal.KillContents();

	CDialog::OnCancel();
}

void VTSDevicesTreeDlg::OnOK() 
{
	// Unbind all ports using devices
	((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->UnbindPortsToDevices();

	for ( int n = 0; n < m_pdevices->GetSize(); n++ )
		(*m_pdevices)[n]->Deactivate();

	// copy names array...   then copy array.  Elements remain allocated
	m_pdevices->KillContents();

	// We need to scan through these guys and turn on the IsArray flag in the properties
	// if there is more than one value attached.
	// copy only the element references... changes ownership of element memory

	for ( int i = 0; i < m_devicesLocal.GetSize(); i++ )
	{
		VTSDevice * p = m_devicesLocal[i];
		VTSDevObjects * pobjects;
		VTSDevProperties * pproperties;
		int j, k;

		for ( j = 0, pobjects = p->GetObjects(); pobjects != NULL && j < pobjects->GetSize(); j++ )
			for ( k = 0, pproperties = (*pobjects)[j]->GetProperties(); pproperties != NULL && k < pproperties->GetSize(); k++ )
				if ( (*pproperties)[k]->GetValues()->GetSize() > 1 && !(*pproperties)[k]->IsArray() )
					(*pproperties)[k]->SetIsArray(true);
			
		m_pdevices->Add(m_devicesLocal[i]);
	}

	// empty list but don't remove elements...  this will avoid destructor
	// killing the memory... which we've already transferred ownership to document's member
	// names list

	m_devicesLocal.RemoveAll();

	// Relink ports that used to point to devices...
	((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->FixupPortToDeviceLinks(false);
	// Rebind all ports using devices.  This avoids shutting down any ports...
	((VTSDoc *) ((VTSApp *) AfxGetApp())->GetWorkspace())->BindPortsToDevices();

	// activate new devices....
	for ( int x = 0; x < m_pdevices->GetSize(); x++ )
		(*m_pdevices)[x]->Activate();

	CDialog::OnOK();
}




BOOL VTSDevicesTreeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_sheet.AddPage(&m_pageNull);
	m_sheet.Create(this, WS_CHILD | WS_VISIBLE, 0);
	m_sheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	m_sheet.ModifyStyle( 0, WS_TABSTOP );

	CRect rcSheet;
	GetDlgItem(IDC_DEVTREEHOLDER)->GetWindowRect( &rcSheet );
	ScreenToClient( &rcSheet );
	m_sheet.SetWindowPos( NULL, rcSheet.left-7, rcSheet.top-7, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );
	
	// initialize the image list.
    m_imagelist.Create( IDB_DEVTREE, 18, 1, RGB(255,0,255) );
    m_treeDevices.SetImageList( &m_imagelist, TVSIL_NORMAL );

	InitTree();				// get device definitions from the database
	return TRUE;
}


void VTSDevicesTreeDlg::InitTree()
{
	if ( m_devicesLocal.GetSize() == 0 )
		m_devicesLocal.Add(new VTSDevice());

	LoadDevices( &m_devicesLocal, NULL );

	// select the first item in the tree
	m_treeDevices.SelectItem(m_treeDevices.GetFirstVisibleItem());
}



void VTSDevicesTreeDlg::LoadDevices( VTSDevices * pdevices, HTREEITEM htreeitemParent )
{
	HTREEITEM htreeitemLast = NULL;

	for (int i = 0; i < pdevices->GetSize(); i++ )
	{
		htreeitemLast = m_treeDevices.InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetNodeTitle(0, (*pdevices)[i]), 0, 0, 0, 0, (LPARAM) (*pdevices)[i], htreeitemParent, htreeitemLast );
		LoadObjects( (*pdevices)[i]->GetObjects(), htreeitemLast );
	}
}


void VTSDevicesTreeDlg::LoadObjects( VTSDevObjects * pdevobjects, HTREEITEM htreeitemParent )
{
	HTREEITEM htreeitemLast = NULL;

	for (int i = 0; i < pdevobjects->GetSize(); i++ )
	{
		htreeitemLast = m_treeDevices.InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetNodeTitle(1, (*pdevobjects)[i]), 1, 1, 0, 0, (LPARAM) (*pdevobjects)[i], htreeitemParent, htreeitemLast );
		LoadProperties( (*pdevobjects)[i]->GetProperties(), htreeitemLast );
	}
}



void VTSDevicesTreeDlg::LoadProperties( VTSDevProperties * pdevproperties, HTREEITEM htreeitemParent )
{
	HTREEITEM htreeitemLast = NULL;

	for (int i = 0; i < pdevproperties->GetSize(); i++ )
	{
		htreeitemLast = m_treeDevices.InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetNodeTitle(2, (*pdevproperties)[i]), 2, 2, 0, 0, (LPARAM) (*pdevproperties)[i], htreeitemParent, htreeitemLast );
		LoadValues( (*pdevproperties)[i]->GetValues(), htreeitemLast, 3 );
	}
}


void VTSDevicesTreeDlg::LoadValues( VTSDevValues * pdevvalues, HTREEITEM htreeitemParent, int nLevel )
{
	HTREEITEM htreeitemLast = NULL;

	if ( pdevvalues == NULL )
		return;

	for (int i = 0; i < pdevvalues->GetSize(); i++ )
	{
		htreeitemLast = m_treeDevices.InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetNodeTitle(nLevel, (*pdevvalues)[i]), nLevel, nLevel, 0, 0, (LPARAM) (*pdevvalues)[i], htreeitemParent, htreeitemLast );
		LoadValues( (*pdevvalues)[i]->GetValueList(), htreeitemLast, 4 );
	}
}




void VTSDevicesTreeDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	
	CMenu menu;

	// I would use LoadMenu here but it won't load as a pupup for some reason...
	//menu.LoadMenu(IDR_DEVTREE);
	
	menu.CreatePopupMenu();

	// Slap together all of the menu items and gray them here instead of letting the updateUI command
	// processor do it... there's something weird about it:  It won't.

	menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_STRING, ID_NEWDEVICE, _T("New Device"));

	// Allow object node creation only if something is selected
	menu.AppendMenu((GetSelectedNodeLevel() > -1 ? MF_ENABLED : MF_GRAYED) | MF_UNCHECKED | MF_STRING, ID_NEWOBJECT, _T("New Object"));

	// Allow property creation only if value, property or object is selected
	menu.AppendMenu((GetSelectedNodeLevel() > 0 ? MF_ENABLED : MF_GRAYED) | MF_UNCHECKED | MF_STRING, ID_NEWPROPERTY, _T("New Property"));

	// Allow value selection only if value or property is selected
	menu.AppendMenu((GetSelectedNodeLevel() > 1 ? MF_ENABLED : MF_GRAYED) | MF_UNCHECKED | MF_STRING, ID_NEWVALUE, _T("New Value"));

	// Allow value component selection only if value or value component is selected
	menu.AppendMenu((GetSelectedNodeLevel() > 2 ? MF_ENABLED : MF_GRAYED) | MF_UNCHECKED | MF_STRING, ID_NEWCOMPONENT, _T("New Value Component"));

	menu.AppendMenu(MF_ENABLED | MF_UNCHECKED | MF_SEPARATOR);

	// Can only delete things that are selected...
	menu.AppendMenu((GetSelectedNodeLevel() > -1 ? MF_ENABLED : MF_GRAYED) | MF_UNCHECKED | MF_STRING, ID_DELETE, _T("Delete"));

	menu.TrackPopupMenu( TPM_CENTERALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
}


void VTSDevicesTreeDlg::OnNewDevice() 
{
	InsertNode(0, new VTSDevice());
}


void VTSDevicesTreeDlg::InsertNode(  int nAddLevel, CObject * pobjectNew )
{
	HTREEITEM htreeitemAfter = m_htreeitemSelected;
	HTREEITEM htreeitemParent = m_htreeitemSelected;

	// We have to back up to the appropriate parent... for devices...
	if ( GetSelectedNodeLevel() < nAddLevel )
		htreeitemAfter = NULL;
	else
	{
		for ( int i = GetSelectedNodeLevel(); i > nAddLevel; i-- )
			htreeitemAfter = m_treeDevices.GetParentItem(htreeitemAfter);

		htreeitemParent =  m_treeDevices.GetParentItem(htreeitemAfter);
	}

	// now get the final parent for the new node
	CObArray * pobarray;

	switch( nAddLevel )
	{
		case 0:		pobarray = (CObArray *) &m_devicesLocal;		break;
		case 1:		pobarray = (CObArray *) ((VTSDevice *) m_treeDevices.GetItemData(htreeitemParent))->GetObjects();	break;
		case 2:		pobarray = (CObArray *) ((VTSDevObject *) m_treeDevices.GetItemData(htreeitemParent))->GetProperties(); 	break;
		case 3:		pobarray = (CObArray *) ((VTSDevProperty *) m_treeDevices.GetItemData(htreeitemParent))->GetValues();		break;
		case 4:		// special case of allocated sister values array... if not there we must create one.
					if ( ((VTSDevValue *) m_treeDevices.GetItemData(htreeitemParent))->GetValueList() == NULL )
						((VTSDevValue *) m_treeDevices.GetItemData(htreeitemParent))->AllocateNewValueList();
					pobarray = (CObArray *) ((VTSDevValue *) m_treeDevices.GetItemData(htreeitemParent))->GetValueList();		break;
		default:	ASSERT(0);
	}

	int nIndexDataArray = FindDataInsertionIndex( pobarray, htreeitemAfter == NULL ? NULL : (LPARAM) m_treeDevices.GetItemData(htreeitemAfter) );

	// Account for case where nothing is selected
	if ( nIndexDataArray == -1 )
		pobarray->Add(pobjectNew);
	else
		pobarray->InsertAt(nIndexDataArray, pobjectNew);

	HTREEITEM htreeitemNew = m_treeDevices.InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetNodeTitle(nAddLevel, pobjectNew), nAddLevel, nAddLevel, 0, 0, (LPARAM) pobjectNew, htreeitemParent, htreeitemAfter );

	// make sure the record is visible and selected
	m_treeDevices.EnsureVisible(htreeitemNew);
	m_treeDevices.SelectItem(htreeitemNew);
	m_treeDevices.SetFocus();
}


int VTSDevicesTreeDlg::FindDataInsertionIndex( CObArray * pobarray, LPARAM lparam )
{
	ASSERT(pobarray != NULL);
	int i;

	for ( i = 0; i < pobarray->GetSize() && (LPARAM) (*pobarray)[i] != lparam; i++ );
	if ( i >= pobarray->GetSize() - 1 )
		i = -2;

	return i + 1;
}


void VTSDevicesTreeDlg::OnUpdateNewDevice(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	// Always on...
}


void VTSDevicesTreeDlg::OnNewObject() 
{
	InsertNode(1, new VTSDevObject());
}


void VTSDevicesTreeDlg::OnUpdateNewObject(CCmdUI* pCmdUI) 
{
	// Allow object node creation only if something is selected... 
	pCmdUI->Enable(GetSelectedNodeLevel() > -1);
}


void VTSDevicesTreeDlg::OnNewProperty() 
{
	InsertNode(2, new VTSDevProperty());
}


void VTSDevicesTreeDlg::OnUpdateNewProperty(CCmdUI* pCmdUI) 
{
	// This menu should only be disabled in the case of a device selection.  Object, sister property or child
	// value should allow creation of a new property.

	pCmdUI->Enable(GetSelectedNodeLevel() > 0);
}


void VTSDevicesTreeDlg::OnNewValue() 
{
	InsertNode(3, new VTSDevValue());
}


void VTSDevicesTreeDlg::OnUpdateNewValue(CCmdUI* pCmdUI) 
{
	// This menu should only be disabled in the case of a device selection or object selection.
	// Sister value and parent propery should allow creation of new value.

	pCmdUI->Enable(GetSelectedNodeLevel() > 1);
}



void VTSDevicesTreeDlg::OnNewValueComponent() 
{
	InsertNode(4, new VTSDevValue());
}


void VTSDevicesTreeDlg::OnUpdateNewValueComponent(CCmdUI* pCmdUI) 
{
	// This menu should only be disabled in the case of a device selection or object selection.
	// Sister value and parent propery should allow creation of new value.

	pCmdUI->Enable(GetSelectedNodeLevel() > 2);
}


int VTSDevicesTreeDlg::GetSelectedNodeLevel() 
{
	if ( m_pnodeSelected == NULL )
		return -1;

	if ( m_pnodeSelected->IsKindOf(RUNTIME_CLASS(VTSDevice)) )
		return 0;
	
	if ( m_pnodeSelected->IsKindOf(RUNTIME_CLASS(VTSDevObject)) )
		return 1;

	if ( m_pnodeSelected->IsKindOf(RUNTIME_CLASS(VTSDevProperty)) )
		return 2;

	// selected node is a VTSDevValue type... now.. is its PARENT that type as well?  If so
	// then the selected node must be level 4.. if not it's level 3

	if ( m_treeDevices.GetParentItem(m_htreeitemSelected) != NULL  &&  ((CObject *) m_treeDevices.GetItemData(m_treeDevices.GetParentItem(m_htreeitemSelected)))->IsKindOf(RUNTIME_CLASS(VTSDevValue)) )
		return 4;

	// must be root value ...
	return 3;
}



void VTSDevicesTreeDlg::UpdateNodeName( LPCSTR lpszNodeName )
{
	m_treeDevices.SetItemText(m_htreeitemSelected, lpszNodeName);
}



void VTSDevicesTreeDlg::DataChangeNotification(void)
{
	// called by child property pages... we don't care about things here...
	UpdateNodeName( GetNodeTitle(GetSelectedNodeLevel(), m_pnodeSelected) );
}


void * VTSDevicesTreeDlg::GetActiveData(void)
{
	return (void *) m_pnodeSelected;
}


CString VTSDevicesTreeDlg::GetNodeTitle( int nLevel, CObject * pobjNode )
{
	if ( pobjNode != NULL )
		switch(nLevel)
		{
			// would be nice for a base class... don't really need it though...
			case 0:		return ((VTSDevice *) pobjNode)->GetDescription();
			case 1:		return ((VTSDevObject *) pobjNode)->GetDescription();
			case 2:		return ((VTSDevProperty *) pobjNode)->GetDescription();
			case 3:
			case 4:		return ((VTSDevValue *) pobjNode)->GetDescription();
		}

	return CString();
}


void VTSDevicesTreeDlg::OnSelchangedDeviceTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	m_htreeitemSelected = pNMTreeView == NULL ? NULL : m_treeDevices.GetSelectedItem();
	m_pnodeSelected = m_htreeitemSelected != NULL ? (CObject *) m_treeDevices.GetItemData(m_htreeitemSelected) : NULL;

	// Following will cause GetCurrentNode() callback so set it first...

	CPropertyPage * ppageCurrent = m_sheet.GetPage(0);
	CPropertyPage * ppageShouldBe;

	switch(GetSelectedNodeLevel())
	{
		case 0:		ppageShouldBe = &m_pageDev; break;
		case 1:		ppageShouldBe = &m_pageObj; break;
		case 2:		ppageShouldBe = &m_pageProp; break;
		case 3:		ppageShouldBe = &m_pageValue; break;
		case 4:		ppageShouldBe = &m_pageValue; break;
		default:	ppageShouldBe = &m_pageNull;
	}

	if ( ppageShouldBe != ppageCurrent )
	{
		m_sheet.RemovePage(0);
		m_sheet.AddPage(ppageShouldBe);
	}

	m_sheet.SetActivePage(ppageShouldBe);

	if ( pResult != NULL )
		*pResult = 0;
}

void VTSDevicesTreeDlg::OnDelete() 
{
	HTREEITEM htreeitemNextSelect = m_treeDevices.GetNextItem(m_htreeitemSelected, TVGN_NEXT);
	HTREEITEM htreeitemParent = m_treeDevices.GetParentItem(m_htreeitemSelected);

	// attempt to get the handle to the item that will be selected after we delete the current...
	// first go for the next sibling... already attempted.  If null, try for the previous sibling.

	if ( htreeitemNextSelect == NULL )
		htreeitemNextSelect = m_treeDevices.GetNextItem(m_htreeitemSelected, TVGN_PREVIOUS);
	
	// if there aren't any siblings... go for the parent.  Still may be null.
	if ( htreeitemNextSelect == NULL )
		htreeitemNextSelect = m_treeDevices.GetNextItem(m_htreeitemSelected, TVGN_PARENT);

	// Before we can delete the item from the tree we need to find its position in the data structure
	// and kill it and all of the children too.

	CObArray * pparentobarray;

	switch( GetSelectedNodeLevel() )
	{
		case 0:		pparentobarray = (CObArray *) &m_devicesLocal;		break;
		case 1:		pparentobarray = (CObArray *) ((VTSDevice *) m_treeDevices.GetItemData(htreeitemParent))->GetObjects();	break;
		case 2:		pparentobarray = (CObArray *) ((VTSDevObject *) m_treeDevices.GetItemData(htreeitemParent))->GetProperties(); 	break;
		case 3:		pparentobarray = (CObArray *) ((VTSDevProperty *) m_treeDevices.GetItemData(htreeitemParent))->GetValues();		break;
		case 4:		pparentobarray = (CObArray *) ((VTSDevValue *) m_treeDevices.GetItemData(htreeitemParent))->GetValueList();		break;
		default:	ASSERT(0);
	}

	// now find the index into the array
	int i;
	for ( i = 0; i < pparentobarray->GetSize() && (DWORD) (*pparentobarray)[i] != m_treeDevices.GetItemData(m_htreeitemSelected); i++ );

	// we should find the item... if not, ASSERT
	ASSERT(i < pparentobarray->GetSize());

	if ( i >= pparentobarray->GetSize() )
		return;

	// Select the next item.  We HAVE to select the item first, before we delete the memory so
	// the property pages don't attempt to suck out the control data into the meory

	HTREEITEM htreeitemDelete = m_htreeitemSelected;

	// if there is another item to select, no problem... pages manage correctly.  If not, we need to fugde
	// the destruction of the current page.

	if ( htreeitemNextSelect != NULL )
	{
		m_treeDevices.EnsureVisible(htreeitemNextSelect);
		m_treeDevices.SelectItem(htreeitemNextSelect);
	}
	else
	{
		OnSelchangedDeviceTree(NULL, NULL);
	}

	// now kill the actual item...  all items MUST have virtual destructors for cascading delete to work!
	delete (CObject *) (*pparentobarray)[i];
	pparentobarray->RemoveAt(i);

	// handles all children too
	m_treeDevices.DeleteItem(htreeitemDelete);
	m_treeDevices.SetFocus();
}


void VTSDevicesTreeDlg::OnUpdateDelete(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_pnodeSelected != NULL);
}



void VTSDevicesTreeDlg::OnKeydownDevicetree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here

	switch(pTVKeyDown->wVKey)
	{
		case VK_DELETE: 

			if ( m_htreeitemSelected != NULL )
				OnDelete();
			break;
	}
	
	*pResult = 0;
}

void VTSDevicesTreeDlg::OnImport()
{
	
}

void VTSDevicesTreeDlg::OnExport()
{
	FILE *mfile;
	// TODO: open file for our information
	mfile=fopen("VTS_Devices_01.ini","w+");

	fprintf(mfile, "VTS Devices\n");
	fprintf(mfile, "{\n");

	// VTSDevices
	for ( int i = 0; i < m_devicesLocal.GetSize(); i++ )
	{
		VTSDevice * p = m_devicesLocal[i];
		VTSDevObjects * pobjects;
		VTSDevProperties * pproperties;
		VTSDevValues * pvalues;

		int j, k, l, m;

		// TODO: write device information out to file
		fprintf(mfile, "{\n");
		fprintf(mfile, "Instance=%d\n", p->m_nInstance);
		fprintf(mfile, "object-name=%s\n", p->m_strName);
		fprintf(mfile, "number-of-APDU-retries=%d\n", p->m_nAPDURetries);
		fprintf(mfile, "apdu-segment-timeout=%d\n", p->m_nAPDUSegmentTimeout);
		fprintf(mfile, "apdu-timeout=%d\n", p->m_nAPDUTimeout);
		fprintf(mfile, "max-APDU-length-supported=%d\n", p->m_nMaxAPDUSize);
		fprintf(mfile, "max-segments-accepted=%d\n", p->m_nSegmentSize);
		fprintf(mfile, "vendor-identifier=%d\n", p->m_nVendorID);
		fprintf(mfile, "segmentation-supported=%d\n", p->m_segmentation);
		fprintf(mfile, "WindowSize=%d\n", p->m_nWindowSize);



		for ( j = 0, pobjects = p->GetObjects(); pobjects != NULL && j < pobjects->GetSize(); j++ )
		{
			// TODO: now write the object information
			fprintf(mfile, "{\n");
			fprintf(mfile, "object-identifier=%d:%d\n", (*pobjects)[j]->GetType(), (*pobjects)[j]->GetInstance() );

			for ( k = 0, pproperties = (*pobjects)[j]->GetProperties(); pproperties != NULL && k < pproperties->GetSize(); k++ )
			{
				// TODO: now write the property information
				fprintf(mfile, "{\n");
				fprintf(mfile, "Property=%d\n", (*pproperties)[k]->GetID() );
				for ( l = 0, pvalues = (*pproperties)[k]->GetValues(); pvalues != NULL && l < pvalues->GetSize(); l++ )
				{
					fprintf(mfile, "Value=%d:", (*pvalues)[l]->m_nLength);
					for ( m = 0; m < (*pvalues)[l]->m_nLength; m++ )
					{
						fprintf(mfile, "%x ",(*pvalues)[l]->m_abContent[m] );
					}
					fprintf(mfile, "\n");
				}
				fprintf(mfile, "} End of Property\n");
			}
			fprintf(mfile, "} End of Object\n");

		}
		fprintf(mfile, "} End of Device\n");
	}

	// TODO: close file
	fprintf(mfile, "} End of Devices\n");
	fclose(mfile);

}

