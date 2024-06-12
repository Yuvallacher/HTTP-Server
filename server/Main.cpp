#include <string.h>
#include <time.h>
#include "HTTPServer.h"
using namespace std;

const int PORT = 27015;
const int MAX_SOCKETS = 60;

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);

struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;

void main()
{
	WSAData wsaData;

	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "HTTP Server: Error at WSAStartup()\n";
		return;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == listenSocket)
	{
		cout << "HTTP Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(PORT);

	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "HTTP Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "HTTP Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN);

	while (true)
	{
		fd_set waitRecv;
		fd_set waitSend;
		FD_ZERO(&waitRecv);
		FD_ZERO(&waitSend);

		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].recv == RECEIVE && sockets[i].receivedMessage)
			{
				time_t currentTime = time(NULL);
				if (currentTime - sockets[i].lastReceivedTime >= 120)
				{
					cout << "HTTP Server: Closing idle connection for socket " << sockets[i].id << endl;
					closesocket(sockets[i].id);
					removeSocket(i);
				}
			}
		}

		int nfd = select(0, &waitRecv, &waitSend, NULL, NULL);

		if (nfd == SOCKET_ERROR)
		{
			cout << "HTTP Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i);
					break;

				case RECEIVE:
					receiveMessage(i);
					break;
				}
			}
			
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					sendMessage(i);
					break;
				}
			}
		}
	}

	cout << "HTTP Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			sockets[i].receivedMessage = false;
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "HTTP Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "HTTP Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "HTTP Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(int index) 
{
	SOCKET msgSocket = sockets[index].id;

	sockets[index].receivedMessage = true;
	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);
	sockets[index].lastReceivedTime = time(NULL);

	if (SOCKET_ERROR == bytesRecv) {
		cout << "HTTP Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (bytesRecv == 0) {
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	else {
		sockets[index].buffer[len + bytesRecv] = '\0';
		cout << "HTTP Server: Recieved: " << bytesRecv << " bytes of message:\n\"" << &sockets[index].buffer[len] << "\"\n\n";

		sockets[index].len += bytesRecv;
		parseHTTPRequest(sockets[index]);
	}
}

void sendMessage(int index) 
{
	int bytesSent = 0;
	SOCKET msgSocket = sockets[index].id;

	bytesSent = send(msgSocket, sockets[index].buffer, sockets[index].len, 0);
	if (SOCKET_ERROR == bytesSent) {
		cout << "HTTP Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "HTTP Server: Sent: " << bytesSent << "\\" << sockets[index].len << " bytes of message:\n\"" << sockets[index].buffer << "\"\n\n";
	sockets[index].receivedMessage = false;
	sockets[index].len = 0;
	sockets[index].send = IDLE;
}