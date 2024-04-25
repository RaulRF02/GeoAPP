#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <omp.h>
#include <thread>
#include <chrono>
#include <algorithm>

#define PORT 8080
#define MAX_CLIENTS 3
#define MAX_COUNTRIES 3
#define BUFFER_TAM 1024

using namespace std;

struct PrecioEnergia {
    string pais;
    float precio;
};

// Matrices para enviar al cliente
int matrix1[2][2] = {{1, 2}, {3, 4}};
int matrix2[2][2] = {{5, 6}, {7, 8}};

/**
 * @brief Calcula la mediana de un vector de PrecioEnergia ordenado por precio.
 * @param[in,out] datosPrecios Vector desordenado de PrecioEnergia.
 * @return Valor de la mediana.
 */
double calcularMediana(vector<PrecioEnergia> precios) {
    // Ordenar el vector según el precio
    sort(precios.begin(), precios.end(), [](const PrecioEnergia& a, const PrecioEnergia& b) {
        return a.precio < b.precio;
    });

    // Calcular la posición del valor medio
    size_t n = precios.size();
    size_t mitad = n / 2;

    // Calcular la mediana
    if (n % 2 == 0) {
        // Si hay un número par de elementos, promediar los dos valores del medio
        return (precios[mitad - 1].precio + precios[mitad].precio) / 2.0;
    } else {
        // Si hay un número impar de elementos, tomar el valor del medio
        return precios[mitad].precio;
    }
};

/**
 * @brief Determina el país con el precio de la energía más barato en función de los datos recibidos.
 * @param paises Vector de objetos PrecioEnergia.
 * @return Índice del país con el precio más barato en el vector paises.
 */
int determinarPaisMasBarato(const std::vector<PrecioEnergia>& paises) {

    for (int i = 0; i < paises.size(); i++){
        cout << "pais: " << paises[i].pais << ", precio: " << paises[i].precio << endl;
    }
    int indicePaisBarato = 0;
    float precioBarato = paises[0].precio;
    for (int j = 1; j < paises.size(); j++) {
        if (paises[j].precio < precioBarato) {
            precioBarato = paises[j].precio;
            indicePaisBarato = j;
        }
    }
    cout << "el indice del pais mas barato es: " << indicePaisBarato << ", "<< paises[indicePaisBarato].pais << endl;

    return indicePaisBarato;
}

