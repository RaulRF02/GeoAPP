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
#include <signal.h>
#include <fstream>
#include <mutex>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "../include/Matrix.hpp"
#include "../include/VolatilidadesExtranjero.hpp"
#include <boost/asio.hpp>


#define PORT 8080
#define BUFFER_TAM 1024
#define MATRIX_SIZE 10000


using namespace std;
using boost::asio::ip::tcp;


bool stopTask = false;
int socketServer;
std::mutex mtx; // Mutex para proteger el acceso al recurso compartido

int A[MATRIX_SIZE][MATRIX_SIZE], B[MATRIX_SIZE][MATRIX_SIZE], C[MATRIX_SIZE][MATRIX_SIZE];

void sendMatrix(int client_socket, int matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    int buffer[BUFFER_TAM / sizeof(int)];
    int elements_per_buffer = BUFFER_TAM / sizeof(int);

    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; j += elements_per_buffer) {
            int block_size = min(elements_per_buffer, MATRIX_SIZE - j);
            memcpy(buffer, &matrix[i][j], block_size * sizeof(int));
            send(client_socket, buffer, block_size * sizeof(int), 0);
            memset(buffer, 0, sizeof(buffer));

        }
    }
}

void receiveMatrix(int server_socket, int matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    int buffer[BUFFER_TAM / sizeof(int)];
    int elements_per_buffer = BUFFER_TAM / sizeof(int);

    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; j += elements_per_buffer) {
            int block_size = min(elements_per_buffer, MATRIX_SIZE - j);
            recv(server_socket, buffer, block_size * sizeof(int), 0);
            memcpy(&matrix[i][j], buffer, block_size * sizeof(int));
        }
    }
}


/**
 * @brief Guarda el estado de la tarea en un archivo temporal.
 *
 * @param state El estado de la tarea a guardar.
 */
void saveState(int row) {
    std::lock_guard<std::mutex> lock(mtx);  // Asegura el acceso exclusivo al archivo
    std::ofstream file("taskState.tmp");
    if (file.is_open()) {
        file << row;  // Escribe el número de la fila actual en el archivo
        file.close();
        char ackMessage[BUFFER_TAM] = {0};
        sprintf(ackMessage, "ACK_SAVE_STATE:%d", row);
        send(socketServer, ackMessage, strlen(ackMessage), 0);

        // Serializar la matriz C y enviarla al servidor
        // sendMatrix(socketServer, C);

        std::cout << "Estado guardado: Fila " << row << std::endl;
    } else {
        perror("Error al abrir el archivo taskState.tmp.");
    }
    stopTask = false;
/*
    // Enviar el estado al servidor
    char stateMessage[BUFFER_TAM]  = {0};
    sprintf(stateMessage, "STATE:%d", row);
    send(socketServer, stateMessage, strlen(stateMessage), 0);
    std::cout << "Estado enviado al servidor: Fila " << row << std::endl;   
    */
}


int loadState() {
    std::ifstream file("taskState.tmp");
    int row = 0;
    if (file.is_open()) {
        file >> row;  // Lee el número de la fila desde el archivo
        file.close();
        std::cout << "Estado restaurado: Continuando desde la fila " << row << std::endl;
    } else {
        std::cout << "No se encontró estado previo. Comenzando desde la fila 0." << std::endl;
    }
    return row;
}



/**
 * @brief Obtiene las horas formateadas para el inicio y fin de una tarea.
 *
 * @param formattedStartTime La hora de inicio formateada.
 * @param formattedEndTime La hora de fin formateada.
 */
