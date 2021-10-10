#ifndef SYNCPIPESERVER_H
#define SYNCPIPESERVER_H

#define DATA_PIPE_NAME TEXT("\\\\.\\pipe\\server_data")
#define DATA_BUFFER_SIZE 0x400
#define COMMAND_BUFFER_SIZE 0x100

class SyncPipeServer
{
public:
	SyncPipeServer(bool start_monitoring = true);
	void MonitorDataConnection();
private:
	static unsigned long __stdcall DataThread(void*);
};
#endif

