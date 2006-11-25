#pragma once

class Classifier
{
public:

	Classifier() {
	    isTrained = false;
		nPosSamples = 0;
		nNegSamples = 0;
	}
	virtual ~Classifier() {}

	virtual void StartTraining(TrainingSet*) = 0;
	virtual void ClassifyFrame(IplImage*, list<Rect>*) = 0;

	bool isTrained;

protected:
		int nPosSamples, nNegSamples;
};
