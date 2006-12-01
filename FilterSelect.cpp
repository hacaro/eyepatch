#include "precomp.h"
#include "FilterSelect.h"

CFilterSelect::CFilterSelect(CWindow *caller) {
    parent = caller;
}

CFilterSelect::~CFilterSelect(void) {
}

LRESULT CFilterSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    return 0;
}

LRESULT CFilterSelect::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    parent->SendMessage(uMsg, wParam, lParam);
    return 0;
}

LRESULT CFilterSelect::OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    GetDlgItem(IDC_TRAINBUTTON).EnableWindow(wParam);
    GetDlgItem(IDC_SHOWBUTTON).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_COLOR).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_SHAPE).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_FEATURES).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_APPEARANCE).EnableWindow(wParam);
    return 1;
}
