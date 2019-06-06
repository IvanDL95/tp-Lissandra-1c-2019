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
#include <commons/temporal.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/timeb.h>
#define IP "127.0.0.1"

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


enum bit_modificado{
	NO_MODIFICADO,
	MODIFICADO
};
typedef enum bit_modificado flag;


typedef struct{
	//TODO ver cuanto mide value luego de obtener el tamaño
	int key;
	time_t timestamp;
	char* value;
}t_pagina;

typedef struct{
	t_pagina* pagina;
	flag modificado;
}t_registro;

typedef t_list* tabla_paginas;

typedef struct{
	char* nombre_tabla;
	tabla_paginas tabla;
}t_segmento;

t_list* tabla_segmentos;


/* Memoria Principal */
//Frame == ptr Página -> t_pagina
typedef t_pagina t_frame;

typedef t_frame* array_de_frames;
array_de_frames memoria_principal;


#endif /* SRC_MEMORYPOOL_H_ */
