/**
 * @file EnergiaAPI.hpp
 * @brief Definición de la clase EnergiaAPI para obtener los datos y la estructura RangoPrecio.
 */
#include <string>
#include <vector>
#include <curl/curl.h>

#include "PrecioEnergia.hpp"

/**
 * @class EnergiaAPI
 * @brief Clase que proporciona funcionalidades para interactuar con una API de energía y obtener los datos de la luz en diferentes paises.
 */
class EnergiaAPI {
    /**
     * @struct RangoPrecio
     * @brief Estructura que representa un rango de precios para los paises en los que no hay una api existente.
     */
    struct RangoPrecio {
        double min;
        double max;
    };
    private:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
        CURL* curl;
    public:
        /**
         * @brief Constructor por defecto de EnergiaAPI.
         * @details Inicializa el objeto CURL.
         */
        EnergiaAPI();

        /**
         * @brief Obtiene datos de precios de energía para un país y hora específicos, de los cuales no existe una API. 
         * Por ese motivo usa un archivo especifico llamado precios_luz.txt con los datos de lso rangos de estos paises en funcion de las horas
         * @param[in] pais País para el que se desea obtener datos de precios de energía.
         * @param[in] hora Hora del día para la que se desea obtener datos de precios de energía.
         * @return Precio de la energía para el país y hora especificados.
         */
        double obtenerDatosPrecios(const std::string& pais, int hora);

        /**
         * @brief Parsea la respuesta de la API y devuelve un objeto PrecioEnergia.
         * @param[in] respuesta Respuesta de la API a ser parseada.
         * @return Objeto PrecioEnergia parseado.
         */
        static PrecioEnergia parsearDesdeRespuesta(const std::string& respuesta);

        /**
         * @brief Función para llamar a la API de energia española y obtener datos entre dos momentos en el tiempo.
         * @param[in] startTime Momento de inicio para la consulta.
         * @param[in] endTime Momento de fin para la consulta.
         * @param[out] respuesta Respuesta de la API.
         * @return `true` si la llamada a la API fue exitosa, `false` en caso contrario.
         */
        bool obtenerDatosAPI(const std::string& startTime, const std::string& endTime, std::string& respuesta);
};