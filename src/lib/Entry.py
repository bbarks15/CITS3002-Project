from lib.Helper import string_to_time, valid_time

class Entry:
    def __init__(self, rawstring):
        self.raw_string = rawstring.strip("\n")
        s = self.raw_string.split(",")

        self.id = s[1]
        self.departure_location = s[2].lower()
        self.arrival_location = s[4].lower()

        if valid_time(s[0]) and valid_time(s[3]):
            self.departure_time = string_to_time(s[0])
            self.arrival_time = string_to_time(s[3])
        else:
            raise ValueError("Time not in same day")


    def __repr__(self):
        return self.raw_string