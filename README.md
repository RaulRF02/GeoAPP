# Algoritmo de Distribución Geodistribuido para Ahorro de Energía

## Información General
- **Título del TFG:** Algoritmo de Distribución Geodistribuido para Ahorro de Energía
- **Departamento:** Lenguajes y Sistemas Informáticos
- **Tutor:** Juan José Escobar Pérez

## Descripción del Proyecto
El objetivo de este proyecto es implementar un scheduler en C++ que, en tiempo de ejecución, evalúe el precio de la electricidad cada hora. Según este valor, el algoritmo decidirá distribuir la carga de trabajo de una aplicación de alto rendimiento a computadoras ubicadas en otros países. Dado que no se tienen máquinas reales en esos países, se emplearán diferentes máquinas locales y se simularán las tarifas eléctricas de esos lugares. Para el caso de España, se utilizará una API pública que proporciona datos reales.

Este trabajo de investigación busca demostrar que se puede lograr un ahorro económico significativo distribuyendo el trabajo a máquinas ubicadas en regiones con tarifas eléctricas más favorables.

## Definiciones
- **Lenguajes/Conocimientos Utilizados:**
  - C++
  - OpenMP
  - OpenMPI
  - Librería "socket.h"

## Objetivos
- Crear un scheduler eficiente en C++.
- Evaluar dinámicamente el precio de la electricidad.
- Implementar la geodistribución de carga de trabajo en función de la tarifa eléctrica.
