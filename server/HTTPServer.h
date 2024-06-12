#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#define RESET   "\033[0m"
#define RED     "\033[91m"      
#define GREEN   "\033[32m"      
#define YELLOW  "\033[33m"      
#define BLUE    "\033[94m"      
#define MAGENTA "\033[95m"      
#define CYAN    "\033[36m"
#define GRAY    "\033[90m"
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
	time_t lastReceivedTime;
	bool receivedMessage;
};

const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;

void parseHTTPRequest(SocketState& socket);
void handleHTTPGetOrHeadRequest(SocketState& socket, bool isGet);
void handleHTTPPostRequest(SocketState& socket);
void handleHTTPPutRequest(SocketState& socket);
void handleHTTPDeleteRequest(SocketState& socket);
void handleHTTPTraceRequest(SocketState& socket);
void handleHTTPOptionsRequest(SocketState& socket);
void handleHTTPUnsupportedRequest(SocketState& socket);

string printMessageAndPrepareResponse(string& request, string& requestType, const char color[6]);
string readHTMLFile(const string& lang);

#endif // HTTP_SERVER_H