void manejarComunicacion(vector<int>& client_sockets) {
     int paisActual = -1;

    // Manejar la comunicación con los clientes
    while (true) {
        // Limpiar vector de paises
        vector<PrecioEnergia> precioEnergiaPaises;

        // Solicitar precios de energía a cada cliente
        char request_msg[] = "Por favor, envíe su precio de energía.";
        for (int j = 0; j < client_sockets.size(); j++) {
            if (send(client_sockets[j] , request_msg , strlen(request_msg) , 0 ) == -1) {
                perror("send");
                // Cerrar conexión con el cliente en caso de error
                //close(client_sockets[j]);
                //exit(EXIT_FAILURE);
            }
        }

        // Leer mensajes de precio de energía de los clientes
        for (int i = 0; i < client_sockets.size(); ++i) {
            char buffer[BUFFER_TAM] = {0};
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
            char *nombrePais = strtok(buffer, ",");
            float precioPais = atof(strtok(NULL, ","));

            // Crear un objeto PrecioEnergia y agregarlo al vector de paises
            PrecioEnergia res;
            res.pais = nombrePais;
            res.precio = precioPais;
            precioEnergiaPaises.push_back(res);

            memset(buffer, 0, sizeof(buffer));

        }

        for (int i = 0; i < precioEnergiaPaises.size(); i++){
            cout << "pais numero: " << i << ", " << precioEnergiaPaises[i].pais << endl;
        }

        // Determinar el país más barato
        double umbral = calcularMediana(precioEnergiaPaises);
        
        for (int i = 0; i < precioEnergiaPaises.size(); i++){
            cout << "pais numero: " << i << ", " << precioEnergiaPaises[i].pais << endl;
        }
        float precioBarato = -1;
        int indicePaisBarato = -1;

        #pragma omp critical
        {
            cout << "Hebra " << omp_get_thread_num() << " - Función manejarComunicacion(): El umbral que determina si los precios son caros o baratos es: " << umbral << endl;
        }


        if(paisActual >= 0 && precioEnergiaPaises[paisActual].precio <= umbral){
            precioBarato = precioEnergiaPaises[paisActual].precio;
            indicePaisBarato = paisActual;
        }else{
            indicePaisBarato = determinarPaisMasBarato(precioEnergiaPaises);
            precioBarato = precioEnergiaPaises[indicePaisBarato].precio;
            // Calcular el umbral adicional como un porcentaje de la diferencia entre el precio actual y el nuevo precio más barato
            float umbralAdicional = 0.1; // Por ejemplo, un 10% de umbral adicional
            float diferenciaPrecios = precioBarato - precioEnergiaPaises[paisActual].precio;
            float umbralDiferencia = precioEnergiaPaises[paisActual].precio * umbralAdicional;
            if (diferenciaPrecios > umbralDiferencia && paisActual != -1) {
                // Cambiar de país más barato
                precioBarato = precioEnergiaPaises[paisActual].precio;
                indicePaisBarato = paisActual;
            }else{
                paisActual = indicePaisBarato;
            }
        }

        // Enviar mensaje de país más barato a todos los clientes
        char cheapest_country_msg[BUFFER_TAM];
        sprintf(cheapest_country_msg, "El país más barato es %s con un precio de la luz de %.2f\n", precioEnergiaPaises[indicePaisBarato].pais.c_str(), precioBarato);
        for (int j = 0; j < client_sockets.size(); j++) {
            if (send(client_sockets[j] , cheapest_country_msg , strlen(cheapest_country_msg) , 0 ) == -1) {
                perror("send");
                // Cerrar conexión con el cliente en caso de error
                //close(client_sockets[j]);
                //exit(EXIT_FAILURE);
            }
        }

         // Enviar la primera matriz al cliente
         cout << "Enviando la primera matriz al cliente " << precioEnergiaPaises[indicePaisBarato].pais << " (Socket: " << client_sockets[indicePaisBarato] << ")" << endl;
        send(client_sockets[indicePaisBarato], matrix1, sizeof(matrix1), 0);
        // Enviar la segunda matriz al cliente
        send(client_sockets[indicePaisBarato], matrix2, sizeof(matrix2), 0);

        // nueva orden, contar los segundos de una cantidad determinada de minutos
        cout << "Enviando la nueva orden al cliente " << precioEnergiaPaises[indicePaisBarato].pais << " (Socket: " << client_sockets[indicePaisBarato] << ")" << endl;
        char order[] = "COUNT_MINUTES:5";
        send(client_sockets[indicePaisBarato], order, sizeof(order), 0);
        memset(order, 0, sizeof(order));


        // Esperar la respuesta del cliente
        cout << "Esperando la respuesta del cliente " << precioEnergiaPaises[indicePaisBarato].pais << " (Socket: " << client_sockets[indicePaisBarato] << ")" << endl;
        int seconds_elapsed;
        recv(client_sockets[indicePaisBarato], &seconds_elapsed, sizeof(seconds_elapsed), 0);
        std::cout << "Cliente ha contado " << seconds_elapsed << " segundos." << std::endl;


        this_thread::sleep_for(chrono::minutes(1));
        paisActual = indicePaisBarato;
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
    address.sin_port = htons( PORT );

    // Enlazar el socket al puerto
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0) {
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
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen))<0) {
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
                // Hebra para manejar la comunicación con los clientes
                manejarComunicacion(client_sockets);
            }

            #pragma omp section
            {
                // Hebra para aceptar nuevas conexiones de clientes 
                while (true) {
                    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                       (socklen_t*)&addrlen))<0) {
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
