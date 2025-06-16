# ProyectSimu
# Documentación

Este proyecto representa un laberinto basado en una malla de pentágonos utilizando SFML. La clase principal `PentagonGrid` permite:

1. Cargar el mapa desde un archivo de texto.
2. Construir una lista de adyacencia de los pentágonos transitables.
3. Identificar y manipular el nodo de inicio y el nodo final.
4. Exportar la lista de adyacencia a un archivo `.txt`.

---

## Formato del archivo de mapa (`map.txt`)

Cada línea del archivo representa una fila de la cuadrícula. Los caracteres permitidos son:

- `1` - Pentágono transitable
- `0` - Pentágono bloqueado (muro)
- `a` - Nodo de inicio (jugador comienza aquí)
- `b` - Nodo destino (meta)

**Ejemplo de ****\`\`****:**

```
111111
10a011
101011
10b011
111111
```

---

## Importar mapa desde archivo

```cpp
std::vector<std::string> layout = PentagonGrid::loadLayoutFromFile("map.txt");
PentagonGrid grid(layout, 30.0f, window.getSize());
```

---

## Obtener lista de adyacencia

Una vez creado el objeto `PentagonGrid`, puedes obtener la lista de adyacencia usando:

```cpp
const std::vector<std::vector<int>>& adjList = grid.getAdjacencyList();
```

Cada nodo tiene un ID único, y la lista contiene los vecinos a los que puede moverse.

---

## Exportar lista de adyacencia

```cpp
grid.exportAdjacencyListToFile("adyacencia.txt");
```

Esto generará un archivo con la representación del grafo. Por ejemplo:

```
0: 1 6
1: 0 2 7
...
```

---

## Nodo del jugador (inicio) y nodo destino (final)

- `grid.getPlayerNodeId()` devuelve el ID del nodo donde está el jugador actualmente.
- `grid.getEndNodeId()` devuelve el ID del nodo de destino (`b`).


---

## Requisitos

- SFML 2.5.1 o superior
- Compilador compatible con C++17 o superior

---

Para más detalles revisa el archivo `main.cpp` o extiende `PentagonGrid` según tus necesidades.

