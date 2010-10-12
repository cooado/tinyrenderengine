// ImportCOLLADA.cpp : implementation file
//

#include "stdafx.h"
#include "EditorUI.h"
#include "ImportCOLLADA.h"
#include "afxdialogex.h"


// ImportCOLLADA dialog

IMPLEMENT_DYNAMIC(ImportCOLLADA, CDialogEx)

ImportCOLLADA::ImportCOLLADA(CWnd* pParent /*=NULL*/)
	: CDialogEx(ImportCOLLADA::IDD, pParent)
{

}

ImportCOLLADA::~ImportCOLLADA()
{
}

void ImportCOLLADA::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ImportCOLLADA, CDialogEx)
	ON_BN_CLICKED(ID_IMPORT_COLLADA_OK, &ImportCOLLADA::OnBnClickedImportColladaOk)
	ON_BN_CLICKED(ID_IMPORT_COLLADA_CANCEL, &ImportCOLLADA::OnBnClickedImportColladaCancel)
END_MESSAGE_MAP()


// ImportCOLLADA message handlers


void ImportCOLLADA::OnBnClickedImportColladaOk()
{
	// TODO: Add your control notification handler code here
	EndDialog(IDOK);
}


void ImportCOLLADA::OnBnClickedImportColladaCancel()
{
	// TODO: Add your control notification handler code here
	EndDialog(IDCANCEL);
}
