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

#define TAMANIO_PAGINA (sizeof(int)+sizeof(time_t)+tamanio_value)
#define CANTIDAD_FRAMES config_MP.TAM_MEM/TAMANIO_PAGINA



typedef char byte;
static byte *memoria_principal;
t_bitarray* frame_bitarray;
bool esta_full_memoria;

typedef struct _frame{
    struct _frame *next;
    byte* memAddr;
}t_frame;

t_frame* frame;

/*
typedef struct{
	//TODO ver cuanto mide value luego de obtener el tamaño
	int key;
	time_t timestamp;
	void* value;
}t_frame;
*/


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
void hacer_journal();

/************* COLA LRU **************************/

t_queue* cola_LRU;

typedef struct{
	int nro_pagina;
	t_segmento* segmento;
}t_cola_LRU;

#endif /* SRC_MEMORIAPRINCIPAL_H_ */
