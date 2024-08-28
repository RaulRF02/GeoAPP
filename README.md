
# GeoAPP    

The objective of this project is to implement a C++ scheduler that evaluates hourly electricity prices at run time. Based on these values, the algorithm will geo-distribute the workload of an application or task.

## General Information

- **Author** Raúl Rodríguez Fernández

-  **Tutor:** Juan José Escobar Pérez
- **Department:** Computer Languages ​​and System

  


## Setup

### Requirements

Before you compile and run this project, make sure you have the following dependencies installed on your system:

- A C++ compiler compatible with C++11 (e.g., `g++`).
- The `libcurl` library.
- OpenMP.

### Compilation
To compile the project, use the included Makefile. This Makefile defines two main targets: `server` and `client`.
 
1. Clone the repository to your local machine
2. Run the `make`commandto compile both targets

  This command will generate two executables: `server` and `client`.

### Execution

To run the server, execute the server executable with `./server`
To run the client, execute the `client` executable followed by the country you want the client to simulate. Currently, it supports Spain, Germany, France, and Italy. Example: `./client España`

With these instructions, you should be able to compile, and execute the project without any issues.

## Funding

This work has been funded by:

  

- Spanish Ministerio de Ciencia, Innovación y Universidades under grant number PID2022-137461NB-C32.

- European Regional Development Fund (ERDF).

- Universidad de Granada, under grant number PPJIA2023-025.



<div  style="text-align: right">

<img  src="https://raw.githubusercontent.com/efficomp/Hpmoon/main/docs/logos/miciu.jpg"  height="60">

<img  src="https://raw.githubusercontent.com/efficomp/Hpmoon/main/docs/logos/erdf.png"  height="60">

<img  src="logos/Imagen1.png"  height="60">

</div>
