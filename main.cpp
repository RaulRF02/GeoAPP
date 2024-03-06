/**
 * @file main.cpp
 * @brief Implementación del programa principal que utiliza la clase EnergiaAPI para obtener y analizar datos de precios de energía.
 */

#include "EnergiaAPI.hpp"
#include "VolatilidadesExtranjero.hpp"

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cstdlib>
#include <random>
#include <algorithm>

using namespace std;


/**
 * @brief Tamaño de la matriz cuadrada.
 */
const int TAMANO_MATRIZ = 3;

/**
 * @brief Multiplica dos matrices cuadradas.
 * @param matriz1 Primera matriz de entrada.
 * @param matriz2 Segunda matriz de entrada.
 * @param resultado Matriz resultante de la multiplicación.
 */
void multiplicarMatrices(int matriz1[TAMANO_MATRIZ][TAMANO_MATRIZ], int matriz2[TAMANO_MATRIZ][TAMANO_MATRIZ], int resultado[TAMANO_MATRIZ][TAMANO_MATRIZ]) {
    for (int i = 0; i < TAMANO_MATRIZ; ++i) {
        for (int j = 0; j < TAMANO_MATRIZ; ++j) {
            resultado[i][j] = 0;
            for (int k = 0; k < TAMANO_MATRIZ; ++k) {
                resultado[i][j] += matriz1[i][k] * matriz2[k][j];
            }
        }
    }
}

/**
 * @brief Muestra una matriz en la salida estándar.
 * @param matriz Matriz a mostrar.
 */
void mostrarMatriz(int matriz[TAMANO_MATRIZ][TAMANO_MATRIZ]) {
    std::cout << "Matriz resultante:\n";
    for (int i = 0; i < TAMANO_MATRIZ; ++i) {
        for (int j = 0; j < TAMANO_MATRIZ; ++j) {
            std::cout << matriz[i][j] << " ";
        }
        std::cout << "\n";
    }
}

/**
 * @brief Calcula la mediana de un vector de PrecioEnergia ordenado por precio.
 * @param[in,out] datosPrecios Vector desordenado de PrecioEnergia.
 * @return Valor de la mediana.
 */
double calcularMediana(vector<PrecioEnergia>& datosPrecios) {
    // Ordenar el vector según el precio
    sort(datosPrecios.begin(), datosPrecios.end(), [](const PrecioEnergia& a, const PrecioEnergia& b) {
        return a.precio < b.precio;
    });

    // Calcular la posición del valor medio
    size_t n = datosPrecios.size();
    size_t mitad = n / 2;

    // Calcular la mediana
    if (n % 2 == 0) {
        // Si hay un número par de elementos, promediar los dos valores del medio
        return (datosPrecios[mitad - 1].precio + datosPrecios[mitad].precio) / 2.0;
    } else {
        // Si hay un número impar de elementos, tomar el valor del medio
        return datosPrecios[mitad].precio;
    }
}


/**
 * @brief Función principal del programa que obtiene, analiza y muestra datos de precios de energía.
 * @return Código de salida del programa.
 */
