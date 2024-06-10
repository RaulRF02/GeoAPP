CFLAGS = -std=c++11 -lcurl -fopenmp -lboost_system -lboost_serialization
INC_DIR = include
SRC_DIR = src
SERVER_DIR = server
CLIENT_DIR = client

all: servidor cliente

servidor: $(SERVER_DIR)/server.cpp
	$(CXX) -o servidor $(SERVER_DIR)/server.cpp $(CFLAGS) -I/usr/include/boost

cliente: $(CLIENT_DIR)/client.cpp $(SRC_DIR)/*.cpp
	$(CXX) -o cliente $(CLIENT_DIR)/client.cpp $(SRC_DIR)/*.cpp -I$(INC_DIR) $(CFLAGS) -I/usr/include/boost

clean:
	rm -f servidor cliente
