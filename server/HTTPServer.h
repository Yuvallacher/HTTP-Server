#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <iostream>
#include <fstream>
#define BUFFER_SIZE 1024
using namespace std;

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	int sendSubType;	// Sending sub-type
	char buffer[BUFFER_SIZE];
	int len;
};

const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;

void parseHTTPRequest(SocketState& socket);
void handleHTTPGetRequest(SocketState& socket, const std::string& query);
void handleHTTPPutRequest(SocketState& socket);
void handleHTTPPostRequest(SocketState& socket);
void handleHTTPDeleteRequest(SocketState& socket);

string readHTMLFile(const string& lang);

#endif // HTTP_SERVER_H
