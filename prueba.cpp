#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <string>

// Estructura para almacenar el uso de CPU
struct CPUUsage {
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;
};

// Función para obtener el uso de CPU desde /proc/stat
CPUUsage getCPUUsage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);
    std::istringstream ss(line);

    std::string cpu;
    CPUUsage usage;
    ss >> cpu >> usage.totalUser >> usage.totalUserLow >> usage.totalSys >> usage.totalIdle;
    usage.total = usage.totalUser + usage.totalUserLow + usage.totalSys + usage.totalIdle;
    return usage;
}

// Función para calcular el porcentaje de uso del CPU
double calculateCPUUsage(CPUUsage prev, CPUUsage curr) {
    unsigned long long totalDiff = curr.total - prev.total;
    unsigned long long idleDiff = curr.totalIdle - prev.totalIdle;
    return (100.0 * (totalDiff - idleDiff)) / totalDiff;
}

int main() {
    // Definir el TDP del procesador (en vatios) y el costo del MWh (en dólares)
    const double TDP = 65.0; // Ejemplo: 65 vatios
    const double costPerMWh = 100.0; // Ejemplo: 100 dólares por MWh

    // Obtener la medición inicial
    CPUUsage prevUsage = getCPUUsage();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Esperar un segundo
    CPUUsage currUsage = getCPUUsage();

    // Calcular el uso del CPU
    double cpuUsage = calculateCPUUsage(prevUsage, currUsage);
    std::cout << "CPU Usage: " << cpuUsage << "%" << std::endl;

    // Calcular el consumo de energía
    double duration = 1.0; // Duración en segundos (1 segundo en este ejemplo)
    double energyConsumption = (cpuUsage / 100.0) * TDP * duration / 3600.0; // Consumo en vatios-hora
    std::cout << "Energy Consumption: " << energyConsumption << " Wh" << std::endl;

    // Calcular el costo de la energía
    double cost = (energyConsumption / 1000.0) * costPerMWh; // Convertir Wh a kWh
    std::cout << "Cost of Energy: $" << cost << std::endl;

    return 0;
}
