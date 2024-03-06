#pragma once

/**
 * @file PrecioEnergia.hpp
 * @brief Definición de la estructura PrecioEnergia.
 */
#include <string>

/**
 * @struct PrecioEnergia
 * @brief Estructura que representa información sobre el precio de la energía en cada país.
 */
struct PrecioEnergia {
    std::string pais;   /**< País asociado al precio de la energía. */
    std::string fecha;  /**< Fecha correspondiente al precio de la energía. */
    double precio;      /**< Precio de la energía. */

    /**
     * @brief Constructor por defecto de PrecioEnergia.
     * @details Inicializa las cadenas de texto vacías y el precio a 0.0.
     */
    PrecioEnergia();

    /**
     * @brief Constructor de PrecioEnergia con parámetros.
     * @param[in] pais País asociado al precio de la energía.
     * @param[in] fecha Fecha correspondiente al precio de la energía.
     * @param[in] precio Precio de la energía.
     */
    PrecioEnergia(const std::string& pais, const std::string& fecha, double precio);

};