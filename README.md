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

## Estructura del Proyecto
El proyecto está estructurado en directorios separados para el servidor y los clientes, así como para el código fuente y las cabeceras. La estructura de directorios es la siguiente:
- `src/`: Contiene el código fuente compartido.
- `include/`: Contiene las cabeceras compartidas.
- `server/`: Contiene el código específico del servidor.
- `client/`: Contiene el código específico del cliente.

## Compilación y Ejecución
El proyecto cuenta con un Makefile para facilitar la compilación. Para compilar el proyecto, simplemente ejecuta el comando `make` en la raíz del proyecto. Esto generará los ejecutables `servidor` y `cliente`.

Para ejecutar el servidor, utiliza el comando `./servidor`. Los clientes se ejecutan por terminales separadas con `./cliente` seguido del país, por ejemplo, `./cliente Alemania`. Actualmente, el proyecto soporta Italia, Francia y Alemania.

## Funding
This work has been funded by:

- Spanish Ministerio de Ciencia, Innovación y Universidades under grant number PID2022-137461NB-C32.
- European Regional Development Fund (ERDF).
- Universidad de Granada, under grant number PPJIA2023-025.
- 
<div style="text-align: right">
  <img src="https://raw.githubusercontent.com/efficomp/Hpmoon/main/docs/logos/miciu.jpg" height="60">
  <img src="https://raw.githubusercontent.com/efficomp/Hpmoon/main/docs/logos/erdf.png" height="60">
  <img src="logos/Imagen1.png" height="60">
</div>
