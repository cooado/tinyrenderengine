#include "stdafx.h"
#include "EditorUI.h"

#include "ResourceManagerUI.h"
#include "PackageResource.h"
#include "ImportCOLLADA.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CResourceManagerFrame

IMPLEMENT_DYNAMIC(CResourceManagerFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CResourceManagerFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_COMMAND(ID_BUTTON_IMPORT_COLLADA, &CResourceManagerFrame::OnButtonImportCollada)
	ON_COMMAND(ID_BUTTON_LOAD_PACKAGE, &CResourceManagerFrame::OnButtonLoadPackage)
	ON_COMMAND(ID_BUTTON_CLEAR_OUTPUT, &CResourceManagerFrame::OnButtonClearOutput)
END_MESSAGE_MAP()

// CResourceManagerFrame construction/destruction

CResourceManagerFrame::CResourceManagerFrame() : m_ResourcePanel(NULL)
{
	// TODO: add member initialization code here
}

CResourceManagerFrame::~CResourceManagerFrame()
{
}

int CResourceManagerFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	//BOOL bNameValid;
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON_RESOURCE_MANAGER);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Resource Panel
	m_ResourcePanel = new PackageResource(&m_wndView);
	m_ResourcePanel->Create(IDD_DIALOG_RESOURCE_MANAGER, &m_wndView);
	m_ResourcePanel->ShowWindow(SW_SHOW);
	
	return 0;
}

BOOL CResourceManagerFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// CResourceManagerFrame diagnostics

#ifdef _DEBUG
void CResourceManagerFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CResourceManagerFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CResourceManagerFrame message handlers

void CResourceManagerFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CResourceManagerFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CResourceManagerFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(TRUE);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CResourceManagerFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CResourceManagerFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWndEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( m_wndView.m_hWnd != NULL && m_ResourcePanel != NULL )
	{
		RECT rect;
		m_wndView.GetClientRect(&rect);
		LONG width = rect.right - rect.left;
		LONG height = rect.bottom - rect.top;
		CTreeCtrl* tc = (CTreeCtrl*)m_ResourcePanel->GetDlgItem(IDC_TREE_PACKAGE);
		CMFCPropertyGridCtrl* pgc = (CMFCPropertyGridCtrl*)m_ResourcePanel->GetDlgItem(IDC_MFCPROPERTYGRID_PACKAGE);
		CRichEditCtrl* rec = (CRichEditCtrl*)m_ResourcePanel->GetDlgItem(IDC_RICHEDIT2_PACKAGE_OUTPUT);

		m_ResourcePanel->MoveWindow(0, 0, width, height);
		tc->MoveWindow(0, 0, width/2, height);
		pgc->MoveWindow(width/2, 0, width/2, height*3/4);
		rec->MoveWindow(width/2, height*3/4, width/2, height/4);

	}
}


void CResourceManagerFrame::OnButtonImportCollada()
{
	// TODO: Add your command handler code here
	
	CFileDialog dlg(TRUE);
	if(IDOK == dlg.DoModal() )
	{
		CString filepath = dlg.GetFileName();

		ImportCOLLADA ic(this);
		ic.DoModal();
	}
}


void CResourceManagerFrame::OnButtonLoadPackage()
{
	// TODO: Add your command handler code here
}


void CResourceManagerFrame::OnButtonClearOutput()
{
	// TODO: Add your command handler code here
}
