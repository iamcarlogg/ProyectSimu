# ProyectSimu
# Documentación

Este proyecto representa un laberinto basado en una malla de pentágonos utilizando SFML. La clase principal `PentagonGrid` permite:

1. Cargar el mapa desde un archivo de texto.
2. Construir una lista de adyacencia de los pentágonos transitables.
3. Identificar y manipular el nodo de inicio y el nodo final.
4. Exportar la lista de adyacencia a un archivo `.txt`.
5. Realizar un autocompletado por medio de BFS

---

## Formato del archivo de mapa (`map.txt`)

Cada línea del archivo representa una fila de la cuadrícula. Los caracteres permitidos son:

- `1` - Pentágono transitable
- `0` - Pentágono bloqueado (muro)
- `v` - Pentágono volatil (transitable en turno impar e intransitable en turno par)
- `a` - Nodo de inicio (jugador comienza aquí)
- `b` - Nodo destino (meta)

**Ejemplo de ****\`\`****:**

```
111111
10a011
101v11
10b011
111111
```

---

## Importar mapa desde archivo
Antes de iniciar el juego, se pedira en consola ingresar la ruta del archivo de mapa en forma de ruta absoluta o ruta relativa.
Ejemplo:

```bash

C:\Users\usuarioRandom\Desktop\map.txt
||
.\map.txt
```
## Como Jugar

1) Para moverte necesitaras hacer click derecho en una celda libre.

2) Hay celdas que en turno par estan bloqueadas y en turno impar son transitables.

3) Cada 4 turnos podas romper un muro arbitrario.

4) Al momento de llegar a la meta, ganaras.

### Controles

1) click derecho: Control principal, todas las acciones se realizan por medio de este click.
2) Rueda del ratón: zoom in y zoom out






## Requisitos

- SFML 2.5.1 o superior
- Compilador compatible con C++17 o superior

---

Para más detalles revisa el archivo `main.cpp` o extiende `PentagonGrid` según tus necesidades.

