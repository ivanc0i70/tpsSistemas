#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/config.h>

#include "utilsCPU/utilsCPU.h"

t_log* loggerCPU;
t_config* configCPU;
int contadorinterrupciones = 0; //esto deberia ser global
t_list* fetch(uint32_t PC, int conexion);
void ejecutar(t_list* list, contextoEjecución*contx, int conexion);
bool check_interrupt(int conexInterrupt);
// ==================================================[CONEXIONES C/ MÓDULOS]===================================================

int conexionDesdeKERNEL() {
    char* puertoCPU = config_get_string_value(configCPU, "PUERTO_CPU_DISPATCH");
    int serverCPUdis_fd = iniciar_servidor(loggerCPU, "CPU", puertoCPU);
    log_info(loggerCPU, "CPU listo para recibir al KERNEL!(Dispatch)");
    return esperar_cliente(loggerCPU, serverCPUdis_fd);
}

int conexionDesdeKERNELInterrupt() {
    char* puertoCPU = config_get_string_value(configCPU, "PUERTO_CPU_INTERRUPT");
    int serverCPUint_fd = iniciar_servidor(loggerCPU, "CPU", puertoCPU);
    log_info(loggerCPU, "CPU listo para recibir al KERNEL!(Interrupt)");
    return esperar_cliente(loggerCPU, serverCPUint_fd);
}

int conexionConMEMORIA() {
    char* ipMEMORIA = config_get_string_value(configCPU, "IP_MEMORIA");
    char* puertoMEMORIA = config_get_string_value(configCPU, "PUERTO_MEMORIA");
    return crear_conexion(loggerCPU, "MEMORIA", ipMEMORIA, puertoMEMORIA);  
}

// ==================================================[SERIALIZACIONES]===================================================
void pedir_instruccion(uint32_t programCounter, int conexionConMEMORIA) { // Le envía el PC a memoria
    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_unit32(buffer, programCounter);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROGRAM_COUNTER; 
    paquete->buffer = buffer;

    void* a_enviar = malloc(paquete->buffer->size + sizeof(int) + sizeof(uint32_t));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    // Por último enviamos
    send(conexionConMEMORIA, a_enviar, paquete->buffer->size + sizeof(int) + sizeof(uint32_t), 0);
    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar); free(paquete->buffer->stream); free(paquete->buffer); free(paquete);
}

void enviar_contextoejecucion(contextoEjecución*contexto, int conexKernelInter) { // Le envía el PC a memoria
    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_contexto(buffer, contexto);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = CONTEXT_EXEC_PROCESO; 
    paquete->buffer = buffer;
    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code));
    int offset = 0;
        
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code)); 
    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    
    send(conexKernelInter, a_enviar, paquete->buffer->size + sizeof(int) + sizeof(uint32_t), 0);
    
    free(a_enviar); free(paquete->buffer->stream); free(paquete->buffer); free(paquete);
}

void enviar_IO_GEN_SLEEP(char* nombre, int tiempo, int serverKERNEL_fd) {
    peticionIO peticionIO;
    peticionIO.nombreUnico = nombre;
    peticionIO.nombre_length = strlen(nombre) + 1;
    peticionIO.unidadesTrabajo = tiempo;

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = IO_SOLICITADA_GEN_SLEEP;

    paquete->buffer = buffer_create(sizeof(uint32_t) * 2 + peticionIO.nombre_length);
    buffer_add_unit32(paquete->buffer, tiempo);
    buffer_add_string(paquete->buffer, peticionIO.nombre_length, peticionIO.nombreUnico);

    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code));
    int offset = 0;
        
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));  offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
        
    send(serverKERNEL_fd, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    free(a_enviar); free(paquete->buffer->stream); free(paquete->buffer); free(paquete);

}

