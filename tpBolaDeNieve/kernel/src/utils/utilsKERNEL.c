#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <utils/hello.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "utilsKERNEL.h"

// ===========================================[SEPARADOR DE TEMA]=============================================
// [TITULO]: Funciones utilizadas para complementar al TP; c/u será utilizada por lo menos en algúna operación del kernel.


int buscarFuncion(char* text) {
    int posFuncion = -1;
    const char* funciones[7] = {"EJECUTAR_SCRIPT", "INICIAR_PROCESO", "FINALIZAR_PROCESO", "DETENER_PLANIFICACION", "INICIAR_PLANIFICACION", "MULTIPROGRAMACION", "PROCESO_ESTADO"};
    for (int i = 0; i < 7; i++) {
        if(strcmp(text, funciones[i]) == 0){
            posFuncion = (i + 1);
        }
    }
    return posFuncion;
}

char* ingresarConReadline() {
    char *linea;
    linea = readline("");
    if (linea) {
        add_history(linea);
    }
    return linea;
    free(linea);
}

inputText textoIngresado() {
    int posFuncion = -1;
    bool fallo = false;
    char *funcionCompleta, *nombreFuncion, *pathProceso;
    inputText funcion;

    do {
        funcionCompleta = ingresarConReadline();
        
        if(funcionCompleta == NULL || strcmp(funcionCompleta, "") == 0) { // Contempla el caso en el que no ingrese nada por consola
            printf("\n [ERROR] | Pero papu, ingresame algo por consola: ");
            fallo = false;
            continue;
        }

        nombreFuncion = strtok(funcionCompleta, " ");       // <-- obtenemos el nombre de la función a corroborar exista.
        for (int i = 0; nombreFuncion[i]; i++) {
            nombreFuncion[i] = toupper(nombreFuncion[i]);
        }

        pathProceso = strtok(NULL, " "); // <-- Obtenemos el path del pseudocodigo
       
        posFuncion = buscarFuncion(nombreFuncion);
        if (nombreFuncion == NULL || pathProceso == NULL) { // <-- si nosotros ingresamos únicamente una sola palabra, esto lo soluciona
            switch (posFuncion) {
            case 1:
                printf("\n [ERROR] | Ingrese una funcion y un path, separados por un espacio: "); // Corrobora que se hayan ingresado por lo menos 2 palabras 
                fallo = false;
                continue;
                break;
            case 2:
                printf("\n [ERROR] | Ingrese una funcion y un path, separados por un espacio: "); 
                fallo = false;
                continue;
                break;
            case 3:
                printf("\n [ERROR] | Ingresa el PID separado por un espacio, canchero: "); 
                fallo = false;
                continue;
                break;
            case 6:
                printf("\n [ERROR] | Ingresa el nuevo valor del grado separado por un espacio, canchero: "); 
                fallo = false;
                continue;
                break;
            case -1:
                printf("\n [ERROR] | Esa funcion no existe. Ingrese nuevamente: ");
                fallo = false;
                continue;
                break;
            default:
                funcion.nombreFuncion = malloc(strlen(nombreFuncion) + 1); 
                strcpy(funcion.nombreFuncion, nombreFuncion);
                funcion.path_or_numberLength = 0; funcion.path_or_number = "";
                fallo = true;
                return funcion;
                free(funcion.nombreFuncion);
                break;
            }
        } else { // Si se ingresan 2 palabras en funciones que no deberían, esto lo arregla
            if(posFuncion == 4 || posFuncion == 5 || posFuncion == 7) {
                printf("\n [ERROR] | Solo tenés que ingresar la funcion macho: ");
                fallo = false;
            } else {
                fallo = true;
                funcion.nombreFuncion = malloc(strlen(nombreFuncion) + 1); 
                strcpy(funcion.nombreFuncion, nombreFuncion);
                funcion.path_or_numberLength = strlen(pathProceso) + 1;
                funcion.path_or_number = malloc(funcion.path_or_numberLength);
                strcpy(funcion.path_or_number, pathProceso);
                return funcion;
            }
        }
        free(funcionCompleta);
    } while(!fallo);
}

interfazIO* obtenerInterfaz(char* stringCompleto){
    interfazIO *interfazConectada;
    const char* config_pos = strrchr(stringCompleto, 'c');
    if(config_pos == NULL) {
        perror("ERROR | No se encontró el archivo.config al separar el string recibido desde IO");
        interfazConectada->archivoConfig = strdup("");
        interfazConectada->nombreUnico = strdup("");
        return interfazConectada;
    }

    int base_pos = config_pos - stringCompleto + strlen("config");

    interfazConectada->archivoConfig = (char*)malloc(base_pos + 1);
    if(interfazConectada->archivoConfig == NULL){
        perror("ERROR | No se pudo asignar memoria para el config de la interfaz YA conectada");
        exit(EXIT_FAILURE);
    }

    strncpy(interfazConectada->archivoConfig, stringCompleto, base_pos);
    interfazConectada->archivoConfig[base_pos] = '\0';

    interfazConectada->nombreUnico = strdup(stringCompleto + base_pos);
    if(interfazConectada->nombreUnico == NULL){
        perror("ERROR | No se pudo asignar memoria para el nombreUnico de la interfaz YA conectada");
        exit(EXIT_FAILURE);
    }

    return interfazConectada;
}

