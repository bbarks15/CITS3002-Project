#!/usr/bin/env python3
import sys
import os
import socket
import select
import queue
import datetime
import time
import subprocess

from lib.Helper import *
from lib.Entry import *
from lib.HTTPRequest import *
from lib.Station import *

# Brandon Barker
# 22507204


HOST = "127.0.0.1"
MAXBUFLEN = 512

# ----------------------------------------------------------------------------
#  Query Functions
# ----------------------------------------------------------------------------

# query to adjacent stations searching for the dest
def query_adjacent_stations(query, adj_stations):
    base_query = query.split("/")[0]

    # iterate through adjacent stations
    for p in adj_stations:

        # new query with arrival time
        msg = base_query + "/" + time_to_string(adj_stations[p].arrival_time)

        # send to adjacent station
        try:
            station.udp_socket.sendto(bytes(msg,"utf-8"), (HOST, p))
        except socket.error as e:
            print("query_adjacent_stations sento(): ", e, file=sys.stderr)
            sys.exit(1)

        # wait for valid response from search
        data_str = ""
        while True:
            try:
                data, addr = station.udp_socket.recvfrom(MAXBUFLEN)
            except:
                print("query_adjacent_stations recvfrom(): ", e)
                sys.exit(1)

            if data:
                data_str = str(data, "utf-8")

                # return name for query
                if data_str == "?name":
                    name = bytes(station.name, "utf-8")
                    try: 
                        station.udp_socket.sendto(name, addr)
                    except socket.error as e:
                        print("query_adjacent_stations sendto(): \
                         ", e, file=sys.stderr)
                        sys.exit(1)

                # if query is invalid
                elif data_str[0:4] == "?to=":
                    nf = bytes("NF", "utf-8")
                    try: 
                        station.udp_socket.sendto(nf, addr)
                    except socket.error as e:
                        print("query_adjacent_stations sendto(): \
                         ", e, file=sys.stderr)
                        sys.exit(1)
            else:
                print("query_adjacent_stations recvfrom(): no data", \
                    file=sys.stderr)
                sys.exit(1)

            # if response is valid break loop
            if (addr[1] == p and data_str[0] != "?") or (data_str == "NF"):
                break
        
        # return response if found
        if data_str[0:5] == "FOUND":
            return data_str + "/" + adj_stations[addr[1]].raw_string

    return "NF"


# Start the search for a station
def find_station(query):

    dest = dest_from_query(query)
    split_query = query.split("/")
    current_time = string_to_time(split_query[1])

    # find the next timetable entrys leaving the station
    next = station.find_next_departures(current_time)
    if len(next) == 0:
        return "NF"

    adjacent_stations = {}

    # Found dest in timetable 
    if dest in next:
        return "FOUND/" + time_to_string(next[dest].arrival_time) \
                + "/" + next[dest].raw_string

    # iterate througha adjacent stations querying for name
    for p in station.adjacent_stations:
        msg = b"?name"

        try:
            station.udp_socket.sendto(msg,(HOST, p))
        except socket.error as e:
            print("find_station sendto(): ", e, file=sys.stderr)

        try:
            response, addr = station.udp_socket.recvfrom(MAXBUFLEN)
        except socket.error as e:
            print("find_station recvfrom(): ", e, file=sys.stderr)

        if response:
            response = str(response, "utf-8").lower()
        else:
            print("find_station recvfrom(): no data", file=sys.stderr)
            sys.exit(1)

        # if in next add to adjacent stations
        if response in next.keys():
            adjacent_stations[p] = next[response]

    # if no adjacent stations return NF
    if len(adjacent_stations.keys()) == 0:
        return "NF"

    return query_adjacent_stations(query, adjacent_stations)

# handle udp query
def handle_udp_query(query):
    if query[0] == "?":
        if query == "?name":
            return station.name
        if query[0:4] == "?to=":
            return find_station(query)
    elif query[0:5] == "FOUND":
        return query
    return query

# handle tcp query
def handle_tcp_query(request):
    # check method and query
    if request.method != "GET" or request.uri[0:4] != "?to=":
        return "Invalid Query"

    # if dest is to itself return invalid
    dest = dest_from_query(request.uri)
    if dest == station.name:
        return "Invalid Destination"

    # time = time_to_string(datetime.now())
    time = "12:30"
    
    query = "?to=" + dest.lower() + "/" + time

    return find_station(query)


