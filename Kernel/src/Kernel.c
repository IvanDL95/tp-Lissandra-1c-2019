/*
 * Kernel.c
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

// This code doesn't have a license. Feel free to copy.

#include "../src/Kernel.h"

#include <stdlib.h>
#include <stdio.h>



int main(void){

	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando Kernel\n");

	get_configuracion();


return 0;
}

void get_configuracion(){
	log_info(logger, "Levantando archivo de configuracion del proceso Kernel\n");

	t_config* archivo_configuracion = config_create(pathKernelconfig);

	if (archivo_configuracion == NULL) {
		log_info(logger, "Error al abrir Archivos de Configuracion");
		return;
	} else {
		config_Kernel.IP_MEMORIA = copy_string(get_campo_config_string(archivo_configuracion, "IP_MEMORIA"));
		config_Kernel.PUERTO_MEMORIA = get_campo_config_int(archivo_configuracion, "PUERTO_MEMORIA");
		config_Kernel.QUANTUM = get_campo_config_int(archivo_configuracion, "QUANTUM");
		config_Kernel.MULTIPROCESAMIENTO = get_campo_config_int(archivo_configuracion, "MULTIPROCESAMIENTO");
		config_Kernel.METADATA_REFRESH = get_campo_config_int(archivo_configuracion, "METADATA_REFRESH");
	}

	config_destroy(archivo_configuracion);
	return;
}

