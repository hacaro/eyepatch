#include "precomp.h"
#include "../constants.h"
#include "Condensation.h"
#include "RandomUtils.h"

/**
* create a new CondensationCondensationSample.  See Set for parameter descriptions
*/
CondensationSample::CondensationSample(int modelNum, 
               int phaseLeft, double alphaLeft, double rhoLeft, 
               int phaseRight, double alphaRight, double rhoRight,
               double weight)
{

    Set(modelNum, phaseLeft, alphaLeft, rhoLeft, phaseRight, alphaRight, rhoRight, weight);
}

/**
* create a new, unitialized sample (all params set to 0)
*/
CondensationSample::CondensationSample()
{
    Set(0,0,0,0,0,0,0,0);
}

/**
* set the parameters of the set
* @param modelNum the index of the modelNum
* @param phase indicates the current position in the model
* @param alpha the amplitude scaling factor
* @param rho the time scaling factor
* @param weight -- the weight of this sample (proportional to its prob)
*/
void CondensationSample::Set(int modelNum, 
                 int phaseLeft, double alphaLeft, double rhoLeft,
                 int phaseRight, double alphaRight, double rhoRight,
                 double weight)
{
    this->modelNum = modelNum;
    this->phaseLeft = phaseLeft;
    this->alphaLeft = alphaLeft;
    this->rhoLeft = rhoLeft;

    this->phaseRight = phaseRight;
    this->alphaRight = alphaRight;
    this->rhoRight = rhoRight;

    this->weight = weight;
}


/****************
* definitions for CondensationSampleSet
*/

/**
* create a new CondensationSampleSet, with numSamples samples in it
*/
CondensationSampleSet::CondensationSampleSet(int numSamples, TrajectoryModel **models, int numModels) {
    int i;
    //store info on the models
    this->numModels = numModels;
    this->models = models;
    this->numSamples = numSamples;

    samples = new CondensationSample*[numSamples];
    nextSamples = new CondensationSample*[numSamples];
    modelWeights = new double[numModels];
    modelCompletionProbs = new double[numModels];
    cumulativeSamples = new CumulativeCondensationSample*[numSamples];

    for(i = 0; i < numSamples; i++) {
        samples[i] = new CondensationSample();   
        nextSamples[i] = new CondensationSample();
        cumulativeSamples[i] = new CumulativeCondensationSample();
    }

    srand(time(NULL)); //seed random number generate  

    this->numObservations=0;
    Init();
}

/**
* clean up memory allocated in constructor
*/
CondensationSampleSet::~CondensationSampleSet()
{
    for(int i = 0; i < numSamples; i++) {
        delete samples[i];
        delete nextSamples[i];
    }

    delete[] samples;
    delete[] nextSamples;
    delete[] modelWeights;
    delete[] modelCompletionProbs;
    delete[] cumulativeSamples;
}

double CondensationSampleSet::GetModelProbability(int modelNum) {
    if ((modelNum < 0) || (modelNum > numModels)) return 0;
    return modelWeights[modelNum];
}

double CondensationSampleSet::GetModelCompletionProbability(int modelNum) {
    if ((modelNum < 0) || (modelNum > numModels)) return 0;
    return modelCompletionProbs[modelNum];
}


/**
* for now, just initialize as in Black & Jepson paper
*/
void CondensationSampleSet::Init()
{
    for(int i = 0; i < numSamples; i++) {
        InitSample(samples[i], 0);
    }
}

