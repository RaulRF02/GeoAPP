#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_TAM 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_TAM] = {0};
    float precioEnergia;

    // Crear el socket del cliente
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir la dirección IP a binario
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // Simular obtención del precio de la energía
    srand(time(NULL));
    precioEnergia = rand() % 10 + 1; // Precio de la energía aleatorio entre 1 y 10

    // Enviar precio de la energía al servidor
    char message[BUFFER_TAM];
    sprintf(message, "España,%.2f", precioEnergia);
    send(sock , message , strlen(message) , 0 );

    // Recibir respuesta del servidor
    valread = read(sock, buffer, BUFFER_TAM);
    std::cout << buffer << std::endl;

    return 0;
}
