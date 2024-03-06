cmake_minimum_required(VERSION 3.0)
project(TFG)

# Agrega tus archivos fuente al ejecutable
add_executable(scheduler main.cpp PrecioEnergia.cpp EnergiaAPI.cpp VolatilidadesExtranjero.cpp)

# Configura libcurl (deberías ajustar esto según tu sistema)
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(scheduler ${CURL_LIBRARY})