#include "EnergiaAPI.hpp"
#include <curl/curl.h>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <iostream>

using namespace std;

EnergiaAPI::EnergiaAPI() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

size_t EnergiaAPI::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

double obtenerValorAleatorio(double min, double max) {
    return min + static_cast<double>(rand()) / RAND_MAX * (max - min);

}

PrecioEnergia EnergiaAPI::parsearDesdeRespuesta(const std::string& respuesta) {
    PrecioEnergia resultado;
    // Buscar la sección correspondiente a "Precio mercado spot"
    size_t posInicio = respuesta.find("\"title\":\"Precio mercado spot");
    if (posInicio != string::npos) {
        // Avanzar hasta la sección de valores
        posInicio = respuesta.find("\"values\":[", posInicio);
        if (posInicio != string::npos) {
            // Encontrar el valor correspondiente a la segunda aparición
            posInicio = respuesta.find("\"value\":", posInicio);
            if (posInicio != string::npos) {
                posInicio = respuesta.find(":", posInicio);
                size_t posFin = respuesta.find(",", posInicio);
                if (posFin != string::npos) {
                    string valorStr = respuesta.substr(posInicio + 1, posFin - posInicio - 1);
                    resultado.precio = stod(valorStr);
                }
            }

            // Encontrar la fecha correspondiente a la segunda aparición
            posInicio = respuesta.find("\"datetime\":", posInicio);
            if (posInicio != string::npos) {
                posInicio = respuesta.find(":", posInicio);
                size_t posFin = respuesta.find("}", posInicio + 1);
                if (posFin != string::npos) {
                    resultado.fecha = respuesta.substr(posInicio + 1, posFin - posInicio - 1);
                }
            }
        }
    }

    // Añadir el país si es necesario
    resultado.pais = "España";

    return resultado;
}

double EnergiaAPI::obtenerDatosPrecios(const string& pais, int hora) {
    ifstream archivo("../precios_luz.txt");
    if (!archivo.is_open()) {
        cerr << "Error al abrir el archivo de precios." << endl;
        return -1.0;
    }

    unordered_map<string, unordered_map<int, RangoPrecio>> preciosPorPaisHora;

    string linea;
    string paisActual;
    while (getline(archivo, linea)) {
        if (!linea.empty() && linea.back() == '\r') {
            linea.pop_back();  // Elimina el retorno de carro si existe
        }

        if (linea.empty()) {
            continue;
        }

        if (linea.find(':') == string::npos) {
            paisActual = linea;
        } else {
            istringstream ss(linea);
            int horaInicio, horaFin;
            char separador;
            double precioMin, precioMax;

            ss >> horaInicio >> separador >> horaFin >> separador >> precioMin >> separador >> precioMax;

            for (int i = horaInicio; i <= horaFin; ++i) {
                preciosPorPaisHora[paisActual][i] = {precioMin, precioMax};
            }
        }
    }

    auto itPais = preciosPorPaisHora.find(pais);
    if (itPais != preciosPorPaisHora.end()) {
        auto itHora = itPais->second.find(hora);
        if (itHora != itPais->second.end()) {
            return obtenerValorAleatorio(itHora->second.min, itHora->second.max);
        }
    }

    cerr << "No se encontraron datos para el país o la hora especificada." << endl;
    return -1.0;
}

    bool EnergiaAPI::obtenerDatosAPI(const std::string& startTime, const std::string& endTime, std::string& respuesta){
        if (curl) {
        // Construir la URL con la hora actual y la hora siguiente
        std::string apiUrl = "https://apidatos.ree.es/es/datos/mercados/precios-mercados-tiempo-real?start_date=";
        apiUrl += startTime;
        apiUrl += "&end_date=" + endTime + "&time_trunc=hour";

        // Configurar la URL en la solicitud
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);

        // Realizar la solicitud
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return false;
        }

        return true;
    }

    return false;
    }
