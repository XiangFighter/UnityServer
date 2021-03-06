// LoginExampleServer.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock2.h>
#include <Mswsock.h>
#include <process.h>	//包含过程实现的有关函数
#include <list>		//list将元素按顺序储存在链表中，允许快速的插入和删除，但是随机访问较慢
#include "CommandDispatcher.h"
using namespace std;
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")


#define PORT 8080 
#define DATA_BUFSIZE 8192 

typedef struct
{
	OVERLAPPED Overlapped;	//OVERLAPPED是一个用于异步输入输出信息的结构体
	WSABUF DataBuf;	 //用来接收WSASocket数据的缓冲
	CHAR Buffer[DATA_BUFSIZE];	//缓冲区大小
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;


typedef struct
{
	SOCKET Socket;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

//DWORD全称Double Word，指注册表的键值，每个word为两个字节的长度2，双字节为4，即32位
DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID);

void main(void)
{
	CoInitialize(nullptr);
	SOCKADDR_IN InternetAddr;
	SOCKET Listen;
	SOCKET Accept;
	HANDLE CompletionPort;
	SYSTEM_INFO SystemInfo;
	LPPER_HANDLE_DATA PerHandleData;
	LPPER_IO_OPERATION_DATA PerIoData;
	int i;
	DWORD RecvBytes;
	DWORD Flags;
	DWORD ThreadID;
	WSADATA wsaData;
	DWORD Ret;

	//WSAStartup函数必须是应用程序或DLL调用的第一个Windows Socket函数，它允许应用程序或DLL指明Windows Sockets API
	//的版本号及获得特定的Windows Sockets实现细节。应用程序或DLL只能在一次成功的WSAStartup函数调用之后才能调用
	//进一步的Windows Sockets API函数。
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d/n", Ret);
		return;
	}

	//CreateIoCompletionPort函数关注一个已打开的文件实例和新建的或已存在的I/O完成端口，或者创建一个未关联任何文件的I/O完成端口
	if ((CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
	{
		printf("CreateIoCompletionPort failed with error: %d/n", GetLastError());
		return;
	}
	GetSystemInfo(&SystemInfo);	//返回当前系统信息

	for (i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++)
	{
		HANDLE ThreadHandle;
		if ((ThreadHandle = CreateThread(NULL, 0, ServerWorkerThread, CompletionPort,
			0, &ThreadID)) == NULL)
		{
			printf("CreateThread() failed with error %d/n", GetLastError());
			return;
		}
		CloseHandle(ThreadHandle);
	}

	if ((Listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("WSASocket() failed with error %d/n", WSAGetLastError());
		return;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d/n", WSAGetLastError());
		return;
	}

	if (listen(Listen, 64) == SOCKET_ERROR)
	{
		printf("listen() failed with error %d/n", WSAGetLastError());
		return;
	}

	while (TRUE)
	{
		if ((Accept = accept(Listen, NULL, NULL)) == SOCKET_ERROR)
		{
			printf("WSAAccept() failed with error %d/n", WSAGetLastError());
			return;
		}
		if ((PerHandleData = new PER_HANDLE_DATA) == NULL)
		{
			printf("GlobalAlloc() failed with error %d/n", GetLastError());
			return;
		}

		printf("Socket number %d connected/n", Accept);
		PerHandleData->Socket = Accept;

		if (CreateIoCompletionPort((HANDLE)Accept, CompletionPort, (DWORD)PerHandleData,
			0) == NULL)
		{
			printf("CreateIoCompletionPort failed with error %d/n", GetLastError());
			return;
		}

		if ((PerIoData = new PER_IO_OPERATION_DATA) == NULL)
		{
			printf("GlobalAlloc() failed with error %d/n", GetLastError());
			return;
		}

		ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));
		PerIoData->DataBuf.len = DATA_BUFSIZE;
		PerIoData->DataBuf.buf = PerIoData->Buffer;

		Flags = 0;
		if (WSARecv(Accept, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags,
			&(PerIoData->Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				printf("WSARecv() failed with error %d/n", WSAGetLastError());
				return;
			}
		}
	}
}

DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID)
{
	CommandDispatcher dispatcher;
	HANDLE CompletionPort = (HANDLE)CompletionPortID;
	DWORD BytesTransferred;

	LPPER_HANDLE_DATA PerHandleData;
	LPPER_IO_OPERATION_DATA PerIoData;
	DWORD SendBytes, RecvBytes;
	DWORD Flags;

	while (TRUE)
	{
		if (GetQueuedCompletionStatus(CompletionPort, &BytesTransferred,
			(LPDWORD)&PerHandleData, (LPOVERLAPPED *)&PerIoData, INFINITE) == 0)
		{
			printf("GetQueuedCompletionStatus failed with error %d/n", GetLastError());
			return 0;
		}

		if (BytesTransferred == 0)
		{
			/*printf("Closing socket %d/n", PerHandleData->Socket);

			if (closesocket(PerHandleData->Socket) == SOCKET_ERROR)
			{
				printf("closesocket() failed with error %d/n", WSAGetLastError());
				return 0;
			}*/
			CommandDispatcher::DispatchCommand("Disconnect()", PerHandleData->Socket);
			delete PerHandleData;
			delete PerIoData;
			continue;
		}
		PerIoData->DataBuf.buf[BytesTransferred] = 0;//添加字符串结束符
		dispatcher.DispatchCommand(PerIoData->DataBuf.buf, PerHandleData->Socket);//分发请求
		ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));
		PerIoData->DataBuf.len = DATA_BUFSIZE;
		PerIoData->DataBuf.buf = PerIoData->Buffer;
		Flags = 0;
		if (WSARecv(PerHandleData->Socket, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags,
			&(PerIoData->Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				printf("WSARecv() failed with error %d/n", WSAGetLastError());
				CommandDispatcher::DispatchCommand("Disconnect()", PerHandleData->Socket);
				delete PerHandleData;
				delete PerIoData;
				return 0;
			}
		}
	}
}

