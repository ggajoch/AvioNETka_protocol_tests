#include <iostream>
#include <protocol/NetworkSender.h>
#include <protocol/NetworkLayer.h>
#include <protocol/ApplicationLayer.h>


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

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
    // AF_INET = 2, The Internet Protocol version 4 (IPv4) address family, TCP protocol
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
    ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

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
    getsockname(SendingSocket, (SOCKADDR *)&ServerAddr, (int *)sizeof(ServerAddr));
    printf("Client: Receiver IP(s) used: %s\n", inet_ntoa(ServerAddr.sin_addr));
    printf("Client: Receiver port used: %d\n", htons(ServerAddr.sin_port));
}


#include "++FreeRTOS.h"

using namespace std;


void firstCall(bool x) {
    printf("got value %d\n",x);
}
BoolDataDescriptor data({1, true}, firstCall);

void secondCall(float x) {
    printf("got value %f\n",x);
}
FloatDataDescriptor data2({2, true}, secondCall);

FloatDataDescriptor data3({2, true});


using namespace FreeRTOS;
Queue<DataDescriptor> RxQueue(100), TxQueue(100);

class FreeRTOSMock : public PHYInterface {
    void sendPacket(PHYDataStruct & data) {
        static uint8_t buffer[100];
        uint16_t address = 2;
        memcpy(buffer, &address, 2);
        memcpy(buffer+2, data.data, data.len);

        printf("[RTOS] data receiving from module: ");
        print_byte_table(buffer, data.len+2);

        BytesSent = send(SendingSocket, (char*)buffer, data.len+2, 0);
        context::delay(10);
    }
public:
    void mockData(PHYDataStruct * data) {
        printf("[RTOS] sending data to module: ");
        print_byte_table(data->data, data->len);
        net->dataReceived(*data);
    }
};


ApplicationLayer * app;
FreeRTOSMock PHYFree;


void TaskMockPC(void * p) {
    char recvBuf[100];
    while(1) {
        printf("[TCP] testing");
        int res = recv(SendingSocket, recvBuf, 100, 0);
        if( res > 0 ) {
            recvBuf[res] = 0;
            printf("[TCP] received: ");
            print_byte_table(recvBuf, res);
            PHYDataStruct data;
            memcpy(data.data, recvBuf+2, res-2);
            data.len = res-2;
            PHYFree.mockData(&data);
            printf("[TCP] Processed!\n");
        } else {
            printf("connection failed %d\n", res);
        }
        vTaskDelay(10);
    }
}

Semaphore unlockTest;
void starter(void * p) {
    DataDescriptor * descriptors[] = {&data, &data2, &data3};
    ApplicationLayer appX(&PHYFree, descriptors, 3);
    app = &appX;

    printf("Hello!!!\n");
    PHYDataStruct data;
    data.data[0] = 252;
    data.len = 1;
    PHYFree.mockData(&data);

    unlockTest.give();
    while(1) {
        context::delay(portMAX_DELAY);
    }
}

void test(void * p) {
    unlockTest.take();

//    app->sendData(data, false);

//    app->sendData(data, true);
//
//    app->sendData(data2, 1.5f);

    float x = 0;
    while(1) {
        app->sendData(data2, x);
        x += 1.5;
        printf("---------------------------\n\n");
        vTaskDelay(2000);
    }
}



int main() {
    initSocket();
//    BytesSent = send(SendingSocket, sendbuf, strlen(sendbuf), 0);

    Task::create(TaskMockPC, "Rx", 1000, 2);
    Task::create(starter, "st", 1000, 2);
    Task::create(test, "test", 1000, 2);

    control::startScheduler();

    return 0;
}
