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


extern "C" {
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
}

using namespace std;


void firstCall(bool x) {
    printf("got value %d\n",x);
}
BoolDataDescriptor data({1, true}, firstCall);

void secondCall(float x) {
    printf("got value %f\n",x);
}
FloatDataDescriptor data2({2, true}, secondCall);

FloatDataDescriptor data3({2, false});


xQueueHandle RxQueue, TxQueue;
class FreeRTOSMock : public PHYInterface {
    void sendPacket(PHYDataStruct & data) {
        printf("[RTOS] data receiving from module: ");
        print_byte_table(data.data, data.len);

//        xQueueSend(TxQueue, &data, portMAX_DELAY);
        BytesSent = send(SendingSocket, (char*)data.data, data.len, 0);
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
            memcpy(data.data, recvBuf, res);
            data.len = res;
            PHYFree.mockData(&data);
            printf("[TCP] Processed!\n");
        } else {
            printf("connection failed %d\n", res);
        }
        vTaskDelay(10);
        /*PHYDataStruct data;
        if (xQueueReceive(TxQueue, &data, portMAX_DELAY)) {
            //data received from module
            printf("[MOCK]: data received!: ");
            print_byte_table(data.data, data.len);
            if (app->ackRequired(data.data[0])) {
                //send ACK
                printf("[MOCK] Sending ACK\n");
                //vTaskDelay(100);
                printf("[MOCK] Wait end\n");
                NetworkDataStruct ack = makeACKPacket();
                PHYDataStruct ph;
                ph.append(ack.id);
                ph.append(ack.data, ack.len);
                PHYFree.mockData(&ph);
            }
        }*/
        //vTaskDelay(1);
    }
}

xSemaphoreHandle unlockTest;
void starter(void * p) {
    DataDescriptor * descriptors[] = {&data, &data2, &data3};
    ApplicationLayer appX(&PHYFree, descriptors, 3);
    app = &appX;

    printf("Hello!!!\n");
    PHYDataStruct data;
    data.data[0] = 252;
    data.len = 1;
    PHYFree.mockData(&data);

    xSemaphoreGive(unlockTest);
    xSemaphoreHandle x = xSemaphoreCreateBinary();
    xSemaphoreTake(x, 0);
    while(1) {
        xSemaphoreTake(x, portMAX_DELAY);
    }
}

void test(void * p) {
//    xSemaphoreTake(app->link_established, portMAX_DELAY);
//    xSemaphoreGive(app->link_established);
    xSemaphoreTake(unlockTest, portMAX_DELAY);

    app->sendData(data, false);

    app->sendData(data, true);

    app->sendData(data2, 1.5f);
    while(1) {
        app->sendData(data2, 1.5f);
        printf("---------------------------\n\n");
        vTaskDelay(2000);
    }
    vTaskDelay(portMAX_DELAY);
}



int main() {
    unlockTest = xSemaphoreCreateBinary();
    xSemaphoreTake(unlockTest, 0);
    initSocket();
    BytesSent = send(SendingSocket, sendbuf, strlen(sendbuf), 0);


    RxQueue = xQueueCreate(100, sizeof(DataDescriptor));
    TxQueue = xQueueCreate(100, sizeof(DataDescriptor));

    xTaskCreate(TaskMockPC, "Rx", 1000, NULL, 2, NULL);
    xTaskCreate(starter, "st", 1000, NULL, 2, NULL);
    xTaskCreate(test, "ts", 1000, NULL, 2, NULL);

    vTaskStartScheduler();


    return 0;
}
