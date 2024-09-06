#include <../include/ciclo_de_instruccion.h>


t_instruccion* convertir_data_a_instruccion(memoria_cpu_data *data_cpu) {
	t_instruccion *instruccion = malloc(sizeof(*instruccion));

	instruccion->identificador = data_cpu->programCounter;
	instruccion->parametro_1 = data_cpu->parametro_1;
	instruccion->parametro_2 = data_cpu->parametro_2;
	instruccion->parametro_3 = data_cpu->parametro_3;

	return instruccion;
}


t_instruccion* fetch(t_pcb *pcb) {
	memoria_cpu_data* data_cpu = malloc(sizeof(*data_cpu));

    data_cpu->flag = INICIAR_PROCESO_CPU;
	int pid = pcb->pid;
	data_cpu->pid = pid;
	int pc = pcb->programCounter;
	data_cpu->programCounter = pc;
	data_cpu->parametro_1 = "";
	data_cpu->parametro_2 = "";
	data_cpu->parametro_3 = "";
	data_cpu->direccion = 0;
	data_cpu->registro_valor = 0;

	enviar_cpu_memoria(data_cpu, fd_memoria, INICIAR_PROCESO_CPU);

	memoria_cpu_data *data_cpu2 = recibir_memoria_cpu(fd_memoria);

	t_instruccion *instruccion_recibida = convertir_data_a_instruccion(data_cpu2);

	pcb->programCounter += 1;

	free(data_cpu);
	free(data_cpu2);

	return instruccion_recibida;
} 



bool verifica_interrupcion() {
	bool interrupcion = false;

	pthread_mutex_lock(&mutex_interrupt);

	if (existe_interrupcion) {
		interrupcion = true;
		enviar_interrupt = true;
		existe_interrupcion = false;
	}

	pthread_mutex_unlock(&mutex_interrupt);
	return !interrupcion;
}



void set_registro_valor_uint8(registros_cpu *registros, char *registro, uint8_t valor) {
	if (strcmp(registro, "AX") == 0) {
		registros->AX = valor;
	}
	else if (strcmp(registro, "BX") == 0) {
		registros->BX = valor;
	}
	else if (strcmp(registro, "CX") == 0) {
		registros->CX = valor;
	} 
	else if (strcmp(registro, "DX") == 0) {
		registros->DX = valor;
	}
}
//FALTA REGISTROS uint32

uint8_t buscar_registro_entero_uint8(registros_cpu *registros, char *registro) {
	int valor = 0;
	char *token = strtok(registro, "\n");
	if (string_equals_ignore_case(registro, "AX"))
		valor = registros->AX;
	if (string_equals_ignore_case(registro, "BX"))
		valor = registros->BX;
	if (string_equals_ignore_case(registro, "CX"))
		valor = registros->CX;
	if (string_equals_ignore_case(registro, "DX"))
		valor = registros->DX;
	return valor;
}
//FALTA REGISTROS uint32


void decode(t_instruccion *instruccion, t_pcb *pcb) {

	switch (instruccion->identificador) {

	case SET:
		log_info(cpu_logger, "PID: %d - Ejecutando: SET - %s - %s", pcb->pid, instruccion->parametro_1, instruccion->parametro_2);
		set_registro_valor_uint8(pcb->registros_cpu, instruccion->parametro_1, atoi(instruccion->parametro_2));
		break;

	case SUM:
		log_info(cpu_logger, "PID: %d - Ejecutando: SUM - %s - %s", pcb->pid, instruccion->parametro_1, instruccion->parametro_2);

		uint8_t destino_int = buscar_registro_entero_uint8(pcb->registros_cpu, instruccion->parametro_1);
		uint8_t origen_int = buscar_registro_entero_uint8(pcb->registros_cpu, instruccion->parametro_2);

		uint8_t suma = destino_int + origen_int;

		set_registro_valor_uint8(pcb->registros_cpu, instruccion->parametro_1, suma);

		break;

	case SUB:
		log_info(cpu_logger, "PID: %d - Ejecutando: SUB - %s - %s", pcb->pid, instruccion->parametro_1, instruccion->parametro_2);
		uint8_t destino = buscar_registro_entero_uint8(pcb->registros_cpu, instruccion->parametro_1);
		uint8_t origen = buscar_registro_entero_uint8(pcb->registros_cpu, instruccion->parametro_2);

		uint8_t resta = destino - origen;

		set_registro_valor_uint8(pcb->registros_cpu, instruccion->parametro_1, resta);
		break;

	case JNZ:
		log_info(cpu_logger, "PID: %d - Ejecutando: JNZ - %s - %s", pcb->pid, instruccion->parametro_1, instruccion->parametro_2);
		uint8_t valor = buscar_registro_entero_uint8(pcb->registros_cpu, instruccion->parametro_1);

		if (valor != 0) {
			pcb->programCounter = atoi(instruccion->parametro_2);
		}
		break;

	case IO_GEN_SLEEP:
		log_info(cpu_logger, "PID: %d - Ejecutando: SLEEP - %s ", pcb->pid, instruccion->parametro_1);
		pcb->sleep = atoi(instruccion->parametro_1);
		enviar_cpu_kernel(pcb, fd_kernel_dispatch, i_IO_GEN_SLEEP);
		break;

	case EXIT:
		log_info(cpu_logger, "PID: %d - Ejecutando: EXIT ", pcb->pid);
		enviar_cpu_kernel(pcb, fd_kernel_dispatch, i_EXIT);
		break;

	default:
		break;
	}
}


bool condicion_continuar_ejecucion(t_identificador identificador) {
	if (identificador == SET 
	|| identificador == SUM
	|| identificador == SUB
	|| identificador == JNZ
	|| identificador == IO_GEN_SLEEP) {
		return true;
	}
	return false;
}



//FALTA LOGGERS
void ejecutar_ciclo_de_instruccion(pcb *pcb, int fd_kernel_dispatch) {

	bool continua_ejecucion = 1;

	while (continua_ejecucion && verifica_interrupcion()) {
		t_instruccion *instruccion = fetch(pcb);
		decode(instruccion, pcb);

		if (condicion_continuar_ejecucion(instruccion->identificador)) {
			continua_ejecucion = 0;
		}
		free(instruccion);
	}

	enviar_cpu_kernel(pcb, fd_kernel_dispatch, INTERRUPT_CPU_EXITO);
	enviar_interrupt = false;
}

