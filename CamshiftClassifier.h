#pragma once
#include "Classifier.h"

class CamshiftClassifier : public Classifier {
public:
    CamshiftClassifier();
    ~CamshiftClassifier();

	void PrepareData(TrainingSet*);
	void StartTraining();
	void ClassifyFrame(IplImage*, list<Rect>*);

private:
	CvScalar hsv2rgb(float);

	IplImage *sampleimage, *image, *hsv, *hue, *mask, *backproject, *histimg;
	IplImage *samplehsv, *samplehue, *samplemask;
	CvHistogram *hist;


	int backproject_mode;
	int select_object;
	int track_object;
	int show_hist;
	CvPoint origin;
	CvRect selection;
	CvRect track_window;
	CvBox2D track_box;
	CvConnectedComp track_comp;
	int hdims;
	float hranges_arr[2];
	float* hranges;
	int vmin, vmax, smin;
};
