#!/bin/bash

# Directorio para guardar los logs
LOG_DIR="./energy_logs"
mkdir -p $LOG_DIR

# Función para medir el consumo de energía
measure_energy() {
    local prefix=$1
    sudo powertop --time=10 --html=${LOG_DIR}/${prefix}_powertop_report.html
}

# Inicio de la medición
measure_energy "before_task"

# Ejecutar el cliente (asegúrate de que el cliente realice la tarea de multiplicación)
./cliente "Alemania"

# Fin de la medición
measure_energy "after_task"

echo "Medición completada. Verifica los archivos en ${LOG_DIR} para los detalles."
