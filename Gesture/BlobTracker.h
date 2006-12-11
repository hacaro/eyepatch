#pragma once

#include "TrajectoryList.h"

class BlobTracker
{
public:
    BlobTracker();
    ~BlobTracker(void);
    void LearnTrajectories(CvCapture* pCap);
    void GetTrajectories(vector<MotionTrack> *trackList);

private:
    bool isTrained;

    CvBlobTrackerAutoParam1 param;
    CvBlobTrackerAuto *pTracker;

    TrajectoryList trajectories;
};
