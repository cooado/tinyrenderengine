#pragma once


// ImportCOLLADA dialog

class ImportCOLLADA : public CDialogEx
{
	DECLARE_DYNAMIC(ImportCOLLADA)

public:
	ImportCOLLADA(CWnd* pParent = NULL);   // standard constructor
	virtual ~ImportCOLLADA();

// Dialog Data
	enum { IDD = IDD_DIALOG_IMPORT_COLLADA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedImportColladaOk();
	afx_msg void OnBnClickedImportColladaCancel();
};
