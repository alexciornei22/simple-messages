CXX=g++
CXXFLAGS=-Wall
OBJ=server.o subscriber.o
BIN=server subscriber

build: server subscriber

server: server.o
	$(CXX) -o $@ $^

subscriber: subscriber.o
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $^

clean:
	rm -rf $(BIN) $(OBJ)
