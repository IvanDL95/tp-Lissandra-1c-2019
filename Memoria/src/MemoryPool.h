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

pthread_t hilo_server,hilo_consola;
un_socket socket_FS = 0;
t_list* tabla_gossiping;
int tamanio_value;
int conectarse_con_FS();

enum bit_modificado{
	NO_MODIFICADO,
	MODIFICADO
};
typedef enum bit_modificado flag;


typedef struct{
	int numero_memoria;
	char* dir_IP;
	char* Puerto;
	un_socket* conexion;
}t_gossip;

t_list* iniciar_gossiping();

typedef struct{
	//TODO ver cuanto mide value luego de obtener el tamaño
	int key;
	unsigned long int timestamp;
	char* value;
}t_pagina;

//Frame == ptr Página -> t_pagina == 16 bytes
typedef t_pagina* t_frame;

typedef t_frame* array_de_frames;
array_de_frames memoria_principal;
void inicializar_memoria();

t_list* tabla_segmentos;
typedef t_list* tabla_paginas;
typedef tabla_paginas* t_segmento;
typedef struct{
	t_pagina* pagina;
	flag modificado;
}t_registro;

#endif /* SRC_MEMORYPOOL_H_ */
