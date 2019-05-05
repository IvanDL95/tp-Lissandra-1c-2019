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


typedef enum API command_api;

t_log* logger;
char* pathKernelconfig = "KernelConfig.cfg";

typedef struct {
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	int QUANTUM;
	int MULTIPROCESAMIENTO;
	int METADATA_REFRESH;
} Configuracion_Kernel;

Configuracion_Kernel config_Kernel;

un_socket socket_Memoria = 0;

int tamanio_value;

int conectar_con_Memoria();

void mostrarMetricas();

//void iniciar_consola();

//command_api convertir_commando(char*);

#endif /* SRC_KERNEL_H_ */

