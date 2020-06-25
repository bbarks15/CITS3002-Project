CC = g++

CFLAGS = -std=c++11 -Wall -Werror -g

DEPS = lib/Entry.h lib/Station.h lib/Helper.h \
		lib/HTTPRequest.h 

OBJ = lib/Entry.o lib/Station.o lib/Helper.o station.o \
		lib/HTTPRequest.o 

%.o: %.cpp $(DEPS)
		$(CC) $(CFLAGS) -c -o $@ $<

station: $(OBJ)
		$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) station
