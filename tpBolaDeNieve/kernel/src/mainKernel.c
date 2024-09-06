#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <utils/hello.h>
#include <commons/config.h>
#include <time.h>
#include <commons/string.h>
#include "pthread.h"
#include "utils/utilsKERNEL.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
t_log* loggerKERNEL;
t_config* configKERNEL;

int gradoMultiprogramacion;
int cantProcesos = 0;

bool flagFin = false;
bool flagInterrupcion = false;
bool interfazEnUso = false;

/* Semáforos para el planificador de Largo Plazo */
pthread_cond_t condLP = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexPausaLP = PTHREAD_MUTEX_INITIALIZER;
int pausedLP = 1;

/* Semáforos para el planificador de Corto Plazo */
pthread_cond_t condCP = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexPausaCP = PTHREAD_MUTEX_INITIALIZER;
int pausedCP = 1;


ColaProcesos *cola_planificacion_nuevo;
ColaProcesos *cola_planificacion_listo;
ColaProcesos *cola_planificacion_exec;
ColaProcesos *cola_planificacion_blocked;
ColaProcesos *cola_planificacion_exit;

t_list* lista_interfaces;
t_config* configIO;


void iterator(char* value) {
	log_info(loggerKERNEL,"%s", value);
}

// ==================================================[CONEXIONES C/ MÓDULOS]===================================================
int conexionConCPUDispatch() { 
    char* ipCPU = config_get_string_value(configKERNEL, "IP_CPU");
    char* puertoCPU = config_get_string_value(configKERNEL, "PUERTO_CPU_DISPATCH");
    return crear_conexion(loggerKERNEL, "CPU", ipCPU, puertoCPU); // Crear socket cliente por parte del kernel a CPU.
}

int conexionConCPUInterrupt() { // Mandar interrupciones al CPU (como del quantum, por ejemplo; mirar enunciado para ver cuáles son).
    char* ipCPU = config_get_string_value(configKERNEL, "IP_CPU");
    char* puertoInterrupt = config_get_string_value(configKERNEL, "PUERTO_CPU_INTERRUPT");
    return crear_conexion(loggerKERNEL, "CPUInterrupt", ipCPU, puertoInterrupt); // Crear socket cliente por parte del kernel a CPU.
}

int conexionConMemoria() {
    char* ipMEMORIA = config_get_string_value(configKERNEL, "IP_MEMORIA");
    char* puertoMEMORIA = config_get_string_value(configKERNEL, "PUERTO_MEMORIA");
    return crear_conexion(loggerKERNEL, "MEMORIA", ipMEMORIA, puertoMEMORIA); 
}

int conexionDesdeIO(){
    char* puertoKERNEL = config_get_string_value(configKERNEL, "PUERTO_KERNEL");
    int serverKERNEL_fd = iniciar_servidor(loggerKERNEL, "KERNEL", puertoKERNEL);
    log_info(loggerKERNEL, "[KERNEL] listo para recibir a la [INTERFAZ I/O]!");
    return esperar_cliente(loggerKERNEL, serverKERNEL_fd);
}


// ====================================================[SERIALIZACIONES]====================================================
void enviarInterrupcion(int PID, int unSocket){
    t_buffer* buffer = buffer_create(sizeof(int));
    buffer_add_int(buffer,PID); //Es optimo utilizar int(?)

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = INTERRUPT;
    paquete->buffer = buffer;

    void* a_enviar = malloc(paquete->buffer->size + sizeof(int) + sizeof(uint32_t));//No estoy seguro de si esta bien
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int)); offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t)); offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    send(unSocket, a_enviar, paquete->buffer->size + sizeof(int) + sizeof(uint32_t), 0);
    free(a_enviar); free(paquete->buffer->stream); free(paquete->buffer); free(paquete);
}

void enviarProcesoCPU(PCB* proceso, int unSocket){ // Le envía el contexto de ejecución de un proceso a la CPU.
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2 + sizeof(t_CPUregisters) + proceso->path_length);
    buffer_add_unit32(buffer, proceso->programCounter);
    buffer_add_cpuRegisters(buffer, proceso->registros);
    buffer_add_string(buffer, proceso->path_length, proceso->path);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = CONTEXT_EXEC_PROCESO; 
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



