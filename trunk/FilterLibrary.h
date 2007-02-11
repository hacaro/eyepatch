#pragma once

class CFilterLibrary : public CDialogImpl<CFilterLibrary> {
public:
    enum { IDD = IDD_FILTERLIBRARY_DIALOG };

    BEGIN_MSG_MAP(CFilterLibrary)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        MESSAGE_HANDLER(WM_ENABLE, OnEnable)
        NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnTextCallback)
        NOTIFY_CODE_HANDLER(LVN_ENDLABELEDIT, OnNameChange)
        NOTIFY_CODE_HANDLER(LVN_ITEMACTIVATE, OnItemActivate)
        NOTIFY_CODE_HANDLER(LVN_KEYDOWN, OnItemKeyDown)
    END_MSG_MAP()

    CFilterLibrary(CWindow *parent);
    ~CFilterLibrary();
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnCommand(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnEnable(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnTextCallback(int, LPNMHDR, BOOL&);
    LRESULT OnNameChange(int, LPNMHDR, BOOL&);
    LRESULT OnItemActivate(int, LPNMHDR, BOOL&);
    LRESULT OnItemKeyDown(int, LPNMHDR, BOOL&);

    void AddCustomFilter(HWND listView, Classifier* classifier);
    void RemoveCustomFilter(HWND listView, Classifier* classifier);
    void ClearCustomFilters(HWND listView);

private:
    CWindow *parent;
};
