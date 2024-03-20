#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "VolatilidadesExtranjero.hpp"

#define PORT 8080
#define MAX_CLIENTS 5
#define MAX_COUNTRIES 5
#define BUFFER_TAM 1024
/*
struct Country {
    char name[50];
    float electricity_price;

    // Constructor por parámetros
    Country(const char* country_name, float price) {
        strcpy(name, country_name);
        electricity_price = price;
    }
};
*/

int main() {
    int server_fd, new_socket, client_sockets[MAX_COUNTRIES];
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_TAM] = {0};
    int i, cheapest_country_index = -1;
    float cheapest_price = -1.0;

    struct Country countries[MAX_COUNTRIES] = {
        Country("Country1", -1.0),
        Country("Country2", -1.0),
        Country("Country3", -1.0),
        // Agrega más países según necesidad
    };

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
            char *country_name = strtok(buffer, ",");
            float electricity_price = atof(strtok(NULL, ","));

            // Actualizar datos del país
            for (int j = 0; j < MAX_COUNTRIES; j++) {
                if (strcmp(countries[j].name, country_name) == 0) {
                    countries[j].electricity_price = electricity_price;
                    break;
                }
            }

            // Determinar el país más barato
            for (int j = 0; j < MAX_COUNTRIES; j++) {
                if (countries[j].electricity_price > 0 && (cheapest_price < 0 || countries[j].electricity_price < cheapest_price)) {
                    cheapest_price = countries[j].electricity_price;
                    cheapest_country_index = j;
                }
            }

            // Enviar mensaje de país más barato a todos los clientes
            char cheapest_country_msg[BUFFER_TAM];
            sprintf(cheapest_country_msg, "El país más barato es %s con un precio de la luz de %.2f\n", countries[cheapest_country_index].name, cheapest_price);
            for (int j = 0; j < MAX_COUNTRIES; j++) {
                send(client_sockets[j] , cheapest_country_msg , strlen(cheapest_country_msg) , 0 );
            }

            memset(buffer, 0, sizeof(buffer));
        }
        close(new_socket);
    }
    return 0;
}
