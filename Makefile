CXX=g++
CXXFLAGS=-Wall
OBJ=server.o subscriber.o Socket.o TCPServer.o
BIN=server subscriber

all: server subscriber

server: server.o Socket.o TCPServer.o
	$(CXX) -o $@ $^

subscriber: subscriber.o Socket.o TCPServer.o
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $^

clean:
	rm -rf $(BIN) $(OBJ)
