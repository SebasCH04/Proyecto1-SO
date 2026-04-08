# Proyecto #1 de Principios de Sistemas Operativos

## Introducción

Este proyecto implementa una versión multihilos del programa copy en lenguaje C, capaz de copiar recursivamente el contenido de un directorio completo hacia un destino. Para ello se utilizó un pool de hilos estático combinado con una cola de tareas compartida: el hilo principal recorre el directorio fuente y encola cada archivo como una tarea, mientras que un conjunto fijo de hilos trabajadores toma estas tareas y realiza las copias en paralelo.

La sincronización entre hilos se maneja mediante mutex y variables de condición de la biblioteca POSIX (pthreads), garantizando acceso seguro a los recursos compartidos. Adicionalmente, el programa genera un archivo de bitácora en formato CSV con los detalles de cada copia realizada.

## Descripción del problema

El objetivo de este proyecto es comparar el rendimiento de realizar una tarea colaborativa utilizando múltiples hilos. Para ello se desarrolló una
versión multihilos del programa copy que permite copiar el contenido de un
directorio completo.

Se realizaron múltiples pruebas para determinar la cantidad óptima de hilos para ejecutar este tipo de tarea sobre un único directorio muy
grande.

## Definición de estructuras de datos

- Task: Es una estructura que almacena la ubicación del source del archivo, además de su fuente.
- TaskQueue: Representa la cola compartida de tareas listas para ser trabajadas. Guarda un Task por cada archivo en el directorio source. Utiliza un pthread_mutex_t global para controlar acceso a recursos y pthread_cond_t para notificar a los hilos dormidos cuando pueden leer la cola.
- ThreadPool: Agrupa el arreglo de pthread_t, la cantidad de hilos activos, un puntero a la TaskQueue compartida y una lista a la información de cada worker.
- WorkerInfo: Es la estructura que se envía a cada uno de los workers, que recopila un puntero a la cola de tareas, una lista dinámica de CompletedTasks, el nombre asignado al worker, y dos contadores para la lista de CompletedTasks.
- CompletedTask: Es una estructura que guarda los resultados de una copia realizada: la duración, y el nombre del archivo.

## Descripción detallada y explicación de los componentes principales

### Modulo principal

Es el encargado de orquestrar la entrada, la creación de hilos, y el proceso de logging. Primeramente se encarga de validar que se ingresen solo dos parámetros, y luego revisa que sean rutas validas para Linux.

Posteriormente le pasa la tarea al modulo de thread pool para que realize su gestión, y luego al de manejo de directorios para que revise el directorio fuente y llene la TaskQueue. Luego se queda esperando a que los hilos terminen sus tareas

Finalmente una vez los hilos concluyen le pasa los resultados de cada subhilo al módulo de logging para hacer un reporte en CSV de los resultados obtenidos. 


### Módulo de manejo de directorios

Tiene 3 funciones principales:

- Revisar que las rutas solicitadas al invocar el comando sean validas en Linux. 
- Recorrer todo el directorio fuente por medio de Depth First Search, para así garantizar un acceso ordenado. Además el DFS permite crear los directorios desde el hilo principal de modo que se garantize que cada subhilo al copiar un archivo encuentre la ruta destino ya construida.
![subdirectories](img/directories.png)
- Llenar la cola de tareas (TaskQueue), que será accesada por el Thread Pool. Al insertar una Task en la cola avisa con pthread_cond_signal, a un hilo que esté esperando para que así pueda tomarla y ejecutarla.
![alt text](img/TaskQueue.png)


### Módulo de Thread Pool

Tiene 3 funciones principales:

- Una función que inicializa el thread pool con un número fijo de hilos, todos arrancan de inmediato y quedan bloqueados en dequeue_wait esperando tareas.
- Una función worker que corre cada hilo en un loop, toma una tarea de la cola y llama a copy_file para copiarla al destino. Sale cuando la cola está vacía y marcada como terminada.
- Una función que espera a que todos los hilos terminen con pthread_join, además de obtener el WorkerInfo de cada uno y retornarlo.

Los directorios destino los crea el hilo principal antes de encolar los archivos que contienen, para evitar que un worker intente copiar a una carpeta que todavía no existe y de error.

### Módulo de Logging

Este módulo es más simple, tan solo recibe una lista de WorkerInfo desde el main, y transforma dicha información en un CSV.

## Mecanismo de creación y comunicación de hilos

El proceso de creación de hilos se hizo por medio de una función llamada thread_pool_init. La cuál acepta como parámetros un struct ThreadPool, un entero con la cantidad de hilos a crear y una instancia de TaskQueue ya preparada para garantizar accesos integros.

En thread_pool_init primero se asigna espacio en heap para los threads y los WorkerInfo para cada subhilo. Esta estrcuctura es de gran importancia debido que es la que permite la comunicación entre cada subhilo y el hilo principal.

