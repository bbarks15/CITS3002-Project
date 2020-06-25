#include "HTTPRequest.h"

// contructor for HTTPRequest
HTTPRequest::HTTPRequest(string request){
    vector<string> lines = spilt_string(request, '\n');
    string request_line = lines[0];
    vector<string> l = spilt_string(request_line, ' ');
    method = trim(l[0]);
    uri = trim(l[1].erase(0,1));
    http_version = trim(l[2]);
}

// create response 
string HTTPRequest::response(int code, string data) {

    // create header
    string response = http_version; 
    switch (code) {
        case 200:
            response += " 200 OK\r\n";
            break;
        case 400:
            response += " 400 Bad Request\r\n";
            break;
        case 404:
            response += " 404 Not Found\r\n";
            break;
    }
    response += "Content-Type: text/html\r\n";
    response += "Connection: Closed\r\n";
    response += "\r\n";

    UMAP<int,string> title;
    title[200] = "Journey Found";
    title[400] = "Bad Request";
    title[404] = "Not Found";

    // create html response
    response += "<html>\n";
    response += "<body>\n";
    response += "<h1>" + title[code] + "</h1>\n";
    vector<string> split = spilt_string(data, '/');
    if (split.size() == 1) {
        response += "<h2>" + split[0] + "</h2>\n";
    } 
    else {
        for (int i = split.size() - 1; i > 1; i--) {
            response += "<h2>" + result_to_readable(split[i]) + "</h2>\n";
        }
    }
    
    response += "<body>\n";
    response += "<html>\n";
    return response;
}


            