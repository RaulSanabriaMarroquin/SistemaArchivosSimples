# Sistema de Archivos Simple

Implementación en C de un sistema de archivos simple que simula las operaciones básicas de un sistema de archivos real.

## Características

- ✅ Creación de archivos con tamaño especificado
- ✅ Escritura de datos en archivos con offset personalizado
- ✅ Lectura de datos desde archivos
- ✅ Eliminación de archivos y liberación de bloques
- ✅ Listado de todos los archivos en el sistema
- ✅ Manejo completo de errores
- ✅ Gestión de bloques de memoria simulados

## Especificaciones Técnicas

- **Tamaño de bloque**: 512 bytes
- **Máximo de archivos**: 100
- **Almacenamiento máximo**: 1 MB (1,048,576 bytes)
- **Tamaño máximo por archivo**: 1 MB

## Compilación

### Usando Makefile:
```bash
make
```

### Compilación manual:
```bash
gcc -Wall -Wextra -std=c11 -g -o filesystem filesystem.c
```

### Limpiar archivos compilados:
```bash
make clean
```

## Uso

### Ejecutar el programa:
```bash
./filesystem
```

En Windows:
```bash
filesystem.exe
```

### Comandos disponibles:

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| CREATE | `CREATE <archivo> <tamaño>` | Crea un archivo con el tamaño especificado |
| WRITE | `WRITE <archivo> <offset> "<datos>"` | Escribe datos en el archivo desde el offset |
| READ | `READ <archivo> <offset> <tamaño>` | Lee datos del archivo desde el offset |
| DELETE | `DELETE <archivo>` | Elimina un archivo del sistema |
| LIST | `LIST` | Lista todos los archivos en el sistema |
| EXIT | `EXIT` | Sale del programa |

### Ejemplo de uso:

```
> CREATE archivo1.txt 1000
Archivo 'archivo1.txt' creado exitosamente (1000 bytes, 2 bloques).

> WRITE archivo1.txt 0 "Hola, mundo"
Escritos 11 bytes en 'archivo1.txt' (offset 0).

> READ archivo1.txt 0 11
Leídos 11 bytes de 'archivo1.txt' (offset 0).
Salida: "Hola, mundo"

> LIST

Archivos en el sistema:
----------------------------------------
Nombre                         Tamaño (bytes)
----------------------------------------
archivo1.txt                          1000
----------------------------------------
Total: 1 archivo(s), 1000 bytes, 2 bloques utilizados

> DELETE archivo1.txt
Archivo 'archivo1.txt' eliminado exitosamente.
```

## Archivos del Proyecto

- `filesystem.c` - Código fuente principal del sistema de archivos
- `Makefile` - Archivo para compilación automatizada
- `ejemplos_uso.txt` - Ejemplos detallados de uso del sistema
- `INFORME.md` - Informe técnico explicando estructuras de datos y decisiones de diseño
- `README.md` - Este archivo

## Manejo de Errores

El sistema maneja los siguientes errores:

- ❌ Archivo no encontrado
- ❌ Falta de espacio en el sistema
- ❌ Offset que excede el tamaño del archivo
- ❌ Escritura que excede el tamaño del archivo
- ❌ Lectura fuera de límites
- ❌ Número máximo de archivos alcanzado
- ❌ Parámetros inválidos

## Estructura del Código

El código está organizado en:

1. **Estructuras de datos**:
   - `FileEntry`: Representa un archivo individual
   - `FileSystem`: Representa el sistema de archivos completo

2. **Funciones principales**:
   - Gestión de archivos (CREATE, DELETE, LIST)
   - Operaciones de I/O (READ, WRITE)
   - Gestión de bloques (allocate_blocks, free_blocks)
   - Utilidades (find_file, init_filesystem)

## Requisitos

- Compilador GCC con soporte para C11
- Sistema operativo: Linux, macOS, o Windows (con MinGW)

## Autores

Implementado como parte de la Tarea 4 de Sistemas Operativos.

## Licencia

Este proyecto es educativo y se proporciona sin garantías.
