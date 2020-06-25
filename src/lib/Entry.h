#ifndef _ENTRY_H
#define _ENTRY_H

#include <string>
#include <time.h>
#include <vector>

#include "Helper.h"

using namespace std;

// Timetable entry class
class Entry
{
    public:
        string id;
        time_t departure_time;
        time_t arrival_time;
        string departure_location;
        string arrival_location;
        string raw_string;
        Entry();
        Entry(string s);
};


#endif