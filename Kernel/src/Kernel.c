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

	iniciar_consola();


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

void iniciar_consola() {

	char* comando_consola;
	char* comando[5];
	char* command_quit = "Quit";
	int j;

	while (1) {
		comando_consola = NULL;
		comando_consola = readline("Kernel_bash:>");
	        if (strlen(comando_consola)==0 || strcmp(comando_consola,command_quit) == 0) {
	        	break;
	        } else {
	        	log_info(logger, comando_consola);
	        	j=0;
	        	comando[j] = strtok(comando_consola, " ");
	        	while(comando[j] != NULL && j < 5) {
	        		//printf("\n%s", comando[j]);
	        		j++;
	        		comando[j] = strtok(NULL, " ");
	        	}
	        	switch (convertir_commando(comando[0])){
	        	case SELECT:
	        		printf("\nEjecutando SELECT\n");
	        		break;
	        	case INSERT:
	        		printf("\nEjecutando INSERT\n");
	        		break;
	        	case CREATE:
	        		printf("\nEjecutando CREATE\n");
	        		break;
	        	case DESCRIBE:
	        		printf("\nEjecutando DESCRIBE\n");
	        		break;
	        	case DROP:
	        		printf("\nEjecutando DROP\n");
	        		break;
	        	case JOURNAL:
	        		printf("\nEjecutando JOURNAL\n");
	        		break;
	        	case ADD:
	        		printf("\nEjecutando ADD\n");
	        		break;
	        	case RUN:
	        		printf("\nEjecutando RUN\n");
	        		break;
	        	case METRICS:
	        		printf("\nEjecutando METRICS\n");
	        		break;
	        	default:
	        		printf("\nComando no reconocido\n\n");
	        	}
	        	free(comando_consola);
	        }

	    }

	return;
}

command_api convertir_commando(char* command) {
	    static struct comm_api {
	        const char *key;
	        command_api token;
	    } token_table[] = {
	        { "SELECT", SELECT },
	        { "INSERT", INSERT },
			{ "CREATE", CREATE },
			{ "DESCRIBE", DESCRIBE },
			{ "DROP", DROP },
			{ "JOURNAL", JOURNAL },
			{ "ADD", ADD },
			{ "RUN", RUN },
			{ "METRICS", METRICS },
	        { NULL, 99 }
	    };
	    struct comm_api *p = token_table;
	    for(; p->key != NULL && strcmp(p->key, command) != 0; ++p);
	    return p->token;
}
