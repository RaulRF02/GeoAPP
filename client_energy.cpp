#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>

using namespace std;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    // Abre una tubería hacia el comando
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    // Lee la salida del comando
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main() {
    std::string command = "sudo turbostat --quiet --Summary --show PkgWatt,RAMWatt --interval 1"; // Comando que deseas ejecutar
    cout << command << endl;
    try {
        std::string output = exec(command.c_str());
        std::cout << "Output:\n" << output << std::endl;

        // Guardar la salida en un archivo
        std::ofstream outfile("energy_measurements.txt");
        if (outfile.is_open()) {
            outfile << output;
            outfile.close();
        } else {
            std::cerr << "Error: No se pudo abrir el archivo para escritura." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
