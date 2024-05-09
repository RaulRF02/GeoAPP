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

#include "../include/VolatilidadesExtranjero.hpp"
#include "../include/EnergiaAPI.hpp"

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

int main() {
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

    // Simular generación de precio de energía 
    std::string formattedStartTime;
    std::string formattedEndTime;

    obtenerHorasFormateadas(formattedStartTime, formattedEndTime);


    EnergiaAPI apiHandler;
    PrecioEnergia datosPrecio;

    string apiResponse;
        if (apiHandler.obtenerDatosAPI(formattedStartTime, formattedEndTime, apiResponse)) {
            cout << "Datos recibidos:\n" << apiResponse << endl;

            // Crear un objeto PrecioEnergiaAPI y parsear la respuesta
            PrecioEnergia resultadoAPI;
            resultadoAPI = apiHandler.parsearDesdeRespuesta(apiResponse);
            resultadoAPI.fecha = formattedStartTime;
            datosPrecio = resultadoAPI;
        }

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

    // Recibir respuesta del servidor
    int valread;
    if ((valread = read(sock, buffer, BUFFER_TAM)) <= 0) {
        perror("recv");
        return -1;
    }
    std::cout << buffer << std::endl;

    return 0;
}
