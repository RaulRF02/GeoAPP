#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    // Crear el socket del servidor
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888); // Puerto de escucha
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Vincular el socket a la dirección y puerto
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al vincular el socket a la dirección y puerto");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(serverSocket, 5) == -1) {
        perror("Error al poner el socket en modo de escucha");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Aceptar conexiones entrantes
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (true) {
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Error al aceptar la conexión entrante");
            continue;
        }

        // Manejar la conexión del cliente en un nuevo hilo o proceso
        // Puedes implementar esta parte de acuerdo a tus necesidades

        close(clientSocket); // Cierra la conexión después de manejarla (puedes cambiar esto según tus requerimientos)
    }

    close(serverSocket); // Cierra el socket del servidor al finalizar
    return 0;
}
