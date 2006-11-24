#pragma once

class Classifier
{
public:

	Classifier() {
	    isTrained = false;
		readyForTraining = false;
		nPosSamples = 0;
		nNegSamples = 0;
	}
	virtual ~Classifier() {}

	virtual void PrepareData(TrainingSet*) = 0; 
	virtual void StartTraining() = 0;
	virtual void ClassifyFrame(IplImage*, list<Rect>*) = 0;

	bool isTrained, readyForTraining;

protected:
		int nPosSamples, nNegSamples;
};
