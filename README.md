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

## Servidor
Este programa es un servidor que gestiona la comunicación entre varios clientes para determinar el país con el precio de la energía más barato. 
### Definiciones y estructuras
Se definen constantes como el puerto (PORT), el número máximo de clientes (MAX_CLIENTS), y el tamaño del buffer (BUFFER_TAM). También se define una estructura PrecioEnergia para almacenar el nombre del país y su precio de energía. Se utiliza un mutex (mtx) para sincronizar el acceso a recursos compartidos entre hilos, y se declaran variables globales para almacenar los sockets de los clientes, los precios de energía por país, y otros datos necesarios para el funcionamiento del programa.
### Funciones auxiliares
- stopTask(int clientSocket): Envía un mensaje de detención al cliente especificado y espera una confirmación.
- sendTask(int clientSocket): Envía una nueva tarea al cliente especificado, en este caso para simplificar el problema y comprobar que el estado se guarda correctamente al interrumpir la hebra, la tarea trata de contar una cantidad de minutos segundo a segundo.
- calcularMediana(vector<PrecioEnergia> precios): Calcula la mediana de un vector de precios de energía.
- determinarPaisMasBarato(const vector<PrecioEnergia>& paises): Determina el país con el precio de energía más bajo.
- recibirPrecios(vector<int>& client_sockets): Recibe los precios de energía de los clientes y determina el país más barato.
### Función principal (main)
Inicializa el servidor, crea un socket, y configura la dirección y el puerto. Luego, acepta conexiones de hasta MAX_CLIENTS clientes. Utiliza OpenMP para crear dos hilos: uno para recibir los precios de los clientes y otro para aceptar nuevas conexiones de clientes. La función recibirPrecios se encarga de solicitar los precios de energía a cada cliente, procesarlos, y determinar el país con el precio más bajo. Si se detecta un cambio en el país más barato, se envía una nueva tarea al cliente correspondiente.
El servidor decide a qué país llevar la ejecución siguiendo estos pasos:
- Recopilación de Precios: Recibe los precios de energía de los clientes conectados.
- Cálculo de la Mediana: Calcula la mediana de los precios recibidos, ya que esto indicará el umbral en el que el precio de los paises es caro o barato, por lo que si el precio del pais actual está por debajo del umbral se considera que tiene un precio barato y no es necesario interrumpirlo para llevar la ejecución a otro cliente.
- Envío de Tarea al Cliente: Envía una tarea al país más barato.
- Umbrales y Actualización: Si se detecta un nuevo país más barato, actualiza la tarea enviada al país anterior.

## Cliente
Este código del cliente es parte de un sistema que permite a los clientes enviar información sobre el precio de la energía de su país al servidor, y también puede recibir tareas del servidor, como contar minutos o detener una tarea.
### Definiciones y estructuras
Se define el puerto (PORT), el tamaño del buffer (BUFFER_TAM), y se utiliza una variable global stopTask para controlar la ejecución de tareas. También se define una estructura PrecioEnergia para almacenar el nombre del país y su precio de energía.
### Funciones auxiliares
- saveState(int state): Guarda el estado de una tarea en un archivo temporal y envía un mensaje de confirmación al servidor.
- obtenerHorasFormateadas(std::string &formattedStartTime, std::string &formattedEndTime): Obtiene las horas formateadas para el inicio y fin de una tarea.
- countMinutes(int minutes): Cuenta los minutos transcurridos desde un punto de partida hasta un límite, deteniéndose si se recibe una señal de detención.
- cargarVolatilidadPais(const std::string &nombrePais): Carga la volatilidad de un país específico desde un archivo JSON.
- manejarProblemas(int sock): Maneja problemas de conexión o mensajes del servidor.
### Función principal (main): 
Inicializa el cliente, crea un socket, y configura la dirección y el puerto. Luego, intenta conectarse al servidor. Una vez conectado, entra en un bucle donde lee mensajes del servidor y responde según el mensaje recibido. Utiliza OpenMP para manejar múltiples tareas simultáneamente, como enviar precios de energía y contar minutos. <br>
La simulación del precio de energía para países extranjeros se realiza mediante una combinación de variaciones aleatorias basadas en la volatilidad diaria y horaria de cada país. Primero, simularPrecioDia genera una variación aleatoria diaria sumándola a la media diaria del país, y luego acota este valor dentro de un rango diario específico. De manera similar, simularPrecioHora genera una variación aleatoria horaria sumándola a la media horaria del país, acotándola dentro de un rango horario específico. Finalmente, obtenerDatosVarianza suma el precio simulado diario al precio simulado por hora para obtener un precio estimado de energía para un día completo, considerando tanto la variabilidad diaria como la variabilidad horaria. Este precio simulado se utiliza para representar el precio de energía de un país extranjero en la simulación.<br>
En el caso de españa el precio se obtiene mediante una llamada a una API pública.
