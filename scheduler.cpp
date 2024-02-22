#include <iostream>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cstdlib>
#include <random>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

struct PrecioEnergia {
    std::string pais;
    std::string fecha;
    double precio;
};

void parsearRespuesta(const std::string& respuesta, PrecioEnergia& resultado) {
    // Buscar la sección correspondiente a "Precio mercado spot"
    std::size_t posInicio = respuesta.find("\"title\":\"Precio mercado spot");
    if (posInicio != std::string::npos) {
        // Avanzar hasta la sección de valores
        posInicio = respuesta.find("\"values\":[", posInicio);
        if (posInicio != std::string::npos) {
            // Encontrar el valor correspondiente a la segunda aparición
            posInicio = respuesta.find("\"value\":", posInicio);
            if (posInicio != std::string::npos) {
                posInicio = respuesta.find(":", posInicio);
                std::size_t posFin = respuesta.find(",", posInicio);
                if (posFin != std::string::npos) {
                    std::string valorStr = respuesta.substr(posInicio + 1, posFin - posInicio - 1);
                    resultado.precio = std::stod(valorStr);
                }
            }

            // Encontrar la fecha correspondiente a la segunda aparición
            posInicio = respuesta.find("\"datetime\":", posInicio);
            if (posInicio != std::string::npos) {
                posInicio = respuesta.find(":", posInicio);
                std::size_t posFin = respuesta.find("}", posInicio + 1);
                if (posFin != std::string::npos) {
                    resultado.fecha = respuesta.substr(posInicio + 1, posFin - posInicio - 1);
                }
            }
        }
    }

    // Añadir el país si es necesario
    resultado.pais = "España";
}

struct RangoPrecio {
    double min;
    double max;
};

double obtenerValorAleatorio(double min, double max) {
    return min + static_cast<double>(rand()) / RAND_MAX * (max - min);

}

double obtenerPrecioLuz(const std::string& pais, int hora) {
    std::ifstream archivo("precios_luz.txt");
    if (!archivo.is_open()) {
        std::cerr << "Error al abrir el archivo de precios." << std::endl;
        return -1.0;
    }


    std::unordered_map<std::string, std::unordered_map<int, RangoPrecio>> preciosPorPaisHora;

    std::string linea;
    std::string paisActual;
    while (std::getline(archivo, linea)) {
        if (!linea.empty() && linea.back() == '\r') {
            linea.pop_back();  // Elimina el retorno de carro si existe
        }

        if (linea.empty()) {
            continue;
        }

        if (linea.find(':') == std::string::npos) {
            paisActual = linea;
        } else {
            std::istringstream ss(linea);
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

    std::cerr << "No se encontraron datos para el país o la hora especificada." << std::endl;
    return -1.0;
}


int main()
{
    while (true) {
        std::srand(std::time(0));
        std::vector<PrecioEnergia> datosPrecios;

        // Obtener la hora actual
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);

        // Calcular la hora siguiente
        auto nextHour = now + std::chrono::hours(1);
        auto nextHour_c = std::chrono::system_clock::to_time_t(nextHour);

        // Convertir la hora actual y la hora siguiente a un formato de cadena
        std::stringstream formattedStartTime;

        //Quito los minutos para ponerlos a 00 usando la estructura tm y tras ajustarlo devolverlo con mktime a la estructura time_t
        //Necesito la hora en 00 para que la api me devuelve la hora actual, no la siguiente
        tm tmAdjustedTime = *std::localtime(&now_c);
        tmAdjustedTime.tm_min = 0;
        now_c = std::mktime(&tmAdjustedTime);
        formattedStartTime << std::put_time(std::localtime(&now_c), "%Y-%m-%dT%H:%M");

        std::stringstream formattedEndTime;
        formattedEndTime << std::put_time(std::localtime(&nextHour_c), "%Y-%m-%dT%H:%M");

        std::cout << "Hora actual: " << formattedStartTime.str() << std::endl;
        std::cout << "Siguiente hora: " << formattedEndTime.str() << std::endl;

        std::tm tmStartTime = {};
        std::istringstream ssStartTime(formattedStartTime.str());
        ssStartTime >> std::get_time(&tmStartTime, "%Y-%m-%dT%H:%M");
        int horaActual = tmStartTime.tm_hour;

        std::cout << horaActual;


        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            // Construir la URL con la hora actual y la hora siguiente
            std::string apiUrl = "https://apidatos.ree.es/es/datos/mercados/precios-mercados-tiempo-real?start_date=";
            apiUrl += formattedStartTime.str();
            apiUrl += "&end_date=" + formattedEndTime.str() + "&time_trunc=hour";

            // Configurar la URL en la solicitud
            curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
            //curl_easy_setopt(curl, CURLOPT_URL, "https://apidatos.ree.es/es/datos/mercados/precios-mercados-tiempo-real?start_date=2024-02-21T17:59&end_date=2024-02-21T19:03&time_trunc=hour");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            // Realizar la solicitud
            res = curl_easy_perform(curl);

            if (res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            else {
                std::cout << "Datos recibidos:\n" << readBuffer << std::endl;
                PrecioEnergia resultado;
                parsearRespuesta(readBuffer, resultado);
                datosPrecios.push_back(resultado);

            }

            // Limpiar y cerrar la sesión de curl
            curl_easy_cleanup(curl);


            // Obtener la hora de los demas paises, hacer un vector e ir comparando
            for (const std::string& otroPais : {"Alemania", "Francia", "Italia"}) {
                double precio = obtenerPrecioLuz(otroPais, horaActual);

                PrecioEnergia resultado;
                resultado.pais = otroPais;
                resultado.fecha = formattedStartTime.str();
                resultado.precio = precio;

                datosPrecios.push_back(resultado);

            }

            for (const auto& precio : datosPrecios) {
                std::cout << "País: " << precio.pais << std::endl;
                std::cout << "Fecha: " << precio.fecha << std::endl;
                std::cout << "Precio: " << precio.precio << " €/MWh" << std::endl;
                std::cout << std::endl;
            }
                        


        }

        curl_global_cleanup();

        // Esperar una hora antes de realizar la siguiente solicitud
        std::this_thread::sleep_for(std::chrono::hours(1));
    }

    return 0;
}
