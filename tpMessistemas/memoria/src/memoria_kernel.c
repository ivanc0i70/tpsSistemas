#include <../include/memoria_kernel.h>



void atender_crear_proceso_km(t_buffer* un_buffer){
    //[int pid] [char* path] [int size]
    int pid = extraer_int_del_buffer(un_buffer);
    char* path = extraer_string_del_buffer(un_buffer);
    //int size = extraer_int_del_buffer(un_buffer);
    
    kernel_memoria_data* data_kernel =  recibir_kernel_memoria(fd_kernel);

    log_info(memoria_logger, "<PID:%d> <PATH:%s>",pid, path);

    free(path);
}



void atender_memoria_kernel(){
    bool control_key=1;
    t_buffer* un_buffer;

    while (control_key) {
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op) {
            case MENSAJE:
                // recibir_mensaje(cliente_fd);
                break;
            case PAQUETE:
                // lista = recibir_paquete(fd_kernel_dispatch);
                // log_info(cpu_logger, "Me llegaron los siguientes valores:\n");
                // list_iterate(lista, (void*) iterator);
                break;
            case CREAR_PROCESO_KERNEL_MEMORIA: //[int pid][char* path]
                log_info(memoria_logger, "Creado proceso desde el KERNEL a MEMORIA");
                un_buffer = recibir_todo_buffer(fd_kernel);
                atender_crear_proceso_km(un_buffer);
                
                break;
            case -1:
                log_error(memoria_logger, "El KERNEL se desconecto. Terminando servidor");
                control_key=0;
                // return EXIT_FAILURE;
                break;
            default:
                log_warning(memoria_logger,"Operacion desconocida de KERNEL");
                break;
            }
	}
}

