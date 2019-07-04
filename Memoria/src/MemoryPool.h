/*
 * MemoryPool.h
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

#ifndef SRC_MEMORYPOOL_H_
#define SRC_MEMORYPOOL_H_

#include <Libraries.h>
#include <API.h>
#include <ClientServer.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/timeb.h>
#define IP "127.0.0.1"
#define TAMANIO_PAGINA (sizeof(int)+sizeof(time_t)+tamanio_value)
#define CANTIDAD_FRAMES config_MP.TAM_MEM/TAMANIO_PAGINA

t_log* logger;
pthread_mutex_t mutex_logger;
un_socket socket_FS;
int tamanio_value;

typedef struct{
	int numero_memoria;
	char* dir_IP;
	char* Puerto;
	un_socket conexion;
}t_gossip;

typedef struct {
	char* PUERTO_ESCUCHA;
	char* IP_FS;
	char* PUERTO_FS;
	char** IP_SEEDS;
	char** PUERTO_SEEDS;
	unsigned int RETARDO_MEM;
	unsigned int RETARDO_FS;
	unsigned int TAM_MEM;
	unsigned int RETARDO_JOURNAL;
	unsigned int RETARDO_GOSSIPING;
	int MEMORY_NUMBER;
} Configuracion_MP;

Configuracion_MP config_MP;

/*
enum flag_full{
	NOT_FULL,
	FULL
};

typedef enum flag_full estado_memoria;

estado_memoria ocuapdo;
*/
bool esta_full_memoria;

enum bit_modificado{
	NO_MODIFICADO,
	MODIFICADO
};
typedef enum bit_modificado flag;


typedef struct{
	//TODO ver cuanto mide value luego de obtener el tamaño
	int key;
	time_t timestamp;
	void* value;
}t_frame;

typedef struct{
	t_frame* pagina;
	flag modificado;
}t_pagina;

typedef t_list* tabla_paginas;

typedef struct{
	char* nombre_tabla;
	tabla_paginas tabla;
}t_segmento;

t_list* tabla_segmentos;


/* Memoria Principal */
//Frame == ptr Página -> t_pagina

typedef t_frame* array_de_frames;
array_de_frames memoria_principal;

t_queue* cola_LRU;

typedef struct{
	int nro_pagina;
	char* nombre_tabla;
}t_cola_LRU;

#endif /* SRC_MEMORYPOOL_H_ */
