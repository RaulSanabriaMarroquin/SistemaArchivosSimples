/*
 * Sistema de Archivos Simple
 * Simula un sistema de archivos básico con operaciones de creación,
 * escritura, lectura, eliminación y listado de archivos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Constantes del Sistema */
#define BLOCK_SIZE 512                    /* Tamaño de cada bloque en bytes */
#define MAX_FILES 100                     /* Número máximo de archivos */
#define MAX_STORAGE (1024 * 1024)         /* Almacenamiento máximo: 1 MB */
#define MAX_BLOCKS (MAX_STORAGE / BLOCK_SIZE)  /* Número máximo de bloques: 2048 */
#define MAX_FILENAME 256                  /* Longitud máxima del nombre de archivo */
#define MAX_FILE_SIZE (1024 * 1024)       /* Tamaño máximo por archivo: 1 MB */

/* Estructura para representar un archivo */
typedef struct {
    char filename[MAX_FILENAME];          /* Nombre del archivo */
    size_t size;                          /* Tamaño del archivo en bytes */
    size_t num_blocks;                    /* Número de bloques ocupados */
    size_t blocks[MAX_BLOCKS];            /* Índices de bloques ocupados */
    bool in_use;                          /* Indica si la entrada está en uso */
} FileEntry;

/* Estructura principal del sistema de archivos */
typedef struct {
    unsigned char blocks[MAX_BLOCKS][BLOCK_SIZE];  /* Bloques de almacenamiento */
    bool block_map[MAX_BLOCKS];                    /* Mapa de bloques: true = ocupado, false = libre */
    FileEntry file_table[MAX_FILES];               /* Tabla de archivos */
    size_t num_files;                              /* Número de archivos actuales */
    size_t used_blocks;                            /* Número de bloques utilizados */
    size_t total_storage;                          /* Almacenamiento total utilizado */
} FileSystem;

/* Variable global del sistema de archivos */
static FileSystem fs;

/* Prototipos de funciones */
void init_filesystem(void);
int create_file(const char *filename, size_t size);
int write_file(const char *filename, size_t offset, const char *data);
int read_file(const char *filename, size_t offset, size_t size, char *buffer);
int delete_file(const char *filename);
void list_files(void);
FileEntry* find_file(const char *filename);
size_t allocate_blocks(size_t num_blocks, size_t *block_list);
void free_blocks(size_t num_blocks, const size_t *block_list);

/**
 * Inicializa el sistema de archivos
 */
void init_filesystem(void) {
    /* Limpiar todos los bloques */
    memset(fs.blocks, 0, sizeof(fs.blocks));
    
    /* Marcar todos los bloques como libres */
    for (size_t i = 0; i < MAX_BLOCKS; i++) {
        fs.block_map[i] = false;
    }
    
    /* Limpiar tabla de archivos */
    for (size_t i = 0; i < MAX_FILES; i++) {
        fs.file_table[i].in_use = false;
        fs.file_table[i].filename[0] = '\0';
        fs.file_table[i].size = 0;
        fs.file_table[i].num_blocks = 0;
    }
    
    fs.num_files = 0;
    fs.used_blocks = 0;
    fs.total_storage = 0;
    
    printf("Sistema de archivos inicializado.\n");
    printf("  - Tamano de bloque: %d bytes\n", BLOCK_SIZE);
    printf("  - Numero maximo de archivos: %d\n", MAX_FILES);
    printf("  - Almacenamiento maximo: %d bytes (%d KB)\n", MAX_STORAGE, MAX_STORAGE / 1024);
    printf("  - Numero maximo de bloques: %d\n\n", MAX_BLOCKS);
}

/**
 * Busca un archivo en la tabla de archivos
 * @param filename Nombre del archivo a buscar
 * @return Puntero al archivo si existe, NULL en caso contrario
 */
FileEntry* find_file(const char *filename) {
    for (size_t i = 0; i < MAX_FILES; i++) {
        if (fs.file_table[i].in_use && 
            strcmp(fs.file_table[i].filename, filename) == 0) {
            return &fs.file_table[i];
        }
    }
    return NULL;
}

/**
 * Asigna bloques consecutivos para un archivo
 * @param num_blocks Número de bloques a asignar
 * @param block_list Array donde se guardarán los índices de bloques asignados
 * @return Número de bloques asignados exitosamente, 0 si no hay espacio suficiente
 */
