#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <thread>
#include "EnergiaAPI.hpp"  // Suponiendo que aquí está definida la clase EnergiaAPI
#include "VolatilidadesExtranjero.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

// Función para calcular el país más barato
std::string calcularPaisMasBarato(const std::vector<VolatilidadPais>& volatilidades) {
    // Implementa tu lógica para calcular el país más barato aquí
    // Por ejemplo, puedes usar los datos de volatilidad para tomar una decisión
    // Devuelve el nombre del país más barato
    return "España"; // Ejemplo
}

// Función que maneja la conexión con un cliente
void manejarCliente(int clientSocket, std::string& paisActual, std::vector<VolatilidadPais>& volatilidades) {
    char buffer[BUFFER_SIZE];
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::string request(buffer);

    // Procesa la solicitud del cliente
    if (request == "GET_CURRENT_COUNTRY") {
        // Envía al cliente el país actual
        send(clientSocket, paisActual.c_str(), paisActual.size(), 0);
    } else if (request == "UPDATE_COUNTRY") {
        // Recibe el nuevo país del cliente y actualiza el país actual
        recv(clientSocket, buffer, sizeof(buffer), 0);
        paisActual = buffer;

        // Envía confirmación al cliente
        std::string response = "País actualizado a: " + paisActual;
        send(clientSocket, response.c_str(), response.size(), 0);

        // Comprueba si el nuevo país es más barato y realiza las acciones necesarias
        std::string nuevoPaisMasBarato = calcularPaisMasBarato(volatilidades);
        if (nuevoPaisMasBarato != paisActual) {
            // Si el nuevo país más barato no es el actual, debemos cancelar la ejecución
            std::string mensaje = "El país más barato ha cambiado a " + nuevoPaisMasBarato + ". Cancelando ejecución...";
            send(clientSocket, mensaje.c_str(), mensaje.size(), 0);

            // Aquí puedes implementar la lógica para almacenar los datos y enviarlos al nuevo país más barato
            // ...

            // Cierra la conexión con el cliente
            close(clientSocket);

            // Termina la ejecución del servidor
            exit(EXIT_SUCCESS);
        } else {
            // El país más barato sigue siendo el mismo, envía confirmación al cliente
            std::string mensaje = "El país más barato sigue siendo " + paisActual + ". Continuando ejecución...";
            send(clientSocket, mensaje.c_str(), mensaje.size(), 0);
        }
    } else {
        // Si la solicitud no es válida, envía un mensaje de error al cliente
        std::string mensaje = "Error: solicitud no válida.";
        send(clientSocket, mensaje.c_str(), mensaje.size(), 0);
    }

    close(clientSocket); // Cierra la conexión con el cliente
}

int main() {
    // Configura el socket del servidor
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
     if (serverSocket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }
    // Configura la dirección del servidor
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888); // Puerto de escucha
    serverAddr.sin_addr.s_addr = INADDR_ANY;


    // Vincula el socket a la dirección y puerto
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al vincular el socket a la dirección y puerto");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Pone el socket en modo de escucha
    if (listen(serverSocket, 5) == -1) {
        perror("Error al poner el socket en modo de escucha");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }


    // Obtiene la lista de volatilidades de precios de los países
    std::vector<VolatilidadesExtranjero::VolatilidadPais> volatilidades;
    

    // Inicializa el país actual
    std::string paisActual = calcularPaisMasBarato(volatilidades);

    std::cout << "Servidor esperando conexiones..." << std::endl;

    while (true) {
        // Acepta conexiones entrantes
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        // Maneja la conexión con el cliente en un hilo separado
        std::thread clientThread(manejarCliente, clientSocket, std::ref(paisActual), std::ref(volatilidades));
        clientThread.detach(); // Desvincula el hilo principal del hilo del cliente
    }

    close(serverSocket); // Cierra el socket del servidor al finalizar
    return 0;
}
