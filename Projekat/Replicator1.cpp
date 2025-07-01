#include "Replicator.h"

int  main(void)
{
    int numOfProcess = 0;
    SOCKET listenSocket = INVALID_SOCKET;

    InitReplicatorList(&head);
    cb_init(&processBuffer);

    int iResult;


    if (InitializeWindowsSockets() == false)
    {

        return 1;
    }

    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_protocol = IPPROTO_TCP; 
    hints.ai_flags = AI_PASSIVE;     

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,     
        SOCK_STREAM,  
        IPPROTO_TCP); 

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return 1;
    }


    iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(resultingAddress);

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Waiting connection with Replicator2...\n");
    int numberOfClients = 0;

    replicatorSocket = accept(listenSocket, NULL, NULL);

    if (replicatorSocket == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        // socket used to communicate with server
        SOCKET connectSocket = INVALID_SOCKET;

        if (InitializeWindowsSockets() == false)
        {

            return 1;
        }

        // create a socket
        connectSocket = socket(AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP);

        if (connectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // create and initialize address structure
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddress.sin_port = htons(DEFAULT_PORT_R2);
        // connect to server specified in serverAddress and socket connectSocket
        if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
        {
            printf("Unable to connect to server.\n");
            closesocket(connectSocket);
        }
        printf("Connection with Replicator2 established.\n");

        handleConnect = CreateThread(NULL, 0, &handleConnectSocket, &connectSocket, 0, NULL);

        printf("Server initialized, waiting for clients.\n");
    }

    do
    {

        acceptedSockets[numberOfClients] = accept(listenSocket, NULL, NULL);

        if (acceptedSockets[numberOfClients] == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            char input[10];
            gets_s(input, sizeof(input));
            return 1;
        }

        //POKRETANJE NITI ZA SVAKOG KLIJENTA(PROCES)
        
        GUID Id;
        CoCreateGuid(&Id);

        processAdd[numberOfClients] = InitProcess(Id, acceptedSockets[numberOfClients], numOfProcess);

        char output[DEFAULT_BUFLEN];
        guidToString(&Id, output);
        printf("New client added: %s", output);
        PushBack(&head, processAdd[numberOfClients]);
        PrintAllProcesses(&head);
        numOfProcess++;
        handle[numberOfClients] = CreateThread(NULL, 0, &handleSocket, &processAdd[numberOfClients], 0, NULL);

        numberOfClients++;

    } while (appRunning);

    //CloseHandle(handleConnect);

    //for (int i = 0; i < numberOfClients; ++i) {
    //    CloseHandle(handle[i]);
    //}

    //for (int i = 0; i < 10; i++)
    //{
    //    iResult = shutdown(acceptedSockets[i], SD_SEND);
    //    if (iResult == SOCKET_ERROR)
    //    {
    //        printf("shutdown failed with error: %d\n", WSAGetLastError());
    //        closesocket(acceptedSockets[i]);
    //        WSACleanup();
    //        return 1;
    //    }

    //    closesocket(acceptedSockets[i]);
    //}

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}



