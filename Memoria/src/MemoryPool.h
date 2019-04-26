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

t_log* logger;
char* pathMemoriaConfig = "MemoriaConfig.cfg";

pthread_t hilo_server,hilo_consola;

typedef struct {
	uint16_t PUERTO_ESCUCHA;
	char* IP_FS;
	uint16_t PUERTO_FS;
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

un_socket FS = 0;
t_list* tabla_gossiping;
int tamanio_value;
void conectarse_con_FS(un_socket);


void inicializar_memoria();
typedef int* Memoria_principal;

Memoria_principal memoria_principal;

typedef struct{
	int key;
	unsigned int timestamp;
	int value;
}t_registro;

typedef t_list* tabla_paginas;
typedef struct{
	int nro_pagina;
	t_registro* registro;
}t_pagina;

t_list* tabla_segmentos;
typedef struct{
	int nro_segmento;
	tabla_paginas tabla;
}t_segmento;


#endif /* SRC_MEMORYPOOL_H_ */