Posteriormente se inicializa cada uno de los hilos con su WorkerInfo y se asigna espacio para la lista de CompletedTasks que cada uno tendrá. Se decicidió hacer dicha lista dinámicamente con reallocs para así simplificar el diseño (evitar implicar usar valores alambrados o listas enlazadas). 

![alt text](img/TaskQueueDesarrollado.png)


Ahora bien, una vez el Worker inicia intentará leer de la cola de tareas y se bloquerá hasta que el hilo principal le haga un signal (cuando se adjunta una tarea a la cola), o bien cuando se termine el recorrido en el directorio fuente activará el valor done del TaskQueue para avisar a todos los hilos bloqueados que pueden finalizar su ejecución. 

Durante todo el proceso los subhilos crearan registros en formato de CompletedTask con las estadísticas de cada archivo que copien y lo adjuntarán a su lista en WorkerInfo. Así que cuando el hilo principal les haga join también recuperará el contenido de cada uno de los WorkerInfo.

![alt text](img/logging.png)

## Pruebas de Rendimiento

### Entorno de prueba

Las pruebas se realizaron sobre un directorio generado con 200 archivos distribuidos en 4 niveles de subdirectorios, con un tamaño total de 70MB en disco. Los archivos tienen tamaños que varían entre 64KB y 1MB. Se midió el tiempo total de ejecución usando clock_gettime(CLOCK_MONOTONIC) desde que inicia el recorrido del directorio hasta que todos los hilos terminan de copiar.

Cada prueba se corrió con una cantidad diferente de hilos en el pool: 1, 2, 4, 8 y 16.

### Resultados

| Hilos | Tiempo (s) | Speedup vs 1 hilo |
|------:|----------:|------------------:|
|     1 |   0.050773 |              1.00x |
|     2 |   0.022474 |              2.26x |
|     4 |   0.011462 |              4.43x |
|     8 |   0.010046 |              5.05x |
|    16 |   0.007537 |              6.74x |

A continuación, se muestran evidencias de las pruebas realizadas: 

Resultados del benchmark:  
![alt text](img/Benchmark.png)

Archivos copiados:  
![alt text](img/Archivos.png)

Logfile de las pruebas:  
![alt text](img/LogFile.png)

### Análisis

Los resultados muestran una mejora de rendimiento clara al aumentar la cantidad de hilos, aunque con retornos decrecientes a partir de los 4 hilos:

- De 1 a 2 hilos: la reducción del tiempo es de más del 50%, lo que refleja un uso eficiente del paralelismo.
- De 2 a 4 hilos: el tiempo se sigue reduciendo a la mitad aproximadamente, manteniéndose un speedup casi lineal.
- De 4 a 8 hilos: la mejora es menor (12%), lo que indica que el cuello de botella empieza a trasladarse del CPU al I/O del disco.
- De 8 a 16 hilos: continúa una mejora moderada (25%), pero el incremento en número de hilos es del doble, lo que evidencia una saturación del subsistema de disco.

## Conclusiones

En cuanto al rendimiento, los resultados confirman que el uso de múltiples hilos reduce significativamente el tiempo de copia frente a la ejecución secuencial. Sin embargo, esta mejora no es ilimitada: a partir de 4 hilos el speedup empieza a estabilizarse, ya que el disco se convierte en el cuello de botella y los hilos adicionales compiten por el mismo recurso en lugar de trabajar verdaderamente en paralelo. Para el entorno de prueba utilizado, 4 hilos resultaron ser el punto óptimo, ofreciendo un speedup de 4.4x sin generar overhead excesivo de sincronización.

En cuanto al diseño, el uso de un pool de hilos estático combinado con una cola de tareas compartida demostró ser una arquitectura sólida y sencilla de razonar. Crear todos los hilos al inicio y reutilizarlos para múltiples archivos evita el costo de creación y destrucción de hilos en cada tarea. El uso de pthread_mutex_t y pthread_cond_t garantizó un acceso seguro a la cola sin condiciones de carrera, y la decisión de que el hilo principal cree los directorios destino antes de encolar los archivos eliminó una posible fuente de errores de sincronización entre workers.

## Bibliografía
- https://www.geeksforgeeks.org/c/fprintf-in-c/
- https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_cond_wait.html
- https://www.geeksforgeeks.org/c/snprintf-c-library/
- https://www.geeksforgeeks.org/c/snprintf-c-library/
- https://stackoverflow.com/questions/65104962/how-can-i-implement-a-basic-queue-in-c
- https://www.designgurus.io/answers/detail/how-to-recursively-list-directories-in-c-on-linux
- https://www.w3schools.com/c/c_error_handling.php
- https://www.w3schools.com/c/ref_string_strlen.php
- https://www.w3schools.com/c/ref_ctype_isalnum.php
- https://www.geeksforgeeks.org/cpp/command-line-arguments-in-c-cpp/
- https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-clock-gettime-retrieve-time-specified-clock