void obtenerHorasFormateadas(std::string &formattedStartTime, std::string &formattedEndTime)
{
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
    if (usarEndTime)
    {
        // Utilizar la hora siguiente
        tmAdjustedTime.tm_min = 0;
        nextHour_c = std::mktime(&tmAdjustedTime) + 3600; // Sumar 1 hora en segundos
    }
    else
    {
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

void multiplicarMatrices(int N, int startRow) {
    for (int i = startRow; i < N; ++i) {
        if (stopTask) {
            saveState(i);
            break;
        }
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
        if (i % 10 == 0) {
            std::cout << "Fila " << i << " procesada." << std::endl;
        }
    }
}

/**
 * @brief Cuenta los minutos transcurridos desde un punto de partida hasta un límite.
 *
 * @param minutes El número de minutos a contar.
 */
void countMinutes(int minutes)
{
    int seconds = minutes * 60;
    int startFrom = 0;
    std::ifstream file("taskState.tmp");
    if (file.is_open()) {
        // Leer el número desde el archivo
        file >> startFrom;
        file.close();
        std::cout << "Continuando desde: " << startFrom << " segundos." << std::endl;
    } else {
        std::cout << "Comenzando desde 0 segundos." << std::endl;
    }
    for (int i = startFrom; i < seconds; ++i)
    {
        if (stopTask)
        {
            std::cout << "Hilo detenido por señal de STOP_TASK" << std::endl;
            saveState(i);
            break;
        }
        std::cout << "Tiempo transcurrido: " << i + 1 << " segundos." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

/**
 * @brief Carga la volatilidad de un país específico desde un archivo JSON.
 *
 * @param nombrePais El nombre del país para buscar sus datos.
 * @return Un objeto VolatilidadPais con los datos del país.
 */
VolatilidadPais cargarVolatilidadPais(const std::string &nombrePais)
{
    // Abre el archivo JSON
    std::ifstream file("data/datosPaises.json");
    if (!file.is_open())
    {
        std::cerr << "No se pudo abrir el archivo de datos de países." << std::endl;
        exit(1); // Termina el programa si no se puede abrir el archivo
    }

    // Lee el archivo en un buffer
    std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Parsea el JSON
    rapidjson::Document doc;
    doc.Parse(json.c_str());

    // Verifica si hubo errores al parsear
    if (doc.HasParseError())
    {
        std::cerr << "Error al parsear el archivo JSON: " << doc.GetParseError() << std::endl;
        exit(1); // Termina el programa si hay un error de parseo
    }

    // Busca los datos del país específico
    if (doc.HasMember(nombrePais.c_str()))
    {
        const rapidjson::Value &paisData = doc[nombrePais.c_str()];
        // Crea el objeto VolatilidadPais con los datos del país
        return VolatilidadPais(
            nombrePais,
            paisData["media"].GetDouble(),
            paisData["desviacionDia"].GetDouble(),
            paisData["desviacionHora"].GetDouble(),
            paisData["mes"]["min"].GetDouble(),
            paisData["mes"]["max"].GetDouble(),
            paisData["dia"]["min"].GetDouble(),
            paisData["dia"]["max"].GetDouble());
    }
    else
    {
        std::cerr << "No se encontraron datos para el país: " << nombrePais << std::endl;
        exit(1); // Termina el programa si no se encuentran datos para el país
    }
}

/**
 * @brief Maneja problemas de conexión o mensajes del servidor.
 *
 * @param sock El descriptor de socket para recibir mensajes.
 */
void manejarProblemas(int sock)
{
    while (true)
    {
        // Recibir la orden del servidor
        char order[BUFFER_TAM] = {0};
        recv(sock, order, sizeof(order), 0);
        std::cout << "Orden recibida del servidor: " << order << std::endl;
    }
}

/**
 * @brief Punto de entrada principal del programa.
 *
 * @param argc El número de argumentos de línea de comando.
 * @param argv Los argumentos de línea de comando.
 * @return El código de salida del programa.
 */
int main(int argc, char *argv[])
{
    if (argc!= 2)
    {
        cerr << "Uso: " << argv[0] << " <nombre_del_pais>" << endl;
        return 1;
    }

    string pais = argv[1];

    cout << "Datos para " << pais << endl;

    cout << "Creando socket del cliente..." << endl;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_TAM] = {0};

    // Crear el socket del cliente
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir la dirección IP a binario, ojo la ip del servidor fuera es 192.168.1.162
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("invalid address/ Address not supported");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connection failed");
        return -1;
    }

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        cout << buffer << endl;
        int solicitud;
        if ((solicitud = read(sock, buffer, BUFFER_TAM)) <= 0)
        {
            perror("Error al recibir datos del servidor");
            close(sock); // Cierra el socket en caso de error
            return -1;
        }
        cout << "Mensaje recibido del servidor: " << buffer << endl;

    #pragma omp parallel num_threads(2)
            {
    #pragma omp sections
                {
    #pragma omp section
                    {
                        if (strcmp(buffer, "SEND_PRICE") == 0)
                        {
                            // Simular generación de precio de energía
                            std::string formattedStartTime;
                            std::string formattedEndTime;

                            obtenerHorasFormateadas(formattedStartTime, formattedEndTime);
                            PrecioEnergia datosPrecio;

                            if (pais == "España"){
                                EnergiaAPI apiHandler;
                                string apiResponse;
                                    if (apiHandler.obtenerDatosAPI(formattedStartTime, formattedEndTime, apiResponse)) {
                                        cout << "Datos recibidos:\n" << apiResponse << endl;

                                        // Crear un objeto PrecioEnergiaAPI y parsear la respuesta
                                        PrecioEnergia resultadoAPI;
                                        resultadoAPI = apiHandler.parsearDesdeRespuesta(apiResponse);
                                        resultadoAPI.fecha = formattedStartTime;
                                        datosPrecio = resultadoAPI;
                                    }
                            } else{
                                VolatilidadPais datosPais = cargarVolatilidadPais(pais);
                                double precio = obtenerDatosVarianza(datosPais);
                                datosPrecio.pais = pais;
                                datosPrecio.fecha = formattedStartTime;
                                datosPrecio.precio = precio;
                            }
                            

                            std::cout << datosPrecio.pais << std::endl;

                            // Mostrar resultados

                            std::cout << "País: " << datosPrecio.pais << std::endl;
                            std::cout << "Fecha: " << datosPrecio.fecha << std::endl;
                            std::cout << "Precio: " << datosPrecio.precio << " €/MWh" << std::endl;
                            std::cout << std::endl;

                            // Enviar mensaje de precio de energía al servidor
                            memset(buffer, 0, sizeof(buffer));
                            char message[BUFFER_TAM] = {0};
                            sprintf(message, "%s,%.2f", datosPrecio.pais.c_str(), datosPrecio.precio);
                            std::cout << "Enviando mensaje al servidor: " << message << std::endl;
                            send(sock, message, strlen(message), 0);
                            cout << message << endl;
                            socketServer = sock;
                        }
                    }
    #pragma omp section
                    {
                        if (strncmp(buffer, "COUNT_MINUTES", 13) == 0)
                        {
                            // Contar los minutos
                            stopTask = false;
                            int minutes_to_count;
                            if (sscanf(buffer, "COUNT_MINUTES:%d", &minutes_to_count) == 1)
                            {
                                std::thread counting_thread(countMinutes, minutes_to_count);
                                counting_thread.detach();
                            }
                        }
                    }
    #pragma omp section
                    {
                        if (strncmp(buffer, "MULTIPLY_MATRICES", 17) == 0) {
                            stopTask = false;
                            int N, state = 0;
                            if (sscanf(buffer, "MULTIPLY_MATRICES:%d,STATE:%d", &N, &state) >= 1) {

                                receiveMatrix(socketServer, A);
                                receiveMatrix(socketServer, B);
                                receiveMatrix(socketServer, C);
                                
                                std::thread multiplication_thread([N, state]() {
                                    multiplicarMatrices(N, state);
                                });
                                multiplication_thread.detach();
                            }
                        }
                    }
                #pragma omp section
                {
                    if (strcmp(buffer, "STOP_TASK") == 0) {
                        stopTask = true;
                    }
                }
            }
        }
    }

    return 0;
}