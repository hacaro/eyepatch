#pragma once

#include "TrajectoryModel.h"

// 30% temporal and spatial scaling
#define ALPHA_MIN 0.7
#define ALPHA_MAX 1.3
#define RHO_MIN 0.7
#define RHO_MAX 1.3

// in Black and Jepson they take standard deviations as 1.0 for model trajectories (?)
#define SIGMA_I 1.0

#define DIFFUSION_RHO .01
#define DIFFUSION_MODEL_NUM .05
#define DIFFUSION_ALPHA .01
#define WINDOW 10
#define MAX_UPDATE_ATTEMPTS 10

//the percent of the sample set we reinitialize
//at each update to avoid local maxima
#define REINIT_PERCENT (.1)

class CondensationSample
{
public:
    CondensationSample(int modelNum, int phaseLeft, double alphaLeft, double rhoLeft, 
        int phaseRight, double alphaRight, double rhoRight, double weight);  
    CondensationSample();

    ~CondensationSample() { };

    void Set(int modelNum, int phaseLeft, double alphaLeft, double rhoLeft, 
        int phaseRight, double alphaRight, double rhoRight, double weight);
    void SetWeight(double weight) { this->weight = weight; }

    double GetWeight() { return weight;}
    int GetModelNum() { return modelNum; }
    int GetPhaseLeft() { return phaseLeft; }
    double GetAlphaLeft() { return alphaLeft; }
    double GetRhoLeft() { return rhoLeft; }

    int GetPhaseRight() { return phaseRight; }
    double GetAlphaRight() { return alphaRight; }
    double GetRhoRight() { return rhoRight; }


private:
    int modelNum;

    double phaseLeft;
    double alphaLeft;
    double rhoLeft;

    double phaseRight;
    double alphaRight;
    double rhoRight;

    double weight;
};

/**
* a simple class to hold a CondensationSample and the cumulative weight of that
* sample in a cumulative table. 
* Also has a comparator to be used by bsearch
*/
class CumulativeCondensationSample 
{
public:
    CumulativeCondensationSample() { sample = NULL; cumulativeWeight = 0;}

    CondensationSample *sample;
    double cumulativeWeight;

    static int Comparator(const void *key, const void *element);
};

class CondensationSampleSet
{
public:
    CondensationSampleSet(int numSamples, TrajectoryModel **models, int numModels);
    ~CondensationSampleSet();
    void Init();
    void InitSample(CondensationSample *sample, int startPhase);
    //update the set with new data
    void Update(double velX, double velY, double sizeX, double sizeY); 
    double GetModelProbability(int modelNum);
    double GetModelCompletionProbability(int modelNum);

private:
    void AddObservation(double velX, double velY, double sizeX, double sizeY);
    void MakeCumulativeTable(CondensationSample **samplesToAccumulate, int nSamples);
    CondensationSample *ChooseRandomSample();
    double CalcWeight(int modelNum, int phaseLeft, double rhoLeft, 
        double alphaLeft, int phaseRight, double rhoRight,
        double alphaRight); 

    double *modelWeights, *modelCompletionProbs;
    int numSamples;
    CondensationSample **samples; //array of pointers to CondensationSamples
    CondensationSample **nextSamples; //used during prediction phase

    CumulativeCondensationSample **cumulativeSamples; //cumulative table of samples

    int numModels;  //number of models
    TrajectoryModel **models; //array of ptrs to models

    double sizeXObservations[WINDOW];
    double sizeYObservations[WINDOW];
    double velXObservations[WINDOW];
    double velYObservations[WINDOW];

    int numObservations;  
};
