/*
 * Planificador.h
 *
 *  Created on: 9 jun. 2019
 *      Author: Alejandro González
 */

#ifndef SRC_PLANIFICACION_H_
#define SRC_PLANIFICACION_H_

#include <commons/collections/list.h>
#include "headers/structs.h"
#include <pthread.h>
#include <stdio.h>
#include "API.h"

t_list* colaNew;
t_list* colaReady;
t_list* colaExec;
t_list* colaExit;

typedef enum{
	NEW,
	READY,
	EXEC,
	EXIT
} estadosDeScript;

typedef enum{
	REQUEST_API,
	ARCHIVO
} tipoRequest;

typedef struct{
	int scriptID;
	tipoRequest tipo;
	t_requestAMemoria dataRequest;
	FILE* rutaDelArchivo;
	int punteroProximaLinea;
	estadosDeScript estado;
}t_SCB; //Script Control Block

t_SCB data;

int planificador(int[]);

int planificarRequest(t_requestAMemoria);

t_SCB crearEstructuraPlanificacion(t_requestAMemoria);

int ejecutarRequest(t_requestAMemoria);

#endif /* SRC_PLANIFICACION_H_ */
