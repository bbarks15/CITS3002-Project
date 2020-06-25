#include "Helper.h"

// stolen from https://beej.us/guide/bgnet/
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
// stolen from https://beej.us/guide/bgnet/
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// get port from sockaddr
uint16_t get_port(struct sockaddr *s) 
{
    struct sockaddr_in *sin = (struct sockaddr_in *)s;
    return htons(sin->sin_port);
}

// split strings
vector<string> spilt_string(const string &str, char delim)
{
    stringstream ss(str);
    vector<string> split;
    string token;
    while (getline(ss, token, delim)) {
        split.push_back(token);
    }
    return split;
}

// convert string to time
time_t string_to_time(string s)
{
    // if (valid_time(s)) {
        time_t t;
        time(&t);
        struct tm *timeinfo;
        timeinfo = localtime(&t);
        strptime(s.c_str(), "%H:%M", timeinfo);
        return mktime(timeinfo);
    // }
}

// convert time to string
string time_to_string(time_t t) 
{
    struct tm  *timeinfo;
    char time[10];
    timeinfo = localtime(&t);
    strftime(time, 10, "%H:%M", timeinfo);
    time[5] = '\0';
    return string(time);
}

// extract dest from query
string dest_from_query(string s) 
{
    vector<string> temp = spilt_string(s, '/');
    vector<string> temp2 = spilt_string(temp[0], '=');
    return temp2[1];
}

// check if time is valid (same day)
bool valid_time(string s) 
{
    vector<string> str = spilt_string(s, ':');
    if (stoi(str[0]) < 24 && stoi(str[0]) >= 0) {
        return stoi(str[1]) < 60 && stoi(str[1]) >= 0;
    }
    else return false;
}

// compare timetable last modify times
bool compare_modify_time(timespec t1, timespec t2)
{
    if (t1.tv_sec == t2.tv_sec) {
        return t1.tv_nsec < t2.tv_nsec;
    }
    else return t1.tv_sec < t2.tv_sec;
}

// simple max function
int max(int a, int b)
{
	return a > b ? a : b;
}

// trim left side of string
string ltrim(const string& s)
{
    const string WHITESPACE = " \n\r\t\f\v";
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == string::npos) ? "" : s.substr(start);
}

// trim right side of string
string rtrim(const string& s)
{
    const string WHITESPACE = " \n\r\t\f\v";
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == string::npos) ? "" : s.substr(0, end + 1);
}

// trim left and right side of string
string trim(const string& s)
{
	return rtrim(ltrim(s));
}

// join vector of strings to a string
string join_string_vector(vector<string> vec) {
    std::string s;
    for (const auto &piece : vec) s += piece;
    return s;
}

// covert timetable raw strings into readable strings
string result_to_readable(string s) {
    vector<string> split = spilt_string(s, ',');
    return split[0] + " - " + split[3] + " Catch " + split[1] \
        + " from " + split[2] + " to " + split[4];

}