// CHECKPOINT 2: Recibir interfaz y enviarla a IO para comprobar esté bien!
void enviarUsoInterfazGenerica(peticionIO* interfaz, int unSocket){ // Envía la OP de la interfaz (ya chequeada) a IO.
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 3 + interfaz->nombre_length + interfaz->nombreOp_length) ;

    buffer_add_unit32(buffer, interfaz->unidadesTrabajo);
    buffer_add_string(buffer, interfaz->nombre_length, interfaz->nombreUnico);
    buffer_add_string(buffer, interfaz->nombreOp_length, interfaz->nombreOp);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PETICION_INTERFAZ; 
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

void crearProceso(int unSocket, inputText textoConsola) { // Le envía a Memoria el path para que cree el proceso.
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + textoConsola.path_or_numberLength);
    buffer_add_string(buffer, textoConsola.path_or_numberLength, textoConsola.path_or_number);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = SOLICITUD_PROCESO; 
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

void enviarPathCPU(int unSocket, pathPrueba path) { // Envía el path de la función 1 al CPU
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + path.path_length);
    buffer_add_string(buffer, path.path_length, path.path);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PATH_PRUEBA; 
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

peticionIO* deserializarInterfazDeCPU(t_buffer* buffer) {
    peticionIO* interfazSolicitada = malloc(sizeof(peticionIO));
    void* stream = buffer->stream;
    memcpy(&(interfazSolicitada->unidadesTrabajo), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(interfazSolicitada->nombre_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t); interfazSolicitada->nombreUnico = malloc(interfazSolicitada->nombre_length);
    memcpy(interfazSolicitada->nombreUnico, stream, interfazSolicitada->nombre_length);
    return interfazSolicitada;
}

PCB* deserializarProceso(t_buffer* buffer) {
    PCB* proceso = malloc(sizeof(PCB));
    void* stream = buffer->stream;
    memcpy(&(proceso->PID), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(proceso->quantum), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(proceso->programCounter), stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    memcpy(&(proceso->estado), stream, sizeof(uint8_t)); stream += sizeof(uint8_t);
    memcpy(&(proceso->registros), stream, sizeof(t_CPUregisters)); stream += sizeof(t_CPUregisters);
    memcpy(&(proceso->path_length), stream, sizeof(uint32_t)); stream += sizeof(uint32_t); proceso->path = malloc(proceso->path_length);
    memcpy(proceso->path, stream, proceso->path_length);
    return proceso;
}


void recibirPaquete(int socketEmisor_fd) {
    
    // Acá recibe el cod_op
    t_paquete* paquete = malloc(sizeof(t_paquete));
    crear_buffer(paquete); int size;


    while(1){
        int cod_op = recibir_operacion(socketEmisor_fd);
        switch (cod_op) {
        case PROCESO:
            recv(socketEmisor_fd, &(paquete->buffer->size), sizeof(uint32_t), 0); paquete->buffer->stream = malloc(paquete->buffer->size);
            recv(socketEmisor_fd, paquete->buffer->stream, paquete->buffer->size, 0);
            encolarProceso(cola_planificacion_nuevo, deserializarProceso(paquete->buffer));
            log_warning(loggerKERNEL, "Se crea el proceso <%d> en NEW", cola_planificacion_nuevo->fin->proceso->PID);
            break;
        
        case INTERFACES_CONECTADAS_IO: // Interfaces cargadas desde IO
            bool flag = false, creado = false;
            lista_interfaces = list_create();
            t_list* listaAux = recibir_paquete(socketEmisor_fd); // Me devuelve la lista ya completa;
            while(!flag) {
                char* stringCompleto = list_remove(listaAux, (listaAux->elements_count - 1));
                flag = (listaAux->elements_count == 0) ? 1 : 0; /* Lo había hecho con list_is_empty la comparación de si estaba vacío; no me funcionó */
                interfazIO* interfazConectada = obtenerInterfaz(stringCompleto);
                if(!creado) { // Pongo esto para que no 
                    configIO = config_create(interfazConectada->archivoConfig);
                    creado = true;
                }; 
                list_add(lista_interfaces, interfazConectada->nombreUnico); // Pongo en la lista solo el nombre de la interfaz
        
            }  
            close(socketEmisor_fd);
            break;

        case IO_SOLICITADA_GEN_SLEEP:
            
            recv(socketEmisor_fd, &(paquete->buffer->size), sizeof(uint32_t), 0); paquete->buffer->stream = malloc(paquete->buffer->size);
            recv(socketEmisor_fd, paquete->buffer->stream, paquete->buffer->size, 0);
            peticionIO* interfazSolicitada = deserializarInterfazDeCPU(paquete->buffer); // Recibe interfaz de CPU a validar.
            PCB* proceso;

            if(!interfazEnUso){
            proceso = desencolarProceso(cola_planificacion_exec); // Habría que ponerle mutex a esto
            }else{
                perror("La interfaz esta siendo utilizada, espera papu");
                exit(EXIT_FAILURE);
            }
            if (list_is_empty(lista_interfaces)) { // La lista de interfaces no está vacía
                log_error(loggerKERNEL, "La lista de interfaces conectadas esta vacia (wtf?)");
                log_warning(loggerKERNEL, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXIT>", proceso->PID);
                encolarProceso(cola_planificacion_exit, proceso);
                break;
            }
            if(strcmp(config_get_string_value(configIO, "TIPO_INTERFAZ"), "GENERICA") != 0) { // Chequea que la interfaz conectada sea la generica && el nombre de la interfaz esté también
                log_error(loggerKERNEL, "la interfaz [GENERICA] no está conectada | interfaz conectada: %s", config_get_string_value(configIO, "TIPO_INTERFAZ"));
               log_warning(loggerKERNEL, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXIT>", proceso->PID);
                encolarProceso(cola_planificacion_exit, proceso);
                break;
            }
            if(!interfazBuscadaPorNombre(lista_interfaces, interfazSolicitada->nombreUnico)){ // Busca si la interfaz está conectada
                log_error(loggerKERNEL, "no se encontro el nombre de la interfaz | nombre de la interfaz solicitada: %s", interfazSolicitada->nombreUnico);
                log_warning(loggerKERNEL, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXIT>", proceso->PID);
                encolarProceso(cola_planificacion_exit, proceso);
                break;
            }
            char* operacion = "IO_GEN_SLEEP";
            interfazSolicitada->nombreOp = operacion;
            interfazSolicitada->nombreOp_length = strlen(operacion);

            // Validar el tipo de operación

            flagInterrupcion = true;
            encolarProceso(cola_planificacion_blocked, proceso);
            
            enviarUsoInterfazGenerica(interfazSolicitada, socketEmisor_fd);


            close(socketEmisor_fd);

            break;

        case FIN_PROCESO:
            recv(socketEmisor_fd, &(paquete->buffer->size), sizeof(uint32_t), 0); paquete->buffer->stream = malloc(paquete->buffer->size);
            recv(socketEmisor_fd, paquete->buffer->stream, paquete->buffer->size, 0);
            //no se si aca seria ideal meter el codigo de encolar y desencolar el proceso :/
            // por ahora dejo aca... ojala mi amigo bergon viera esto y me diga che hice el rr :D
            flagFin = true;
            

            break;
        
        case -1:
            log_error(loggerKERNEL, "El cliente se ha ido!");
            return EXIT_FAILURE;
            break;
        default: 
            log_warning(loggerKERNEL, "Operacion desconocida chango; no quieras meter la pata eh");
            break;
        }
    }
    
    eliminar_paquete(paquete);
}
// ====================================================[PLANIFICADORES]====================================================
    
    //Aca abajo voy a trabajar con las colas de planificación FIFO y RR
    //  -> (No se como va a ser completamente su implementación, asi que seguro vaya a ser modificada)
    //  -> Directamente saca de la cola de 'Ready' al proceso

void planificacionFIFO(ColaProcesos *cola, int unSocket){
    PCB* proceso = desencolarProceso(cola);
    proceso->estado = EXECUTE;
    log_warning(loggerKERNEL, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXECUTE>", proceso->PID);
    enviarProcesoCPU(proceso, unSocket);
}

void planificacionRR(int socketDispatch, int socketInterrupt){
    encolarProceso(cola_planificacion_exec,desencolarProceso(cola_planificacion_listo));
    enviarProcesoCPU(cola_planificacion_exec->inicio->proceso, socketDispatch);
    sleep(atoi(config_get_string_value(configKERNEL, "QUANTUM")));
}

void planificacionVRR(ColaProcesos* colaReady, int unSocket){
    bool controlKey = true;
    ColaProcesos* colaReadyAux = crearCola();
    PCB* pcbActual;
    contextoEjecución* contextoProcActual;
    /* 
    VRR (Virtual Round Robin) → similar a RR, pero con una cola de READY auxiliar de mayor prioridad
    que la cola de READY original a la que llegan los procesos que no aprovechan todo su quantum.
        o Siempre se atienden primero los procesos de la cola de READY auxiliar, salvo que esté vacía.
        o Si un proceso se bloquea (por una E/S) …
            -- … habiendo aprovechado todo su quantum → el proceso se sitúa al final de la cola de
            READY (igual que en RR).
            -- … sin haber aprovechado todo su quantum → el proceso se sitúa al final de la cola de
            READY auxiliar de mayor prioridad, pero cuando le toque ejecutarse lo hará con el
            quantum remanente, esto es, con lo que no terminó de usar del quantum inicial.
   
    necesitamos un recieve en el algoritmo por si necesitan una interfaz, 
    osea una interrupcion

    tenemos que avisar desde kernel si el quantum terminó y ahi desalojarlo 
    desde el cpu y el algoritmo

    y necesitamos otros recvs por si terminó el proceso per se, 
    o si necesita que se replanifique algo para su continuacion
    */


    do{
        if(colaReadyAux->inicio != NULL || colaReady->inicio != NULL){
            
            if(colaReadyAux->inicio != NULL){
                pcbActual = desencolarProceso(colaReadyAux); 
                contextoProcActual->path = pcbActual->path;
                contextoProcActual->path_length = pcbActual->path_length;
                contextoProcActual->programCounter = pcbActual->programCounter;
                contextoProcActual->registros = pcbActual->registros;
            }else{
                pcbActual = desencolarProceso(colaReady);
                contextoProcActual->path = pcbActual->path;
                contextoProcActual->path_length = pcbActual->path_length;
                contextoProcActual->programCounter = pcbActual->programCounter;
                contextoProcActual->registros = pcbActual->registros;
            }
            pcbActual->estado = EXECUTE;
            
            encolarProceso(cola_planificacion_exec, pcbActual);
            enviarProcesoCPU(pcbActual, unSocket);
           
            int fd_cpuDispatch = conexionConCPUDispatch();
            int quantum = atoi(config_get_string_value(configKERNEL, "QUANTUM"));
            
            while(quantum > 0 || (!flagInterrupcion) || (!flagFin)){
                usleep(1000000);
                quantum -= 1000;
                // Aca se deberia colocar una funcion 
                // No se como se actualizaria desde el kernel este contexto, sabiendo que se 
                // ejecuta en el CPU el proceso.
                // "enviarContextoEjecucionACpu"
                recibirPaquete(fd_cpuDispatch);
                
                log_info(loggerKERNEL, "Quantum restante: [%d]", quantum);
            }
            //Siento que los valores se deberian actualizar con algun retorno de alguna funcion
            // y no solo usando el desencolarProceso, si bien aun no lo pude testear, no creo
            // que actualice correctamente los valores de pcbActual.
            // Ademas, falta hacer el free del espacio de memoria de pcbActual luego de cada uso.
            if(quantum == 0){
                
                log_info(loggerKERNEL, "El QUANTUM ha finalizado, el proceso de PID [%d] sera desalojado", pcbActual->PID);
                pcbActual = desencolarProceso(cola_planificacion_exec);
                pcbActual-> estado = READY;
                encolarProceso(colaReady, pcbActual);
            
            } else if (flagFin){
                log_info(loggerKERNEL, "El Proceso de PID [%d] ha finalizado su ejecucion.", pcbActual->PID);
                pcbActual = desencolarProceso(cola_planificacion_exec);
                pcbActual->estado = EXIT;
                encolarProceso(cola_planificacion_exit, pcbActual);
            } else{

                log_info(loggerKERNEL, "Ha llegado una interrupcion IO desde el CPU, el proceso de PID [%d] sera colocado en la cola ready auxiliar ya que no aprovecho todo el quantum restante: [%d]", pcbActual->PID, quantum);
                pcbActual = desencolarProceso(cola_planificacion_exec);
                pcbActual->estado = READY;
                encolarProceso(colaReadyAux, pcbActual);

            }
        }else{
            perror("No hay procesos en la cola de ready papu, lo siento");
            controlKey = false;
        }
    }while(controlKey);

    
   
    //Proceso ya enviado; debemos esperar a que vuelva
    /*
    No se como manejar el desalojo desde el cpu, pero si es por una interrupcion de io podriamos
    usar una especie de flag para que se active y poder hacer lo necesario de la interrupcion
    
    Tambien habria que hacerlo un while infinito para que continue para muchos procesos y lo pueda 
    manejar un hilo, cuando lo implementemos.
    De esa forma cuando se termine la planificacion y salga de ese hilo podriamos hacer un free global
    de todo lo que se uso en el algoritmo. 
    */

    

}

void* planificacionLargoPlazo(void* arg) { 
    while (1) {
        pthread_mutex_lock(&mutexPausaLP);
        while (pausedLP) {
            pthread_cond_wait(&condLP, &mutexPausaLP);
        }
        pthread_mutex_unlock(&mutexPausaLP);

        /* [Eliminación de procesos] */
        if(cola_planificacion_exit->inicio != NULL) {
            PCB* proceso = desencolarProceso(cola_planificacion_exit);
            log_warning(loggerKERNEL, "Finaliza el proceso <%d> - Motivo: SUCCESS", proceso->PID);
            // Solicitud a memoria de liberar todas las estrucutras que se asocien al proceso y marcandolas como libre.
            free(proceso); 
            cantProcesos--; 
        }
        
        if(cantProcesos < gradoMultiprogramacion){
            
            PCB* proceso = desencolarProceso(cola_planificacion_nuevo);
            proceso->estado = READY;
            encolarProceso(cola_planificacion_listo, proceso);
            log_warning(loggerKERNEL, "PID: <%d> - Estado Anterior: <NEW> - Estado Actual: <READY>", proceso->PID);

            char* listaPID = leerCola(cola_planificacion_listo);
            log_warning(loggerKERNEL, "Cola Ready <cola_planificacion_listo>: %s", listaPID);
            free(listaPID);

            sleep(5);

            cantProcesos++;

        } else {
            log_error(loggerKERNEL, "Ya no se admiten más procesos debido al GrMp!");
            sleep(5);
        }
    }
    return NULL;
}

void* planificacionCortoPlazo(void* arg) { 
    while (1) {
        pthread_mutex_lock(&mutexPausaCP);
        while (pausedCP) {
            pthread_cond_wait(&condCP, &mutexPausaCP);
        }
        pthread_mutex_unlock(&mutexPausaCP);
        int cpuDispatch_fd = conexionConCPUDispatch();
        
        
        /* Planificadores */
        /* [FIFO]:
            . Hay que mandar solo el contexto de ejecución
            . Recibir proceso desde CPU: Acá tenemos que hacer la lógica de las interfaces
            . para esto tenemos que guardar las interfaces en una lista primero, y luego ver si están conectadas
            . lo que dejo abajo es lo que había hecho en su momento para verificar las interfaces; no lo saquen xfa
        */
        planificacionFIFO(cola_planificacion_listo, cpuDispatch_fd);
        recibirPaquete(cpuDispatch_fd);
        sleep(10);
        /* Para probar el pasaje de procesos en colas y su manejo con el GrMp
        PCB* proceso = desencolarProceso(cola_planificacion_listo);
        proceso->estado = EXIT;
        encolarProceso(cola_planificacion_exit, proceso);
        
        */
    }
    return NULL;
}



void recibir_iosleep(int conexion){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion = recibir_operacion(conexion);
    uint32_t tiempo;
    uint32_t len;
    char* nombre;
    if(paquete->codigo_operacion =! "IO_CPU"){
        log_error(loggerKERNEL, "Se desconecto el cliente");
        return;
    }
    
    recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);

    paquete->buffer->stream = malloc(paquete->buffer->size);
    
    recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);

    void* stream = paquete->buffer->stream;

    memcpy(&tiempo, stream, sizeof(uint32_t)); 
    stream += sizeof(uint32_t);
    memcpy(&len, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    nombre = malloc(len);
    memcpy(nombre, stream, len);

    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    
}

void planificacionLargoPlazoAux(){
    while(cola_planificacion_nuevo->cantidad != 0 && (cola_planificacion_listo->cantidad + cola_planificacion_exec->cantidad + cola_planificacion_blocked->cantidad) < atoi(config_get_string_value(configKERNEL, "GRADO_MULTIPROGRAMACION"))){
        encolarProceso(cola_planificacion_listo,desencolarProceso(cola_planificacion_nuevo));
    }
}

// ====================================================[MAIN del Kernel]====================================================
int main(int argc, char* argv[]) {

    loggerKERNEL = log_create("kernel.log", "kernel", true, LOG_LEVEL_INFO);
    configKERNEL = config_create("/home/utnso/Desktop/tp-2024-1c-Bola-de-nieve-II/kernel/kernel.config");
    gradoMultiprogramacion = atoi(config_get_string_value(configKERNEL, "GRADO_MULTIPROGRAMACION"));

    int quantum = atoi(config_get_string_value(configKERNEL, "QUANTUM"));

    cola_planificacion_nuevo = crearCola();
    cola_planificacion_listo = crearCola();
    cola_planificacion_exec = crearCola();
    cola_planificacion_blocked = crearCola();
    cola_planificacion_exit = crearCola();

    pthread_t planiLargoPlazo;
    if(pthread_create(&planiLargoPlazo, NULL, planificacionLargoPlazo, NULL) != 0){
        log_error(loggerKERNEL, "ERROR | No se creo el hilo del planificador de Largo plazo.");
        return EXIT_FAILURE;
    }

    pthread_t planiCortoPlazo;
    if(pthread_create(&planiCortoPlazo, NULL, planificacionCortoPlazo, NULL) != 0){
        log_error(loggerKERNEL, "ERROR | No se creo el hilo del planificador de Corto plazo.");
        return EXIT_FAILURE;
    }

    lista_interfaces = list_create();
    recibirPaquete(conexionDesdeIO()); // Recibe las interfaces conectadas

    printf("================================= [Módulo: KERNEL] =================================");
    printf("\n Estas son las funciones que puede ejecutar (no pongas los corchetes!): \n 1) EJECUTAR_SCRIPT [path]. \n 2) INICIAR_PROCESO [path]. \n 3) FINALIZAR_PROCESO [pid]. \n 4) DETENER_PLANIFICACION. \n 5) INICIAR_PLANIFICACION. \n 6) MULTIPROGRAMACION [valor]. \n 7) PROCESO_ESTADO. "); 
    printf("\nIngrese una funcion (y lo que esta necesite): ");
    while(true){
        inputText textoConsola = textoIngresado();
        switch(buscarFuncion(textoConsola.nombreFuncion)) {     
            case 1: // EJECUTAR_SCRIPT [PATH]
                pathPrueba path;
                path.path = textoConsola.path_or_number;
                path.path_length = textoConsola.path_or_numberLength;
                
                int cpuDispatch_fd = conexionConCPUDispatch();
                if(cpuDispatch_fd == 0){
                    log_warning(loggerKERNEL, "Iniciaste el modulo de CPU?!");
                    continue; // Si no pongo esto, entra en un while infinito; holy shit
                } else {
                    enviarPathCPU(cpuDispatch_fd, path);
                    recibirPaquete(cpuDispatch_fd);
                    close(cpuDispatch_fd);
                }
            
                break;

            case 2: // [HECHA]
            // Requisito: INICIAR_PROCESO [PATH];
            /* . En esta función lo único que debemos hacer es crear al proceso junto al path enviado desde 'memoria'. 
               . -> Estado del proceso: 'NEW'
               . -> Ponerlo en la cola de nuevos.
            */
            int memoria_fd = conexionConMemoria();
            if(memoria_fd == 0){
                log_warning(loggerKERNEL, "Iniciaste el modulo de la memoria?!");
                continue; // Si no pongo esto, entra en un while infinito; holy shit
            } else {
                crearProceso(memoria_fd, textoConsola);
                recibirPaquete(memoria_fd); // Recibe un proceso desde memoria
                close(memoria_fd);
            }
                break;
            case 3: // FINALIZAR_PROCESO [PID]; [HECHA]
            if(atoi(textoConsola.path_or_number) <= 0) {
                printf("Ingresa un numero o algo mayor a 0: ");
            } else {
                int PIDsolicitado = atoi(textoConsola.path_or_number);
                PCB* procesoEncontrado = NULL;
                bool encontrado = false;
                int i = 0;
                ColaProcesos* arrayColas[5] = {cola_planificacion_nuevo, cola_planificacion_listo, cola_planificacion_exec, cola_planificacion_exit, cola_planificacion_blocked};
                do {
                    if(procesoEncontrado == NULL) {
                        procesoEncontrado = buscarYEliminarProcesoEnCola(arrayColas[i], PIDsolicitado);
                    } else {
                        encontrado = true;
                    }
                    i++;
                } while(!encontrado && i < 5);
                if(!encontrado) {
                    log_error(loggerKERNEL, "El proceso no fue encontrado en ninguna cola");
                } else {
                    if(procesoEncontrado->estado == EXECUTE || procesoEncontrado->estado == BLOCKED || procesoEncontrado->estado == READY) {
                        // Acá habría que replicar lo que se hace en el exit; eliminar estructuras del proceso -> mirar enunciado
                        cantProcesos--;
                    }
                }
                free(procesoEncontrado);
                   
            }        
                break;
            
            case 4:
            // DETENER_PLANIFICACIÓN;
            /* . Esta función pausa la planificacion de corto y largo plazo.
                -> Si estado del proceso es 'EXEC' no lo desalojamos.
                -> Cuando salga de 'EXEC' se lo pausa.
                -> Mismo criterio para los 'READY' o 'BLOCKED'.
            */  
                
                pausedLP = pausedCP = 1;
                
                break;

            case 5:
            // INICIAR PLANIFICACIÓN;
            /* . Retoma (si es que está pausada) la planificacion de corto y largo plazo.
                -> si no está pausada, ignorar mensaje.
                * debería mandarle el proceso al CPU en teoría
                -> mandar proceso según algortimo de planificación en 'READY' -supongo, jeje-.
            */


                pausedLP = pausedCP = 0;
                pthread_cond_signal(&condLP);
                pthread_cond_signal(&condCP);

            //Codigo para probar RR, comentar
                int socketDispatch = conexionConCPUDispatch();
                sleep(0.1);
                int socketInterrupt = conexionConCPUInterrupt();
                planificacionLargoPlazoAux();
                planificacionRR(socketDispatch,socketInterrupt);

                break;

            case 6: // [HECHA].
            if(atoi(textoConsola.path_or_number) <= 0) {
                printf("Ingresa un numero o algo mayor a 0: ");
            } else {
                gradoMultiprogramacion = atoi(textoConsola.path_or_number);
                log_warning(loggerKERNEL, "El valor del GP es: %d", gradoMultiprogramacion);
            }
                break;

            case 7:
                char* listaPID = leerCola(cola_planificacion_nuevo);
                printf("Proceso en NEW: %s\n", listaPID);
                listaPID = leerCola(cola_planificacion_listo);
                printf("Proceso en READY: %s\n", listaPID);
                listaPID = leerCola(cola_planificacion_exec);
                printf("Proceso en EXECUTION: %s\n", listaPID);
                listaPID = leerCola(cola_planificacion_blocked);
                printf("Proceso en BLOCKED: %s\n", listaPID);
                listaPID = leerCola(cola_planificacion_exit);
                printf("Proceso en EXIT: %s\n", listaPID);
                free(listaPID);
                break;

            default:
                log_error(loggerKERNEL, "Esa función no existe papu");
                break;
        }
    }
    pthread_join(planificacionCortoPlazo, NULL);
    pthread_join(planificacionLargoPlazo, NULL);
    return 0;
}