/**
* initialize a specific sample uniformly
*/
void CondensationSampleSet::InitSample(CondensationSample *sample, int startPhase)
{
    double weight = 1.0 / numSamples;

    int modelNum;
    int phaseLeft;
    double alphaLeft;
    double rhoLeft;
    int phaseRight;
    double alphaRight;
    double rhoRight;

    int modelLength;
    double sqr;


    modelNum = RandomInt(0, numModels); //pick modelNum uniformly

    //pick phase as (1 - sqrt(y))/sqrt(y) 
    //must be a legitimate phase for the given model
    modelLength = models[modelNum]->GetLength();
    do {
        sqr = sqrt(RandomDouble(.00000000000001, 1));
        phaseLeft = startPhase + (int)((1.0 - sqr) / sqr);
    } while(phaseLeft > modelLength - 1);

    do {
        sqr = sqrt(RandomDouble(.00000000000001, 1));
        phaseRight = startPhase + (int)((1.0 - sqr) / sqr);
    } while(phaseRight > modelLength - 1);


    alphaLeft = RandomDouble(ALPHA_MIN, ALPHA_MAX);
    alphaRight = RandomDouble(ALPHA_MIN, ALPHA_MAX);

    rhoLeft = RandomDouble(RHO_MIN, RHO_MAX);
    rhoRight = RandomDouble(RHO_MIN, RHO_MAX);


    /*
    printf("initializing to: %d, %d, %f, %f, %f\n", modelNum, phase,
    alpha, rho, weight);
    */

    sample->Set(modelNum, phaseLeft, alphaLeft, rhoLeft, 
        phaseRight, alphaRight, rhoRight, weight);
}

/**
* update with new observations
*/
void CondensationSampleSet::Update(double velX, double velY, double sizeX, double sizeY) {
    CondensationSample *sample, *nextSample;
    int modelNum, phaseLeft, phaseRight;
    double alphaLeft, rhoLeft, oldAlphaLeft, oldRhoLeft;
    double alphaRight, rhoRight, oldAlphaRight, oldRhoRight; 
    double weight;

    AddObservation(velX, velY, sizeX, sizeY);
    MakeCumulativeTable(samples, numSamples);

    //samples to reinit
    int numHoldout = (int)(REINIT_PERCENT * numSamples);

    for(int i = 0; i < numHoldout; i++) {
        nextSamples[i]->SetWeight(-1);
    }

    int numBad = numHoldout; //number of "bad" samples that need to be reinited
    double totalWeight = 0.0;
    for(int i = numHoldout; i < numSamples; i++) {
        sample = ChooseRandomSample();
        nextSample = nextSamples[i]; 

        for(int numAttempts = 0; numAttempts < MAX_UPDATE_ATTEMPTS;
            numAttempts++) {
                modelNum = sample->GetModelNum();

                phaseLeft = (int)((double)sample->GetPhaseLeft() + sample->GetRhoLeft() +
                    RandomGaussian(0, DIFFUSION_MODEL_NUM));

                phaseRight = (int)((double)sample->GetPhaseRight() + sample->GetRhoRight() +
                    RandomGaussian(0, DIFFUSION_MODEL_NUM));


                int length = models[modelNum]->GetLength();
                if(phaseLeft >= length || phaseRight >= length) {
                    nextSample->SetWeight(-1); //will be reinited later
                    numBad++;
                    //InitSample(nextSample); //past the end of the phase, reinit
                    break; //break out of attempts loop
                } else {
                    oldAlphaLeft = sample->GetAlphaLeft();

                    do {
                        alphaLeft = oldAlphaLeft + RandomGaussian(0, DIFFUSION_ALPHA);
                    } while(alphaLeft < ALPHA_MIN || alphaLeft > ALPHA_MAX);

                    oldAlphaRight = sample->GetAlphaRight();


                    do {
                        alphaRight = oldAlphaRight + RandomGaussian(0, DIFFUSION_ALPHA);
                    } while(alphaRight < ALPHA_MIN || alphaRight > ALPHA_MAX);


                    oldRhoLeft = sample->GetRhoLeft();
                    do {
                        rhoLeft = oldRhoLeft + RandomGaussian(0, DIFFUSION_RHO);
                    } while(rhoLeft < RHO_MIN || rhoLeft > RHO_MAX);

                    oldRhoRight = sample->GetRhoRight();

                    do {
                        rhoRight = oldRhoRight + RandomGaussian(0, DIFFUSION_RHO);
                    } while(rhoRight < RHO_MIN || rhoRight > RHO_MAX);


                    weight = CalcWeight(modelNum, phaseLeft, rhoLeft, alphaLeft, 
                        phaseRight, rhoRight, alphaRight); 

                    if(weight > 0.0) {
                        nextSample->Set(modelNum, phaseLeft, alphaLeft, rhoLeft, 
                            phaseRight, alphaRight, rhoRight, weight);
                        totalWeight += weight;
                        break;
                    }
                }

                //our last time through the loop and we've failed
                if(numAttempts == MAX_UPDATE_ATTEMPTS - 1) {
                    nextSample->SetWeight(-1); //will be reinited later
                    numBad++;
                    //InitSample(nextSample); //generate a random initial sample
                }
        }

    }

    //renormalize
    totalWeight += numBad * (totalWeight / (numSamples - numBad)); 

    for(int i = 0; i < numSamples; i++) {
        nextSample = nextSamples[i];
        weight = nextSample->GetWeight();
        if(weight == -1) {
            InitSample(nextSample, numObservations);
        } else {
            nextSample->SetWeight(weight/totalWeight);
        }
    }

    //swap buffers
    CondensationSample **temp = samples;
    samples = nextSamples;
    nextSamples = temp;

}

