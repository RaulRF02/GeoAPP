#include <iostream>
#include <vector>
#include <cstring> // Para strtok
#include <cstdlib> // Para atof
#include <cstdio> // Para sprintf
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "PrecioEnergia.hpp"

#define PORT 8080
#define MAX_CLIENTS 5
#define MAX_COUNTRIES 5
#define BUFFER_TAM 1024

int main() {
    int server_fd, new_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_TAM] = {0};
    int i;
    
    std::vector<PrecioEnergia> paises;

    // Crear el socket del servidor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Asignar el socket a la dirección y el puerto
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Enlazar el socket al puerto
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Escuchar en el puerto
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Aceptar conexiones entrantes
    for (i = 0; i < MAX_COUNTRIES; ++i) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        client_sockets[i] = new_socket;

        // Manejar comunicación con el cliente
        int valread;
        while ((valread = read(new_socket, buffer, BUFFER_TAM)) > 0) {
            printf("Mensaje recibido: %s\n",buffer);
            // Extraer el nombre y el precio de la luz del mensaje
            char *nombrePais = strtok(buffer, ",");
            float precioPais = atof(strtok(NULL, ","));
            
            // Creación de un objeto PrecioEnergia
            PrecioEnergia res;
            res.pais = nombrePais;
            res.precio = precioPais;
            paises.push_back(res); // Agregar país al vector

            // Determinar el país más barato
            int indicePaisBarato = 0;
            float precioBarato = paises[0].precio;
            for (int j = 1; j < paises.size(); j++) {
                if (paises[j].precio < precioBarato) {
                    precioBarato = paises[j].precio;
                    indicePaisBarato = j;
                }
            }

            // Enviar mensaje de país más barato a todos los clientes
            char cheapest_country_msg[BUFFER_TAM];
            sprintf(cheapest_country_msg, "El país más barato es %s con un precio de la luz de %.2f\n", paises[indicePaisBarato].pais.c_str(), precioBarato);
            for (int j = 0; j < MAX_COUNTRIES; j++) {
                send(client_sockets[j] , cheapest_country_msg , strlen(cheapest_country_msg) , 0 );
            }

            memset(buffer, 0, sizeof(buffer));
        }
        close(new_socket);
    }
    return 0;
}
