#include <../include/kernel_memoria.h>

void atender_kernel_memoria(){

    bool control_key=1;
//    t_buffer* un_buffer;
    
    while (control_key) {
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op) {
            case MENSAJE:

                break;
            case PAQUETE:

                break;
            // case CREAR_PROCESO_KERNEL_MEMORIA:
            //     log_info(kernel_logger, "Creado proceso desde el KERNEL a MEMORIA");
            //     un_buffer = recibir_todo_buffer(fd_memoria);


            //     break;
            case -1:
                log_error(kernel_logger, "La MEMORIA se desconecto. Terminando servidor");
                control_key=0;

                break;
            default:
                log_warning(kernel_logger,"Operacion desconocida de MEMORIA");
                break;
            }
	}
}