int main()
{
    while (true) {
        srand(time(0));
        vector<PrecioEnergia> datosPrecios;

        // Declarar matrices de tamaño TAMANO_MATRIZ x TAMANO_MATRIZ
        int matriz1[TAMANO_MATRIZ][TAMANO_MATRIZ] = {{1, 2, 3},
                                                    {4, 5, 6},
                                                    {7, 8, 9}};

        int matriz2[TAMANO_MATRIZ][TAMANO_MATRIZ] = {{9, 8, 7},
                                                    {6, 5, 4},
                                                    {3, 2, 1}};

        int resultado[TAMANO_MATRIZ][TAMANO_MATRIZ];

        // Obtener la hora actual
        auto now = chrono::system_clock::now();
        auto now_c = chrono::system_clock::to_time_t(now);

        // Calcular la hora siguiente
        auto nextHour = now + chrono::hours(1);
        auto nextHour_c = chrono::system_clock::to_time_t(nextHour);

        // Convertir la hora actual y la hora siguiente a un formato de cadena
        stringstream formattedStartTime;

        //Quito los minutos para ponerlos a 00 usando la estructura tm y tras ajustarlo devolverlo con mktime a la estructura time_t
        //Necesito la hora en 00 para que la api me devuelve la hora actual, no la siguiente
        tm tmAdjustedTime = *localtime(&now_c);
        tmAdjustedTime.tm_min = 0;
        now_c = mktime(&tmAdjustedTime);
        formattedStartTime << put_time(localtime(&now_c), "%Y-%m-%dT%H:%M");

        stringstream formattedEndTime;
        formattedEndTime << put_time(localtime(&nextHour_c), "%Y-%m-%dT%H:%M");

        cout << "Hora actual: " << formattedStartTime.str() << endl;
        cout << "Siguiente hora: " << formattedEndTime.str() << endl;

        tm tmStartTime = {};
        istringstream ssStartTime(formattedStartTime.str());
        ssStartTime >> get_time(&tmStartTime, "%Y-%m-%dT%H:%M");
        int horaActual = tmStartTime.tm_hour;

        cout << horaActual;

        // Crear un objeto APIHandler y de varianzas
        EnergiaAPI apiHandler;
        VolatilidadesExtranjero apiHandlerVolatil;

        // Obtener datos de la API
        string apiResponse;
        if (apiHandler.obtenerDatosAPI(formattedStartTime.str(), formattedEndTime.str(), apiResponse)) {
            cout << "Datos recibidos:\n" << apiResponse << endl;

            // Crear un objeto PrecioEnergiaAPI y parsear la respuesta
            PrecioEnergia resultadoAPI;
            resultadoAPI = apiHandler.parsearDesdeRespuesta(apiResponse);
            resultadoAPI.fecha = formattedStartTime.str();
            datosPrecios.push_back(resultadoAPI);
        }

        // Obtener datos de los demas paises que no van por API
        for (const string& otroPais : {"Alemania", "Francia", "Italia"}) {
            double precio = apiHandler.obtenerDatosLista(otroPais, horaActual);

            PrecioEnergia resultadoAPI(otroPais, formattedStartTime.str(), precio);
            datosPrecios.push_back(resultadoAPI);
        }

        //Segunda forma de obtener datos
        for(const string& otroPais : {"Alemania", "Francia", "Italia"}) {
            double precio = apiHandlerVolatil.obtenerDatosVarianza(otroPais);

            PrecioEnergia resultadoAPI(otroPais, formattedStartTime.str(), precio);
            datosPrecios.push_back(resultadoAPI);
        }

        // Mostrar resultados
        for (const auto& precio : datosPrecios) {
            std::cout << "País: " << precio.pais << std::endl;
            std::cout << "Fecha: " << precio.fecha << std::endl;
            std::cout << "Precio: " << precio.precio << " €/MWh" << std::endl;
            std::cout << std::endl;
        }

        //Hacer la mediana de los datos para obtener el umbral de precios "baratos" y "caros"
        double umbral = calcularMediana(datosPrecios);

        cout << "El umbral que determina si los precios son caros o baratos es: " << umbral << endl;

        if (datosPrecios.at(0).precio <= umbral ){
            cout << "El precio del pais actual es barato, multiplico las matrices" << endl;
            // Multiplicar las matrices
            multiplicarMatrices(matriz1, matriz2, resultado);

            // Mostrar el resultado utilizando la función mostrarMatriz
            mostrarMatriz(resultado);
        } else {
            // Aqui deberia tomar la decision de pausar la ejecucion o pasar los datos a otra máquina
        
        }
        

        // Esperar una hora antes de realizar la siguiente solicitud
        this_thread::sleep_for(chrono::hours(1));
    }

    return 0;
}
