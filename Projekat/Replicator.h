#pragma once

#ifndef REPLICATOR_H
#define REPLICATOR_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <winsock.h>
#include <tchar.h>
#include "..\Common\ReplicatorList.h"
//#include "..\Common\ProcessList.h"
#include "..\Common\CircularBuffer.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"
#define DEFAULT_PORT_R2 27017
#define GUID_FORMAT "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX"
#define GUID_ARG(guid) guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]

bool InitializeWindowsSockets();

DWORD WINAPI handleSocket(LPVOID lpParam);
DWORD WINAPI handleConnectSocket(LPVOID lpParam);
DWORD WINAPI handleData(LPVOID lpParam);

char* guidToString(const GUID* id, char* out);
GUID stringToGUID(const std::string& guid);

NODE_REPLICATOR* head;
//NODE_PROCESS* headProcessReceive;
//NODE_PROCESS* headProcessSend;
SOCKET replicatorSocket = INVALID_SOCKET;
// Socket used for communication with client
SOCKET acceptedSockets[10];

bool appRunning = true;

char recvbuf[DEFAULT_BUFLEN];

//NODE_PROCESS* processList;

circular_buffer processBuffer;

HANDLE handleConnect;
HANDLE handle[10];

PROCESS processAdd[10];

#endif