#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <ctime>
#include <cstdlib>
#include <random>

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

int main() {
    std::srand(std::time(0));

    double precioAlemania = obtenerPrecioLuz("Alemania", 8);
    double precioFrancia = obtenerPrecioLuz("Francia", 14);

    std::cout << "Precio de la luz en Alemania a las 8:00: " << precioAlemania << std::endl;
    std::cout << "Precio de la luz en Francia a las 14:00: " << precioFrancia << std::endl;

    return 0;
}
