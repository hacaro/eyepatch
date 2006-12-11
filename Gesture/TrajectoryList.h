#pragma once

class TrajectoryList : public CvBlobTrackGen {
public:
    TrajectoryList(int BlobSizeNorm = 0);
    ~TrajectoryList();
    void SetFileName(char* pFileName);
    void AddBlob(CvBlob* pBlob);
    void Process(IplImage* pImg = NULL, IplImage* pFG = NULL);
    void Release();

    int GetTracks(vector<MotionTrack> *trackList);

protected:
    int         m_Frame;
    char*       m_pFileName;
    CvBlobSeq   m_TrackList;
    int         m_BlobSizeNorm;
};
