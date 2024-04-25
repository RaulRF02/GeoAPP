#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>

#include "VolatilidadesExtranjero.hpp"


#define PORT 8080
#define BUFFER_TAM 1024

using namespace std;

void obtenerHorasFormateadas(std::string& formattedStartTime, std::string& formattedEndTime) {
    // Obtener la hora actual
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);

    // Calcular la hora siguiente
    auto nextHour = now + std::chrono::hours(1);
    auto nextHour_c = std::chrono::system_clock::to_time_t(nextHour);

    // Verificar si los minutos son mayores a 30 para decidir qué variable utilizar
    std::tm tmAdjustedTime = *std::localtime(&now_c);
    bool usarEndTime = tmAdjustedTime.tm_min > 30;

    // Ajustar la hora actual y la hora siguiente según la condición
    if (usarEndTime) {
        // Utilizar la hora siguiente
        tmAdjustedTime.tm_min = 0;
        nextHour_c = std::mktime(&tmAdjustedTime) + 3600; // Sumar 1 hora en segundos
    } else {
        // Utilizar la hora actual
        tmAdjustedTime.tm_min = 0;
        now_c = std::mktime(&tmAdjustedTime);
    }

    // Convertir la hora actual y la hora siguiente a un formato de cadena
    std::stringstream formattedStartTimeStream;
    formattedStartTimeStream << std::put_time(std::localtime(&now_c), "%Y-%m-%dT%H:%M");
    formattedStartTime = formattedStartTimeStream.str();

    std::stringstream formattedEndTimeStream;
    formattedEndTimeStream << std::put_time(std::localtime(&nextHour_c), "%Y-%m-%dT%H:%M");
    formattedEndTime = formattedEndTimeStream.str();

    // Obtener la hora actual en formato de hora solamente
    std::tm tmStartTime = {};
    std::istringstream ssStartTime(formattedStartTime);
    ssStartTime >> std::get_time(&tmStartTime, "%Y-%m-%dT%H:%M");
}

void countMinutes(int minutes) {
    int seconds = minutes * 60;
    for (int i = 0; i < seconds; ++i) {
        std::cout << "Tiempo transcurrido: " << i + 1 << " segundos." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    // Cliente Alemania
    VolatilidadPais Alemania("Alemania", 60.7, 14.5, 52, 15, 90, -40, 40);

    std::cout << "Creando socket del cliente..." << std::endl;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_TAM] = {0};

    // Crear el socket del cliente
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir la dirección IP a binario
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        perror("invalid address/ Address not supported");
        return -1;
    }

    // Conectar al servidor
    std::cout << "Conectando al servidor..." << std::endl;
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        return -1;
    }

    while (true){    
        // Recibir mensaje del servidor solicitando el precio de energía
        memset(buffer, 0, sizeof(buffer));
        int solicitud;
        if ((solicitud = read(sock, buffer, BUFFER_TAM)) <= 0) {
            perror("recv");
            return -1;
        }
        std::cout << "Mensaje recibido del servidor: " << buffer << std::endl;
        memset(buffer, 0, sizeof(buffer));


        // Simular generación de precio de energía 
        std::string formattedStartTime;
        std::string formattedEndTime;

        obtenerHorasFormateadas(formattedStartTime, formattedEndTime);
        
        
        double precio = obtenerDatosVarianza(Alemania);
        PrecioEnergia datosPrecio(Alemania.pais, formattedStartTime, precio);
            
        std::cout << Alemania.pais << std::endl;

        // Mostrar resultados

                std::cout << "País: " << datosPrecio.pais << std::endl;
                std::cout << "Fecha: " << datosPrecio.fecha << std::endl;
                std::cout << "Precio: " << datosPrecio.precio << " €/MWh" << std::endl;
                std::cout << std::endl;


        // Enviar mensaje de precio de energía al servidor
        char message[BUFFER_TAM];
        sprintf(message, "%s,%.2f", datosPrecio.pais.c_str(), datosPrecio.precio);
        std::cout << "Enviando mensaje al servidor: " << message << std::endl;
        send(sock , message , strlen(message) , 0 );

        // Recibir respuesta del servidor, pero primero hay que limpiar el buffer
        int valread;
        if ((valread = read(sock, buffer, BUFFER_TAM)) <= 0) {
            perror("recv");
            return -1;
        }
        std::cout << buffer << std::endl;
        memset(buffer, 0, sizeof(buffer));

    
         // Recibir la primera matriz del servidor
        int matrix1[2][2];
        recv(sock, matrix1, sizeof(matrix1), 0);

        // Recibir la segunda matriz del servidor
        int matrix2[2][2];
        recv(sock, matrix2, sizeof(matrix2), 0);

        
        // Realizar la multiplicación de matrices (ejemplo)
        int result[2][2] = {{0, 0}, {0, 0}};
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    result[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
        }

        // Mostrar el resultado de la multiplicación
        std::cout << "Result:" << std::endl;

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                std::cout << result[i][j] << " ";
            }
            std::cout << std::endl;
        }

       // Recibir la orden del servidor
        char order[BUFFER_TAM] = {0};
        recv(sock, order, sizeof(order), 0);
        std::cout << "Orden recibida del servidor: " << order << std::endl;


        // Contar los minutos
        int minutes_to_count;
        if (sscanf(order, "COUNT_MINUTES:%d", &minutes_to_count) == 1) {
            std::thread counting_thread(countMinutes, minutes_to_count);
            counting_thread.join();
        }

    }
    return 0;
}