bool interfazBuscadaPorNombre(t_list* lista_interfaces, char* nombre) {
    bool flag = false, encontrado = false;
    t_list* listaAux = list_duplicate(lista_interfaces);
    char* nombreAux;
    while(!flag && !encontrado) {
        nombreAux = list_remove(listaAux, (listaAux->elements_count - 1));
        flag = (listaAux->elements_count == 0) ? 1 : 0; 
        encontrado = (strcmp(nombreAux, nombre) == 0) ? true : false;
    }
    list_destroy(listaAux);
    return encontrado;
}

/*
bool verificarOperacion(interfazIO* interfazPedida, t_log* logger) { // Verifica si la OP en la interfaz enviada desde CPU existe.
    int valido;
    switch (interfazPedida->tipoInterfaz) {
    case GENERICA:
        valido = (interfazPedida->tipoOperacion == IO_GEN_SLEEP) ? 1 : 0;
        break;
    case STDIN:
        valido = (interfazPedida->tipoOperacion == IO_STDIN_READ) ? 1 : 0;
        break;
    case STDOUT:
        valido = (interfazPedida->tipoOperacion == IO_STDOUT_WRITE) ? 1 : 0;
        break;
    case DIALFS:
        valido = (interfazPedida->tipoOperacion == IO_FS_CREATE)   ? 1:
                 (interfazPedida->tipoOperacion == IO_FS_DELETE)   ? 1:
                 (interfazPedida->tipoOperacion == IO_FS_READ)     ? 1:
                 (interfazPedida->tipoOperacion == IO_FS_TRUNCATE) ? 1:
                 (interfazPedida->tipoOperacion == IO_FS_WRITE)    ? 1: 0;
    default:
        log_error(logger, "[ERROR] | No reconoce tipo de interfaz al validar la operación.");
        break;
    }
}
*/

// ===========================================================[SEPARADOR DE TEMA]===============================================

/*  [TITULO]: Funciones relacionadas a la creación de la estructura tipo 'cola' para la cola de planificación de los estados
    que se encontraran en estado 'Nuevo'.
*/

ColaProcesos* crearCola(){
    ColaProcesos* cola = (ColaProcesos*) malloc(sizeof(ColaProcesos));
    int cantidad = 0;
    cola->fin = NULL;
    cola->inicio = NULL;
    return cola;
}

NodoProceso* crearNodoNuevo(PCB* proceso){
    NodoProceso* nodo = (NodoProceso *) malloc(sizeof(NodoProceso));
    nodo->proceso = proceso;
    nodo->next = NULL;
    return nodo;
}

void destruirNodo(NodoProceso* nodo){
    nodo->proceso = NULL;
    nodo->next = NULL;
    free(nodo);
}

void encolarProceso(ColaProcesos* cola, PCB* proceso) {
    NodoProceso* nodo = crearNodoNuevo(proceso);
    if (nodo == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria.\n");
        exit(EXIT_FAILURE);
    }
    if (cola->inicio == NULL) {
        cola->inicio = nodo;
        cola->fin = nodo;
    } else {
        cola->fin->next = nodo;
        cola->fin = nodo;
    }
    cola->cantidad = cola->cantidad + 1;
}

// Función para desencolar un elemento de la cola
// En caso de error, retornara un PCB con PID = -1 
PCB* desencolarProceso(ColaProcesos *cola) {
    PCB *proceso = malloc(sizeof(PCB));
    proceso->PID = -1;
    if (cola->inicio == NULL) {
        fprintf(stderr, "Error: La cola está vacía.\n");
    } else {
        NodoProceso *aux = cola->inicio;
        proceso = cola->inicio->proceso;
        cola->inicio = cola->inicio->next;
        if(cola->cantidad == 1){
            cola->fin = NULL;
        }
        free(aux);
        cola->cantidad = cola->cantidad - 1;
    }
    return proceso;
}

PCB* buscarYEliminarProcesoEnCola(ColaProcesos *cola, int PID) {
    NodoProceso *aux = cola->inicio; 
    NodoProceso *anterior = NULL;
    
    while (aux != NULL) {
        if(PID == aux->proceso->PID){
            PCB* procesoAEliminar = aux->proceso; // Una vez encontramos el proceso, rearmamos la cola; dios bendiga las listas
            if(anterior == NULL) { // Acá reestructura si el proceos que eliminamos está al frente de la cola
                cola->inicio = aux->next;
                if(cola->inicio == NULL) { // Acá reestructura si la cola queda vacía
                    cola->fin = NULL;
                }
            } else {
                anterior->next = aux->next;
                if(aux->next == NULL) {
                    cola->fin = anterior;
                }
            }
            free(aux);
            return procesoAEliminar;
        }
        anterior = aux;
        aux = aux->next;
    }
    return NULL;
}

char* leerCola(ColaProcesos* cola) {
    NodoProceso* aux = cola->inicio;
    size_t length = 2; // Para los corchetes inicial y final
    NodoProceso* temp = aux;
    while (temp != NULL) {
        length += snprintf(NULL, 0, "%d", temp->proceso->PID) + 1;
        temp = temp->next;
    }

    char* result = (char*)malloc(length);
    if (result == NULL) {
        return NULL; // Manejo simple de error de asignación de memoria
    }

    char* ptr = result;
    ptr += sprintf(ptr, "[");
    while (aux != NULL) {
        if (aux->next == NULL) {
            ptr += sprintf(ptr, "%d", aux->proceso->PID);
        } else {
            ptr += sprintf(ptr, "%d ", aux->proceso->PID);
        }
        aux = aux->next;
    }
    sprintf(ptr, "]");

    return result;
}


// ===========================================[SEPARADOR DE TEMA]=============================================
