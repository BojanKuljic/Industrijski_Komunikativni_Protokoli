#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include "ClientOptions.h"
#include "Test.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <combaseapi.h>
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define REP1_PORT 27016
#define REP2_PORT 27017

#define SERVER_IP_ADDERESS "127.0.0.1"
#define IP_ADDRESS_LEN 16
#define BUFFER_SIZE 1024

sockaddr_in serverAddress;
int sockAddrLen = sizeof(struct sockaddr);

SOCKET clientSocket;


bool InitializeWindowsSockets();


int main()
{
    int serviceID = 0;
    int serviceType = 0;

    printf("Welcome to replication service.\n");

    printf("Choose type of service sync or async (enter 1 or 2): \n");
    while (serviceType != 1 && serviceType != 2) {
        char input[DEFAULT_BUFLEN];
        if (gets_s(input, sizeof(input)) != nullptr)
            serviceType = atoi(input);
        fflush(stdin);

        if (serviceType != 1 && serviceType != 2) {
            printf("Invalid input. Please enter 1 or 2.\n");
            continue;
        }
    }
    if (serviceType == 1) {
        while (true)
        {
            InitializeWindowsSockets();

            memset((char*)&serverAddress, 0, sizeof(serverAddress));

            printf("Select one of the options: \n");
            printf("    1.Register service \n");
            printf("    2.Send data \n");
            printf("    3.Callback \n");
            printf("    4.Test \n");
            printf("    5.Close \n");

            char input[DEFAULT_BUFLEN];
            int option = 0;
            if (gets_s(input, sizeof(input)) != nullptr)
                option = atoi(input);
            fflush(stdin);

            if (option == 1) {
                serviceID = RegistrationHandler(serviceID, serviceType);
                if (serviceID == -1) return 1;
            }
            else if (option == 2) {
                if (!SendDataHandler(serviceID)) return 1;
            }
            else if (option == 3) {
                if (!CallbackHandler(serviceID)) return 1;
            }
            else if (option == 4) {
                if (!TestHandler(serviceID, serviceType)) return 1;
            }
            else if (option == 5) {
                if (CloseHandler(serviceID))
                    return 0;
                else
                    return 1;
            }
        }
    }
    else {
        while (true)
        {
            InitializeWindowsSockets();

            memset((char*)&serverAddress, 0, sizeof(serverAddress));

            printf("Select one of the options: \n");
            printf("    1.Register service \n");
            printf("    2.Send data \n");
            printf("    3.Replicate \n");
            printf("    4.Callback \n");
            printf("    5.Test \n");
            printf("    6.Close \n");

            char input[DEFAULT_BUFLEN];
            int option = 0;
            if (gets_s(input, sizeof(input)) != nullptr)
                option = atoi(input);
            fflush(stdin);

            if (option == 1) {
                serviceID = RegistrationHandler(serviceID, serviceType);
                if (serviceID == -1) return 1;
            }
            else if (option == 2) {
                if (!SendDataHandler(serviceID)) return 1;
            }
            else if (option == 3) {
                if (!ReplicateHandler(serviceID)) return 1;
            }
            else if (option == 4) {
                if (!CallbackHandler(serviceID)) return 1;
            }
            else if (option == 5) {
                if (!TestHandler(serviceID, serviceType)) return 1;
            }
            else if (option == 6) {
                if (CloseHandler(serviceID))
                    return 0;
                else
                    return 1;
            }
        }
    }

    return 0;
}


int RegistrationHandler(int ServiceID, int ServiceType) {
    if (ServiceID != 0) {
        printf("Client is already reqisterd to replicator%d\n", ServiceID);
        return ServiceID;
    }

    int id = 0;
    printf("Enter service ID to reqister to a replicator (1 or 2): \n");
    while (id != 1 && id != 2) {
        char input[DEFAULT_BUFLEN];
        if (gets_s(input, sizeof(input)) != nullptr)
            id = atoi(input);
        fflush(stdin);

        if (id != 1 && id != 2) {
            printf("Invalid input. Please enter 1 or 2.\n");
            continue;
        }
        ServiceID = id;
        if (!RegisterService(id, ServiceType))
        {
            gets_s(input, sizeof(input));
            return -1;
        }
    }
    return ServiceID;
}


bool SendDataHandler(int ServiceID) {
    if (ServiceID == 0) {
        printf("Service not reqisterd\n");
        return true;
    }
    printf("Enter message: ");
    char outgoingBuffer[BUFFER_SIZE];
    gets_s(outgoingBuffer, BUFFER_SIZE);
    fflush(stdin);

    if (!SendData(ServiceID, outgoingBuffer, strlen(outgoingBuffer)))
    {
        char input[10];
        gets_s(input, sizeof(input));
        return false;
    }
    return true;
}


bool CallbackHandler(int ServiceID) {
    if (ServiceID == 0) {
        printf("Service not reqisterd\n");
        return true;
    }
    char input[10];
    char data[BUFFER_SIZE] = "CallBack!@#$%^&&*";
    if (!ReceiveData(data, strlen(data)))
    {
        gets_s(input, sizeof(input));
        return false;
    }
    return true;
}


