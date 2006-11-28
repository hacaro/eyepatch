#pragma once

class CFilterSelect : public CDialogImpl<CFilterSelect> {
public:
   enum { IDD = IDD_FILTERSELECT_DIALOG };

    BEGIN_MSG_MAP(CFilterSelect)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    END_MSG_MAP()

    CFilterSelect();
    ~CFilterSelect();
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
};
