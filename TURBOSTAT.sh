#!/bin/bash

# Verifica que turbostat esté instalado
if ! command -v turbostat &> /dev/null; then
    echo "turbostat no está instalado. Por favor, instálalo e intenta de nuevo."
    exit 1
fi

# Verifica que se esté ejecutando como superusuario
if [[ $EUID -ne 0 ]]; then
    echo "Este script debe ejecutarse como root."
    exit 1
fi

# Inicializa variables
cpu_energy=0
ram_energy=0
counter=0

# Función para manejar la interrupción y calcular el consumo de energía
cleanup() {
    echo "Interrupción recibida, calculando el consumo de energía..."
    if [[ $counter -gt 0 ]]; then
        total_energy_wh=$(echo "scale=6; ($cpu_energy + $ram_energy) / $counter" | bc)
        total_energy_mw=$(echo "scale=6; $total_energy_wh / 1000" | bc)
        echo "Energía total consumida por la CPU y la RAM: $total_energy_mw MW"
    else
        echo "No se han recogido suficientes datos para calcular el consumo de energía."
    fi
    exit 0
}

# Captura la señal de interrupción (Ctrl+C)
trap cleanup SIGINT

# Ejecuta turbostat y procesa la salida
turbostat --interval 1 --Summary 2>/dev/null | while read -r line; do
    # Imprime la línea para depuración
    echo "DEBUG: $line"

    # Salta la línea de encabezado
    if [[ $line == *"PkgWatt"* ]]; then
        continue
    fi

    # Extrae las columnas de energía de la CPU y RAM
    cpu_value=$(echo $line | awk '{print $12}')
    ram_value=$(echo $line | awk '{print $13}')

    # Imprime los valores extraídos para depuración
    echo "DEBUG: CPU Value: $cpu_value, RAM Value: $ram_value"

    # Verifica si los valores son numéricos y razonables
    if [[ $cpu_value =~ ^[0-9.]+$ ]] && [[ $ram_value =~ ^[0-9.]+$ ]] && (( $(echo "$cpu_value < 100" | bc -l) )) && (( $(echo "$ram_value < 100" | bc -l) )); then
        cpu_energy=$(echo "$cpu_energy + $cpu_value" | bc)
        ram_energy=$(echo "$ram_energy + $ram_value" | bc)
        counter=$((counter + 1))
        
        # Muestra el consumo actual de energía de la CPU y la RAM
        echo "Consumo actual - CPU: $cpu_value W, RAM: $ram_value W"
    fi
done
