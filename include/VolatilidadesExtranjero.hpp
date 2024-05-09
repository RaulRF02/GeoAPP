#pragma once

#include <string>
#include <vector>
#include "EnergiaAPI.hpp"


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



    // Función para simular el precio del día para un país
    double simularPrecioDia(VolatilidadPais pais);

     // Función para simular el precio por hora para un país
    double simularPrecioHora(VolatilidadPais pais);

    double obtenerDatosVarianza(VolatilidadPais pais);

