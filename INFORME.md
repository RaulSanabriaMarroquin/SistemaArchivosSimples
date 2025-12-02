# Informe: Sistema de Archivos Simple

## Descripción General

Este documento describe la implementación de un sistema de archivos simple en C que simula las operaciones básicas de un sistema de archivos real, incluyendo la creación, escritura, lectura, eliminación y listado de archivos.

## Estructuras de Datos Utilizadas

1. Estructura `FileEntry`

Representa un archivo individual en el sistema:

```c
typedef struct {
    char filename[MAX_FILENAME];    // Nombre del archivo (máx 256 caracteres)
    size_t size;                     // Tamaño del archivo en bytes
    size_t num_blocks;               // Número de bloques ocupados
    size_t blocks[MAX_BLOCKS];       // Índices de los bloques ocupados
    bool in_use;                     // Indica si la entrada está en uso
} FileEntry;
```

**Decisión de diseño:** Esta estructura almacena los metadatos del archivo y mantiene una lista de los índices de bloques asignados. Esto permite acceso directo a los bloques sin necesidad de búsquedas adicionales.

2. Estructura `FileSystem`

Representa el sistema de archivos completo:

```c
typedef struct {
    unsigned char blocks[MAX_BLOCKS][BLOCK_SIZE];  // Bloques de almacenamiento
    bool block_map[MAX_BLOCKS];                    // Mapa de bloques (libre/ocupado)
    FileEntry file_table[MAX_FILES];               // Tabla de archivos
    size_t num_files;                              // Número de archivos actuales
    size_t used_blocks;                            // Bloques utilizados
    size_t total_storage;                          // Almacenamiento total usado
} FileSystem;
```

**Decisión de diseño:** 
- El sistema mantiene un array bidimensional para simular bloques de almacenamiento físico.
- Un mapa de bloques (`block_map`) permite verificación rápida de disponibilidad (O(1)).
- La tabla de archivos es un array de tamaño fijo para simplicidad y acceso directo.

## Constantes del Sistema

- **BLOCK_SIZE**: 512 bytes (tamaño estándar de bloque en sistemas de archivos)
- **MAX_FILES**: 100 archivos máximo
- **MAX_STORAGE**: 1 MB (1,048,576 bytes) de almacenamiento total
- **MAX_BLOCKS**: 2,048 bloques (calculado como MAX_STORAGE / BLOCK_SIZE)
- **MAX_FILENAME**: 256 caracteres

## Principales Decisiones de Diseño

1. Gestión de Bloques

**Estrategia de asignación:**
- Primero se intenta asignar bloques consecutivos (mejor rendimiento para lectura/escritura secuencial).
- Si no hay bloques consecutivos disponibles, se asignan bloques dispersos.
- Esta estrategia balancea entre eficiencia y flexibilidad.

**Justificación:** Los sistemas de archivos reales también usan estrategias similares (como ext2/3 con bloques consecutivos cuando es posible).

2. Almacenamiento de Índices de Bloques

Los índices de bloques se almacenan en el `FileEntry` en lugar de calcularlos dinámicamente. Esto permite:
- Acceso directo a los bloques (O(1))
- Soporte para fragmentación de archivos
- Simplicidad en la implementación de lectura/escritura

3. Manejo de Offset y Lectura/Escritura

La lectura y escritura manejan correctamente:
- Cálculo del bloque inicial basado en el offset
- Posición dentro del bloque donde comenzar
- Escritura/lectura que cruza múltiples bloques
- Validación de límites del archivo

**Cálculo utilizado:**
```c
start_block = offset / BLOCK_SIZE;    // Bloque donde comienza
start_pos = offset % BLOCK_SIZE;      // Posición dentro del bloque
```

4. Validación de Errores

El sistema implementa validación exhaustiva:
- Verificación de parámetros NULL
- Validación de límites (tamaño de archivo, offset)
- Verificación de existencia de archivos
- Verificación de espacio disponible
- Mensajes de error descriptivos

## Funciones Principales

### `init_filesystem()`
Inicializa el sistema de archivos, limpiando todos los bloques y marcándolos como libres.