# ----------------------------------------------------------------------------
#  Main
# ----------------------------------------------------------------------------

if __name__ == "__main__":
    argv = sys.argv
    argc = len(argv)

    # check args
    if argc < 4:
        raise ValueError(f"Usage: {argv[0]} tcp_port" + \
                            "udp_port adjacent_stations ...\n ")
        exit(1)

    # create station
    station = Station(argv[1])

    # load adjacent station ports
    for i in range(4, argc):
        station.add_adjacent_station(argv[i])

    # load timetable
    station.load_timetable()

    # start sockets
    station.start_tcp_socket(int(argv[2]))
    station.start_udp_socket(int(argv[3]))

    # fd sets
    inputs = [station.tcp_socket, station.udp_socket]
    outputs = []

    # message queues
    tcp_message_queues = {}
    udp_message_queues = {}

    # select() code is adapted from
    # https://pymotw.com/2/select/  
    while inputs:
        # check for timetable changes
        station.load_timetable()

        # select()
        readable, writable, exceptional = select.select(
            inputs, outputs, inputs)

        # readable sockets
        for s in readable:
            # if udp socket
            if s is station.udp_socket:
                try:
                    data, addr = s.recvfrom(1024)
                except socket.error as e:
                    print("udp recvfrom: ", e, file=sys.stderr)
                    exit(1)
                else:
                    if data and addr:
                        data = str(data, "utf-8")
                        udp_message_queues[s] = queue.Queue()
                        udp_message_queues[s].put(UDPPair(data, addr))
                        if s not in outputs:
                            outputs.append(s)
                    else:
                        if s in outputs:
                            outputs.remove(s)
                        inputs.remove(s)
                        s.close()
                        del udp_message_queues[s]
            # if new tcp socket
            elif s is station.tcp_socket:
                try:
                    connection, client_address = s.accept()
                except socket.error as e:
                    print("tcp accept(): ", e, file=sys.stderr)
                else:
                    inputs.append(connection)
                    tcp_message_queues[connection] = queue.Queue()
            #if tcp socket sending data
            else:
                try:
                    data = s.recv(1024)
                except socket.error as e:
                    print("tcp socket recv(): ", e, file=sys.stderr) 
                    sys.exit(1)
                else:
                    if data:
                        tcp_message_queues[s].put(data)
                        if s not in outputs:
                            outputs.append(s)
                    else:
                        print("tcp socket recv(): no data", file=sys.stderr) 
                        if s in outputs:
                            outputs.remove(s)
                        inputs.remove(s)
                        s.close()
                        del tcp_message_queues[s]

        # writable sockets
        for s in writable:
            # if udp connection
            if s.type == socket.SocketKind.SOCK_DGRAM:
                try:
                    request = udp_message_queues[s].get_nowait()
                except queue.Empty:
                    outputs.remove(s)
                else:
                    msg = handle_udp_query(request.data)
                    try:
                        s.sendto(bytes(msg,"utf-8"), request.addr)
                    except socket.error as e:
                        print("udp sento(): ", e, file=sys.stderr)
                        sys.exit(1)
                    else: 
                        outputs.remove(s)

            # if tcp connection
            elif s.type == socket.SocketKind.SOCK_STREAM:
                try:
                    next_msg = tcp_message_queues[s].get_nowait()
                except queue.Empty:
                    outputs.remove(s)
                else:
                    request = HTTPRequest(str(next_msg, "utf-8"))
                    message = handle_tcp_query(request)
                    msg = ""
                    if message == "NF":
                        msg = request.response(404, "No Journey Found")
                    elif message == "Invalid Destination" or \
                         message == "Invalid Query":
                        msg = request.response(400, message)
                    else:
                        msg = request.response(200, message)
                    try:
                        s.send(bytes(msg, "utf-8"))
                    except socket.error as e:
                        print("tcp send(): ", e, file=sys.stderr)
                        sys.exit(1)
                    else:
                        if s in outputs:
                            outputs.remove(s)
                        inputs.remove(s)
                        s.close()
                        del tcp_message_queues[s]

        for s in exceptional:
            inputs.remove(s)
            if s in outputs:
                outputs.remove(s)
            s.close()
            del tcp_message_queues[s]
