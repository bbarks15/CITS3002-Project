#include <string>
#include <vector>
#include <sys/types.h>

#ifndef _STATION_H
#define _STATION_H

#include "Entry.h"
#include "Helper.h"

using namespace std;

// class for Station
class Station
{
    public:
        string name;
        int tcp_fd;
        int udp_fd;
        vector<int> adjacent;
        vector<Entry> timetable;
        struct timespec tt_last_modify;
        Station(string n);
        Station();
        void load_timetable();
        void start_tcp_socket(char *port);
        void start_udp_socket(char *port);
        unordered_map<string, Entry> find_next_depatures(time_t t);
        Entry find_next_time(time_t t, string location);
};

#endif