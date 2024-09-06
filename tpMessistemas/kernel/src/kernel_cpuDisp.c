#include <../include/kernel_cpuDisp.h>

void atender_kernel_cpuD(){

   bool control_key=1;

    while (control_key) {
		int cod_op = recibir_operacion(fd_cpu_dispatch);
		switch (cod_op) {
            case MENSAJE:

                break;
            case PAQUETE:

                break;
            case -1:
                log_error(kernel_logger, "El CPU Dispatch se desconecto. Terminando servidor");
                control_key=0;

                break;
            default:
                log_warning(kernel_logger,"Operacion desconocida de CPU Dispatch.");
                break;
            }
	}
}