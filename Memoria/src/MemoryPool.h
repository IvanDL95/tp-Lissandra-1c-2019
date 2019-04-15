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
#include <commons/log.h>

t_log* logger;
char* pathMemoriaConfig = "MemoriaConfig.cfg";

typedef struct {
	char* PUERTO_ESCUCHA;
	char* IP_FS;
	char* PUERTO_FS;
	char** IP_SEEDS;
	char** PUERTO_SEEDS;
	int RETARDO_MEM;
	int RETARDO_FS;
	int TAM_MEM;
	int RETARDO_JOURNAL;
	int RETARDO_GOSSIPING;
	int MEMORY_NUMBER;
} Configuracion_MP;

Configuracion_MP config_MP;

int maximo_value;

#endif /* SRC_MEMORYPOOL_H_ */
