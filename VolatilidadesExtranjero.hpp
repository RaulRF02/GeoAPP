#pragma once

#include <string>
#include <vector>
#include "EnergiaAPI.hpp"

class VolatilidadesExtranjero {
public:
    struct VolatilidadPais {
        std::string pais;
        double media;
        double desviacionHora;
        double desviacionDia;
        EnergiaAPI::RangoPrecio mes;
        EnergiaAPI::RangoPrecio dia;

        VolatilidadPais(std::string p, double m, double dd, double dh, double mesMin, double mesMax, double diaMin, double diaMax)
        : pais(p), media(m), desviacionHora(dh), desviacionDia(dd), mes{mesMin, mesMax}, dia{diaMin, diaMax} {}
    };

    VolatilidadesExtranjero();

    // Obtener el vector de volatilidades extranjero
    const std::vector<VolatilidadPais>& obtenerVolatilidades() const;

    // Función para simular el precio del día para un país
    double simularPrecioDia(std::string pais);

     // Función para simular el precio por hora para un país
    double simularPrecioHora(std::string pais);

    double obtenerDatosVarianza(std::string pais);

private:
    // Vector para almacenar volatilidades extranjero
    std::vector<VolatilidadPais> volatilidades;
};
