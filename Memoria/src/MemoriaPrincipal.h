/*
 * MemoriaPrincipal.h
 *
 *  Created on: 23 jul. 2019
 *      Author: ivan
 */

#ifndef SRC_MEMORIAPRINCIPAL_H_
#define SRC_MEMORIAPRINCIPAL_H_

#include <Libraries.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <sys/time.h>
#include <time.h>
#include "Config_&_Log.h"

int tamanio_value;

#define KEY_T uint16_t
#define TIME_T uint64_t
#define KEY_OFFSET sizeof(KEY_T)
#define TIME_OFFSET sizeof(TIME_T)
#define TAMANIO_PAGINA (sizeof(KEY_T)+sizeof(TIME_T)+tamanio_value)
#define CANTIDAD_FRAMES config_MP.TAM_MEM/TAMANIO_PAGINA


t_log* logger_mem;

typedef char* byte;
static void *memoria_principal;

static bool esta_full_memoria;

typedef struct _frame{
	byte base;
	uint16_t nro_frame;
	size_t offset;
}t_frame;

static t_frame* frame;
t_bitarray* frame_bitarray;
static char* bitarray_block;

void inicializar_memoria();
void hacer_journal();

/************** SEGMENTOS ****************/

typedef t_list* tabla_paginas;

typedef struct{
	char* nombre_tabla;
	tabla_paginas tabla;
}t_segmento;

struct mem_struct{
	KEY_T key;
	TIME_T timestamp;
	char* value;
};

t_list* tabla_segmentos;

t_segmento* crear_nuevo_segmento(const char*);
void destruir_segmento(char*);
t_segmento* buscar_segmento(const char* nombre_tabla);

/*********************** PAGINAS ***********************/

enum bit_modificado{
	NO_MODIFICADO,
	MODIFICADO
};
typedef enum bit_modificado flag;

typedef struct{
	unsigned int nro;
	t_frame* frame;
	flag modificado;
}t_pagina;

t_pagina* buscar_key(t_segmento*, KEY_T);
t_pagina* solicitar_pagina(t_segmento*, const char* value, const KEY_T);
void actualizar_pagina(t_segmento*,t_pagina*,const KEY_T,const char* new_value);
char* obtener_valor(t_segmento*, int nro_pag);
TIME_T obtener_timestamp(t_segmento*, int nro_pag);
KEY_T obtener_key(t_pagina*);
//  int asignar_key(tabla_paginas page_table);

/************* COLA LRU **************************/

t_queue* cola_LRU;

typedef struct{
	int nro_pagina;
	t_segmento* segmento;
}t_cola_LRU;

t_cola_LRU* revisar_cola(const char* nombre_tabla, KEY_T);

#endif /* SRC_MEMORIAPRINCIPAL_H_ */
