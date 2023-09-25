#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <signal.h>
#include "socketFunctions.h"

// TODO Make inline.
long bufferToLong(char* buffer)
{
    return strtol(buffer, (char **)NULL, 10);
}

void clearBuffer(char* buffer)
{
    memset(buffer, 0, 1024);
}

void handleReceivedBuffer(char *buffer, int *socket, char *connectedIP, int *server_fd)
{
    char fileName[256];
    char filePath[256] = "received\\";
    long code = -1;
    int valread;
    code = bufferToLong(buffer);

    printf("Received buffer code is %d\n", code);

    switch (code)
    {
        // Login data file
        case 1:
            printf("Receiving data file\n");
            // File name
            valread = recv(*socket, fileName, 32, 0);
            handleReceiveResult(valread, server_fd, socket);
            strcat(filePath, fileName);
            strcat(filePath, connectedIP); // TODO Check for duplicate file names
            printf("%s\n", fileName);
            
            // File size
            valread = recv(*socket, buffer, 1024, 0);
            handleReceiveResult(valread, server_fd, socket);
            long fileSize = bufferToLong(buffer);
            printf("File size: %d\n", fileSize);
            printf("%d\n", valread);

            // Receive file
            char* fileBuffer = malloc(fileSize); // TODO Would it be better to receive chunks and write to file after receiving?
            int dataReceived = 0;

            while (dataReceived < fileSize)
            {
                char fileBufferNew[4096];
                valread = recv(*socket, fileBufferNew, 4096, 0);
                handleReceiveResult(valread, server_fd, socket);
                memcpy(fileBuffer + dataReceived, fileBufferNew, min(4096, fileSize - dataReceived)); // This was causing a heap corruption which was only being detected when calling fwrite
                printf("Recv file buffer position %-10d start byte 0x%-10x end byte 0x%-10x\n", dataReceived, fileBuffer[dataReceived], fileBuffer[dataReceived + 4095]);
                dataReceived += 4096;
            }

            // Write received file to disk
            printf("Writing file\n");
            char browserName[100];

            FILE* fp;
            if ((fp = fopen(filePath, "wb")))
            {
                fwrite(fileBuffer, 1, fileSize, fp);
                fclose(fp);
                printf("File written\n");
            } else printf("Could not write file\n");
            free(fileBuffer);
            break;
        default:
            printf("Undefined opcode: %d\n", code);
            break;
    }
}

int main(int argc, char* argv[])
{
    int server_fd, socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char* hello = "Hello from server";

    CreateDirectory("received", NULL);

    if (initialize(&server_fd, &address, opt)) return -1;
    if (listenAndAccept(&server_fd, &socket, &address, addrlen)) return -1;
    
    char connectedIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr.S_un.S_addr, connectedIP, INET_ADDRSTRLEN);

    printf("Received connection from: %s\n", connectedIP);
    
    // Receive hello message
    valread = recv(socket, buffer, 1024, 0);
    handleReceiveResult(valread, &server_fd, &socket);
    //printf("Recv status: %d\n", valread);
    printf("Received buffer: %s\n", buffer);

    // Send hello message
    handleSendResult(send(socket, hello, strlen(hello), 0), &server_fd, &socket);
    printf("Hello message sent\n");

    while (1) // TODO Always check for incoming connections
    {
        clearBuffer(buffer);
    
        valread = recv(socket, buffer, 1024, 0);
        handleReceiveResult(valread, &server_fd, &socket);
        handleReceivedBuffer(buffer, &socket, connectedIP, &server_fd);
    }

    closeSocket(&server_fd, &socket);
    return 0;
}