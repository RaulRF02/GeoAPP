#include "EnergiaAPI.hpp"

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



int main()
{
    while (true) {
        srand(time(0));
        vector<PrecioEnergia> datosPrecios;

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

        // Crear un objeto APIHandler
        EnergiaAPI apiHandler;

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
            double precio = apiHandler.obtenerDatosPrecios(otroPais, horaActual);

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


        // Esperar una hora antes de realizar la siguiente solicitud
        this_thread::sleep_for(chrono::hours(1));
    }

    return 0;
}
