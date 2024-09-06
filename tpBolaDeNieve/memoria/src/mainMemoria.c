#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/config.h>
#include <commons/collections/list.h>

t_log* loggerMEMORIA;
t_config* configMEMORIA;
// =================================================[CONEXIONES C/ MÓDULOS]=================================================

void *recibirPaquete(int *arg);

void conexionDesdeModulos() {
    char* puertoMEMORIA = config_get_string_value(configMEMORIA, "PUERTO_MEMORIA");
    int serverMEMORIA_fd = iniciar_servidor(loggerMEMORIA, "MEMORIA", puertoMEMORIA);
    log_info(loggerMEMORIA, "[MEMORIA] | listo para recibir a clientes!");
    while(1) {
        pthread_t thread;
        int *fd_conexion_ptr = malloc(sizeof(int));
        *fd_conexion_ptr = accept(serverMEMORIA_fd, NULL, NULL);
        log_warning(loggerMEMORIA, "[MEMORIA] | Se conecto un cliente!");
        pthread_create(&thread, NULL, recibirPaquete, (void*)fd_conexion_ptr);
        pthread_detach(thread); 
    }
}

//[----------------------------------------------------CONEXION CON CPU----------------------------------------------------]
typedef struct{
    char linea[100];
    int num_lin;
} t_instmem;


t_instmem *crearSTRUCT(char *line, int num) {
    t_instmem *new = malloc(sizeof(t_instmem));
    strcpy(new->linea,line);
    new->num_lin = num;
    return new;
}

void iterator(t_instmem* value) {
	log_info(loggerMEMORIA,"%s,%i", value->linea, value->num_lin);
}

t_list* leer_archivo(){
    FILE*arch=fopen("/home/utnso/Desktop/tp-2024-1c-Bola-de-nieve-II/memoria/src/archivosPseudo/script_io_basico1.txt","rb+"); 
    t_instmem aux;
    aux.num_lin=0;
    t_list* lista = list_create();
    while(fgets(aux.linea,sizeof(aux.linea),arch)){
        aux.linea[strcspn(aux.linea, "\n")] = 0;
       list_add(lista,crearSTRUCT(aux.linea,aux.num_lin));
       aux.num_lin ++;

    }
     log_info(loggerMEMORIA, "lo valore:\n");
    list_iterate(lista, (void*) iterator);
    return lista;
}

