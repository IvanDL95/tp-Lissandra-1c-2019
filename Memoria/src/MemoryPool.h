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
#include <commons/log.h>
#include <pthread.h>
#define IP "127.0.0.1"

t_log* logger;
pthread_t hilo_server,hilo_consola,hilo_gossiping;
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
	unsigned long int timestamp;
	char* value;
}t_pagina;


static int tamanio_base_pagina = sizeof(int) + sizeof(long int);

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


/* Memoria Principañ */
//Frame == ptr Página -> t_pagina
typedef t_pagina t_frame;

typedef t_frame* array_de_frames;
array_de_frames memoria_principal;


#endif /* SRC_MEMORYPOOL_H_ */
