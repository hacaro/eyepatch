#include "precomp.h"
#include "ClassifierOutputData.h"


ClassifierOutputData::ClassifierOutputData() {
}

ClassifierOutputData::~ClassifierOutputData() {
}

void ClassifierOutputData::AddVariable(string name, int value) {
	ClassifierOutputVariable var(name, value);
	data.push_back(var);
}

void ClassifierOutputData::AddVariable(string name, float value) {
	ClassifierOutputVariable var(name, value);
	data.push_back(var);
}

void ClassifierOutputData::AddVariable(string name, string value) {
	ClassifierOutputVariable var(name, value);
	data.push_back(var);
}

void ClassifierOutputData::AddVariable(string name, IplImage* value) {
	ClassifierOutputVariable var(name, value);
	data.push_back(var);
}

void ClassifierOutputData::AddVariable(string name, CvSeq* value) {
	ClassifierOutputVariable var(name, value);
	data.push_back(var);
}

ClassifierOutputVariable ClassifierOutputData::GetVariable(string name) {
	for (int i=0; i<data.size(); i++) {
		ClassifierOutputVariable var = data[i];
		if (name.compare(var.GetName()) == 0) {
			return var;
		}
	}
	ClassifierOutputVariable error;
	return error;
}

bool ClassifierOutputData::HasVariable(string name) {
	for (int i=0; i<data.size(); i++) {
		ClassifierOutputVariable var = data[i];
		if (name.compare(var.GetName()) == 0) {
			return true;
		}
	}
	return false;
}

int ClassifierOutputData::GetIntData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetIntData();
}

float ClassifierOutputData::GetFloatData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetFloatData();
}

string ClassifierOutputData::GetStringData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetStringData();
}

IplImage* ClassifierOutputData::GetImageData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetImageData();
}

CvSeq* ClassifierOutputData::GetSequenceData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetSequenceData();
}
