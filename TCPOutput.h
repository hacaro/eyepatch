#pragma once
#include "OutputSink.h"

// Socket server includes
#include "../SocketServer/SocketServer.h"

class TCPOutput : public OutputSink {

public:
    TCPOutput();
    ~TCPOutput();
	void ProcessInput(IplImage* image);
    void ProcessOutput(IplImage* image, IplImage* mask, CvSeq* contours, char* filterName);

private:
    SocketServer server;
    HANDLE m_hThread;
	DWORD threadID;
	static DWORD WINAPI ServerListenThread(SocketServer*);
};
