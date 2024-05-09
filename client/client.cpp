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

#include "../include/VolatilidadesExtranjero.hpp"

#define PORT 8080
#define BUFFER_TAM 1024

using namespace std;

bool stopTask = false;
int socketServer;
std::mutex mtx; // Mutex para proteger el acceso al recurso compartido

/**
 * @brief Guarda el estado de la tarea en un archivo temporal.
 *
 * @param state El estado de la tarea a guardar.
 */
void saveState(int state)
{
    mtx.lock();
    cout << "Guardando estado..." << endl;
    FILE* file = fopen("taskState.tmp", "w");
    if (file != NULL)
    {
        fprintf(file, "%d", state);
        fclose(file);
        const char* ackMessage = "ACK_SAVE_STATE";
        send(socketServer, ackMessage, strlen(ackMessage), 0);
    }
    else
    {
        perror("Error al abrir el archivo taskState.tmp.");
    }
    mtx.unlock();
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

    // Convertir la dirección IP a binario
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("invalid address/ Address not supported");
        return -1;
    }

    // Conectar al servidor
    cout << "Conectando al servidor..." << endl;
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connection failed");
        return -1;
    }

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
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
                            char message[BUFFER_TAM];
                            sprintf(message, "%s,%.2f", datosPrecio.pais.c_str(), datosPrecio.precio);
                            std::cout << "Enviando mensaje al servidor: " << message << std::endl;
                            send(sock, message, strlen(message), 0);
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
                        if (strcmp(buffer, "STOP_TASK") == 0)
                        {
                            stopTask = true;
                        }
                    }
                }
            }
        }
    
    return 0;
}
