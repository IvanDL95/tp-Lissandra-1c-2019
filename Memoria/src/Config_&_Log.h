/*
 * Config_&_Log.h
 *
 *  Created on: 23 jul. 2019
 *      Author: ivan
 */

#ifndef SRC_CONFIG___LOG_H_
#define SRC_CONFIG___LOG_H_

t_log* logger;
un_socket socket_FS;

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

#endif /* SRC_CONFIG___LOG_H_ */
