#include <iostream>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>


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

int main()
{
    while (true) {
        // Obtener la hora actual
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);

        // Calcular la hora siguiente
        auto nextHour = now + std::chrono::hours(1);
        auto nextHour_c = std::chrono::system_clock::to_time_t(nextHour);

        // Convertir la hora actual y la hora siguiente a un formato de cadena
        std::stringstream formattedStartTime;
        formattedStartTime << std::put_time(std::localtime(&now_c), "%Y-%m-%dT%H:%M");

        std::stringstream formattedEndTime;
        formattedEndTime << std::put_time(std::localtime(&nextHour_c), "%Y-%m-%dT%H:%M");

        std::cout << "Hora actual: " << formattedStartTime.str() << std::endl;
        std::cout << "Siguiente hora: " << formattedEndTime.str() << std::endl;

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
            //curl_easy_setopt(curl, CURLOPT_URL, "https://apidatos.ree.es/es/datos/mercados/precios-mercados-tiempo-real?start_date=2024-02-01T11:00&end_date=2024-02-01T12:00&time_trunc=hour");
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
                std::cout << "País: " << resultado.pais << std::endl;
                std::cout << "Fecha: " << resultado.fecha << std::endl;
                std::cout << "Precio: " << resultado.precio << " €/MWh" << std::endl;
            }

            // Limpiar y cerrar la sesión de curl
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();

        // Esperar una hora antes de realizar la siguiente solicitud
        std::this_thread::sleep_for(std::chrono::hours(1));
    }

    return 0;
}
