#include <string>
#include <vector>
#include <curl/curl.h>

#include "PrecioEnergia.hpp"

class EnergiaAPI {
    struct RangoPrecio {
    double min;
    double max;
    };
    private:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
        CURL* curl;
    public:
        EnergiaAPI();
        double obtenerDatosPrecios(const std::string& pais, int hora);
        static PrecioEnergia parsearDesdeRespuesta(const std::string& respuesta);
        //Funcion para llamar a la API española
        bool obtenerDatosAPI(const std::string& startTime, const std::string& endTime, std::string& respuesta);
};