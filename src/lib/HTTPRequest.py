from lib.Helper import result_to_readable

class HTTPRequest:

    #Constructor for HTTPRequest
    def __init__(self, request):
        lines = request.split("\r\n")
        request_line = lines[0]
        split = request_line.split(" ")
        self.method = split[0].strip()
        self.uri = split[1].strip("/")
        self.http_version = split[2].strip()

    # creates the http response for the corresponding
    # error code and data
    def response(self, code, data):
        response = self.http_version

        # Insert code into header
        if code == 200:
            response += " 200 OK\r\n"
        elif code == 400:
            response += " 400 Bad Request\r\n"
        elif code == 404:
            response += " 404 Not Found\r\n"

        title = {}
        title[200] = "Journey Found"
        title[400] = "Bad Request"
        title[404] = "Not Found"

        # Fill in rest of the respone
        response += "Content-Type: text/html\r\n"
        response += "Connection: Closed\r\n"
        response += "\r\n"
        response += "<html>\n"
        response += "<body>\n"
        response += "<h1>" + title[code] + "</h1>\n"

        split = data.split("/")
        if len(split) == 1:
            response += "<h2>" + split[0] + "</h2>\n"
        else:
            for i in range(len(split) - 1, 1, -1):
                response += "<h2>" + result_to_readable(split[i]) + "</h2>\n"

        response += "<body>\n"
        response += "<html>\n"
        return response