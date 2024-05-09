CC = g++
CFLAGS = -std=c++11 -lcurl -fopenmp
INC_DIR = include
SRC_DIR = src
SERVER_DIR = server
CLIENT_DIR = client

all: servidor cliente

servidor: $(SERVER_DIR)/server.cpp
	$(CC) -o servidor $(SERVER_DIR)/server.cpp $(CFLAGS)

cliente: $(CLIENT_DIR)/client.cpp $(SRC_DIR)/*.cpp
	$(CC) -o cliente $(CLIENT_DIR)/client.cpp $(SRC_DIR)/*.cpp -I$(INC_DIR) $(CFLAGS)

clean:
	rm -f servidor cliente
