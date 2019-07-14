/*
 * Kernel.h
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

#ifndef SRC_KERNEL_H_
#define SRC_KERNEL_H_

#include <Libraries.h>
#include <API.h>
#include <commons/log.h>
#include "headers/structs.h"
#include "Planificacion.h"

typedef enum API command_api;

t_log* logger;

enum Criterios_Memoria{
	Null,
	SC, //Criterio Strong Consistency
	SHC, //Criterio Strong-Hash Consistency
	EC //Criterio Eventual Consistency
};

typedef enum Criterios_Memoria criterio_memoria;

typedef struct {
	int id;
	char *ip;
	int puerto;
	criterio_memoria criterio;
} t_memorias;

t_list* memoriasSC;
t_list* memoriasEC;
t_list* memoriasSHC;

t_requestAMemoria requestAPlanificar;

int tamanio_value;

t_requestAMemoria crearEstructuraRequest(t_list*, command_api);

int conectar_con_Memoria();

void parsear_archivo_lql(char*);

void mostrarMetricas();

void asignar_memoria_inicial();

pthread_t thread_parser;


#endif /* SRC_KERNEL_H_ */

