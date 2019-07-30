/*
 * structs.h
 *
 *  Created on: 21 jun. 2019
 *      Author: Alejandro González
 */

#ifndef SRC_HEADERS_STRUCTS_H_
#define SRC_HEADERS_STRUCTS_H_

#include <Libraries.h>
#include <API.h>

typedef enum API command_api;

typedef struct {
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	int QUANTUM;
	int MULTIPROCESAMIENTO;
	int METADATA_REFRESH;
} Configuracion_Kernel;

typedef struct {
	int socketMemoria;
	t_list* listaArgumentos;
	command_api comando;

} t_requestAMemoria;

#endif /* SRC_HEADERS_STRUCTS_H_ */
