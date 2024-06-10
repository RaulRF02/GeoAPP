#!/bin/bash

# Nombre del reporte antes de la tarea
BEFORE_REPORT="before_task_report.html"
# Nombre del reporte después de la tarea
AFTER_REPORT="after_task_report.html"
# Comando para ejecutar la tarea
TASK_COMMAND="./cliente Alemania"

# Paso 1: Generar un reporte antes de la ejecución de la tarea
echo "Generando reporte antes de la tarea..."
sudo powertop --html=$BEFORE_REPORT

# Paso 2: Ejecutar la tarea
echo "Ejecutando la tarea: $TASK_COMMAND"
$TASK_COMMAND

# Paso 3: Generar un reporte después de la ejecución de la tarea
echo "Generando reporte después de la tarea..."
sudo powertop --html=$AFTER_REPORT

# Paso 4: Analizar los reportes y calcular la potencia promedio

# Función para extraer la potencia promedio de un reporte HTML
extract_power() {
    local report=$1
    grep -oP '(?<=<li class="summary_list"> <b> CPU:  </b> ).+?(?=% usage)' $report | awk '{ sum += $1 } END { if (NR > 0) print sum / NR }'
}

# Extraer la potencia promedio de los reportes antes y después
before_power=$(extract_power $BEFORE_REPORT)
after_power=$(extract_power $AFTER_REPORT)

# Calcular la potencia promedio durante la tarea
average_power=$(echo "($before_power + $after_power) / 2" | bc -l)
echo "La potecia before es de $BEFORE_REPORT "
echo "La potencia after es de $AFTER_REPORT "
echo "($before_power + $after_power) / 2 = $average_power"
# Mostrar el resultado
echo "La Potencia promedio consumida por el sistema durante la ejecución de la tarea en W ha sido: $average_power W"
