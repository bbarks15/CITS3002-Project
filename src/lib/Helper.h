#ifndef _HELPER_H
#define _HELPER_H

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
#include <algorithm>

using namespace std;

// ----------------------------------------------------------------------------
//  Defines
// ----------------------------------------------------------------------------

#define BACKLOG 20

#define MAXBUFLEN 512

#define FOR(i,n) for(int i = 0; i < n; i++)

#define SO_ADDR(x) (struct sockaddr *)&(x)

#define UMAP unordered_map

#define TOLOWER(s) std::transform(s.begin(), s.end(), s.begin(), \
    [](unsigned char c){ return std::tolower(c); })

// ----------------------------------------------------------------------------
//  Functions
// ----------------------------------------------------------------------------

// stolen from https://beej.us/guide/bgnet/
void sigchld_handler(int s);

// get sockaddr, IPv4 or IPv6:
// stolen from https://beej.us/guide/bgnet/
void *get_in_addr(struct sockaddr *sa);

// get port from sockaddr
uint16_t get_port(struct sockaddr *s);

// split strings
vector<string> spilt_string(const string &str, char delim);

// convert string to time
time_t string_to_time(string s);

// convert time to string
string time_to_string(time_t t);

// check if time is valid (same day)
bool valid_time(string s);

// compare timetable last modify times
bool compare_modify_time(timespec t1, timespec t2);

// extract dest from query
string dest_from_query(string s);

// simple max function
int max(int a, int b);

// trim left side of string
string ltrim(const string& s);

// trim right side of string
string rtrim(const string& s);

// trim both sides of string
string trim(const string& s);

// join vector of strings into a string
string join_string_vector(vector<string> vec);

// covert timetable raw strings into readable strings
string result_to_readable(string s);
 
#endif