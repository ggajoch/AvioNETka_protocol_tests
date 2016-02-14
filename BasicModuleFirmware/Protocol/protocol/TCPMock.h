//
// Created by gregg on 2/14/2016.
//

#ifndef PROTOCOL_TCPMOCK_H
#define PROTOCOL_TCPMOCK_H


#include <stdio.h>
#include "StackInterfaces.h"
#include "useful.h"


#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <++FreeRTOS.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET SendingSocket;
char sendbuf[1024] = "This is a test string from sender";
int BytesSent;
int initSocket() {
    WSADATA              wsaData;

    // Server/receiver address
    SOCKADDR_IN          ServerAddr, ThisSenderInfo;
    // Server/receiver port to connect to
    unsigned int         Port = 7171;
    int  RetCode;
    // Be careful with the array bound, provide some checking mechanism...


    // Initialize Winsock version 2.2
    WSAStartup(MAKEWORD(2,2), &wsaData);
    printf("Client: Winsock DLL status is %s.\n", wsaData.szSystemStatus);

    // Create a new socket to make a client connection.
    // AF_INET = 2, The Internet Protocol version 4 (IPv4) address family, TCP old
    SendingSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(SendingSocket == INVALID_SOCKET)
    {
        printf("Client: socket() failed! Error code: %ld\n", WSAGetLastError());
        // Do the clean up
        WSACleanup();
        // Exit with error
        return -1;
    }
    else
        printf("Client: socket() is OK!\n");

    // Set up a SOCKADDR_IN structure that will be used to connect
    // to a listening server on port 5150. For demonstration
    // purposes, let's assume our server's IP address is 127.0.0.1 or localhost

    // IPv4
    ServerAddr.sin_family = AF_INET;
    // Port no.
    ServerAddr.sin_port = htons(Port);
    // The IP address
    //ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ServerAddr.sin_addr.s_addr = inet_addr("192.168.8.102");

    // Make a connection to the server with socket SendingSocket.
    RetCode = connect(SendingSocket, (SOCKADDR *) &ServerAddr, sizeof(ServerAddr));
    if(RetCode != 0)
    {
        printf("Client: connect() failed! Error code: %ld\n", WSAGetLastError());
        // Close the socket
        closesocket(SendingSocket);
        // Do the clean up
        WSACleanup();
        // Exit with error
        return -1;
    }
    else
    {
        printf("Client: connect() is OK, got connected...\n");
        printf("Client: Ready for sending and/or receiving data...\n");
    }

    // At this point you can start sending or receiving data on
    // the socket SendingSocket.

    // Some info on the receiver side...
//    getsockname(SendingSocket, (SOCKADDR *)&ServerAddr, (int *)sizeof(ServerAddr));
//    printf("Client: Receiver IP(s) used: %s\n", inet_ntoa(ServerAddr.sin_addr));
//    printf("Client: Receiver port used: %d\n", htons(ServerAddr.sin_port));
}

using namespace FreeRTOS;


Queue<PHYDataStruct> TxQueue(100);

class TCP_PHYLayer : public PHYInterface {
public:
    void mockData(const PHYDataStruct & data) {
        printf("[PHY] received data: ");
        print_byte_table(data.data, data.len);
        netInterface->passUp(data);
    }

    virtual StackError passDown(const PHYDataStruct & data) {
        TxQueue.sendToBack(data);
    }
};

TCP_PHYLayer phy;

void TCPReceiverTask(void *p) {
    char recvBuf[100];
    while(1) {
        int res = recv(SendingSocket, recvBuf, 100, 0);
        if( res > 0 ) {
            recvBuf[res] = 0;
            printf("[TCP] received: ");
            print_byte_table(recvBuf, res);
            PHYDataStruct data;
            memcpy(data.data, recvBuf+2, res-2);
            data.len = res-2;
            phy.mockData(data);
        } else {
            printf("connection failed %d\n", res);
            vTaskDelay(1000);
        }
        vTaskDelay(1);
    }
}

void TCPSender(void * p) {
    while(1) {
        PHYDataStruct data;
        TxQueue.receive(&data, portMAX_DELAY);
        static uint8_t buffer[100];
        uint16_t address = 0xAAAA;
        memcpy(buffer, &address, 2);
        memcpy(buffer + 2, data.data, data.len);

        printf("[TCP] sending data: ");
        print_byte_table(buffer, data.len + 2);

        BytesSent = send(SendingSocket, (char *) buffer, data.len + 2, 0);
        context::delay(1);
    }
}

void TCP_SETUP() {
    Task::create(TCPReceiverTask, "rx", 1000, 2);
    Task::create(TCPSender, "tx", 1000, 3);
}
#endif //PROTOCOL_TCPMOCK_H
