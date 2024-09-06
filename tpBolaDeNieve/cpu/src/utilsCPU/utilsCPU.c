#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/config.h>
#include <time.h>

#include "pthread.h"
#include "utilsCPU.h"

void buffer_add_contexto(t_buffer *buffer, contextoEjecución*data) {
	memcpy(buffer->stream + buffer->offset, &data, sizeof(contextoEjecución)); buffer->offset += sizeof(contextoEjecución); 
}