### `create_file(filename, size)`
- Valida parámetros y espacio disponible
- Calcula bloques necesarios
- Asigna bloques usando `allocate_blocks()`
- Crea entrada en la tabla de archivos
- Retorna 0 si es exitoso, -1 en caso de error

### `write_file(filename, offset, data)`
- Busca el archivo en la tabla
- Valida offset y tamaño
- Calcula bloques y posiciones
- Escribe datos bloque por bloque
- Maneja escritura que cruza múltiples bloques

### `read_file(filename, offset, size, buffer)`
- Busca el archivo en la tabla
- Valida offset y ajusta tamaño si es necesario
- Lee datos bloque por bloque
- Retorna datos en el buffer proporcionado

### `delete_file(filename)`
- Busca y elimina el archivo
- Libera bloques usando `free_blocks()`
- Actualiza estadísticas del sistema
- Limpia entrada en la tabla de archivos

### `list_files()`
- Recorre la tabla de archivos
- Muestra nombre y tamaño de cada archivo
- Muestra estadísticas del sistema

### `allocate_blocks(num_blocks, block_list)`
- Intenta asignar bloques consecutivos primero
- Si no es posible, asigna bloques dispersos
- Actualiza el mapa de bloques
- Retorna número de bloques asignados

### `free_blocks(num_blocks, block_list)`
- Libera bloques marcándolos como libres
- Limpia el contenido de los bloques liberados
- Actualiza contador de bloques usados

## Limitaciones y Consideraciones

### Limitaciones Actuales:

1. **Tamaño de datos en WRITE:** Limitado por el tamaño del buffer en la función `main()`. En un sistema real, se leerían datos de cualquier tamaño.

2. **Fragmentación:** El sistema permite fragmentación de archivos pero no implementa desfragmentación automática.

3. **Sincronización:** No hay manejo de concurrencia o protección contra acceso simultáneo (no requerido para este proyecto).

4. **Persistencia:** Los datos no se guardan en disco; son solo en memoria durante la ejecución.

### Consideraciones para Mejoras Futuras:

1. **Desfragmentación:** Implementar un algoritmo para reorganizar bloques y reducir fragmentación.

2. **Persistencia:** Guardar el sistema de archivos en un archivo real del sistema operativo.

3. **Subdirectorios:** Extender para soportar estructura de directorios jerárquica.

4. **Permisos:** Implementar sistema de permisos de lectura/escritura.

5. **Metadatos adicionales:** Agregar fecha de creación, modificación, etc.

## Complejidad Computacional

- **CREATE:** O(n) donde n es MAX_FILES para buscar entrada libre, más O(m) donde m es MAX_BLOCKS para asignar bloques.
- **WRITE/READ:** O(k) donde k es el número de bloques a leer/escribir.
- **DELETE:** O(1) para búsqueda (con índice) + O(b) donde b es número de bloques del archivo.
- **LIST:** O(n) donde n es MAX_FILES.
- **FIND_FILE:** O(n) donde n es MAX_FILES (búsqueda lineal).

En un sistema optimizado, se podría usar una tabla hash para `find_file()` reduciendo a O(1) promedio, pero para 100 archivos máximo, la búsqueda lineal es aceptable.

## Pruebas Realizadas

El sistema ha sido probado con:
- Creación de archivos de diversos tamaños
- Escritura y lectura de datos
- Operaciones con múltiples archivos
- Manejo de errores (archivo inexistente, límites excedidos, etc.)
- Eliminación de archivos y liberación de bloques
- Casos límite (archivos que ocupan exactamente un bloque, archivos grandes, etc.)

Ver archivo `ejemplos_uso.txt` para ejemplos detallados de uso.

## Compilación y Uso

### Compilación:
```bash
make
# o
gcc -Wall -Wextra -std=c11 -g -o filesystem filesystem.c
```

### Ejecución:
```bash
./filesystem
# o en Windows
filesystem.exe
```

### Comandos disponibles:
- `CREATE <archivo> <tamaño>`
- `WRITE <archivo> <offset> "<datos>"`
- `READ <archivo> <offset> <tamaño>`
- `DELETE <archivo>`
- `LIST`
- `EXIT`
