#pragma once

#include "precomp.h"

//
// Recognizer class constants
//
#define NumTemplates 16
#define NumPoints 64
#define SquareSize 250.0
#define HalfDiagonal (0.5 * sqrt(250.0 * 250.0 + 250.0 * 250.0))
#define AngleRange 45.0
#define AnglePrecision 2.0
#define Phi (0.5 * (-1.0 + Math.sqrt(5.0)))

class OneDollarPoint {
public:
	double m_x, m_y;
	OneDollarPoint(double x, double y) {
		m_x = x;
		m_y = y;
	}
};


class OneDollarRectangle {
public:
	double m_x, m_y, m_width, m_height;
	OneDollarRectangle(double x, double y, double width, double height) {
		m_x = x;
		m_y = y;
		m_width = width;
		m_height = height;
	}
};

class Template {
public:
	string m_name;
	vector<OneDollarPoint> m_points;

	Template(string name, vector<OneDollarPoint> points); 
	Template(FILE *src);
	void WriteToFile(FILE *dst);
	int GetLength();
};

class Result {
public:
	string m_name;
	double m_score;
	int m_index;
	Result(string name, double score, int index) {
		m_name = name;
		m_score = score;
		m_index = index;
	}
};

class Recognizer {
public:
	vector<Template> m_templates;
	Recognizer();
	Result Recognize(vector<OneDollarPoint> points);
	Result BackRecognize(vector<OneDollarPoint> points);
	int AddTemplate(string name, OneDollarPoint* points, int npoints);
	int AddTemplate(string name, vector<OneDollarPoint> points);
	int AddTemplate(string name, Template t);
	int DeleteUserTemplates();
};

vector<OneDollarPoint> Resample(vector<OneDollarPoint> points, int n);
vector<OneDollarPoint> RotateToZero(vector<OneDollarPoint> points);
vector<OneDollarPoint> RotateBy(vector<OneDollarPoint> points, double theta);
vector<OneDollarPoint> ScaleToSquare(vector<OneDollarPoint> points, double size);
vector<OneDollarPoint> TranslateToOrigin(vector<OneDollarPoint> points);
vector<OneDollarPoint> ReverseOrder(vector<OneDollarPoint> points);
double DistanceAtBestAngle(vector<OneDollarPoint> points, Template T, double a, double b, double threshold);
double DistanceAtAngle(vector<OneDollarPoint> points, Template T, double theta);
OneDollarPoint Centroid(vector<OneDollarPoint> points);
OneDollarRectangle BoundingBox(vector<OneDollarPoint> points);
double PathDistance(vector<OneDollarPoint> pts1, vector<OneDollarPoint> pts2);
double PathLength(vector<OneDollarPoint> points);
double Distance(OneDollarPoint p1, OneDollarPoint p2);

