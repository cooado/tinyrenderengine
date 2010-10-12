// PackageResource.cpp : implementation file
//

#include "stdafx.h"
#include "EditorUI.h"
#include "PackageResource.h"
#include "afxdialogex.h"


// PackageResource dialog

IMPLEMENT_DYNAMIC(PackageResource, CDialogEx)

PackageResource::PackageResource(CWnd* pParent /*=NULL*/)
	: CDialogEx(PackageResource::IDD, pParent)
{

}

PackageResource::~PackageResource()
{
}

void PackageResource::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PackageResource, CDialogEx)
END_MESSAGE_MAP()


// PackageResource message handlers
