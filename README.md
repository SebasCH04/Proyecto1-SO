# Proyecto #1 de Principios de Sistemas Operativos

## Introducción


## Descripción del problema

El objetivo de este proyecto es comparar el rendimiento de realizar una tarea colaborativa utilizando múltiples hilos. Para ello se desarrolló una
versión multihilos del programa copy que permite copiar el contenido de un
directorio completo.

Se realizaron múltiples pruebas para determinar la cantidad óptima de hilos para ejecutar este tipo de tarea sobre un único directorio muy
grande.

## Definición de estructuras de datos

- Task: Es una estructura que almacena la ubicación del source del archivo, además de su fuente.
- TaskQueue: Representa la cola compartida de tareas listas para ser trabajadas. Guarda un Task por cada archivo en el directorio source. Utiliza un pthread_mutex_t global para controlar acceso a recursos y pthread_cond_t para notificar a los hilos dormidos cuando pueden leer la cola.
- CompletedTask: Es una estructura que guarda los resultados de una copia realizada: El subhilo encargado, la duración, y el nombre del archivo.

## Descripción detallada y explicación de los componentes principales

### Modulo principal

Es el encargado de orquestrar la entrada, la creación de hilos, y el proceso de logging. Primeramente se encarga de validar que se ingresen solo dos parámetros, y luego revisa que sean rutas validas para Linux.

Posteriormente le pasa la tarea al modulo de thread pool para que realize su tarea, y luego al de manejo de directorios para que revise el directorio fuente y llene la TaskQueue. Luego se queda esperando a que los hilos terminen sus tareas

Finalmente una vez los hilos concluyen le pasa los resultados de cada subhilo al módulo de logging para hacer un reporte en CSV de los resultados obtenidos. 


### Módulo de manejo de directorios

Tiene 3 funciones principales:

- Revisar que las rutas solicitadas al invocar el comando sean validas en Linux. 
- Recorrer todo el directorio fuente por medio de Depth First Search, para así garantizar un acceso ordenado.
![subdirectories](img/directories.png)
- Llenar la cola de tareas (TaskQueue), que será accesada por el Thread Pool. Al insertar una Task en la cola avisa con pthread_cond_signal, a un hilo que esté esperando para que así pueda tomarla y ejecutarla.
![alt text](img/TaskQueue.png)


### Módulo de Thread Pool

Este modulo consiste de dos tareas:
- Una función que inicializa el thread pool con un número fijo de hilos.
- Una función que ejecutará cada hilo que tiene como objetivo acceder a la cola y dormirse hasta que se le notifique que puede leer. Una vez hecho esto, se encargará de hacer la copia del archivo o directorio en el directorio destino. En el proceso recolectas la siguiente información: nombre del archivo, ID del hilo, tiempo de ejecución.

### Módulo de Logging
...