#include "Station.h"

// constructor for station
Station::Station(string n)
{
    name = n;
    memset(&tt_last_modify, 0, sizeof tt_last_modify);
}

// empty constructor for station
Station::Station() {}

// checks and loads if the timetable if time of 
// last modifiy has changed
void Station::load_timetable()
{
    
    string line;
    string tt_location = "../timetables/tt-" + name;

    // stat the timetable file
    struct stat tt_stat;
    if (stat(tt_location.c_str(), &tt_stat) != 0) {
        perror("timetable stat");
        exit(EXIT_FAILURE);
    }

    // check if modified else return and do nothing
    if(compare_modify_time(tt_last_modify, tt_stat.st_mtim)) {
        tt_last_modify = tt_stat.st_mtim;
    }
    else return;

    // empties the timetable
    timetable.clear();

    // open timetable
    ifstream file(tt_location);
    
    // read entries
    if (file.is_open()) {
        getline (file, line);
        while (getline (file, line)) {
            try {
                timetable.push_back(Entry(line));
            }
            catch(invalid_argument& e) {
                // Do nothing because entry is not on the same day
            }
        }
        file.close();
    }
    else {
        perror("failed to open timetable");
        exit(EXIT_FAILURE);
    }
}

// start tcp socket
// adapted from https://beej.us/guide/bgnet/
void Station::start_tcp_socket(char *port)
{
	int status;
    int yes = 1;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "tcp getaddrinfo error: %s\n", gai_strerror(status));
        std::exit(EXIT_FAILURE);
    }

    if ((tcp_fd = socket(res->ai_family, res->ai_socktype,
            res->ai_protocol)) == -1) {
        std::perror("tcp socket: socket");
        std::exit(EXIT_FAILURE);
    }

    if (setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
            sizeof yes) == -1) {
        std::perror("setsockoutput");
        std::exit(EXIT_FAILURE);
    }

    if (bind(tcp_fd, res->ai_addr, res->ai_addrlen) == -1) {
        close(tcp_fd);
        std::perror("tcp socket: bind");
        std::exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if (listen(tcp_fd, BACKLOG) == -1) {
        std::perror("listen");
        std::exit(1);
    }
    return;

}

// start udp socket
// adapted from https://beej.us/guide/bgnet/
void Station::start_udp_socket(char* port)
{
    int status;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "udp getaddrinfo error: %s\n", gai_strerror(status));
        std::exit(EXIT_FAILURE);
    }

    if ((udp_fd = socket(res->ai_family, res->ai_socktype,
            res->ai_protocol)) == -1) {
        perror("udp socket: socket");
        std::exit(EXIT_FAILURE);
    }

    if (bind(udp_fd, res->ai_addr, res->ai_addrlen) == -1) {
        close(udp_fd);
        perror("udp socket: bind");
        std::exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

}

// find the next timetable entries leaving the station
unordered_map<string, Entry> Station::find_next_depatures(time_t t) {
    unordered_map<string, Entry> departures;
    for (Entry e: timetable) {
        if (e.departure_time > t && 
            departures.find(e.arrival_location) == departures.end()){
            departures[e.arrival_location] = e;
        }
    }
    return departures;
}

// find the next departure time of a dest
Entry Station::find_next_time(time_t t, string location) {
    Entry next = timetable[0];
    for (Entry e: timetable) {
        if (e.departure_time > t &&  e.arrival_location == location ) {
            next = e;
        }
    }
    return next;
}