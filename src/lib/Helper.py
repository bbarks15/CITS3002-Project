import datetime

# convert string to time
def string_to_time(s):
    split = s.split(":")
    return datetime.time(int(split[0]), int(split[1]))

# convert time to string
def time_to_string(t):
    return t.strftime("%H:%M")

# check if a time is valid 
# returns true if on the same day, false otherwise
def valid_time(s):
    split = s.split(":")
    if int(split[0]) < 24 and int(split[1]) >= 0:
        return int(split[1]) < 60 and int(split[1]) >=0
    else:
        return False

# get dest in query
def dest_from_query(s):
    split = s.split("/")
    split = split[0].split("=")
    return split[1]

# convert timetable raw string to readable string
def result_to_readable(s):
    split = s.split(",")
    return split[0] + " - " + split[3] + " Catch " + split[1] \
        + " from " + split[2] + " to " + split[4]

# Class to store the addr and data from recvfrom()
class UDPPair:
    def __init__(self, data, addr):
        self.addr = addr
        self.data= data

    def __repr__(self):
        return f"addr = {self.addr} data = {self.data}"