#include "../include/consola.h"


void iniciar_consola_funciones(){

    printf("ingrese un comando: \n");
    char* leido = readline("> ");
   // bool validacion_leido;
/*
    while(strcmp(leido, "\0") != 0){
        validacion_leido = validacion_de_instruccion_de_consola(leido);
        if(!validacion_leido){
            log_error(kernel_logger, "Comando DESCONOCIDO!!");
            free(leido);
            leido = readline("> ");
            continue; //saltea y continua con la iteracion
        }
*/
        atender_instruccion_validada(leido);
        free(leido);
        leido = readline("> ");
    
    free(leido);
}
/*
bool validacion_de_instruccion_de_consola(char* leido){
    bool resultado_validacion = false;

    //Falta validacion de los demas parametros [PATH], [PID], [VALOR]
    // cantidad de parametros, no haya solo espacio entre parametros, que cada parametro respete su tipo

    char** comando_consola = string_split(leido, " ");

    if(strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "INICIAR_PROCESO") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){
        resultado_validacion = true;
    }else if(strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){
        resultado_validacion = true;
    }else {
        log_error(kernel_logger, "Comando Desconocido NO VALIDADO!!");
        resultado_validacion = false;
    }

    string_array_destroy(comando_consola);

    return resultado_validacion;
}
*/


void f_iniciar_proceso(t_buffer* un_buffer){
    
    printf("Llega a iniciar proceso de envio de paquete");

    char* path = extraer_string_del_buffer(un_buffer);
    //char* size = extraer_string_del_buffer(un_buffer);

    log_trace(kernel_logger, "[PATH:%s]", path);
    
    //destruir_buffer(un_buffer);

    int pid = asignar_pid();
    //int size_num = atoi(size);f_iniciar_proceso
   

    //avisar a memoria [int pid] [char* path]

    t_buffer* buffer_enviar = crear_buffer();
    cargar_int_al_buffer(buffer_enviar, pid);
    cargar_string_al_buffer(buffer_enviar, path);
    //cargar_int_al_buffer(un_buffer, size_num);

    t_paquete* un_paquete = crear_paquete(CREAR_PROCESO_KERNEL_MEMORIA, buffer_enviar);
    enviar_paquete(un_paquete, fd_memoria);
    destruir_paquete(un_paquete);

    printf("Llega a enviar el paquete");

    //hacer el resto de logica para KERNEL
}


void atender_instruccion_validada (char* leido){
    char** comando_consola = string_split(leido, " ");
//pthread_t un_hilo;
    t_buffer* un_buffer = crear_buffer();

    printf("Llega a atender la conexión");

    if(strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0){
    //    cargar_string_al_buffer(un_buffer, comando_consola[1]); //[path]
    //    f_ejecutar_script(un_buffer);
    //pthread_create(&un_hilo, NULL, (void*)f_ejecutar_script, un_buffer);
    //pthread_detach(un_hilo);
    }else if(strcmp(comando_consola[0], "INICIAR_PROCESO") == 0){
        cargar_string_al_buffer(un_buffer, comando_consola[1]);   //[path]
        f_iniciar_proceso(un_buffer);

    }else if(strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){
       
    }else if(strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){
        
    }else if(strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){
        
    }else if(strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){
        
    }else if(strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){
        
    }else {
        log_error(kernel_logger, "Comando Desconocido PERO VALIDADO!!");
        exit(EXIT_FAILURE);
    }

string_array_destroy(comando_consola);

}