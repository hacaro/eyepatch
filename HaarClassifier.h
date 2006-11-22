#pragma once

class HaarClassifier
{
public:
    HaarClassifier();
    ~HaarClassifier(void);

	void PrepareData(TrainingSet*);
	void StartTraining();
	void Train();
	void CancelTraining();

	void ClassifyFrame(IplImage*, list<Rect>*);
    bool isTrained, readyForTraining;
	int nStages;

private:
	CSimpleDialog<IDD_HAAR_DIALOG> m_progressDlg;

	CvHaarClassifierCascade* cascade;
    CvMemStorage* storage;

	static DWORD WINAPI ThreadCallback(HaarClassifier*);
	DWORD threadID;
	HANDLE m_hThread;

    char vecFilename[MAX_PATH];
    char negFilename[MAX_PATH];
    char classifierPathname[MAX_PATH];
    char classifierName[MAX_PATH];
	int nPosSamples, nNegSamples, nCompletedStages;

};
