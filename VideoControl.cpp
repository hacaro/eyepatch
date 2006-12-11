#include "precomp.h"
#include "VideoControl.h"

CVideoControl::CVideoControl(CWindow *caller) {
    parent = caller;
}

CVideoControl::~CVideoControl(void) {
}

LRESULT CVideoControl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    return 0;
}

LRESULT CVideoControl::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    parent->SendMessage(uMsg, wParam, lParam);
    return 0;
}

LRESULT CVideoControl::OnTrack(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    parent->SendMessage(uMsg, wParam, lParam);
    return 0;
}

LRESULT CVideoControl::OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    GetDlgItem(IDC_VIDEOSLIDER).EnableWindow(wParam);
    GetDlgItem(IDC_FRAMELEFT).EnableWindow(wParam);
    GetDlgItem(IDC_FRAMERIGHT).EnableWindow(wParam);
    return 1;
}

void CVideoControl::EnableSelectionRange(BOOL enabled) {
    GetDlgItem(IDC_MARKIN).EnableWindow(enabled);
    GetDlgItem(IDC_MARKOUT).EnableWindow(enabled);
    GetDlgItem(IDC_GRABRANGE).EnableWindow(enabled);

    if (enabled) {
        GetDlgItem(IDC_VIDEOSLIDER).SetWindowLong(GWL_STYLE,  WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_ENABLESELRANGE | TBS_BOTH );
        SendDlgItemMessage(IDC_VIDEOSLIDER, TBM_CLEARSEL, TRUE, 0);
    } else {
        GetDlgItem(IDC_VIDEOSLIDER).SetWindowLong(GWL_STYLE,  WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_BOTH );
        SendDlgItemMessage(IDC_VIDEOSLIDER, TBM_CLEARSEL, TRUE, 0);
    }
}

