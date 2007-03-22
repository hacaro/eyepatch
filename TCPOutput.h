#pragma once
#include "OutputSink.h"

// Socket server includes
#include "../SocketServer/SocketServer.h"

class TCPOutput : public OutputSink {
public:
    TCPOutput();
    ~TCPOutput();
    void OutputData(IplImage* image, IplImage* mask, char* filterName);

private:
    SocketServer server;
    CvMemStorage *contourStorage;
    HANDLE m_hThread;
	DWORD threadID;
	static DWORD WINAPI ServerListenThread(SocketServer*);
};