size_t allocate_blocks(size_t num_blocks, size_t *block_list) {
    if (num_blocks == 0 || num_blocks > MAX_BLOCKS) {
        return 0;
    }
    
    if (fs.used_blocks + num_blocks > MAX_BLOCKS) {
        return 0;  /* No hay suficiente espacio */
    }
    
    size_t allocated = 0;
    
    /* Buscar bloques libres consecutivos */
    for (size_t i = 0; i < MAX_BLOCKS && allocated < num_blocks; i++) {
        if (!fs.block_map[i]) {
            /* Encontrar una secuencia de bloques libres */
            bool found_sequence = true;
            for (size_t j = 0; j < num_blocks; j++) {
                if (i + j >= MAX_BLOCKS || fs.block_map[i + j]) {
                    found_sequence = false;
                    break;
                }
            }
            
            if (found_sequence) {
                /* Asignar los bloques */
                for (size_t j = 0; j < num_blocks; j++) {
                    fs.block_map[i + j] = true;
                    block_list[j] = i + j;
                }
                allocated = num_blocks;
                fs.used_blocks += num_blocks;
                return allocated;
            }
        }
    }
    
    /* Si no hay bloques consecutivos, asignar bloques dispersos */
    allocated = 0;
    for (size_t i = 0; i < MAX_BLOCKS && allocated < num_blocks; i++) {
        if (!fs.block_map[i]) {
            fs.block_map[i] = true;
            block_list[allocated] = i;
            allocated++;
            fs.used_blocks++;
        }
    }
    
    return allocated;
}

/**
 * Libera bloques de memoria
 * @param num_blocks Número de bloques a liberar
 * @param block_list Array con los índices de bloques a liberar
 */
void free_blocks(size_t num_blocks, const size_t *block_list) {
    for (size_t i = 0; i < num_blocks; i++) {
        if (block_list[i] < MAX_BLOCKS && fs.block_map[block_list[i]]) {
            fs.block_map[block_list[i]] = false;
            /* Limpiar el contenido del bloque */
            memset(fs.blocks[block_list[i]], 0, BLOCK_SIZE);
            fs.used_blocks--;
        }
    }
}

/**
 * Crea un nuevo archivo en el sistema
 * @param filename Nombre del archivo
 * @param size Tamaño del archivo en bytes
 * @return 0 si es exitoso, -1 en caso de error
 */
int create_file(const char *filename, size_t size) {
    /* Validaciones */
    if (filename == NULL || strlen(filename) == 0) {
        printf("Error: Nombre de archivo inválido.\n");
        return -1;
    }
    
    if (size == 0) {
        printf("Error: El tamano del archivo debe ser mayor que cero.\n");
        return -1;
    }
    
    if (size > MAX_FILE_SIZE) {
        printf("Error: El tamano del archivo excede el límite maximo (%d bytes).\n", MAX_FILE_SIZE);
        return -1;
    }
    
    /* Verificar si el archivo ya existe */
    if (find_file(filename) != NULL) {
        printf("Error: El archivo '%s' ya existe.\n", filename);
        return -1;
    }
    
    /* Verificar si hay espacio para más archivos */
    if (fs.num_files >= MAX_FILES) {
        printf("Error: Se ha alcanzado el numero maximo de archivos (%d).\n", MAX_FILES);
        return -1;
    }
    
    /* Calcular número de bloques necesarios */
    size_t num_blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;  /* Redondeo hacia arriba */
    
    /* Verificar espacio disponible */
    if (fs.used_blocks + num_blocks > MAX_BLOCKS) {
        printf("Error: No hay suficiente espacio en el sistema de archivos.\n");
        printf("  Bloques disponibles: %zu\n", MAX_BLOCKS - fs.used_blocks);
        printf("  Bloques requeridos: %zu\n", num_blocks);
        return -1;
    }
    
    /* Buscar una entrada libre en la tabla de archivos */
    size_t file_index = MAX_FILES;
    for (size_t i = 0; i < MAX_FILES; i++) {
        if (!fs.file_table[i].in_use) {
            file_index = i;
            break;
        }
    }
    
    if (file_index == MAX_FILES) {
        printf("Error: No hay espacio en la tabla de archivos.\n");
        return -1;
    }
    
    /* Asignar bloques */
    size_t allocated = allocate_blocks(num_blocks, fs.file_table[file_index].blocks);
    if (allocated < num_blocks) {
        printf("Error: No se pudieron asignar todos los bloques necesarios.\n");
        /* Liberar bloques ya asignados */
        if (allocated > 0) {
            free_blocks(allocated, fs.file_table[file_index].blocks);
        }
        return -1;
    }
    
    /* Crear entrada del archivo */
    strncpy(fs.file_table[file_index].filename, filename, MAX_FILENAME - 1);
    fs.file_table[file_index].filename[MAX_FILENAME - 1] = '\0';
    fs.file_table[file_index].size = size;
    fs.file_table[file_index].num_blocks = num_blocks;
    fs.file_table[file_index].in_use = true;
    
    fs.num_files++;
    fs.total_storage += size;
    
    printf("Archivo '%s' creado exitosamente (%zu bytes, %zu bloques).\n", 
           filename, size, num_blocks);
    return 0;
}

