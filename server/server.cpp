#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <csignal>
#include <omp.h>

#define PORT 8080
#define MAX_CLIENTS 3
#define BUFFER_TAM 1024

using namespace std;

/**
 * @brief Estructura para almacenar el precio de la energía por país.
 */
struct PrecioEnergia {
    string pais;
    float precio;
};

mutex mtx;
bool newPriceFound = false;
int indicePaisBarato = -1;
vector<int> client_sockets;
vector<PrecioEnergia> precioEnergiaPaises;

/**
 * @brief Envía un mensaje de detención al cliente especificado.
 * @param clientSocket Socket del cliente al que enviar el mensaje.
 */
void stopTask(int clientSocket) {
    const char* stopMessage = "STOP_TASK";
    if (send(clientSocket, stopMessage, strlen(stopMessage), 0) < 0) {
        perror("Error al enviar el mensaje STOP_TASK al cliente");
    }
    else {
        std::cout << "Mensaje STOP_TASK enviado al cliente: " << clientSocket << std::endl;

        // Esperar la confirmación del cliente
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Asegurarse de terminar la cadena
            if (strcmp(buffer, "ACK_SAVE_STATE") == 0) {
                std::cout << "Confirmación recibida del cliente: " << clientSocket << std::endl;
            }
        }
    }
}

/**
 * @brief Envía una nueva tarea al cliente especificado.
 * @param clientSocket Socket del cliente al que enviar la tarea.
 */
void sendTask(int clientSocket){
    cout << "Enviando la nueva orden al cliente " << precioEnergiaPaises[clientSocket].pais << " (Socket: " << client_sockets[clientSocket] << ")" << endl;
    char order[] = "COUNT_MINUTES:5";
    send(client_sockets[clientSocket], order, sizeof(order), 0);
    memset(order, 0, sizeof(order));
}

/**
 * @brief Calcula la mediana de un vector de PrecioEnergia ordenado por precio.
 * @param[in,out] datosPrecios Vector desordenado de PrecioEnergia.
 * @return Valor de la mediana.
 */
double calcularMediana(vector<PrecioEnergia> precios) {
    sort(precios.begin(), precios.end(), [](const PrecioEnergia& a, const PrecioEnergia& b) {
        return a.precio < b.precio;
    });

    size_t n = precios.size();
    size_t mitad = n / 2;

    if (n % 2 == 0) {
        return (precios[mitad - 1].precio + precios[mitad].precio) / 2.0;
    }
    else {
        return precios[mitad].precio;
    }
};

/**
 * @brief Determina el país con el precio de la energía más barato en función de los datos recibidos.
 * @param paises Vector de objetos PrecioEnergia.
 * @return Índice del país con el precio más barato en el vector paises.
 */
int determinarPaisMasBarato(const vector<PrecioEnergia>& paises) {
    int indicePaisBarato = 0;
    float precioBarato = paises[0].precio;
    for (int j = 1; j < paises.size(); j++) {
        if (paises[j].precio < precioBarato) {
            precioBarato = paises[j].precio;
            indicePaisBarato = j;
        }
    }
    cout << "el indice del pais mas barato es: " << indicePaisBarato << ", " << paises[indicePaisBarato].pais << endl;

    return indicePaisBarato;
}

/**
 * @brief Recibe los precios de energía de los clientes y determina el país más barato.
 * @param client_sockets Vector de sockets de los clientes conectados.
 */
