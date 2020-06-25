#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <cctype>
#include <algorithm>

#include "lib/Helper.h"
#include "lib/Entry.h"
#include "lib/Station.h"
#include "lib/HTTPRequest.h"

using namespace std;

//  Lots of this code has been adapted from 
//  http://beej.us/guide/bgnet/html/

// ----------------------------------------------------------------------------
//  Globals
// ----------------------------------------------------------------------------

Station station;

// ----------------------------------------------------------------------------
//  Query Functions
// ----------------------------------------------------------------------------

// query adjacent stations
string query_adjacent_stations(string query, UMAP<uint16_t, Entry> adj_stations) 
{

    // base query
    string base_query = spilt_string(query, '/').front();

    // setup addrinfo for getaddrinfo
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // iterate through adacjent stations
    for (auto s: adj_stations) {

        // new query with arrival time
        string msg = base_query + "/" + time_to_string(s.second.arrival_time);
        int numbytes;
        int status;
        struct addrinfo *to;

        // get addrinfo for socket to send query
        char port[10];
        sprintf(port, "%d", s.first);
        if ((status = getaddrinfo(NULL, port, &hints, &to)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
            exit(EXIT_FAILURE);
        }

        string str;

        // send query to adjacent station
        if ((numbytes = sendto(station.udp_fd, msg.c_str(), 
             strlen(msg.c_str()), 0, to->ai_addr,  to->ai_addrlen)) == -1 ) {
            perror("query_adjacent_stations: sento");
            exit(EXIT_FAILURE);
        }

        // ports sent to and recieved from
        uint16_t p_from;
        uint16_t p_to;

        // wait for valid response from search
        do {
            char buf[MAXBUFLEN];
            memset(&buf, 0, sizeof buf);
            struct sockaddr_storage from;
            socklen_t from_length = sizeof from;
        
            // recvfrom
            if ((numbytes = recvfrom(station.udp_fd, buf, MAXBUFLEN - 1, 0,
                SO_ADDR(from), &from_length)) == -1 ) {
                perror("query_adjacent_stations: recvfrom");
                exit(EXIT_FAILURE);
            }
            buf[numbytes] = '\0';
            str = buf;

            // assign to and from
            p_from = get_port(SO_ADDR(from));
            p_to = get_port((struct sockaddr *)to->ai_addr);


            // return station name
            if (str == "?name") {
                if ((numbytes = sendto(station.udp_fd, station.name.c_str(),
                     strlen(station.name.c_str()), 0, SO_ADDR(from),
                     from_length)) == -1 ) {
                    perror("query_adjacent_stations: sento");
                    exit(EXIT_FAILURE);
                }
            }
            // return invalid query
            else if (str.substr(0,4) == "?to=") {
                if ((numbytes = sendto(station.udp_fd, "NF",
                     strlen("NF"), 0, SO_ADDR(from),
                     from_length)) == -1 ) {
                    perror("query_adjacent_stations: sento");
                    exit(EXIT_FAILURE);
                }
            }
        // break statement  iff its from the sender and not a query
        } while (((p_to != p_from) || (str[0] == '?')) && (str != "NF"));
        
        // return response if found
        if (str.substr(0,5) == "FOUND") {
            return str + "/" + adj_stations[p_from].raw_string;
        }

        freeaddrinfo(to);
    }

    return "NF";
}

// start search for a station
string find_station(string query) 
{

    string dest = dest_from_query(query);
    vector<string> split_query = spilt_string(query, '/');
    time_t current_time = string_to_time(split_query[1]);

    // find next timetable entries leaving the station
    UMAP<string, Entry> next = station.find_next_depatures(current_time);
    if (next.size() == 0) {
        return "NF";
    }


    UMAP<uint16_t, Entry> adj_stations;

    // found dest in timetable
    if (next.find(dest) != next.end()) {
        return "FOUND/" + time_to_string(next[dest].arrival_time) \
                + "/" + next[dest].raw_string;
    }

    // hints for addrinfo
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // iterate through the adjacent stations
    for (int p: station.adjacent) {
        char msg[] = "?name";
        int numbytes;
        int status;
        struct addrinfo *to;

        // getaddrinfo
        char port[10];
        sprintf(port, "%d", p);
        if ((status = getaddrinfo(NULL, port, &hints, &to)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
            exit(EXIT_FAILURE);
        }
        
        // query name for adjacent station
        if ((numbytes = sendto(station.udp_fd, msg, strlen(msg), 0,
             to->ai_addr,  to->ai_addrlen)) == -1 ) {
            perror("find_station: sento");
            exit(EXIT_FAILURE);
        }

        char buf[MAXBUFLEN];
        memset(&buf, 0, sizeof buf);

        // recv name
        if ((numbytes = recvfrom(station.udp_fd, buf, MAXBUFLEN - 1, 0,
             NULL, NULL)) == -1 ) {
            perror("find_station: recvfrom");
            exit(EXIT_FAILURE);
        }
        buf[numbytes] = '\0';
        string buf_string = buf;
        TOLOWER(buf_string);

        // if station is in next then add to adjacent stations
        if (next.find(buf_string) != next.end()) {
            adj_stations[p] = next[buf_string];
        }

        freeaddrinfo(to);
    }


    // if adjacent stations is empty return "NF"
    if (adj_stations.size() == 0) {
        return "NF";
    }

    return query_adjacent_stations(query, adj_stations);

}


// handle udp query
string handle_udp_query(string query) 
{
    if (query[0] == '?') {
        if (query == "?name") {
            return station.name;
        }
        if (query.substr(0,4) == "?to=") {
            return find_station(query);
        }
    }
    else if (query.substr(0,5) == "FOUND") {
        return query;
    }

    return query;
}

// handle tcp query
string handle_tcp_query(HTTPRequest request) 
{
    // invalid request
    if (request.method != "GET" || request.uri.substr(0,4) != "?to=") {
        return "Invalid Query";
    }   

    // if dest = current station return invalid destintaion
    string dest = dest_from_query(request.uri);
    if (dest == station.name) {
        return "Invalid Destination";
    }
    TOLOWER(dest);


    // time_t current_time = time(NULL);
    // string time = time_to_string(current_time);
    string time = "12:30";

    string query = "?to=" +  dest + "/" + time;
    return find_station(query);
}

// ----------------------------------------------------------------------------
//  Main
// ----------------------------------------------------------------------------


// lots of this code has been adapted from 
// https://beej.us/guide/bgnet/
int main(int argc, char *argv[])
{
    // check argc
    if (argc < 4) {
        fprintf(stderr, "Usage: %s tcp_port udp_port "
                        "adjacent_stations ...\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    // create station
    station = Station(argv[1]);

    // add adjacent stations
    FOR(i, argc -4) {
        station.adjacent.push_back(atoi(argv[4 + i]));
    }

    // load timetable
    station.load_timetable();

    // start sockets
    station.start_tcp_socket(argv[2]);
    station.start_udp_socket(argv[3]);

    struct sigaction sa;

    // from https://beej.us/guide/bgnet/
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        std::exit(1);
    }

    // file descriptor sets
    fd_set inputs;
    fd_set outputs;
    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;
    FD_ZERO(&inputs);
    FD_ZERO(&outputs);
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    // max file descriptor
    int fdmax;

    // add sockets to inputs
    FD_SET(station.tcp_fd, &inputs);
    FD_SET(station.udp_fd, &inputs);
    fdmax = max(station.tcp_fd, station.udp_fd) + 1;
	
    int nbytes;
    char buf[MAXBUFLEN];
    memset(&buf, 0, sizeof buf);

    // stuff new sockets
    int new_fd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    // storage for udp addr
    UMAP<int, pair<sockaddr_storage, socklen_t>> udp_fd_addr; 

    // message queues
    UMAP<int, string> tcp_message_queue;
    UMAP<int, string> udp_message_queue;

    // adapted from https://beej.us/guide/bgnet/
    while(1) {
        // check timetable for changes
        station.load_timetable();

        // assigne file descriptor sets
        read_fds = inputs;
        write_fds = outputs;

        // select()
        if(select(fdmax, &read_fds, &write_fds, &except_fds, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Read set
        FOR(i, fdmax) {
            if (FD_ISSET(i, &read_fds)) {
                // if new tcp socket
                if (i == station.tcp_fd) {
                    memset(&buf, 0, sizeof  buf);
                    addrlen = sizeof remoteaddr;
                    new_fd = accept(station.tcp_fd, SO_ADDR(remoteaddr),
                                     &addrlen);
                    if (new_fd == -1) {
                        perror("tcp accept");
                    }
                    else {
                        FD_SET(new_fd, &inputs);
                        if (new_fd >= fdmax) {
                            fdmax = new_fd + 1;
                        }
                    }
                }
                // if udp socket
                else if (i == station.udp_fd) {
                    memset(&buf, 0, sizeof  buf);
                    addrlen = sizeof remoteaddr;
                    if ((nbytes = recvfrom(i, buf, MAXBUFLEN - 1, 0, 
                        SO_ADDR(remoteaddr), &addrlen)) == -1) {
                        if (FD_ISSET(i, &outputs)) {
                            FD_CLR(i, &outputs);
                        }
                        FD_CLR(i, &inputs);
                        close(i);
                        udp_message_queue.erase(i);
                        perror("recvfrom");
                        exit(EXIT_FAILURE);
                    }
                    else {
                        string s = buf;
                        if (s == "NF" || s == "INVALID") {
                            break;
                        }
                        udp_message_queue[i] = s;
                        udp_fd_addr[i] = make_pair(remoteaddr, addrlen);
                        if (!FD_ISSET(i, &outputs)) {
                            FD_SET(i, &outputs);
                        }
                    }
                }
                // if tcp socket recv data
                else {
                    memset(&buf, 0, sizeof  buf);
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        if (nbytes == 0) {
                        }
                        else {
                            perror("tcp recv");
                        }

                        if (FD_ISSET(i, &outputs)) {
                            FD_CLR(i, &outputs);
                        }
                        FD_CLR(i, &inputs);
                        tcp_message_queue.erase(i);
                    }
                    else {
                        tcp_message_queue[i] = string(buf);
                        if (!FD_ISSET(i, &outputs)) {
                            FD_SET(i, &outputs);
                        }
                    }
                }
            }
        }

        // Write set
        FOR(i, fdmax) {
            if (FD_ISSET(i, &write_fds)) {
                int type;
                socklen_t length = sizeof type;
                getsockopt(i, SOL_SOCKET, SO_TYPE, &type, &length);

                // IF UDP
                if (type == SOCK_DGRAM) {
                    string msg = handle_udp_query(udp_message_queue[i]);
                    sockaddr_storage to = udp_fd_addr[i].first;
                    socklen_t to_len = udp_fd_addr[i].second;

                    // send response
                    if ((nbytes = sendto(i, msg.c_str(), strlen(msg.c_str()),
                         0, SO_ADDR(to), to_len)) == -1 ) {
                        perror("udp sendto");
                        std::exit(EXIT_FAILURE);
                    }

                    // remove from outputs
                    udp_fd_addr.erase(i);
                    if (FD_ISSET(i, &outputs)) {
                        FD_CLR(i, &outputs);
                    }
                }
                // IF TCP
                else if (type == SOCK_STREAM) {
                    // handle tcp request 
                    string r = tcp_message_queue[i];
                    tcp_message_queue.erase(i);
                    HTTPRequest request =  HTTPRequest(r);
                    string s = handle_tcp_query(request);

                    // construct response
                    string msg;
                    if (s == "NF") {
                        msg = request.response(404, "No Journey Found");
                    }
                    else if (s == "Invalid Destination" || 
                                s == "Invalid Query") {
                        msg = request.response(400, s);
                    }
                    else msg = request.response(200, s);

                    // send response
                    if (send(i, msg.c_str(), strlen(msg.c_str()), 0) == -1) {
				        perror("send");
			            close(new_fd);
			            exit(EXIT_FAILURE);
                    }
                    // remove from inputs and outputs
                    else {
                        if (FD_ISSET(i, &outputs)) {
                            FD_CLR(i, &outputs);
                        }
                        FD_CLR(i, &inputs);
                        close(i);
                    }

                }
            }

        }

        // except set
        FOR(i, fdmax) {
            if (FD_ISSET(i, &except_fds)) {
                FD_CLR(i, &inputs);
                if (FD_ISSET(i, &outputs)) {
                    FD_CLR(i, &outputs);
                }
                tcp_message_queue.erase(i);
                udp_message_queue.erase(i);
                close(i);
            }
        }
        
    }

    return 0;
}
