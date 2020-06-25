#ifndef _HTTPREQUEST_H
#define _HTTPREQUEST_H

#include <string>
#include <vector>

#include "Helper.h"

// class for HTTPRequest
class HTTPRequest
{
    public:
        string http_version;
        string method;
        string uri;
        HTTPRequest(string request);
        string response(int code, string data);
};

#endif