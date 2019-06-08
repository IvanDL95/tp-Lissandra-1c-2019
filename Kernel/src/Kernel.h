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
#include <commons/collections/queue.h>

typedef enum API command_api;

t_log* logger;

typedef struct {
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	int QUANTUM;
	int MULTIPROCESAMIENTO;
	int METADATA_REFRESH;
} Configuracion_Kernel;

enum Criterios_Memoria{
	Null,
	SC, //Criterio Strong Consistency
	SHC, //Criterio Strong-Hash Consistency
	EC //Criterio Eventual Consistency
};

typedef enum Criterios_Memoria criterio_memoria;
Configuracion_Kernel config_Kernel;

un_socket socket_Memoria = 0;

int tamanio_value;

int conectar_con_Memoria();

void parsear_archivo_lql(char*);

void mostrarMetricas();

void asignar_Criterios_Memoria();

//void iniciar_consola();

//command_api convertir_commando(char*);

pthread_t thread_parser;

t_queue queue_Ready;

#endif /* SRC_KERNEL_H_ */

