/*
 * API.h
 *
 *  Created on: 20 abr. 2019
 *      Author: ivan
 */

#ifndef API_H_
#define API_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/error.h>
#include <pthread.h>

enum API{
	SELECT,
	INSERT,
	CREATE,
	DESCRIBE,
	DROP,
	JOURNAL,
	ADD,
	RUN,
	METRICS
};

typedef enum API command_api;

int ejecutar_API(command_api);

void* iniciar_consola(t_log*);

command_api convertir_commando(char* command);

#endif /* API_H_ */
