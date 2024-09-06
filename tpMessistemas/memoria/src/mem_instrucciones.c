
#include <../include/mem_instrucciones.h>



char* leer_archivo_pseudocodigo(char* archivo_a_leer){
    //leer del directorio el archivo indicado por PC
    DIR *directorio = opendir(PATH_INSTRUCCIONES);
    struct dirent *entrada;
    char* archivo_con_extension;
    asprintf(&archivo_con_extension, "%s.txt", archivo_a_leer);
    char* path = NULL;
    asprintf(&path, "%s %s", PATH_INSTRUCCIONES, archivo_a_leer);
	free(archivo_con_extension);
	closedir(dir);
    return path;
}


void atender_crear_proceso_km(t_buffer* un_buffer){
    //[int pid] [char* path] [int size]
    int pid = extraer_int_del_buffer(un_buffer);
    char* path = extraer_string_del_buffer(un_buffer);
    //int size = extraer_int_del_buffer(un_buffer);
    
    log_info(memoria_logger, "<PID:%d> <PATH:%s>",pid, path);

    free(path);

    kernel_memoria_data* data_kernel =  recibir_kernel_memoria(fd_kernel);

    switch(data_kernel->flag){
			case INICIAR_PROCESO_KERNEL: //preparo estructuras necesarias + recibo de kernel
				log_info(memoria_logger, "se recibio un iniciar procesos");
				FILE* archivo;
                char* nombre = data_kernel->nombre;
                char* path = leer_archivo_pseudocodigo(nombre);
				archivo = fopen(path, "r");

            break;
    }
    free(data_kernel);

}