contextoEjecución* serializarContextoEjecucion(t_buffer* buffer) {
    contextoEjecución* contExec = malloc(sizeof(PCB));
    void* stream = buffer->stream;

    memcpy(&(contExec->programCounter), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(contExec->registros), stream, sizeof(t_CPUregisters)); stream += sizeof(t_CPUregisters);
    memcpy(&(contExec->path_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t); contExec->path = malloc(contExec->path_length);
    memcpy(contExec->path, stream, contExec->path_length);
    return contExec;
}

pathPrueba* deserializarPath(t_buffer* buffer) {
    pathPrueba* path;
    void* stream = buffer->stream;
    memcpy(&(path->path_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t); path->path = malloc(path->path_length);
    memcpy(path->path, stream, path->path_length);
    return path;
}

/*// CHECKPOINT 2: Generar interfaz y mandarla para comprobación.
peticionIO iniciarInterfaz() {
    peticionIO solicitudInterfaz;
    char* nombreUnico = "interfaz1";
    solicitudInterfaz.nombreUnico = nombreUnico;
    solicitudInterfaz.nombre_length = strlen(nombreUnico) + 1;
    solicitudInterfaz.unidadesTrabajo = 250;
    
    return solicitudInterfaz;
}

void peticionInterfazGenerica(int unSocket) {
    peticionIO interfazSolicitada = iniciarInterfaz();
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2 + interfazSolicitada.nombre_length);

    buffer_add_unit32(buffer, interfazSolicitada.unidadesTrabajo);
    buffer_add_string(buffer, interfaz.config_length, interfaz.archivoConfig);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = INTERFAZ_SOLICITADA_CPU; 
    paquete->buffer = buffer;

    void* a_enviar = malloc(paquete->buffer->size + sizeof(int) + sizeof(uint32_t));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    // Por último enviamos
    send(unSocket, a_enviar, paquete->buffer->size + sizeof(int) + sizeof(uint32_t), 0);
    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar); free(paquete->buffer->stream); free(paquete->buffer); free(paquete);
}
*/

void* recibirPaquete(int socketEmisor_fd) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    crear_buffer(paquete); int size;
    while(1){
        int cod_op = recibir_operacion(socketEmisor_fd);
        switch (cod_op) {

            case PATH_PRUEBA:
                
                recv(socketEmisor_fd, &(paquete->buffer->size), sizeof(uint32_t), 0); paquete->buffer->stream = malloc(paquete->buffer->size);
                recv(socketEmisor_fd, paquete->buffer->stream, paquete->buffer->size, 0);

                pathPrueba* path = deserializarPath(paquete->buffer);
                log_warning(loggerCPU, "El path es: %s", path->path);

                enviar_IO_GEN_SLEEP("interfaz5", 250, socketEmisor_fd);    

                break;
                
            case CONTEXT_EXEC_PROCESO:

                
                recv(socketEmisor_fd, &(paquete->buffer->size), sizeof(uint32_t), 0); paquete->buffer->stream = malloc(paquete->buffer->size);
                recv(socketEmisor_fd, paquete->buffer->stream, paquete->buffer->size, 0);
                contextoEjecución* proceso_CE = serializarContextoEjecucion(paquete->buffer); // Recibe PC - Registros - Path
                proceso_CE->path = "/home/utnso/Desktop/tp-2024-1c-Bola-de-nieve-II/memoria/src/archivosPseudo/script_io_basico1.txt";
                proceso_CE->programCounter = 0;
                while(1){

                    t_list* instrucciones = fetch(proceso_CE->programCounter,socketEmisor_fd);
                    ejecutar(instrucciones, proceso_CE ,conexionConMEMORIA());
                    if(check_interrupt(conexionDesdeKERNELInterrupt())){
                        enviar_contextoejecucion(proceso_CE, socketEmisor_fd);
                        //mutex
                        contadorinterrupciones--;
                        //mutex
                        break;
                    }
                    else{
                        log_error(loggerCPU, "No hay interrupciones");
                    }
                    char* aux = list_get(instrucciones, 0);
                    if(strcmp(aux, "EXIT") == 0){
                        break;
                    }
                }
                // Acá iría una función que haga el ciclo de instrucción del contexto recibido
                // En algún momento del ciclo mandarán (o no) la solicitud de interfaz 'IO_GEN_SLEEP'; suponiendo que lo hace:

                //peticionInterfazGenerica(socketEmisor_fd);
        


                free(proceso_CE);
                break;

            case -1:
                log_error(loggerCPU, "El cliente se ha ido!");
                return EXIT_FAILURE;
                break;
            default:
                log_warning(loggerCPU, "Operacion desconocida chango; no quieras meter la pata eh");
                break;
        }

        eliminar_paquete(paquete);
        
    }
}

void reset_registers(t_CPUregisters* registers) {
    registers->AX = 0;
    registers->BX = 0;
    registers->CX = 0;
    registers->DX = 0;
    registers->EAX = 0;
    registers->EBX = 0;
    registers->ECX = 0;
    registers->EDX = 0;
    registers->SI = 0;
    registers->DI = 0;
}

void iterator(char* value) {
	log_info(loggerCPU,"%s", value);
}

/*void rcvstream(int conexion){
    char* msj;
    recv(conexion,msj,5,0);
    //printf("Mensaje recibido: %s\n", buffer);
    log_info(loggerCPU, "Mensaje recibido: %s", msj);
    t_list* lista = recibir_paquete(conexion);
    log_info(loggerCPU, "Me llegaron los siguientes valores:\n");
    list_iterate(lista, (void*) iterator);
    //list_destroy(lista);
}
*/
typedef struct {
    op_code codigo_operacion;
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_bufferD;

char* deserializarLinea(int conexion){

    t_bufferD* tamaño = malloc(sizeof(t_bufferD));

    recv(conexion, &(tamaño->codigo_operacion), sizeof(op_code), 0);
    recv(conexion, &(tamaño->size), sizeof(uint32_t), 0);
    tamaño->stream = malloc(tamaño->size);
    recv(conexion, tamaño->stream, tamaño->size, 0); 
    
    uint32_t Length;
    memcpy(&Length, tamaño->stream, sizeof(uint32_t));

    char* linea = malloc(sizeof(Length));
    memcpy(linea, tamaño->stream + sizeof(uint32_t), Length);
    
    return linea;
}
    
//--------------------------------------------------EJECUCION DE INSTRUCCIONES--------------------------------------------------
t_list* fetch(uint32_t PC, int conexion){
    
  
    pedir_instruccion(PC, conexion);
    char* instruccion = deserializarLinea(conexion);

    t_list* lista = list_create();
    char* leido;

    leido = strtok(instruccion, " ");
    
    while(leido != NULL) {
        list_add(lista, leido);
        log_info(loggerCPU,"%s\n", leido);
        leido = strtok(NULL, " ");
    }
    return lista;   
}

uint32_t* get_32bit_register(t_CPUregisters* registers, char* register_name) {
    if(strcmp(register_name, "EAX") == 0) {
        return &registers->EAX;
    } else if(strcmp(register_name, "EBX") == 0) {
        return &registers->EBX;
    } else if(strcmp(register_name, "ECX") == 0) {
        return &registers->ECX;
    } else if(strcmp(register_name, "EDX") == 0) {
        return &registers->EDX;
    } else if(strcmp(register_name, "SI") == 0) {
        return &registers->SI;
    } else if(strcmp(register_name, "DI") == 0) {
        return &registers->DI;
    } else {
        printf("Registro Desconocido: %s\n", register_name);
        return NULL;
    }
}

uint8_t* get_8bit_register(t_CPUregisters* registers, char* register_name) {
    if(strcmp(register_name, "AX") == 0) {
        return (uint8_t*)&registers->AX;
    } else if(strcmp(register_name, "BX") == 0) {
        return (uint8_t*)&registers->BX;
    } else if(strcmp(register_name, "CX") == 0) {
        return (uint8_t*)&registers->CX;
    } else if(strcmp(register_name, "DX") == 0) {
        return (uint8_t*)&registers->DX;
    } else {
        printf("Registro Desconocido: %s\n", register_name);
        return NULL;
    }
}


void ejecutar(t_list* list, contextoEjecución*contx, int conexion){
    char* instr = list_get(list, 0);
    
    
    if (strcmp(instr, "SUM") == 0) {
        printf("SUM\n");
        char* reg1 = list_get(list, 1);
        char* reg2 = list_get(list, 2);
        if(strlen(reg1) == 2){
            uint8_t* rega = get_8bit_register(&contx->registros, reg1);
            if(strlen(reg2) == 2){
            uint8_t* regb = get_8bit_register(&contx->registros, reg2);
            *rega = *rega + *regb;
            log_info(loggerCPU, "Valor de %s: %d", reg1, *rega);
        }  else if(strlen(reg2) == 3){
        }  else if(strlen(reg2) == 3){
            uint32_t* regb = get_32bit_register(&contx->registros, reg2);
            *rega = *rega + *regb; 
            log_info(loggerCPU, "Valor de %s: %d", reg1, *rega);
        }else{  
            log_error(loggerCPU, "Registro 2 no reconocido");
        }
        }else if(strlen(reg1) == 3){
            uint32_t* rega = get_32bit_register(&contx->registros, reg1);
            if(strlen(reg2) == 2){
            uint8_t* regb = get_8bit_register(&contx->registros, reg2);
            *rega = *rega + *regb;
            log_info(loggerCPU, "Valor de %s: %d", reg1, *rega);
        }else if(strlen(reg2) == 3){
            uint32_t* regb = get_32bit_register(&contx->registros, reg2); 
            *rega = *rega + *regb;
            log_info(loggerCPU, "Valor de %s: %d", reg1, *rega);
        }else{  
            log_error(loggerCPU, "Registro 2 no reconocido");
        }
        }else{ 
            log_error(loggerCPU, "Registro 1 no reconocido");
        }
        
    } else if (strcmp(instr, "SUB") == 0) {
        printf("SUB\n");
 char* reg1 = list_get(list, 1);
        char* reg2 = list_get(list, 2);
        if(strlen(reg1) == 2){
            uint8_t* rega = get_8bit_register(&contx->registros, reg1);
            if(strlen(reg2) == 2){
            uint8_t* regb = get_8bit_register(&contx->registros, reg2);
            *rega = *rega - *regb;
            log_info(loggerCPU, "Valor de %s: %d", reg1, *rega);
        }else if(strlen(reg2) == 3){
            uint32_t* regb = get_32bit_register(&contx->registros, reg2);
            *rega = *rega - *regb; 
            log_info(loggerCPU, "Valor de %s: %d", reg1, *rega);
        }else{  
            log_error(loggerCPU, "Registro 2 no reconocido");
        }
        }else if(strlen(reg1) == 3){
            uint32_t* rega = get_32bit_register(&contx->registros, reg1);
            if(strlen(reg2) == 2){
            uint8_t* regb = get_8bit_register(&contx->registros, reg2);
            *rega = *rega - *regb;
            log_info(loggerCPU, "Valor de %s: %d", reg1, *rega);
        }else if(strlen(reg2) == 3){
            uint32_t* regb = get_32bit_register(&contx->registros, reg2); 
            *rega = *rega - *regb;
            log_info(loggerCPU, "Valor de %s: %d", reg1, *rega);
        }else{  
            log_error(loggerCPU, "Registro 2 no reconocido");
        }
        }else{ 
            log_error(loggerCPU, "Registro 1 no reconocido");
        }
    } else if (strcmp(instr, "SET") == 0) {
        printf("SET\n");
        char* reg = list_get(list, 1);
        char* val = list_get(list, 2);
        if(strlen(reg) == 2){
            uint8_t* reg8 = get_8bit_register(&contx->registros, reg);
            *reg8 = atoi(val);
            log_info(loggerCPU, "Valor de %s: %d", reg, *reg8);
        }else if(strlen(reg) == 3){
            uint32_t* reg32 = get_32bit_register(&contx->registros, reg);
            *reg32 = atoi(val);
            log_info(loggerCPU, "Valor de %s: %d", reg, *reg32);
        }else{
            log_error(loggerCPU, "Registro no reconocido");
        }
    }else if(strcmp(instr, "JNZ" )== 0){
        printf("JNZ\n");
        char* reg = list_get(list, 1);
        char* line = list_get(list, 2);
        if((strlen(reg)) == 2){
           uint8_t* reg8 = get_8bit_register(&contx->registros, reg);
            if(*reg8 == 0){
                contx->programCounter ++;
                log_info(loggerCPU, "Valor de programCounter: %d",contx->programCounter);
            }else{
                contx->programCounter = atoi(line);
                log_info(loggerCPU, "Valor de programCounter: %d",contx->programCounter);
                }
        }else if((strlen(reg)) == 3){
           uint32_t* reg32 = get_32bit_register(&contx->registros, reg);
            if(*reg32 == 0){
                contx->programCounter ++;
                log_info(loggerCPU, "Valor de programCounter: %d", contx->programCounter);
            }else{
                contx->programCounter = atoi(line);
                log_info(loggerCPU, "Valor de programCounter: %d", contx->programCounter); 
            }

        }else{
            log_error(loggerCPU, "Registro no reconocido");
        }
    }
    else if(strcmp(instr, "IO_GEN_SLEEP" )== 0){
        printf("IO_GEN_SLEEP\n");
        char* nombre = list_get(list, 1);
        uint32_t tiempo = atoi(list_get(list, 2));
        log_info(loggerCPU, "Nombre de la interfaz: %s", nombre);
        log_info(loggerCPU, "Tiempo de la interfaz: %d", tiempo);
        enviar_IO_GEN_SLEEP(nombre, tiempo, conexion);
        
    }
    else if(strcmp(instr, "EXIT" )== 0){
        printf("EXIT\n");
        enviar_contextoejecucion(contx, conexion);
    
    }
     else {
        log_error(loggerCPU, "Instruccion no reconocida");
    }
     
    if(strcmp(instr, "JNZ" )!= 0){//este se ejecuta si o si despues de alguna instruccion
       contx->programCounter;
        

        }
}

void recibir_interrupcion(){//deberia estar ejecutandose todo el tiempo
    op_code interrupcion;
    int conexInterrupt= conexionDesdeKERNELInterrupt();
    while(1){
        recv(conexInterrupt, &interrupcion, sizeof(op_code), 0);
        if(interrupcion == FIN_QUANTUM || interrupcion == FIN_PROCESO){
            log_info(loggerCPU, "Interrupcion reconocida");
            //mutex
            contadorinterrupciones++;
            //mutex
        } 
        else{
            log_error(loggerCPU, "Interrupcion no reconocida");
        }
    }
}

bool check_interrupt(int conexInterrupt){
  
   if(contadorinterrupciones > 0){
        log_info(loggerCPU, "Se recibio una interrupcion");
        return true;
    }else{
        log_error(loggerCPU, "No se recibio ninguna interrupcion");
        return false;
    }
}

int main(int argc, char* argv[]) {
    t_CPUregisters registros;
    reset_registers(&registros);
    loggerCPU = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
    configCPU = config_create("/home/utnso/Desktop/tp-2024-1c-Bola-de-nieve-II/cpu/cpu.config");
    
    int socketEmisor_fd = conexionDesdeKERNEL();
    recibirPaquete(socketEmisor_fd);
    close(socketEmisor_fd);

return 0;
}