/**
 * Escribe datos en un archivo
 * @param filename Nombre del archivo
 * @param offset Offset donde comenzar a escribir
 * @param data Datos a escribir
 * @return 0 si es exitoso, -1 en caso de error
 */
int write_file(const char *filename, size_t offset, const char *data) {
    if (filename == NULL || data == NULL) {
        printf("Error: Parámetros inválidos.\n");
        return -1;
    }
    
    /* Buscar el archivo */
    FileEntry *file = find_file(filename);
    if (file == NULL) {
        printf("Error: El archivo '%s' no existe.\n", filename);
        return -1;
    }
    
    /* Validar offset */
    if (offset > file->size) {
        printf("Error: Offset (%zu) excede el tamano del archivo (%zu bytes).\n", 
               offset, file->size);
        return -1;
    }
    
    size_t data_len = strlen(data);
    
    /* Validar que no se exceda el tamaño del archivo */
    if (offset + data_len > file->size) {
        printf("Error: La escritura excede el tamano del archivo.\n");
        printf("  Tamano del archivo: %zu bytes\n", file->size);
        printf("  Intento de escritura: offset %zu + %zu bytes\n", offset, data_len);
        return -1;
    }
    
    /* Calcular en qué bloque y posición dentro del bloque comenzar */
    size_t start_block = offset / BLOCK_SIZE;
    size_t start_pos = offset % BLOCK_SIZE;
    
    size_t bytes_written = 0;
    size_t current_block = start_block;
    size_t current_pos = start_pos;
    
    /* Escribir los datos bloque por bloque */
    while (bytes_written < data_len && current_block < file->num_blocks) {
        size_t block_index = file->blocks[current_block];
        size_t bytes_to_write = data_len - bytes_written;
        
        /* Limitar la escritura al espacio disponible en el bloque actual */
        if (current_pos + bytes_to_write > BLOCK_SIZE) {
            bytes_to_write = BLOCK_SIZE - current_pos;
        }
        
        /* Escribir en el bloque */
        memcpy(&fs.blocks[block_index][current_pos], 
               &data[bytes_written], 
               bytes_to_write);
        
        bytes_written += bytes_to_write;
        current_block++;
        current_pos = 0;
    }
    
    printf("Escritos %zu bytes en '%s' (offset %zu).\n", 
           bytes_written, filename, offset);
    return 0;
}

/**
 * Lee datos de un archivo
 * @param filename Nombre del archivo
 * @param offset Offset donde comenzar a leer
 * @param size Cantidad de bytes a leer
 * @param buffer Buffer donde almacenar los datos leídos
 * @return 0 si es exitoso, -1 en caso de error
 */
int read_file(const char *filename, size_t offset, size_t size, char *buffer) {
    if (filename == NULL || buffer == NULL || size == 0) {
        printf("Error: Parámetros inválidos.\n");
        return -1;
    }
    
    /* Buscar el archivo */
    FileEntry *file = find_file(filename);
    if (file == NULL) {
        printf("Error: El archivo '%s' no existe.\n", filename);
        return -1;
    }
    
    /* Validar offset */
    if (offset >= file->size) {
        printf("Error: Offset (%zu) excede el tamano del archivo (%zu bytes).\n", 
               offset, file->size);
        return -1;
    }
    
    /* Ajustar el tamaño a leer si se excede el tamaño del archivo */
    size_t bytes_to_read = size;
    if (offset + bytes_to_read > file->size) {
        bytes_to_read = file->size - offset;
        printf("Advertencia: Se leerán %zu bytes en lugar de %zu (fin del archivo).\n", 
               bytes_to_read, size);
    }
    
    /* Calcular en qué bloque y posición dentro del bloque comenzar */
    size_t start_block = offset / BLOCK_SIZE;
    size_t start_pos = offset % BLOCK_SIZE;
    
    size_t bytes_read = 0;
    size_t current_block = start_block;
    size_t current_pos = start_pos;
    
    /* Leer los datos bloque por bloque */
    while (bytes_read < bytes_to_read && current_block < file->num_blocks) {
        size_t block_index = file->blocks[current_block];
        size_t bytes_to_read_now = bytes_to_read - bytes_read;
        
        /* Limitar la lectura al espacio disponible en el bloque actual */
        if (current_pos + bytes_to_read_now > BLOCK_SIZE) {
            bytes_to_read_now = BLOCK_SIZE - current_pos;
        }
        
        /* Leer del bloque */
        memcpy(&buffer[bytes_read], 
               &fs.blocks[block_index][current_pos], 
               bytes_to_read_now);
        
        bytes_read += bytes_to_read_now;
        current_block++;
        current_pos = 0;
    }
    
    buffer[bytes_read] = '\0';  /* Agregar terminador de cadena */
    
    printf("Leídos %zu bytes de '%s' (offset %zu).\n", 
           bytes_read, filename, offset);
    return 0;
}

