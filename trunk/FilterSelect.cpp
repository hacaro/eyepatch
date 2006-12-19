#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
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
    GetDlgItem(IDC_SAVEFILTER).EnableWindow(wParam);
    GetDlgItem(IDC_FILTER_LIST).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_COLOR).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_SHAPE).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_BRIGHTNESS).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_FEATURES).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_APPEARANCE).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_MOTION).EnableWindow(wParam);
    GetDlgItem(IDC_RADIO_GESTURE).EnableWindow(wParam);
    return 1;
}

LRESULT CFilterSelect::OnTextCallback(int idCtrl, LPNMHDR pnmh, BOOL&) {

    HWND listView = GetDlgItem(IDC_FILTER_LIST);
    NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pnmh;
    LVITEM item = plvdi->item;
    switch (item.iSubItem) {
        case 0:
            plvdi->item.pszText = ((Classifier*)item.lParam)->GetName();
            break;
        default:
            break;
    }

    return 0;
}

void CFilterSelect::AddSavedFilter(Classifier* classifier) {
    HWND listView = GetDlgItem(IDC_FILTER_LIST);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.lParam = (LPARAM) classifier;
    lvi.pszText = LPSTR_TEXTCALLBACK;
    ListView_InsertItem(listView, &lvi);
}
