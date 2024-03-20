#include "VolatilidadesExtranjero.hpp"
#include <random>
#include <algorithm>
#include <iostream>
/*
VolatilidadesExtranjero() {
    // Inicializa las volatilidades para los países extranjeros

    volatilidades.push_back({"Alemania", 60.7, 14.5, 52, 15, 90, -40, 40});
    volatilidades.push_back({"Italia", 87, 7.4, 37.8, 70, 105, -30, 30});
    volatilidades.push_back({"Francia", 58.5, 10.6, 50.4, 40, 75, -40, 40});
}

*/

double acotarValor(double valor, double min, double max) {
    return std::min(std::max(valor, min), max);
}

double simularPrecioDia(VolatilidadPais pais) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> distribucion(0, pais.desviacionDia); // Media 0, volatilidad dada
        double variacion = distribucion(gen);
        // Acotar el valor dentro del rango [mesMin, mesMax] manualmente
        double precioSimulado = acotarValor(pais.media + variacion, pais.mes.min, pais.mes.max);

//////
        std::cout << "SimularPrecioDia variacion: " << variacion << std::endl;
        std::cout << "SimularPrecioDia: " << precioSimulado << std::endl;
/////////
        return precioSimulado;
    
}

double simularPrecioHora(VolatilidadPais pais) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> distribucion(0, pais.desviacionHora);
        double variacion = distribucion(gen);

        // Acotar el valor dentro del rango [diaMin, diaMax] manualmente
        double precioSimulado = acotarValor(variacion, pais.dia.min, pais.dia.max);

        std::cout << "SimularPrecioHora variacion: " << variacion << std::endl;

        return precioSimulado;
    
}

double obtenerDatosVarianza(VolatilidadPais pais){
                          
        double precioDia = simularPrecioDia(pais);
        precioDia += simularPrecioHora(pais);
        return precioDia;

}