/**
 * Elimina un archivo del sistema
 * @param filename Nombre del archivo a eliminar
 * @return 0 si es exitoso, -1 en caso de error
 */
int delete_file(const char *filename) {
    if (filename == NULL) {
        printf("Error: Nombre de archivo inválido.\n");
        return -1;
    }
    
    /* Buscar el archivo */
    FileEntry *file = find_file(filename);
    if (file == NULL) {
        printf("Error: El archivo '%s' no existe.\n", filename);
        return -1;
    }
    
    /* Liberar bloques */
    free_blocks(file->num_blocks, file->blocks);
    
    /* Actualizar estadísticas */
    fs.total_storage -= file->size;
    fs.num_files--;
    
    /* Limpiar entrada */
    file->in_use = false;
    file->filename[0] = '\0';
    file->size = 0;
    file->num_blocks = 0;
    
    printf("Archivo '%s' eliminado exitosamente.\n", filename);
    return 0;
}

/**
 * Lista todos los archivos en el sistema
 */
void list_files(void) {
    if (fs.num_files == 0) {
        printf("(no hay archivos)\n");
        return;
    }
    
    printf("\nArchivos en el sistema:\n");
    printf("----------------------------------------\n");
    printf("%-30s %12s\n", "Nombre", "Tamano (bytes)");
    printf("----------------------------------------\n");
    
    for (size_t i = 0; i < MAX_FILES; i++) {
        if (fs.file_table[i].in_use) {
            printf("%-30s %12zu\n", 
                   fs.file_table[i].filename, 
                   fs.file_table[i].size);
        }
    }
    
    printf("----------------------------------------\n");
    printf("Total: %zu archivo(s), %zu bytes, %zu bloques utilizados\n\n", 
           fs.num_files, fs.total_storage, fs.used_blocks);
}

/**
 * Función principal - Interfaz de línea de comandos
 */
int main(void) {
    char command[1024];
    char filename[MAX_FILENAME];
    char data[10240];  /* Buffer para datos */
    char buffer[10240];  /* Buffer para lectura */
    size_t size, offset;
    
    printf("========================================\n");
    printf("   Sistema de Archivos Simple v1.0\n");
    printf("========================================\n\n");
    
    init_filesystem();
    
    printf("Comandos disponibles:\n");
    printf("  CREATE <archivo> <tamano>\n");
    printf("  WRITE <archivo> <offset> \"<datos>\"\n");
    printf("  READ <archivo> <offset> <tamano>\n");
    printf("  DELETE <archivo>\n");
    printf("  LIST\n");
    printf("  EXIT\n\n");
    
    while (1) {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        /* Eliminar salto de línea */
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }
        
        if (strlen(command) == 0) {
            continue;
        }
        
        /* Procesar comando CREATE */
        if (sscanf(command, "CREATE %s %zu", filename, &size) == 2) {
            create_file(filename, size);
        }
        /* Procesar comando WRITE */
        else if (sscanf(command, "WRITE %s %zu \"%[^\"]\"", filename, &offset, data) == 3) {
            write_file(filename, offset, data);
        }
        /* Procesar comando READ */
        else if (sscanf(command, "READ %s %zu %zu", filename, &offset, &size) == 3) {
            if (read_file(filename, offset, size, buffer) == 0) {
                printf("Salida: \"%s\"\n", buffer);
            }
        }
        /* Procesar comando DELETE */
        else if (sscanf(command, "DELETE %s", filename) == 1) {
            delete_file(filename);
        }
        /* Procesar comando LIST */
        else if (strcmp(command, "LIST") == 0) {
            list_files();
        }
        /* Procesar comando EXIT */
        else if (strcmp(command, "EXIT") == 0 || strcmp(command, "QUIT") == 0) {
            printf("Saliendo del sistema de archivos...\n");
            break;
        }
        /* Comando no reconocido */
        else {
            printf("Error: Comando no reconocido. Use CREATE, WRITE, READ, DELETE, LIST o EXIT.\n");
        }
    }
    
    return 0;
}
