#pragma once

class TrajectoryModel  {
public:

    TrajectoryModel(MotionTrack track);
	TrajectoryModel(FILE *src);
    ~TrajectoryModel();

	void WriteToFile(FILE* dst);

    int GetLength();
    double InterpolateSizeX(double phase);
    double InterpolateSizeY(double phase);
    double InterpolateVelX(double phase);
    double InterpolateVelY(double phase);

    double *velX; // x velocities
    double *velY; // y velocities
    double *sizeX; // x positions
    double *sizeY; // y positions

private:
    double LinearInterpolate(double *array, double phase);
    int length; //length of model in timesteps
};
