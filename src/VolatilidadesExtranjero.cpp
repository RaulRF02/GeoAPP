#include "../include/VolatilidadesExtranjero.hpp"
#include <random>
#include <algorithm>
#include <iostream>

double acotarValor(double valor, double min, double max) {
    return std::min(std::max(valor, min), max);
}

double simularPrecioDia(VolatilidadPais pais) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> distribucion(0, pais.desviacionDia); 
        double variacion = distribucion(gen);

        double precioSimulado = acotarValor(pais.media + variacion, pais.mes.min, pais.mes.max);
        return precioSimulado;
    
}

double simularPrecioHora(VolatilidadPais pais) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> distribucion(0, pais.desviacionHora);
        double variacion = distribucion(gen);

        double precioSimulado = acotarValor(variacion, pais.dia.min, pais.dia.max);
        return precioSimulado;
    
}

double obtenerDatosVarianza(VolatilidadPais pais){
                          
        double precioDia = simularPrecioDia(pais);
        precioDia += simularPrecioHora(pais);
        return precioDia;

}

