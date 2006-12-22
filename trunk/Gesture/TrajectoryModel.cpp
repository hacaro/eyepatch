#include "precomp.h"
#include "TrajectoryModel.h"

TrajectoryModel::TrajectoryModel(MotionTrack track) {
    length = track.size();

    velX = new double[length];
    velY = new double[length];
    sizeX = new double[length];
    sizeY = new double[length];

    for(int i = 0; i < length; i++) {
        velX[i] = track[i].vx;
        velY[i] = track[i].vy;
        sizeX[i] = track[i].sizex;
        sizeY[i] = track[i].sizey;
    }
}

TrajectoryModel::TrajectoryModel(FILE* src) {
	
	// read length from file
	fread(&length, sizeof(int), 1, src);

    velX = new double[length];
    velY = new double[length];
    sizeX = new double[length];
    sizeY = new double[length];

	// read trajectory data from file
	fread(velX, sizeof(double), length, src);
	fread(velY, sizeof(double), length, src);
	fread(sizeX, sizeof(double), length, src);
	fread(sizeY, sizeof(double), length, src);
}

TrajectoryModel::~TrajectoryModel() {
    delete[] velX;
    delete[] velY;
    delete[] sizeX;
    delete[] sizeY;
}

int TrajectoryModel::GetLength() {
    return length;
}

double TrajectoryModel::InterpolateSizeX(double phase)
{
    return LinearInterpolate(sizeX, phase);
}

double TrajectoryModel::InterpolateSizeY(double phase)
{
    return LinearInterpolate(sizeY, phase);
}

double TrajectoryModel::InterpolateVelX(double phase)
{
    return LinearInterpolate(velX, phase);
}

double TrajectoryModel::InterpolateVelY(double phase)
{
    return LinearInterpolate(velY, phase);
}


/**
* linearly interpolate the value at array[phase],
* where phase is not necessarily an integer
*/
double TrajectoryModel::LinearInterpolate(double *array, double phase)
{
    //not sure what to do in these cases...
    if(phase < 0)
        return array[0];
    else if(phase > length-1)
        return array[length-1];

    int left = (int)phase;
    int right = left+1;

    double leftVal = array[left];
    double rightVal = array[right];

    double slope = rightVal - leftVal;

    double mod = phase - (double)left;

    return leftVal + (mod * slope);
}

void TrajectoryModel::WriteToFile(FILE* dst) {
	fwrite(&length, sizeof(int), 1, dst);
	fwrite(velX, sizeof(double), length, dst);
	fwrite(velY, sizeof(double), length, dst);
	fwrite(sizeX, sizeof(double), length, dst);
	fwrite(sizeY, sizeof(double), length, dst);
}
