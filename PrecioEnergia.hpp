#include <string>

struct PrecioEnergia {
    std::string pais;
    std::string fecha;
    double precio;

    PrecioEnergia();
    PrecioEnergia(const std::string& pais, const std::string& fecha, double precio);

};