import sys
import os
import socket
import datetime
from lib.Entry import Entry

class Station:

    # Station Constructor
    def __init__(self, name):
        self.name = name
        self.tcp_socket = None
        self.udp_socket = None
        self.adjacent_stations = []
        self.timetable = None
        self.timetable_stat = None

    # Adds adjacent station port 
    def add_adjacent_station(self, s):
        self.adjacent_stations.append(int(s))

    # Starts the tcp socket
    def start_tcp_socket(self, tcpport):
        try:
            self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except socket.error as e:
            print("tcp socket(): ", e)
            sys.exit(1)

        try:
            self.tcp_socket.setsockopt(socket.SOL_SOCKET, \
            socket.SO_REUSEADDR, 1)
        except socket.error as e:
            print("tcp setsockopt(): ", e)
            sys.exit(1)

        try:
            self.tcp_socket.bind(('127.0.0.1', tcpport))
        except socket.error as e:
            print("tcp bind(): ", e)
            sys.exit(1)

        try:
            self.tcp_socket.listen(20)
        except socket.error as e:
            print("tcp listen(): ", e)
            sys.exit(1)

    # Starts the udp socket
    def start_udp_socket(self, udpport):
        try:
            self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        except socket.error as e:
            print("udp socket(): ", e)
            sys.exit(1)

        try:
            self.udp_socket.bind(('127.0.0.1', udpport))
        except socket.error as e:
            print("udp bind(): ", e)
            sys.exit(1)


    # Loads the timetable
    def load_timetable(self):
        if self.name == None:
            return None

        current_path = os.path.dirname(os.path.realpath(__file__))

        try:
            tt_stat = os.stat(f"../timetables/tt-{self.name}")
        except FileNotFoundError as e:
            print("stat() timetable: ", e, file=sys.stderr)


        if self.timetable_stat == None:
            pass
        elif self.timetable_stat.st_mtime <= tt_stat.st_mtime:
            self.timetable_stat =  tt_stat
        else:
            return

        try:
            tt = open(f"../timetables/tt-{self.name}", "r")
        except FileNotFoundError as e:
            print("Error opening timetable: ", e, file=sys.stderr)
            sys.exit(1)
        else:
            tt.readline()
            timetable = []
            for line in tt:
                try:
                    timetable.append(Entry(line))
                except:
                    pass

            self.timetable = timetable
            tt.close()

    # Find the next depatures
    def find_next_departures(self, time):
        depatures = {}
        for e in self.timetable:
            if e.departure_time > time:
                if depatures.get(e.arrival_location) == None:
                    depatures[e.arrival_location] = e

        return depatures

    # find next time of location leaving the station
    def find_next_time(self, time, location):
        for e in self.timetable:
            if e.departureTime > time and e.arrivalLocation == location:
                return e
        return None