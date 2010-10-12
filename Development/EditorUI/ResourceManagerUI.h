#pragma once
#include "ChildView.h"
#include "PackageResource.h"

class CResourceManagerFrame : public CFrameWndEx
{
	
public:
	CResourceManagerFrame();
protected: 
	DECLARE_DYNAMIC(CResourceManagerFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CResourceManagerFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCRibbonBar     m_wndRibbonBar;
	CChildView    m_wndView;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	PackageResource* m_ResourcePanel;
public:
	afx_msg void OnButtonImportCollada();
	afx_msg void OnButtonLoadPackage();
	afx_msg void OnButtonClearOutput();
};


