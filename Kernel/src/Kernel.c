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
#include "Globals.h"


int main(int argc, char** argv){

	int arg_planificacion[2];
	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando Kernel\n");

	get_configuracion(argv[1]);

	inicializar_memorias();

	if (conectar_con_Memoria(config_Kernel.IP_MEMORIA, config_Kernel.PUERTO_MEMORIA) == -1) return -1;

	arg_planificacion[0] = config_Kernel.QUANTUM;
	arg_planificacion[1] = config_Kernel.MULTIPROCESAMIENTO;
	pthread_create(&thread_planificador, NULL, (void*) planificador, arg_planificacion);

	iniciar_consola(logger);

	terminar_programa(logger,NULL);
}

void get_configuracion(char* ruta){
	log_info(logger, "Levantando archivo de configuracion del proceso Kernel\n");

	t_config* archivo_configuracion = config_create(ruta);

	if (archivo_configuracion == NULL) {
		log_info(logger, "Error al abrir Archivos de Configuracion");
		return;
	} else {
		config_Kernel.IP_MEMORIA = copy_string(get_campo_config_string(archivo_configuracion, "IP_MEMORIA"));
		config_Kernel.PUERTO_MEMORIA = copy_string(get_campo_config_string(archivo_configuracion, "PUERTO_MEMORIA"));
		config_Kernel.QUANTUM = get_campo_config_int(archivo_configuracion, "QUANTUM");
		config_Kernel.MULTIPROCESAMIENTO = get_campo_config_int(archivo_configuracion, "MULTIPROCESAMIENTO");
		config_Kernel.METADATA_REFRESH = get_campo_config_int(archivo_configuracion, "METADATA_REFRESH");
	}

	puts("\n");
	config_destroy(archivo_configuracion);
	return;
}

char* ejecutar_API(command_api operacion, char** argumentos){
	log_debug(logger, "Ejecutando la API\n");
	t_list * lista_argumentos = list_create();
	int i = 0;
	while(argumentos[i] != NULL) {
		list_add(lista_argumentos, argumentos[i]);
		i++;
	}

	requestAPlanificar = crearEstructuraRequest(lista_argumentos, operacion);

	switch(operacion){
		case SELECT:
			log_info(logger, "Planificando Request Select \n");
			if (planificarRequest(requestAPlanificar) != 0 )
				log_error(logger, "Error al Planificar Request");
			break;
		case INSERT:
			log_info(logger, "Enviando comando INSERT a la Memoria\n");
			if (planificarRequest(requestAPlanificar) != 0 )
				log_error(logger, "Error al Planificar Request");
			break;
		case CREATE:
			log_info(logger, "Enviando comando CREATE a la Memoria\n");
			if (planificarRequest(requestAPlanificar) != 0 )
				log_error(logger, "Error al Planificar Request");
			break;
		case DESCRIBE:
			log_info(logger, "Enviando comando DESCRIBE a la Memoria\n");
			break;
		case DROP:
			log_info(logger, "Enviando comando DROP a la Memoria\n");
			//enviar_listado_de_strings(socket_Memoria, lista_argumentos, DROP);
			break;
		case JOURNAL:
			log_info(logger, "Ejecutar JOURNAL en cada Memoria Asociada\n");
			journal_memorias();
			break;
		case ADD:
			log_info(logger, "Ejecutando comando ADD\n");
//			for(int cont=0; cont <= i; cont++){
//				if(strlen(argumentos[cont]) < 1) {
//					log_error(logger, "No se pudo asignar Memoria - Cantidad de Argumentos incorrectos");
//					break;
//				}
//			}
			if (asignar_memoria_criterio(argumentos) != 0 )
				log_error(logger, "No se pudo asignar Memoria");
			break;
		case RUN:
			log_info(logger, "Ejecutando RUN - Parseando Archivo LQL");
			//pthread_create(&thread_parser, NULL, (void*) parsear_archivo_lql, argumentos[0]);
			parsear_archivo_lql(argumentos[0]);

			break;
		case METRICS:
			printf("\nEjecutando METRICS\n");
			mostrarMetricas();
			break;
		default:
			printf("\nComando no reconocido\n\n");
	}
	return NULL;
}

