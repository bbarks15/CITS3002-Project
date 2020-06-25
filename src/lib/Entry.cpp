#include "Entry.h"

// empty constructor
Entry::Entry() {}

// constructor from string
Entry::Entry(string s) {
    raw_string = s;
    vector<string> split = spilt_string(s, ',');

    // if times are valid add them else throw exception
    if(valid_time(split[0]) && valid_time(split[3])) {
        departure_time = string_to_time(split[0]);
        arrival_time = string_to_time(split[3]);
    }
    else throw invalid_argument("Entry is not on the same day");

    id = split[1];

    string temp = split[2];
    TOLOWER(temp);
    departure_location = temp;

    temp = split[4];
    TOLOWER(temp);
    arrival_location = temp;
}