bool ReplicateHandler(int ServiceID) {
    if (ServiceID == 0) {
        printf("Service not reqisterd\n");
        return true;
    }
    char input[10];
    char data[BUFFER_SIZE] = "Replicate!@#$%^&&*";
    if (!ReceiveData(data, strlen(data)))
    {
        gets_s(input, sizeof(input));
        return false;
    }
    return true;

}


bool TestHandler(int ServiceID, int ServiceType) {

    if (ServiceID == 0) {
        printf("Service not reqisterd\n");
        return true;
    }
    else if (ServiceType == 1) TestSync(ServiceID);
    else if (ServiceType == 2) TestAsync(ServiceID);
    else {
        char input[10];
        printf("The application ran into a problem.");
        gets_s(input, sizeof(input));
        return false;
    }
    return true;
}


bool CloseHandler(int ServiceID) {
    if (ServiceID != 0) {
        char data[BUFFER_SIZE] = "Close!@#$%^&&*";
        char input[10];

        int iResult = send(clientSocket, data, strlen(data) + 1, 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("sendto failed with error: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            gets_s(input, sizeof(input));
            return false;
        }
        iResult = closesocket(clientSocket);
        if (iResult == SOCKET_ERROR)
        {
            printf("closesocket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            gets_s(input, sizeof(input));
            return false;
        }

        iResult = WSACleanup();
        if (iResult == SOCKET_ERROR)
        {
            printf("WSACleanup failed with error: %ld\n", WSAGetLastError());
            gets_s(input, sizeof(input));
            return false;
        }
    }
    return true;
}


bool SendData(int ServiceID, char* data, int dataSize)
{
    int iResult = send(clientSocket, data, dataSize + 1, 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("sendto failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }
    printf("Message sent to replicator%d.\n", ServiceID);
    return true;
}


bool ReceiveData(char* data, int dataSize)
{

    int iResult = send(clientSocket, data, dataSize + 1, 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("sendto failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    char recvBuffer[DEFAULT_BUFLEN];
    iResult = recv(clientSocket, recvBuffer, DEFAULT_BUFLEN, 0);
    if (iResult > 0)
    {
        printf("Bytes received: %d\n", iResult);
        printf("Received data: %s\n", recvBuffer);
    }
    else if (iResult == 0)
    {
        printf("Connection closed by the server.\n");
    }
    else
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}


bool RegisterService(int ServiceID, int ServiceType)
{
    int serverPort = 0;

    if (ServiceID == 1) {
        printf("Trying to reqister to replicator1.\n");
        serverPort = REP1_PORT;
    }
    else if (ServiceID == 2) {
        printf("Trying to reqister to replicator2.\n");
        serverPort = REP2_PORT;
    }

    clientSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDERESS);
    serverAddress.sin_port = htons((u_short)serverPort);

    if (connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(clientSocket);
        WSACleanup();
    }
    int iResult;
    if (ServiceType == 1) {
        char outgoingBuffer[BUFFER_SIZE] = "Synchrone replication.";
        iResult = send(clientSocket, outgoingBuffer, (int)strlen(outgoingBuffer) + 1, 0);
    }
    else {
        char outgoingBuffer[BUFFER_SIZE] = "Asynchrone replication.";
        iResult = send(clientSocket, outgoingBuffer, (int)strlen(outgoingBuffer) + 1, 0);
    }

    if (iResult == SOCKET_ERROR)
    {
        printf("sendto failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    printf("Reqisterd successfully.\n");
    return true;
}


bool TestSync(int ServiceID) {
    if (ServiceID == 0) {
        printf("Service not reqisterd\n");
        return true;
    }
    int cnt = 0;
    char poruka[6] = "Test\0";
    while (cnt < 50) {
        Sleep(1);
        cnt++;
        if (cnt == 50) {
            char poruka[6] = "Last\0";

            int iResult = send(clientSocket, poruka, (int)strlen(poruka) + 1, 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
        }
        else {
            int iResult = send(clientSocket, poruka, (int)strlen(poruka) + 1, 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
        }
    }
    printf("Last message sent: \n");
    char data[BUFFER_SIZE] = "CallBack!@#$%^&&*";
    if (!ReceiveData(data, strlen(data)))
    {
        char input[10];
        gets_s(input, sizeof(input));
        return 1;
    }
}


bool TestAsync(int ServiceID) {
    if (ServiceID == 0) {
        printf("Service not reqisterd\n");
        return true;
    }
    int cnt = 0;
    char poruka[6] = "Test\0";
    while (cnt < 50) {
        Sleep(1);
        cnt++;
        if (cnt == 50) {
            char poruka[17] = "Last Replicated\0";

            int iResult = send(clientSocket, poruka, (int)strlen(poruka) + 1, 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
        }
        else {
            int iResult = send(clientSocket, poruka, (int)strlen(poruka) + 1, 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
        }
    }
    printf("Last message sent before replication: \n");
    char data[BUFFER_SIZE] = "CallBack!@#$%^&&*";
    if (!ReceiveData(data, strlen(data)))
    {
        char input[10];
        gets_s(input, sizeof(input));
        return 1;
    }
    ReplicateHandler(ServiceID);
    printf("Last message after replication: \n");
    if (!ReceiveData(data, strlen(data)))
    {
        char input[10];
        gets_s(input, sizeof(input));
        return 1;
    }
}


bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return false;
    }
    return true;
}