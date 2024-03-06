#include <iostream>
#include <random>
#include <vector>

// Función para simular el precio de la luz
double simularPrecioLuz(double precioBase, double volatilidad) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0, volatilidad);
    return precioBase + d(gen);
}

int main() {
    // Parámetros de simulación para cada país
    std::vector<double> preciosBase = {0.15, 0.18, 0.20, 0.22}; // Precio base en euros/MWh
    std::vector<double> volatilidades = {0.02, 0.03, 0.02, 0.03}; // Volatilidad en euros/MWh
    int simulaciones = 1000; // Número de simulaciones

    // Simulación de precios de la luz para cada país
    std::vector<std::vector<double>> preciosLuz;
    for (int i = 0; i < preciosBase.size(); ++i) {
        std::vector<double> preciosPorPais;
        for (int j = 0; j < simulaciones; ++j) {
            double precio = simularPrecioLuz(preciosBase[i], volatilidades[i]);
            preciosPorPais.push_back(precio);
        }
        preciosLuz.push_back(preciosPorPais);
    }

    // Imprimir algunos precios simulados para cada país
    for (int i = 0; i < preciosLuz.size(); ++i) {
        std::cout << "Precio de la luz simulado para el país " << i+1 << " (euros/MWh): " << std::endl;
        for (int j = 0; j < 10; ++j) {
            std::cout << preciosLuz[i][j] << std::endl;
        }
    }

    return 0;
}
