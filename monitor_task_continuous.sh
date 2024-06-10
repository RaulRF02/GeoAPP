#!/bin/bash

# Nombre del reporte antes de la tarea
BEFORE_REPORT="before_task_report.html"
# Nombre del reporte después de la tarea
AFTER_REPORT="after_task_report.html"
# Comando para ejecutar la tarea
TASK_COMMAND="./cliente Alemania"
# Intervalo de tiempo entre mediciones (en segundos), ahora cada 10 minutos
INTERVAL=600
# Duración total del monitoreo (en segundos), en este caso, 1 hora
DURATION=3600

# Función para extraer la potencia promedio de un reporte HTML
extract_power() {
    local report=$1
    grep -oP '(?<=<li class="summary_list"> <b> CPU:  </b> ).+?(?=% usage)' $report | awk '{ sum += $1 } END { if (NR > 0) print sum / NR }'
}

# Función para monitorizar el consumo de energía durante un periodo de tiempo
monitor_energy() {
    local total_energy=0
    local count=0
    local start_time=$(date +%s)
    local end_time=$((start_time + DURATION))

    while [ $(date +%s) -lt $end_time ]; do
        report=$(mktemp)
        sudo powertop --time=1 --html=$report
        power=$(extract_power $report)
        if [[ ! -z $power ]]; then
            total_energy=$(echo "$total_energy + $power * $INTERVAL / 3600" | bc -l)
            count=$(($count + 1))
        fi
        rm -f $report
        sleep $INTERVAL
    done
    echo "scale=2; $total_energy" | bc
}

# Generar un reporte antes de la ejecución de la tarea
echo "Generando reporte antes de la tarea..."
sudo powertop --html=$BEFORE_REPORT

# Ejecutar la tarea en segundo plano
echo "Ejecutando la tarea: $TASK_COMMAND"
$TASK_COMMAND &

TASK_PID=$!
echo "PID de la tarea: $TASK_PID"

# Monitorizar el consumo de energía durante la ejecución de la tarea
echo "Monitorizando el consumo de energía durante la ejecución de la tarea..."
total_energy=$(monitor_energy)

# Generar un reporte después de la ejecución de la tarea
echo "Generando reporte después de la tarea..."
sudo powertop --html=$AFTER_REPORT

# Mostrar el resultado
echo "La energía total consumida por el sistema durante la ejecución de la tarea en Wh ha sido: $total_energy Wh"