/**
* calculate the weight of a sample, based on its parameters.
* Uses equation 1 in Black & Jepson to calc p(z_t|s_t)
* to look back over a time window and see how well
* the observations match those predicted by the
* model
* @return the weight of the sample
*/
double CondensationSampleSet::CalcWeight(int modelNum, int phaseLeft, double rhoLeft, 
                             double alphaLeft, int phaseRight,
                             double rhoRight, double alphaRight) 
{
    double constant = 1 / (sqrt(2.0 * M_PI) * SIGMA_I);
    //double dividObs = numObservations == 1 ? 1 : numObservations-1;
    double dividObs = numObservations;
    double divisor = 2.0 * SIGMA_I * SIGMA_I * dividObs;

    TrajectoryModel *model = models[modelNum];

    double velXErrorSum = 0.0;
    double velYErrorSum = 0.0;

    double sizeXErrorSum = 0.0;
    double sizeYErrorSum = 0.0;

    double error;
    for(int j = 0; j < numObservations; j++) {
        error = sizeXObservations[numObservations - 1 - j] - 
            alphaLeft * model->InterpolateSizeX(phaseLeft - rhoLeft*((double)j));

        error = error * error;
        sizeXErrorSum += error;


        error = sizeYObservations[numObservations - 1 - j] - 
            alphaLeft *  model->InterpolateSizeY(phaseLeft - rhoLeft*((double)j));

        error = error * error;
        sizeYErrorSum += error;

        error = velXObservations[numObservations - 1 - j] - 
            alphaRight * model->InterpolateVelX(phaseRight - rhoRight*((double)j));

        error = error * error;
        velXErrorSum += error;


        error = velYObservations[numObservations - 1 - j] - 
            alphaRight *  model->InterpolateVelY(phaseRight - rhoRight*((double)j));

        error = error * error;
        velYErrorSum += error;
    }

    double sizeXWeight = constant * exp(-sizeXErrorSum / divisor);
    double sizeYWeight = constant * exp(-sizeYErrorSum / divisor);

    double velXWeight = constant * exp(-velXErrorSum / divisor);
    double velYWeight = constant * exp(-velYErrorSum / divisor);

    double weight = sizeXWeight * sizeYWeight * velXWeight * velYWeight;
    //printf("weight = %e\n", weight);
    return weight;
}

