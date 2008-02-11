#pragma once

typedef enum ClassifierVariableType {
	CVAR_VOID = 0,
    CVAR_INT,
    CVAR_FLOAT,
    CVAR_STRING,
	CVAR_IMAGE,
	CVAR_SEQ
} ClassifierVariableType;

class ClassifierOutputVariable {
public:
	ClassifierOutputVariable() {
		m_name = "ERROR";
		m_type = CVAR_VOID;
	}
	ClassifierOutputVariable(string name, int data) {
		m_name = name;
		m_type = CVAR_INT;
		m_intdata = data;
	}
	ClassifierOutputVariable(string name, float data) {
		m_name = name;
		m_type = CVAR_FLOAT;
		m_floatdata = data;
	}
	ClassifierOutputVariable(string name, string data) {
		m_name = name;
		m_type = CVAR_STRING;
		m_stringdata = data;
	}
	ClassifierOutputVariable(string name, IplImage *data) {
		m_name = name;
		m_type = CVAR_IMAGE;
		m_imagedata = data;
	}
	ClassifierOutputVariable(string name, CvSeq *data) {
		m_name = name;
		m_type = CVAR_SEQ;
		m_sequencedata = data;
	}

	~ClassifierOutputVariable() { }
	string GetName() { return m_name; }
	ClassifierVariableType GetType() { return m_type; }

	int GetIntData() { assert(m_type==CVAR_INT);	return m_intdata; }
	float GetFloatData() { assert(m_type==CVAR_FLOAT);	return m_floatdata; }
	string GetStringData() { assert(m_type==CVAR_STRING);	return m_stringdata; }
	IplImage* GetImageData() { assert(m_type==CVAR_IMAGE);	return m_imagedata; }
	CvSeq* GetSequenceData() { assert(m_type==CVAR_SEQ);	return m_sequencedata; }

private:
	string m_name;
	ClassifierVariableType m_type;
	int m_intdata;
	float m_floatdata;
	string m_stringdata;
	IplImage *m_imagedata;
	CvSeq* m_sequencedata;
};

class ClassifierOutputData {
public:
	ClassifierOutputData();
	~ClassifierOutputData();

	void AddVariable(string name, int value);
	void AddVariable(string name, float value);
	void AddVariable(string name, string value);
	void AddVariable(string name, IplImage* value);
	void AddVariable(string name, CvSeq* value);

	int GetIntData(string name);
	float GetFloatData(string name);
	string GetStringData(string name);
	IplImage* GetImageData(string name);
	CvSeq* GetSequenceData(string name);

	bool HasVariable(string name);
	ClassifierOutputVariable GetVariable(string name);
	vector<ClassifierOutputVariable> data;
private:
};