void recibirPrecios(vector<int>& client_sockets) {
    int paisActual = -1;

    // Manejar la comunicación con los clientes
    while (true) {
        // Limpiar vector de paises
        precioEnergiaPaises.clear();

        // Solicitar precios de energía a cada cliente
        char request_msg[] = "SEND_PRICE";
        for (int j = 0; j < client_sockets.size(); j++) {
            if (send(client_sockets[j], request_msg, strlen(request_msg), 0) == -1) {
                perror("send");
                // Cerrar conexión con el cliente en caso de error
                //close(client_sockets[j]);
                //exit(EXIT_FAILURE);
            }
        }

        // Leer mensajes de precio de energía de los clientes
        for (int i = 0; i < client_sockets.size(); ++i) {
            char buffer[BUFFER_TAM] = { 0 };
            if (recv(client_sockets[i], buffer, BUFFER_TAM, 0) <= 0) {
                perror("recv");
                // Cerrar conexión con el cliente en caso de error
                close(client_sockets[i]);
                exit(EXIT_FAILURE);
            }

#pragma omp critical
            {
                cout << "Hebra " << omp_get_thread_num() << " - Función manejarComunicacion(): Mensaje recibido del cliente en el socket " << client_sockets[i] << ": " << buffer << endl;
            }
            // Extraer el nombre y el precio de la luz del mensaje
            char* nombrePais = strtok(buffer, ",");
            float precioPais = atof(strtok(NULL, ","));

            // Crear un objeto PrecioEnergia y agregarlo al vector de paises
            PrecioEnergia res;
            res.pais = nombrePais;
            res.precio = precioPais;
            precioEnergiaPaises.push_back(res);

            memset(buffer, 0, sizeof(buffer));

        }

        // Determinar el país más barato
        double umbral = calcularMediana(precioEnergiaPaises);
        float precioBarato = -1;
        // int indicePaisBarato = -1;

#pragma omp critical
        {
            cout << "Hebra " << omp_get_thread_num() << " - Función manejarComunicacion(): El umbral que determina si los precios son caros o baratos es: " << umbral << endl;
        }


        if (paisActual >= 0 && precioEnergiaPaises[paisActual].precio < umbral) {
            precioBarato = precioEnergiaPaises[paisActual].precio;
            indicePaisBarato = paisActual;
        }
        else {
            indicePaisBarato = determinarPaisMasBarato(precioEnergiaPaises);
            precioBarato = precioEnergiaPaises[indicePaisBarato].precio;
            if (paisActual >= 0){
                stopTask(client_sockets[paisActual]);
                cout << "Enviando la nueva orden al cliente " << precioEnergiaPaises[indicePaisBarato].pais << " (Socket: " << client_sockets[indicePaisBarato] << ")" << endl;
                char order[] = "COUNT_MINUTES:5";
                send(client_sockets[indicePaisBarato], order, sizeof(order), 0);
                memset(order, 0, sizeof(order));
            } else{
                cout << "Enviando la nueva orden al cliente " << precioEnergiaPaises[indicePaisBarato].pais << " (Socket: " << client_sockets[indicePaisBarato] << ")" << endl;
                char order[] = "COUNT_MINUTES:5";
                send(client_sockets[indicePaisBarato], order, sizeof(order), 0);
                memset(order, 0, sizeof(order));
            }
            paisActual = indicePaisBarato;
            
        }           
        this_thread::sleep_for(chrono::minutes(1));
    }
}





int main() {
    cout << "Creando socket del servidor..." << endl;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Vector para almacenar los sockets de los clientes conectados
    vector<int> client_sockets;

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
    address.sin_port = htons(PORT);

    // Enlazar el socket al puerto
    if (bind(server_fd, (struct sockaddr*)&address,
        sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Escuchar en el puerto
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Aceptar conexiones entrantes
    cout << "Esperando conexiones entrantes..." << endl;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
            (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        client_sockets.push_back(new_socket);
        cout << "Cliente conectado. Socket: " << new_socket << endl;
    }



    // Crear hebra para manejar la comunicación con los clientes
    #pragma omp parallel num_threads(2)
        {
    #pragma omp sections
            {
    #pragma omp section
                {
                    // Hebra para recibir los precios de los clientes
                    recibirPrecios(client_sockets);
                }

    #pragma omp section
                {
                    // Hebra para aceptar nuevas conexiones de clientes 
                    while (true) {
                        if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                            (socklen_t*)&addrlen)) < 0) {
                            perror("accept");
                            exit(EXIT_FAILURE);
                        }
                        client_sockets.push_back(new_socket);
                        cout << "Cliente conectado. Socket: " << new_socket << endl;


                    }
                }
            }
        }

    return 0;
}
