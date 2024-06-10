#include "HTTPServer.h"
#include <cstring>

void parseHTTPRequest(SocketState& socket) {
    string request(socket.buffer);
    if (request.substr(0, 3) == "GET") {
        size_t pos = request.find("?");
        string query = "";
        if (pos != string::npos) {
            query = request.substr(pos + 1);
        }
        handleHTTPGetRequest(socket, query);
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
    else {
        // Handle unsupported methods if necessary
    }
}

void handleHTTPGetRequest(SocketState& socket, const string& query) 
{
    string lang = "en"; // default language
    size_t pos = query.find("lang=");
    if (pos != string::npos) 
    {
        lang = query.substr(pos + 5, 2); 
    }
 
    string response;
    string htmlContent = readHTMLFile(lang);
    size_t contentLength = htmlContent.length();
    size_t totalLength = 0;

    if (!htmlContent.empty())
    {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(contentLength) + "\r\n\r\n" + htmlContent;
        totalLength = response.length();
    }
    else
    {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 22\r\n\r\n<html><body>File not found</body></html>";
        totalLength = response.length();
    }

    memset(socket.buffer, 0, sizeof(socket.buffer));
    strncpy(socket.buffer, response.c_str(), min(totalLength, BUFFER_SIZE - 1));
    socket.len = min(totalLength, BUFFER_SIZE - 1);
    socket.send = SEND;
}

void handleHTTPPutRequest(SocketState& socket) {
    // Handle PUT request
    string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>PUT request handled</body></html>";
    strncpy(socket.buffer, response.c_str(), BUFFER_SIZE);
    socket.send = SEND;
}

void handleHTTPPostRequest(SocketState& socket) {
    // Handle POST request
    string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>POST request handled</body></html>";
    strncpy(socket.buffer, response.c_str(), BUFFER_SIZE);
    socket.send = SEND;
}

void handleHTTPDeleteRequest(SocketState& socket) {
    // Handle DELETE request
    string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>DELETE request handled</body></html>";
    strncpy(socket.buffer, response.c_str(), BUFFER_SIZE);
    socket.send = SEND;
}

string readHTMLFile(const string& lang) 
{
    string filePath = "html/index_" + lang + ".html";
    string content;
    ifstream file(filePath);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            content += line;
        }
        file.close();
    }
    return content;
}