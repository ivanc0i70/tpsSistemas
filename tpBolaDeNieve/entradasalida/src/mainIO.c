#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/config.h>

#include "utilsIO/utilsIO.h"

t_log* loggerIO;
t_config* configIO;

// =================================================[CONEXIONES C/ MÓDULOS]=================================================
int conexionConKERNEL() {
    char* ipKERNEL = config_get_string_value(configIO, "IP_KERNEL");
    char* puertoKERNEL = config_get_string_value(configIO, "PUERTO_KERNEL");
    return crear_conexion(loggerIO, "KERNEL", ipKERNEL, puertoKERNEL); 
}

int conexionConMEMORIA() {
    char* ipMEMORIA = config_get_string_value(configIO, "IP_MEMORIA");
    char* puertoMEMORIA = config_get_string_value(configIO, "PUERTO_MEMORIA");
    return crear_conexion(loggerIO, "MEMORIA", ipMEMORIA, puertoMEMORIA); 
}

// ====================================================[SERIALIZACIONES]====================================================
peticionIO* serializarInterfaz(t_buffer* buffer) {
    peticionIO* interfaz = malloc(sizeof(interfazIO));
    void* stream = buffer->stream;
    memcpy(&(interfaz->unidadesTrabajo), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(interfaz->nombre_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t); interfaz->nombreUnico = malloc(interfaz->nombre_length);
    memcpy(interfaz->nombreUnico, stream, interfaz->nombre_length);
    memcpy(&(interfaz->nombreOp_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t); interfaz->nombreOp = malloc(interfaz->nombreOp_length);
    memcpy(interfaz->nombreOp, stream, interfaz->nombreOp_length);
    return interfaz;
}


void enviarInterfacesConectadas(int socketKernel_fd){
    int cantInterfaces = 10; // Cantidad de interfaces a enviar al Kernel
    
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = INTERFACES_CONECTADAS_IO; 

    for (int i = 0; i < cantInterfaces; i++) {
        interfazIO interfaz = inicializarInterfaz(i);
        char* strgCompleto = strcatDeInterfaz(interfaz);
        agregar_a_paquete(paquete, strgCompleto, interfaz.interfaz_length);
        free(strgCompleto);
    }
    
    enviar_paquete(paquete, socketKernel_fd);
    eliminar_paquete(paquete);
}

void notificarKernel(int socket_cliente) {
    char* mensaje = "SE USO LA INTERFAZ!";
    t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void recibirPaquete(int unSocket) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    crear_buffer(paquete); int size;
    while(1){
        int cod_op = recibir_operacion(unSocket);
        switch (cod_op) {
        case PETICION_INTERFAZ:
            recv(unSocket, &(paquete->buffer->size), sizeof(uint32_t), 0); paquete->buffer->stream = malloc(paquete->buffer->size);
            recv(unSocket, paquete->buffer->stream, paquete->buffer->size, 0);

            peticionIO* solicitudInterfaz = serializarInterfaz(paquete->buffer);
                          
            switch (solicitudInterfaz->nombreOp) {
            case "IO_GEN_SLEEP":
                
                sleep(solicitudInterfaz->unidadesTrabajo) * 1000;

                break;
            
            default:


                break;
            }


            break;
            
        case -1:
            log_error(loggerIO, "El cliente se ha ido!");
            return EXIT_FAILURE;
            break;
        default: 
            log_warning(loggerIO, "Operacion desconocida chango; no quieras meter la pata eh");
            break;
        }
    }
    eliminar_paquete(paquete);
    close(unSocket);
}


// ====================================================[Funciones | Main]====================================================


void realizarSLEEP(int tiempo) {
    usleep(tiempo * 10000);
}

int main(int argc, char* argv[]) {
    loggerIO = log_create("IO.log", "I/O", true, LOG_LEVEL_INFO);
    configIO = config_create("/home/utnso/Desktop/tp-2024-1c-Bola-de-nieve-II/entradasalida/inout.config");

    // Envía las interfaces que estén conectadas al Kernel
    int socketKernel_fd = conexionConKERNEL();

    enviarInterfacesConectadas(socketKernel_fd);
    
    recibirPaquete(socketKernel_fd);

    close(socketKernel_fd);
    
    
    /*
    switch (interfazAUsarse->nombreInterfaz) {
    case GENERICA:
        int tiempoTrabajo = 250; //atoi(config_get_string_value(interfazAUsarse->archivoConfig, "TIEMPO_DE_TRABAJO"));
        realizarSLEEP(tiempoTrabajo);
        log_info(loggerIO, "Se hizo el sleep!");
        notificarKernel(serverKERNEL_fd);
        // Enviar notifiacion al kernel
        break;
    default:
        log_error(loggerIO, "[ERROR] | No reconoce tipo de interfaz al validar la operación.");
        break;
    }
    */

   

    return 0;
}

