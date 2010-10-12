#pragma once


// PackageResource dialog

class PackageResource : public CDialogEx
{
	DECLARE_DYNAMIC(PackageResource)

public:
	PackageResource(CWnd* pParent = NULL);   // standard constructor
	virtual ~PackageResource();

// Dialog Data
	enum { IDD = IDD_DIALOG_RESOURCE_MANAGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