int conectar_con_Memoria(char* ip_memoria, char* puerto_memoria){
	t_memoria* nueva_memoria;
	int un_socket;
	un_socket = conectar_a(ip_memoria, puerto_memoria);

	if(un_socket != -1){
		log_info(logger, "Conectado a la Memoria en %s:%s / socket:%d",ip_memoria,puerto_memoria,un_socket);
	}else{
		log_error(logger, "No se pudo conectar a la Memoria.\n");
		return -1;
	}

	if (realizar_handshake(un_socket)) //recibir TAMANIO_VALUE
		log_info(logger,"Handshake con Memoria : Realizado");
	else
		log_error(logger, "Handshake con Memoria : No Realizado");
	t_paquete* paquete_recibido = malloc(sizeof(t_paquete));
	// Debo recibir a qué memoria me estoy conectando
	/* Ver de recibir el paquete de vuelta */
//	paquete_recibido = recibir(socket_Memoria);
//
//	if(paquete_recibido->codigo_operacion == cop_ok){
//			tamanio_value = deserializar_int(paquete_recibido->data, 0);
//	}

	nueva_memoria->id = 1;
	nueva_memoria->ip = ip_memoria;
	nueva_memoria->puerto = atoi(puerto_memoria);
	nueva_memoria->socket_memoria = un_socket;

	asignar_memoria(nueva_memoria);

	liberar_paquete(paquete_recibido);

	return 0;
}

int asignar_memoria_criterio(char** argumentos) {
	int index_argumentos = 0;
	int index_mem;
	int num_memoria = atoi(argumentos[1]);
	string_to_upper(argumentos[0]);
	string_to_upper(argumentos[2]);
	string_to_upper(argumentos[3]);
	criterio_memoria criterio_a_asignar = Null;
	t_memoria* memoria_encontrada;
	switch(index_argumentos) {
	case 0:
		if(strcmp("MEMORIA", argumentos[0]) != 0) {
			return -1;
		}
		index_argumentos++; // @suppress("No break at end of case")
	case 1:
		if(num_memoria < 1) {
			return -1;
		}
		index_argumentos++; // @suppress("No break at end of case")
	case 2:
		if(strcmp("TO", argumentos[2]) != 0) {
			return -1;
		}
		index_argumentos++; // @suppress("No break at end of case")
	case 3:
		criterio_a_asignar = convertir_string_criterio(argumentos[3]);
		if(criterio_a_asignar == Null) {
			return -1;
		}
		break;
	}

	index_mem = verificar_existe_memoria(num_memoria);
	if( index_mem < 0){
		log_error(logger, "Memoria No Reconocida");
		return -1;
	}

	memoria_encontrada = list_get(memorias, index_mem);

	switch(criterio_a_asignar) {
	case SC:
		list_add(memoriasSC, memoria_encontrada);
		log_info(logger, "Memoria asignada a criterio SC");
		break;
	case SHC:
		list_add(memoriasSC, memoria_encontrada);
		log_info(logger, "Memoria asignada a criterio SHC");
		break;
	case EC:
		list_add(memoriasSC, memoria_encontrada);
		log_info(logger, "Memoria asignada a criterio EC");
		break;
	case Null:
		break;
	}
	return 0;
}

criterio_memoria convertir_string_criterio(char* string_convertir) {
	static struct criterio_mem {
		const char *key;
		criterio_memoria token;
	} token_table[] = {
		{ "SC", SC },
		{ "SHC", SHC },
		{ "EC", EC },
		{ NULL, Null }
	};
	struct criterio_mem *p = token_table;
	for(; p->key != NULL && strcmp(p->key, string_convertir) != 0; ++p);
	return p->token;
}

int verificar_existe_memoria(int numero_memoria){
	int cantidad_memorias = 0;
	int index_mem = 0;
	t_memoria* memoria_encontrada;
	cantidad_memorias = list_size(memorias);
	printf("\ncantidad_memorias = %i\n", cantidad_memorias);
	if (cantidad_memorias < 1) {
		log_error(logger, "No hay memorias Disponibles - Ejecutar comando ADD");
		return -1;
	}

	while(index_mem + 1 <= cantidad_memorias) {
		memoria_encontrada = list_get(memorias, index_mem);
		if (memoria_encontrada->id == numero_memoria)
			return index_mem;
		else
			index_mem++;
	}
	return -1;
}

