#pragma once

class HaarClassifier;

class HaarClassifierDialog : public CSimpleDialog<IDD_HAAR_DIALOG> {
public:
	HaarClassifierDialog(HaarClassifier*);
	~HaarClassifierDialog();

	BEGIN_MSG_MAP(CListDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CSimpleDialog<IDD_HAAR_DIALOG>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	HaarClassifier *parent;
	static DWORD WINAPI ThreadCallback(HaarClassifierDialog*);
	void Train();
	DWORD threadID;
	HANDLE m_hThread;

};

class HaarClassifier
{
public:
    HaarClassifier();
    ~HaarClassifier(void);

	void PrepareData(TrainingSet*);
	void StartTraining();
	void ClassifyFrame(IplImage*, list<Rect>*);

	bool isTrained, readyForTraining;
	int nStages, nStagesCompleted;

private:

	CvHaarClassifierCascade* cascade;
    CvMemStorage* storage;

    char vecFilename[MAX_PATH];
    char negFilename[MAX_PATH];
    char classifierPathname[MAX_PATH];
    char classifierName[MAX_PATH];
	int nPosSamples, nNegSamples;

	friend class HaarClassifierDialog;
	HaarClassifierDialog m_progressDlg;

};
