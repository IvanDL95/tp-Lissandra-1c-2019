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

#define TAMANIO_PAGINA (sizeof(uint32_t)+sizeof(time_t)+tamanio_value)
#define CANTIDAD_FRAMES config_MP.TAM_MEM/TAMANIO_PAGINA
#define KEY_OFFSET sizeof(uint32_t)
#define TIME_OFFSET sizeof(time_t)

t_log* logger_mem;

typedef unsigned char* byte;
static void *memoria_principal;

static bool esta_full_memoria;

typedef struct _frame{
	byte base;
	size_t offset;
}t_frame;

static t_frame* frame;
static t_bitarray* frame_bitarray;
static char* bitarray_block;

void inicializar_memoria();
void hacer_journal();

/************** SEGMENTOS ****************/

typedef t_list* tabla_paginas;

typedef struct{
	char* nombre_tabla;
	tabla_paginas tabla;
}t_segmento;

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
	t_frame* pagina;
	int nro_frame;
	flag modificado;
}t_pagina;

t_pagina* buscar_key(t_segmento*, int key);
t_frame* solicitar_pagina(t_segmento*, const char* value, int key, flag);
void actualizar_pagina(t_segmento* tabla,t_pagina*,const int key,const char* new_value);
char* obtener_valor(t_segmento* segmento, int nro_pag);
//  int asignar_key(tabla_paginas page_table);

/************* COLA LRU **************************/

t_queue* cola_LRU;

typedef struct{
	int nro_pagina;
	t_segmento* segmento;
}t_cola_LRU;

#endif /* SRC_MEMORIAPRINCIPAL_H_ */