void inicializar_memorias(){

	memorias = list_create();
	memoriasSC = list_create();
	memoriasEC = list_create();
	memoriasSHC = list_create();
	return;
}

void asignar_memoria(t_memoria* estructura_memoria) {
	t_memoria* estructura_prueba;
	list_add(memorias, estructura_memoria);
	log_info(logger, "Memoria asignada a lista de Memorias");
	estructura_prueba = list_get(memorias, 0);
	return;
}

//------------ Funciones de API ------------//

void parsear_archivo_lql(char* path_archivo_lql) {
	char sentencia_lql[1000];
	char* comando[20];
	char* string_value;
	char* aux;
	FILE *archivo_lql;
	t_SCB SCB;

	if ((archivo_lql = fopen(path_archivo_lql, "r")) == NULL)
	{
		log_error(logger, "No se pudo abrir el Archivo : %s", path_archivo_lql);
		// Program exits if file pointer returns NULL.
		//return;
	}

	SCB.estado = NEW;
	SCB.punteroProximaLinea = 1;
	SCB.rutaDelArchivo = archivo_lql;
	SCB.scriptID = 1;

	//enviarAPlanificador(SCB);


	while ( fgets ( sentencia_lql, sizeof(sentencia_lql), archivo_lql ) != NULL ) {
		printf("Data from the file: %s \n", sentencia_lql);
		log_info(logger, sentencia_lql);
		aux = calloc(strlen(sentencia_lql)+1, sizeof(char));
		strcpy(aux, sentencia_lql);
		int j=0;
		comando[j] = strtok(sentencia_lql, " ");
		while(comando[j] != NULL && j < 20) {
			j++;
			comando[j] = strtok(NULL, " ");
		}

		char* argumentos[4];
		for(int i=0;i<4 && comando[i+1] != NULL;i++){
			argumentos[i] = comando[i+1];
		}
		string_to_upper(comando[0]);

		command_api operacion = convertir_commando(comando[0]);

		if(operacion == INSERT) {
			string_value = strtok(aux, "\"");
			string_value = strtok(NULL, "\"");
			strcpy(argumentos[2], string_value);
			argumentos[3] = NULL;
		}

		ejecutar_API(operacion, argumentos);
	}
//	while ( archivo_lql != NULL ) {
//		fscanf(archivo_lql,"%[^\n]", sentencia_lql);
//
//  }
	fclose(archivo_lql);
	//return;
}

void journal_memorias() {

	//enviar_listado_de_strings(requestExec.socketMemoria, requestExec.listaArgumentos, requestExec.comando);

	void ejecutar_journal(t_memoria* una_memoria) {
		//enviar(una_memoria->socket_memoria, int codigo_operacion, int tamanio, void * data);
	}
	list_iterate(memorias, (void*)ejecutar_journal);

	return;
}

void mostrarMetricas() {
	printf("\x1b[32m////////////////////////////////////////////////////////\n\t\t\tMetricas\x1b[0m\n");
	printf("\x1b[32m////////////////////////////////////////////////////////\x1b[0m\n");
	printf("\x1b[33mRead Latency / 30s :\x1b[0m \n");
	//printf("%.2f \n\n",readLatency);
	printf("\x1b[33mWrite Latency / 30s :\x1b[0m \n");
	//printf("%.2f \n\n",writeLatency);
	printf("\x1b[33mReads / 30s :\x1b[0m \n");
	//printf("%d \n\n",cantReads);
	printf("\x1b[33mWrites / 30s :\x1b[0m \n");
	//printf("%d \n\n",cantWrites);
	printf("\x1b[33mMemory Load :\x1b[0m \n");
	//printf("%d \n\n",memoryLoad);
	printf("\x1b[32m////////////////////////////////////////////////////////\x1b[0m\n");
}

t_requestAMemoria crearEstructuraRequest(t_list* argumentosRequest, command_api comandoRequest) {
	t_requestAMemoria dataRequest;
	dataRequest.socketMemoria = socket_Memoria;
	dataRequest.listaArgumentos = argumentosRequest;
	dataRequest.comando = comandoRequest;
	return dataRequest;
}


