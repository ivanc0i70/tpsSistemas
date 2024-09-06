#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/config.h>

#include "utilsIO.h"

char* generarNombre(char* base, int index) {
    // Como quiero cancherear un poquito, voy a hacer 10 interfaces; el nombre va a ser una concatenacion del 'i' y de interfaz
    int totalLen = strlen(base) + snprintf(NULL, 0, "%d", index) + 1; // SUmo longitud de 'interfaz' y del número que le pase
    char* nombreInterfaz = malloc(totalLen);
    if(nombreInterfaz == NULL) {
        printf("[ERROR] | No se pudo asignar memoria al crear el nombre de la interfaz");
        exit(1);
    }
    snprintf(nombreInterfaz, totalLen, "%s%d", base, index);
    return nombreInterfaz;
}

interfazIO inicializarInterfaz(int index) {
    interfazIO interfaz;
    char* path = "/home/utnso/Desktop/tp-2024-1c-Bola-de-nieve-II/entradasalida/inout.config";
    interfaz.archivoConfig = path;
    
    char* nombreUnico = generarNombre("interfaz", (index + 1));
    interfaz.nombreUnico = nombreUnico;

    interfaz.interfaz_length = strlen(nombreUnico) + strlen(path) + 1;

    return interfaz;
}

char* strcatDeInterfaz(interfazIO interfaz) {
    char* strg = malloc(interfaz.interfaz_length);
    if(strg == NULL) {
        printf("ERROR | No se pudo asignar memoria para el string creado en el envío de interfaces al Kernel");
        return 1;
    }
    strcpy(strg, interfaz.archivoConfig);
    strcat(strg, interfaz.nombreUnico);
    return strg;
}