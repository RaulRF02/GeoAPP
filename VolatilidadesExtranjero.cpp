#include "VolatilidadesExtranjero.hpp"
#include <random>
#include <algorithm>
#include <iostream>

VolatilidadesExtranjero::VolatilidadesExtranjero() {
    // Inicializa las volatilidades para los países extranjeros

    volatilidades.push_back({"Alemania", 60.7, 14.5, 52, 15, 90, -40, 40});
    volatilidades.push_back({"Italia", 87, 7.4, 37.8, 70, 105, -30, 30});
    volatilidades.push_back({"Francia", 58.5, 10.6, 50.4, 40, 75, -40, 40});
}

const std::vector<VolatilidadesExtranjero::VolatilidadPais>& VolatilidadesExtranjero::obtenerVolatilidades() const {
    return volatilidades;
}

double acotarValor(double valor, double min, double max) {
    return std::min(std::max(valor, min), max);
}

double VolatilidadesExtranjero::simularPrecioDia(std::string pais) {
    // Busca el país en el vector de volatilidades
    auto it = find_if(volatilidades.begin(), volatilidades.end(),
                           [&pais](const VolatilidadPais& v) { return v.pais == pais; });

    if (it != volatilidades.end()) {
        // Encontró el país, simula el precio del día
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> distribucion(0, it->desviacionDia); // Media 0, volatilidad dada
        double variacion = distribucion(gen);
        // Acotar el valor dentro del rango [mesMin, mesMax] manualmente
        double precioSimulado = acotarValor(it->media + variacion, it->mes.min, it->mes.max);

//////
        std::cout << "SimularPrecioDia variacion: " << variacion << std::endl;
        std::cout << "SimularPrecioDia: " << precioSimulado << std::endl;
/////////
        return precioSimulado;
    } else {
        // No se encontró el país
        throw std::runtime_error("País no disponible.");
    }
}

double VolatilidadesExtranjero::simularPrecioHora(std::string pais) {
    auto it = std::find_if(volatilidades.begin(), volatilidades.end(),
                           [&pais](const VolatilidadPais& v) { return v.pais == pais; });

    if (it != volatilidades.end()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> distribucion(0, it->desviacionHora);
        double variacion = distribucion(gen);

        // Acotar el valor dentro del rango [diaMin, diaMax] manualmente
        double precioSimulado = acotarValor(variacion, it->dia.min, it->dia.max);

        std::cout << "SimularPrecioHora variacion: " << variacion << std::endl;

        return precioSimulado;
    } else {
        throw std::runtime_error("País no disponible.");
    }
}

    double VolatilidadesExtranjero::obtenerDatosVarianza(std::string pais){
     auto it = std::find_if(volatilidades.begin(), volatilidades.end(),
                           [&pais](const VolatilidadPais& v) { return v.pais == pais; });
                           
     if (it != volatilidades.end()) {
        double precioDia = simularPrecioDia(it->pais);
        precioDia += simularPrecioHora(it->pais);
        return precioDia;
    } else {
        throw std::runtime_error("País no disponible.");
    }
   
    }

