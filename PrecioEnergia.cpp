#include "PrecioEnergia.hpp"

PrecioEnergia::PrecioEnergia()
    : pais("defecto"), fecha("defecto"), precio(0.0){}


PrecioEnergia::PrecioEnergia(const std::string& pais, const std::string& fecha, double precio)
    : pais(pais), fecha(fecha), precio(precio) {}

