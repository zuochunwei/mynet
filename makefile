SRC_FILES := $(wildcard *.cpp)
OBJ_FILES := $(patsubst %.cpp, %.o, $(SRC_FILES))

CXX = g++
LD = g++

#CXXFLAGS = -c -std=c++11 -Wall -g -ggdb -O3 -pthread -static-libstdc++
#LDFLAGS = -pthread -static-libstdc++
CXXFLAGS = -c -std=c++11 -Wall -g -ggdb -O3 -pthread
LDFLAGS = -pthread

DEF =

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(DEF) $^ -o $@

%.o : %.cxx
	$(CXX) $(CXXFLAGS) $(DEF) $^ -o $@

all : server client

server : $(OBJ_FILES) servertest.o protocolserver.o
	$(LD) $(LDFLAGS)  $^ -o $@

client : $(OBJ_FILES) clienttest.o protocolclient.o
	$(LD) $(LDFLAGS)  $^ -o $@

.PHONY : clean
clean :
	rm *.o client server