typedef struct {
    op_code codigo_operacion;
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_bufferD;

void serializarLinea(char* linea, int conexion){
    
    t_bufferD* tamaño= malloc(sizeof(t_bufferD));
    uint32_t linea_length = strlen(linea) + 1;
    tamaño->size= sizeof(op_code) //opcode
            + sizeof(uint32_t) //tamaño del payload
 /*stream*/ + sizeof(uint32_t) //tamaño del char* linea
            + linea_length; //char* linea
    
    tamaño->stream = malloc(tamaño->size - sizeof(op_code) - sizeof(uint32_t)) ;
    
    tamaño->codigo_operacion = LINEA;
    memcpy(tamaño->stream, &tamaño->codigo_operacion, sizeof(op_code));
    memcpy(tamaño->stream+ sizeof(op_code), &tamaño->size, sizeof(uint32_t));
    memcpy(tamaño->stream+sizeof(op_code)+sizeof(uint32_t), &linea_length, sizeof(uint32_t));
    memcpy(tamaño->stream+sizeof(op_code)+sizeof(uint32_t)*2, linea, linea_length);
   
    send(conexion, tamaño->stream, tamaño->size, 0);
    free(tamaño->stream);
    free(linea);
    free(tamaño);
}

//[----------------------------------------------------CONEXION CON CPU----------------------------------------------------]

// ==================================================[CREACION DE PROCESO]=================================================

t_CPUregisters iniciarRegistros() {
    t_CPUregisters registros;
    registros.AX = 0;
    registros.BX = 0;
    registros.CX = 0;
    registros.DX = 0;
    registros.EAX = 0;
    registros.EBX = 0;
    registros.ECX = 0;
    registros.EDX = 0;
    registros.SI = 0;
    registros.DI = 0;
    return registros;
}
int pid = 0; 
PCB *iniciar_proceso(char* path){ // Función para crear los procesos.
    PCB *proceso = malloc(sizeof(PCB));
    if (proceso == NULL) {
        log_error(loggerMEMORIA,"Error al crear el proceso! yo que vos me procupo! jeje");
        return NULL;
    } else {
        proceso->path = path;
        proceso->path_length = strlen(path) + 1;
        proceso->PID = pid++; 
        proceso->programCounter = 0;
        proceso->quantum = 0;
        proceso->registros = iniciarRegistros();
        proceso->estado = NEW;
        return proceso;
    }
}

// ====================================================[SERIALIZACIONES]====================================================
void enviarProcesoCreado(PCB* proceso, int unSocket) { // Le envía un proceso creado al kernel
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 4 + sizeof(uint8_t) + sizeof(t_CPUregisters) + proceso->path_length);
    buffer_add_unit32(buffer, proceso->PID);
    buffer_add_unit32(buffer, proceso->quantum);
    buffer_add_unit32(buffer, proceso->programCounter);
    buffer_add_unit8(buffer, proceso->estado);
    buffer_add_cpuRegisters(buffer, proceso->registros);
    buffer_add_string(buffer, proceso->path_length, proceso->path);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PROCESO; 
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

char* deserializarPath(t_buffer* buffer) {
    inputText* textoConsola = malloc(sizeof(inputText));
    void* stream = buffer->stream;
    memcpy(&(textoConsola->path_or_numberLength), stream, sizeof(uint32_t)); stream += sizeof(uint32_t); 
    textoConsola->path_or_number = malloc(textoConsola->path_or_numberLength);
    memcpy(textoConsola->path_or_number, stream, textoConsola->path_or_numberLength);
    return textoConsola->path_or_number;
}

uint32_t* deserializarPC(t_buffer* buffer) {
    uint32_t* pc = malloc(sizeof(uint32_t));
    void* stream = buffer->stream;
    memcpy(pc, stream, sizeof(uint32_t)); stream += sizeof(uint32_t);
    return pc;
}

void *recibirPaquete(int *socketEmisor_fd) {
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    crear_buffer(paquete); int size;
    
    while(1){
        int cod_op = recibir_operacion(*socketEmisor_fd);
        switch (cod_op) {
            
        case SOLICITUD_PROCESO:
            // Acá recibimos el buffer del paquete enviado
            recv(*socketEmisor_fd, &(paquete->buffer->size), sizeof(uint32_t), 0); paquete->buffer->stream = malloc(paquete->buffer->size);
            recv(*socketEmisor_fd, paquete->buffer->stream, paquete->buffer->size, 0);
        
            char* path = deserializarPath(paquete->buffer);
            //log_info(loggerMEMORIA, "El path es: %s", path);
        
            
            PCB* proceso = iniciar_proceso(path); // <- Implementar lo de 'tabla de páginas vacías' una vez visto.
            enviarProcesoCreado(proceso, *socketEmisor_fd);
            free(path); free(proceso);
            

            break;
            
        case PROGRAM_COUNTER:
            recv(*socketEmisor_fd, &(paquete->buffer->size), sizeof(uint32_t), 0); paquete->buffer->stream = malloc(paquete->buffer->size);
            recv(*socketEmisor_fd, paquete->buffer->stream, paquete->buffer->size, 0);
            uint32_t* pc = deserializarPC(paquete->buffer);
            // Utilizar acá el PC para lo que se requiera.
            t_instmem* inst = malloc(sizeof(t_instmem));
            t_list* lista = leer_archivo();
            inst = list_get(lista,(*int)pc);
        
            serializarLinea(inst->linea,*socketEmisor_fd);
    

            free(pc);
            break;

        case -1:
            log_error(loggerMEMORIA, "El cliente se ha ido!");
            return EXIT_FAILURE;
            break;
        default:
            log_warning(loggerMEMORIA, "Operacion inchequeable");
            break;
        }
    }
    eliminar_paquete(paquete);
}


void cerrar_programa(int cliente_fd, int serverMEMORIA_fd){
    close(cliente_fd);
    close(serverMEMORIA_fd);
    config_destroy(configMEMORIA);
    log_destroy(loggerMEMORIA);
}


// ====================================================[MAIN]====================================================
int main(int argc, char* argv[]) {
    loggerMEMORIA = log_create("memoria.log", "memoria", true, LOG_LEVEL_INFO);
    configMEMORIA = config_create("/home/utnso/Desktop/tp-2024-1c-Bola-de-nieve-II/memoria/memoria.config");
    

    
    // Recibir al PC
    /*
    uint32_t* pc = recibirPC(cliente_fd);
    log_info(loggerMEMORIA, "Valor del PC: %d", *pc);
    */

    
    // CPU
   
    
    // CPU
    // Averiguar a que se refiere con Pseudocodigos en general!!!
    // sendSTREAM(cliente_fd);
    //cerrar_programa(cliente_fd, serverMEMORIA_fd);
    

    conexionDesdeModulos();
    

    return EXIT_SUCCESS;

}
