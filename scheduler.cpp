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
#include <algorithm>

using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp)
{
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

struct PrecioEnergia {
    string pais;
    string fecha;
    double precio;
};

void parsearRespuesta(const string& respuesta, PrecioEnergia& resultado) {
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
}

struct RangoPrecio {
    double min;
    double max;
};

double obtenerValorAleatorio(double min, double max) {
    return min + static_cast<double>(rand()) / RAND_MAX * (max - min);

}

double obtenerPrecioLuz(const string& pais, int hora) {
    ifstream archivo("precios_luz.txt");
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

/**
 * Función para calcular la mediana
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


        CURL* curl;
        CURLcode res;
        string readBuffer;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            // Construir la URL con la hora actual y la hora siguiente
            string apiUrl = "https://apidatos.ree.es/es/datos/mercados/precios-mercados-tiempo-real?start_date=";
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
                cout << "Datos recibidos:\n" << readBuffer << endl;
                PrecioEnergia resultado;
                parsearRespuesta(readBuffer, resultado);
                datosPrecios.push_back(resultado);

            }

            // Limpiar y cerrar la sesión de curl
            curl_easy_cleanup(curl);


            // Obtener la hora de los demas paises, hacer un vector e ir comparando
            for (const string& otroPais : {"Alemania", "Francia", "Italia"}) {
                double precio = obtenerPrecioLuz(otroPais, horaActual);

                PrecioEnergia resultado;
                resultado.pais = otroPais;
                resultado.fecha = formattedStartTime.str();
                resultado.precio = precio;

                datosPrecios.push_back(resultado);

            }

            for (const auto& precio : datosPrecios) {
                cout << "País: " << precio.pais << endl;
                cout << "Fecha: " << precio.fecha << endl;
                cout << "Precio: " << precio.precio << " €/MWh" << endl;
                cout << endl;
            }                        
        }

        curl_global_cleanup();

        //Hacer la mediana de los datos para obtener el umbral de precios "baratos" y "caros"
        double umbral = calcularMediana(datosPrecios);

        cout << "El umbral que determina si los precios son caros o baratos es: " << umbral << endl;


        // Esperar una hora antes de realizar la siguiente solicitud
        this_thread::sleep_for(chrono::hours(1));
    }

    return 0;
}