DWORD WINAPI handleSocket(LPVOID lpParam)
{
    PROCESS* process = (PROCESS*)lpParam;
    SOCKET acceptedSocket = process->acceptedSocket;
    GUID Id = process->processId;
    int iResult;
    char recvbuf[512];
    circular_buffer tempBuffer;
    cb_init(&tempBuffer);
    bool first = true;
    bool async = false;

    if (IsSocketNull(&head))
    {
        *process = InitProcess(Id, acceptedSocket, 0);
        AddSocketToID(&head, &process);
    }

    unsigned long mode = 1; //non-blocking mode
    iResult = ioctlsocket(acceptedSocket, FIONBIO, &mode);
    if (iResult != NO_ERROR) {
        printf("ioctlsocket failed with error: %ld\n", iResult);
        closesocket(acceptedSocket);

    }
    fd_set readfds;
    FD_ZERO(&readfds);
    char poruka[] = "We received your message";
    do {
        fd_set readfds;
        FD_ZERO(&readfds);
        // Receive data until the client shuts down the connection
        FD_SET(acceptedSocket, &readfds);
        timeval timeVal;
        timeVal.tv_sec = 2;
        timeVal.tv_usec = 0;
        int result = select(0, &readfds, NULL, NULL, &timeVal);

        if (result == 0)
        {
            // vreme za cekanje je isteklo
            continue;
        }
        else if (result == SOCKET_ERROR)
        {
            //desila se greska prilikom poziva funkcije
            printf("Select failed with error: %d\n", WSAGetLastError());
            closesocket(acceptedSocket);
            WSACleanup();
            return 1;
        }

        else if (FD_ISSET(acceptedSocket, &readfds)) {
            char output[DEFAULT_BUFLEN];
            iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
            if (iResult > 0) {
                if (strcmp(recvbuf, "CallBack!@#$%^&&*") == 0) {
                    cb_print(&processBuffer);
                    char* data = cb_sOne(&processBuffer);
                    if (data == NULL) {
                        char noData[DEFAULT_BUFLEN] = "NULL";
                        int iResult = send(acceptedSocket, noData, (int)strlen(noData) + 1, 0);
                        if (iResult == SOCKET_ERROR)
                        {
                            printf("sendto failed with error: %d\n", WSAGetLastError());
                            closesocket(acceptedSocket);
                            WSACleanup();
                            return false;
                        }
                    }
                    else{
                        int iResult = send(acceptedSocket, data, (int)strlen(data) + 1, 0);
                        if (iResult == SOCKET_ERROR)
                        {
                            printf("sendto failed with error: %d\n", WSAGetLastError());
                            closesocket(acceptedSocket);
                            WSACleanup();
                            return false;
                        }
                    }
                    free(data);
                    continue;
                }
                else if (strcmp(recvbuf, "Replicate!@#$%^&&*") == 0) {
                   
                    DATA d = cb_pop_front(&tempBuffer);
                    while (d.data[0] != '\0') {
                        cb_push_back(&processBuffer, d);

                        static char result[101];
                        strcpy(result, d.data);
                        char temp[DEFAULT_BUFLEN] = "";
                        char num[5];
                        _itoa(process->index, num, 10);
                        strcat(temp, num);
                        strcat(temp, result);
                        send(replicatorSocket, temp, (int)strlen(temp) + 1, 0);
                        d = cb_pop_front(&tempBuffer);
                        Sleep(10);
                    }
                    cb_cleanup(&tempBuffer);
                    cb_init(&tempBuffer);

                    char data[DEFAULT_BUFLEN] = "Successfully replicated messages.";
                    int iResult = send(acceptedSocket, data, (int)strlen(data) + 1, 0);
                    if (iResult == SOCKET_ERROR)
                    {
                        printf("sendto failed with error: %d\n", WSAGetLastError());
                        closesocket(acceptedSocket);
                        WSACleanup();
                        return false;
                    }
                    continue;
                }
                else if (strcmp(recvbuf, "Close!@#$%^&&*") == 0) {
                    guidToString(&process->processId, output);

                    printf("Shuting down connection with %s \n", output);
                    closesocket(acceptedSocket);
                    CloseHandle(handle[process->index]);
                    handle[process->index] = NULL;

                    return 0;
                }
                guidToString(&process->processId, output);
                printf("Message received: %s, from index: %d\n", recvbuf, process->index);
                if (first) {
                    if (strcmp(recvbuf, "Synchrone replication.") == 0) {
                        async = false;
                    }
                    else if (strcmp(recvbuf, "Asynchrone replication.") == 0) {
                        async = true;
                    }
                    else {
                        printf("Bad connection with process: %s", output);
                        closesocket(acceptedSocket);
                        WSACleanup();
                        CloseHandle(handle[process->index]);

                        return 0;
                    }
                    first = false;
                }

                DATA data = InitData(recvbuf);

                if (async) {
                    cb_push_back(&tempBuffer, data);
                }
                else{
                    
                    cb_push_back(&processBuffer, data);
                    char temp[DEFAULT_BUFLEN] = "";



                    char num[5];
                    _itoa(process->index, num, 10);
                    strcat(temp, num);
                    strcat(temp, recvbuf);
                    send(replicatorSocket, temp, (int)strlen(temp) + 1, 0);
                }

            }
            //stavi poruku u njenu listu

        }
    } while (appRunning);
    CloseHandle(handle[process->index]);
    return 0;
}



DWORD WINAPI handleConnectSocket(LPVOID lpParam)
{
    char temp[DEFAULT_BUFLEN];
    SOCKET* acceptedSocket = (SOCKET*)lpParam;
    while (appRunning) {
        int iResult = recv(*acceptedSocket, temp, DEFAULT_BUFLEN, 0);
        int proNum = 0;
        if (iResult > 0) {
            proNum = (int)temp[0] - '0';

            printf("Message forwarded: %s, for process: %d\n", &temp[1], proNum);

            DATA data = InitData(&temp[1]);
            cb_push_back(&processBuffer, data);
        }
        else if (iResult != NOERROR) {
            //desila se greska prilikom poziva funkcije
            printf("Select failed with error: %d\n", WSAGetLastError());
            closesocket(*acceptedSocket);
            WSACleanup();
            return 1;
        }
    }
    closesocket(*acceptedSocket);
    WSACleanup();
    CloseHandle(handleConnect);
    return 0;
}


bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}


char* guidToString(const GUID* id, char* out) {
    int i;
    char* ret = out;
    out += sprintf(out, "%.8lX-%.4hX-%.4hX-", id->Data1, id->Data2, id->Data3);
    for (i = 0; i < sizeof(id->Data4); ++i) {
        out += sprintf(out, "%.2hhX", id->Data4[i]);
        if (i == 1) *(out++) = '-';
    }
    return ret;
}