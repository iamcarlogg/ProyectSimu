Todos los algoritmos utilizados y cómo fueron implementados: 

1. Lectura de Archivos de Texto (Carga de Mapa) 

Algoritmo: Lectura secuencial de líneas de un archivo de texto. 
Implementación: 
En el método estático PentagonGrid::loadLayoutFromFile(const std::string& filename), se abre el archivo de texto, se lee línea por línea y se almacena cada línea no vacía en un vector de strings que representa el layout del mapa. 

 

2. Construcción de la Grilla y Celdas 

Algoritmo: Inicialización de una matriz de objetos y asignación de propiedades según el layout. 
Implementación: 
En el constructor de PentagonGrid, se recorre el vector de strings del layout y, para cada carácter, se crea una celda (PentagonCell) con propiedades como bloqueada, inicio, fin, volátil, etc. También se calcula la posición geométrica de cada pentágono y se asigna un identificador único a cada celda navegable. 