/**
* we keep a window of old observations of length
* WINDOW; add a new observations to it
*/
void CondensationSampleSet::AddObservation(double velX, double velY, double sizeX, double sizeY)
{
    if(numObservations < WINDOW) {
        velXObservations[numObservations] = velX;
        velYObservations[numObservations] = velY;
        sizeXObservations[numObservations] = sizeX;
        sizeYObservations[numObservations] = sizeY;
        numObservations++;
    } else { //shift down one
        for(int i = 0; i < WINDOW-1; i++) {
            velXObservations[i] = velXObservations[i+1];
            velYObservations[i] = velYObservations[i+1];      
            sizeXObservations[i] = sizeXObservations[i+1];
            sizeYObservations[i] = sizeYObservations[i+1];      
        }
        velXObservations[WINDOW-1] = velX;
        velYObservations[WINDOW-1] = velY;    
        sizeXObservations[WINDOW-1] = sizeX;
        sizeYObservations[WINDOW-1] = sizeY;   
    }
}

/**
* build up a table of cumulative weights
*/
void CondensationSampleSet::MakeCumulativeTable(CondensationSample **samplesToAccumulate, int nSamples) {
    double weightSoFar = 0.0;
    CondensationSample *sample;
    CumulativeCondensationSample *cumSample;

    for(int i = 0; i < numModels; i++) {
        modelWeights[i] = 0.0;
        modelCompletionProbs[i] = 0.0;
    }
    for(int i = 0; i < nSamples; i++) {
        sample = samplesToAccumulate[i];

        modelWeights[sample->GetModelNum()] += sample->GetWeight();
        if (sample->GetPhaseRight() > models[sample->GetModelNum()]->GetLength() - GESTURE_PHASE_CUTOFF) {
            // this sample is near the end of the phase, so we add its weight to the 
            // gesture completion probability
            modelCompletionProbs[sample->GetModelNum()] += sample->GetWeight();
        }
        if (sample->GetPhaseLeft() > models[sample->GetModelNum()]->GetLength() - GESTURE_PHASE_CUTOFF) {
            // this sample is near the end of the phase, so we add its weight to the 
            // gesture completion probability
            modelCompletionProbs[sample->GetModelNum()] += sample->GetWeight();
        }

        cumSample = cumulativeSamples[i];
        weightSoFar += sample->GetWeight();
        cumSample->cumulativeWeight = weightSoFar;
        cumSample->sample = sample;
    }
}

/**
* choose a random sample from the cumulative table,
* by picking a uniformly random number and then
* choosing the sample in the cumulative table that is
* >= that number by the least amount
*/
CondensationSample *CondensationSampleSet::ChooseRandomSample()
{
    double total = cumulativeSamples[numSamples-1]->cumulativeWeight;

    double searchKey = RandomDouble(0, total);


    CumulativeCondensationSample **cumSamplePtr = 
        (CumulativeCondensationSample **)bsearch(&searchKey, cumulativeSamples, 
        numSamples, sizeof(CumulativeCondensationSample *),
        CumulativeCondensationSample::Comparator);
    if(cumSamplePtr == NULL) {
        printf("Error, binary search yielded NULL. THIS SHOULD NOT HAPPEN\n");
        printf("total = %f, searchKey = %f\n", total, searchKey);

        return samples[0]; //return a default value to keep from crashing
    }

    CumulativeCondensationSample *cumSample = *cumSamplePtr; //deref our pointer

    return cumSample->sample;
}



/****************************
* definitions for CumulativeCondensationSample class
*/


/**
* used by binary search,
* @param key should be a pointer to a double which is the 
* cumulative weight we are looking for
* @param element is a pointer to a CumulativeCondensationSample * in the array to see
* if the key is less than, greater than, or equal to this element
* @return -1 if the key is less than, 0 if equal, 1 if greater than element
*/
int CumulativeCondensationSample::Comparator(const void *key, const void *element)
{
    double cumWeight = *((double *)key);
    CumulativeCondensationSample *cumSample = *((CumulativeCondensationSample **)element);

    if(cumWeight < cumSample->cumulativeWeight - cumSample->sample->GetWeight())
        return -1;
    if(cumWeight > cumSample->cumulativeWeight)
        return 1;

    return 0; //cumWeight lies in the range spanned by this item
}
