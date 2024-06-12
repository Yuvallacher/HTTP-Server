#include "HTTPServer.h"
#include <cstring>

void parseHTTPRequest(SocketState& socket) 
{
    string request(socket.buffer);
    if (request.substr(0, 3) == "GET") {
        handleHTTPGetOrHeadRequest(socket, true);
    }
    else if (request.substr(0, 4) == "POST") {
        handleHTTPPostRequest(socket);
    }
    else if (request.substr(0, 3) == "PUT") {
        handleHTTPPutRequest(socket);
    }
    else if (request.substr(0, 6) == "DELETE") {
        handleHTTPDeleteRequest(socket);
    }
    else if (request.substr(0, 4) == "HEAD")
    {
        handleHTTPGetOrHeadRequest(socket, false);
    }
    else if (request.substr(0, 5) == "TRACE")
    {
        handleHTTPTraceRequest(socket);
    }
    else if (request.substr(0, 7) == "OPTIONS")
    {
        handleHTTPOptionsRequest(socket);
    }
    else {
        handleHTTPUnsupportedRequest(socket);
    }
}

void handleHTTPGetOrHeadRequest(SocketState& socket, bool isGet) 
{
    string request(socket.buffer);
    size_t pos = request.find("?");
    string query = "";
    if (pos != string::npos) {
        query = request.substr(pos + 1);
    }

    string lang = "en"; // default language
    pos = query.find("lang=");
    if (pos != string::npos) 
    {
        lang = query.substr(pos + 5, 2); 
    }
 
    string response;
    string fileNotFoundMessage = "<html><body>File not found</body></html>";
    string htmlContent = readHTMLFile(lang);
    size_t contentLength = htmlContent.length();
    size_t fileNotFoundMessageLength = fileNotFoundMessage.length();
    size_t totalLength = 0;

    if (!htmlContent.empty())
    {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: " + to_string(contentLength) + "\r\n\r\n";
        if (isGet)
        {
            response += htmlContent;
        }
        totalLength = response.length();
    }
    else
    {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: " + to_string(fileNotFoundMessageLength) + "\r\n\r\n";
        if (isGet)
        {
            response += fileNotFoundMessage;
        }
        totalLength = response.length();
    }

    string type = isGet ? "GET" : "HEAD";
    cout << GREEN << "Received " << type << " request" << RESET << endl << endl;

    memset(socket.buffer, 0, sizeof(socket.buffer));
    strncpy(socket.buffer, response.c_str(), min(totalLength, BUFFER_SIZE - 1));
    socket.len = min(totalLength, BUFFER_SIZE - 1);
    socket.send = SEND;
}

void handleHTTPPostRequest(SocketState& socket)
{
    string request(socket.buffer);
    string type("POST");
    string response = printMessageAndPrepareResponse(request, type, BLUE);
    memset(socket.buffer, 0, sizeof(socket.buffer));
    strncpy(socket.buffer, response.c_str(), BUFFER_SIZE);
    socket.len = response.length();
    socket.send = SEND;
}

void handleHTTPPutRequest(SocketState& socket) 
{
    string request(socket.buffer);
    string type("PUT");
    string response = printMessageAndPrepareResponse(request, type, CYAN);
    memset(socket.buffer, 0, sizeof(socket.buffer));
    strncpy(socket.buffer, response.c_str(), BUFFER_SIZE);
    socket.len = response.length();
    socket.send = SEND;
}

void handleHTTPDeleteRequest(SocketState& socket) 
{
    string request(socket.buffer);
    string type("DELETE");
    string response = printMessageAndPrepareResponse(request, type, RED);
    memset(socket.buffer, 0, sizeof(socket.buffer));
    strncpy(socket.buffer, response.c_str(), BUFFER_SIZE);
    socket.len = response.length();
    socket.send = SEND;
}

void handleHTTPTraceRequest(SocketState& socket)
{
    string request(socket.buffer);
    string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: message/http\r\n";
    response += "Content-Length: " + to_string(request.length()) + "\r\n";
    response += "\r\n" + request;

    cout << YELLOW << "Received TRACE request" << RESET << endl << endl;
    memset(socket.buffer, 0, sizeof(socket.buffer));
    strncpy(socket.buffer, response.c_str(), min(response.length(), BUFFER_SIZE - 1));
    socket.len = min(response.length(), BUFFER_SIZE - 1);
    socket.send = SEND;
}

void handleHTTPOptionsRequest(SocketState& socket)
{
    string response = "HTTP/1.1 200 OK\r\n";
    response += "Allow: OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE\r\n";
    response += "Content-Length: 0\r\n";
    response += "\r\n";

    cout << MAGENTA << "Received OPTIONS request" << RESET << endl << endl;
    memset(socket.buffer, 0, sizeof(socket.buffer));
    strncpy(socket.buffer, response.c_str(), min(response.length(), BUFFER_SIZE - 1));
    socket.len = min(response.length(), BUFFER_SIZE - 1);
    socket.send = SEND;
}

void handleHTTPUnsupportedRequest(SocketState& socket)
{
    string responseBody = "Unsupported request method";
    size_t bodyLength = responseBody.length();

    string response = "HTTP/1.1 405 Method Not Allowed\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + to_string(bodyLength) + "\r\n\r\n";
    response += responseBody;

    cout << GRAY << "Received an unallowed request" << RESET << endl << endl;
    strncpy(socket.buffer, response.c_str(), BUFFER_SIZE);
    socket.len = response.length();
    socket.send = SEND;
}

string readHTMLFile(const string& lang) 
{
    string content = "";
    if (lang == "en" || lang == "he" || lang == "fr") 
    {
        string directory = "C:\\temp\\";
        string filePath = directory + "index_" + lang + ".html";
        ifstream file(filePath);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                content += line;
            }
            file.close();
        }
    }
    return content;
}

string printMessageAndPrepareResponse(string& request, string& requestType, const char color[6])
{
    cout << color << "Received " << requestType << " request" << RESET;
    size_t pos = request.find("\r\n\r\n");
    if (pos != string::npos && pos + 4 < request.length()) {
        string requestBody = request.substr(pos + 4);
        cout << color << ": " << requestBody << RESET;
    }
    cout << endl << endl;

    string messageToSend = "<html><body>" + requestType + " request handled</body></html>";
    size_t contentLength = messageToSend.length();
    string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: " + to_string(contentLength) + "\r\n\r\n" + messageToSend;
    return response;
}