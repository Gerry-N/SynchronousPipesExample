#include "SyncPipeServer.h"
#include "windows.h"
#include <stdio.h>

SyncPipeServer::SyncPipeServer(bool start_monitoring)
{
	if (start_monitoring) MonitorDataConnection();
}

void SyncPipeServer::MonitorDataConnection()
{
	CreateThread(NULL, 0, DataThread, this, 0, 0);
}

DWORD WINAPI SyncPipeServer::DataThread(void* pvoid)
{
	SyncPipeServer* pSyncPipeServer = (SyncPipeServer*)pvoid;

	HANDLE pipe = CreateNamedPipe(DATA_PIPE_NAME, PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE,// | PIPE_READMODE_MESSAGE,
		1, DATA_BUFFER_SIZE, COMMAND_BUFFER_SIZE, 0, NULL);
	if (pipe == INVALID_HANDLE_VALUE)
	{
		printf("CreateNamedPipe failed. Error #%d.\n", GetLastError());
		Sleep(200);
		pSyncPipeServer->MonitorDataConnection();
		return -1;
	}

	if (!ConnectNamedPipe(pipe, NULL))
	{
		int err = GetLastError();
		if (err != ERROR_PIPE_CONNECTED)
		{
			printf("ConnectNamedPipe failed. Error #%d.\n", err);
			Sleep(200);
			FlushFileBuffers(pipe);
			DisconnectNamedPipe(pipe);
			CloseHandle(pipe);
			pSyncPipeServer->MonitorDataConnection();
			return -1;
		}
	}
	static char connected_msg[] = "start.\n";
	if (!WriteFile(pipe, (LPVOID)connected_msg, strlen(connected_msg), 0, NULL))
	{
		printf("WriteFile failed on connection. Error #%d.\n", GetLastError());
		Sleep(200);
		FlushFileBuffers(pipe);
		DisconnectNamedPipe(pipe);
		CloseHandle(pipe);
		pSyncPipeServer->MonitorDataConnection();
		return -1;
	}

	for (;;)
	{
		char read_buffer[COMMAND_BUFFER_SIZE];
		DWORD num_read;
		if (!ReadFile(pipe, (LPVOID)read_buffer, COMMAND_BUFFER_SIZE, &num_read, NULL))
		{
			printf("ReadFile failed. Error #%d.\n", GetLastError());
			Sleep(200);
			FlushFileBuffers(pipe);
			DisconnectNamedPipe(pipe);
			CloseHandle(pipe);
			pSyncPipeServer->MonitorDataConnection();
			return -1;
		}
		if (num_read >= COMMAND_BUFFER_SIZE - 1)
		{
			num_read = COMMAND_BUFFER_SIZE - 1;
		}
		read_buffer[num_read] = 0;
		if (strncmp("stop", read_buffer, 4) != 0)
		{
			printf("command string: %s\n", read_buffer);
			char write_buffer[DATA_BUFFER_SIZE];
			static int msg_counter = 0;
			sprintf_s(write_buffer, "Message #%d.\n", msg_counter++);
			if (!WriteFile(pipe, (LPVOID)write_buffer, strlen(write_buffer), 0, NULL))
			{
				printf("WriteFile failed. Error #%d.\n", GetLastError());
				Sleep(200);
				FlushFileBuffers(pipe);
				DisconnectNamedPipe(pipe);
				CloseHandle(pipe);
				pSyncPipeServer->MonitorDataConnection();
				return -1;
			}
			//sleep a little while between cycles.
			Sleep(1);
		}
		else
		{
			static char stop_msg[] = "stop\n";
			if (!WriteFile(pipe, (LPVOID)stop_msg, strlen(stop_msg), 0, NULL))
			{
				printf("WriteFile failed on stop. Error #%d.\n", GetLastError());
				FlushFileBuffers(pipe);
				DisconnectNamedPipe(pipe);
				CloseHandle(pipe);
				return -1;
			}
			exit(0);
			return 0;
		}
	}
	exit(-1);
	return -1;
}
