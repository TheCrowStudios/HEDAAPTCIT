#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define PORT 8080

void closeSocket(int *fd, int *socket);
int handleReceiveResult(int result, int *fd, int *socket);
int handleSendResult(int result, int *fd, int *socket);
int initialize(int *fd, struct sockaddr_in *address, int opt);
int listenAndAccept(int *fd, int *socket, struct sockaddr_in *address, int addrlen);