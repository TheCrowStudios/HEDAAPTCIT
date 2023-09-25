#include "socketFunctions.h"

void closeSocket(int *fd, int *socket)
{
    printf("Closing socket\n");
    shutdown(*fd, SD_BOTH);
    closesocket(*socket);
    WSACleanup();
}

int handleReceiveResult(int result, int *fd, int *socket)
{
    if (result > 0) return 1;
    else
    {
        printf("Could not receive: %d\n", WSAGetLastError());
        closeSocket(fd, socket);
        exit(-1); // TODO Handle this differently
        return 0;
    }
}

int handleSendResult(int result, int *fd, int *socket)
{
    if (result == SOCKET_ERROR)
    {
        printf("Could not send: %d\n", WSAGetLastError());
        closeSocket(fd, socket);
        exit(-1); // TODO Handle this differently
        return 0;
    }

    return 1;
}

int initialize(int *fd, struct sockaddr_in *address, int opt)
{
    WSADATA wsaData = { 0 };
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket failed");
        return -1;
    }

    if (setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)))
    {
        printf("setsockopt");
        return -1;
    }

    address->sin_family = AF_INET;
    address->sin_addr.S_un.S_addr = INADDR_ANY;
    address->sin_port = htons(PORT);

    if (bind(*fd, (struct sockaddr*)address, sizeof(*address)) < 0)
    {
        printf("Bind failed");
        return -1;
    }

    return 0;
}

int listenAndAccept(int *fd, int *socket, struct sockaddr_in *address, int addrlen)
{
    // TODO Figure out how the listen and accept functions work.
    printf("Listening\n");
    if (listen(*fd, 3) < 0)
    {
        perror("Listen");
        return -1;
    }

    printf("Accepting\n");
    if ((*socket = accept(*fd, (struct sockaddr*)address, (socklen_t*)&addrlen)) < 0)
    {
        perror("Accept");
        return -1;
    }

    return 0;
}