/*
 * Lissandra.c
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

// This code doesn't have a license. Feel free to copy.

#include "../src/Lissandra.h"
#include <Libraries.h>

#include <stdlib.h>
#include <stdio.h>

void analizar_paquete(un_socket*);

int main(int argc, char** argv){

	logger = log_create("lissandra.log", "Lissandra", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando File System\n");

	get_configuracion(argv[1]);

	pthread_t hilo_consola;

	pthread_create(&hilo_consola, NULL, (void*) iniciar_consola, logger);

	log_info(logger, "Levantando servidor\n");
	//TODO uso una IP definida o INADDR_ANY?

	pthread_attr_init(&hilo_attr_Server);
	pthread_attr_setdetachstate(&hilo_attr_Server, PTHREAD_CREATE_DETACHED);
	pthread_mutex_init(&mutex_mensaje, NULL);

	un_socket socket_servidor = socket_escucha(IP,config_LS.PUERTO_ESCUCHA);

    log_info(logger, "Estoy escuchando\n");

    while(1) {  // main accept() loop
        un_socket nuevo_cliente = aceptar_conexion(socket_servidor);

        log_info(logger,"Me llegó una nueva conexión\n");
        pthread_create(&hilo_Server,&hilo_attr_Server,(void*)analizar_paquete,&nuevo_cliente);

    }
	terminar_programa(logger, &socket_servidor);
}

void get_configuracion(char* ruta){
	log_info(logger, "Levantando archivo de configuracion del proceso Lissandra\n");

	t_config* archivo_configuracion = config_create(ruta);

	config_LS.PUERTO_ESCUCHA = copy_string(get_campo_config_string(archivo_configuracion, "PUERTO_ESCUCHA"));
	config_LS.PUNTO_MONTAJE = copy_string(get_campo_config_string(archivo_configuracion, "PUNTO_MONTAJE"));
	config_LS.RETARDO = get_campo_config_int(archivo_configuracion, "RETARDO");
	config_LS.TAMANIO_VALUE = get_campo_config_int(archivo_configuracion, "TAMAÑO_VALUE");
	config_LS.TIEMPO_DUMP = get_campo_config_int(archivo_configuracion, "TIEMPO_DUMP");

	puts("\n");
	config_destroy(archivo_configuracion);
}


void analizar_paquete(un_socket* nuevo_socket){
	while(1){
	/*
	fd_set temp_set, read_set;
	FD_ZERO(&read_set);
	FD_SET(nuevo_socket, &read_set);
	memcpy(&temp_set, &read_set, sizeof(temp_set));

	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	int result = select(nuevo_socket + 1, &temp_set, NULL, NULL, &tv);


	if (result == 0) {
		printf("select() timed out!\n");
		pthread_detach(pthread_self());
	}
	else if (result < 0 && errno != EINTR) {
		printf("Error in select(): %s\n", strerror(errno));
		pthread_detach(pthread_self());
	}
	*/
    t_paquete* paquete_recibido = recibir(*nuevo_socket);
    switch(paquete_recibido->codigo_operacion){
    	case codigo_error:
    		log_info(logger,"Se desconecto una memoria \n");
    	    close(*nuevo_socket);
    	    pthread_detach(pthread_self());
    	    pthread_exit(NULL);
    		break;
    	case cop_handshake:
    		log_info(logger, "Realizando Handshake con Memoria x\n");
    		esperar_handshake(*nuevo_socket, paquete_recibido);
    		log_info(logger, "Handshake exitoso!\n");

    		enviar(*nuevo_socket, cop_ok,sizeof(int),&config_LS.TAMANIO_VALUE);
    		paquete_recibido = recibir(*nuevo_socket);
				if(paquete_recibido->codigo_operacion == cop_ok)
					log_info(logger,"La memoria recibió el tamaño del value\n");
				else{
					if(paquete_recibido->codigo_operacion == codigo_error)
						log_info(logger,"La memoria no recibió el paquete y se cayó\n");
					else
						log_info(logger,"Recibí un paquete fantasma\n");
				}
			break;
    	default:
    	{
    		command_api comando = paquete_recibido->codigo_operacion;
    		/* envio argumentos como lista de strings */
    		int desplazamiento = 0;
    		t_list* lista_argumentos = deserializar_lista_strings(paquete_recibido->data, &desplazamiento);
    		int tamanio_lista = list_size(lista_argumentos);
    		char* argumentos[tamanio_lista];
    		for(int i=0;i<tamanio_lista;i++){
    			argumentos[i] = list_get(lista_argumentos,i);
    		}
    		list_destroy(lista_argumentos);
    		ejecutar_API(comando, argumentos);
    		/* segfault glorioso
    		char* mensaje_retorno = ejecutar_API(comando, argumentos);
    		if(mensaje_retorno == NULL)
    			enviar(*nuevo_socket,codigo_error,0,NULL);
    		else{
    			unsigned int tamanio_buffer = size_of_string(mensaje_retorno)+sizeof(int);
    			void* buffer = malloc(tamanio_buffer);
    			desplazamiento = 0;
    			serializar_string(buffer, &desplazamiento, mensaje_retorno);
    		}
    	*/
    		enviar(*nuevo_socket,codigo_error,0,NULL);
    	}
    	break;
    	}
    	liberar_paquete(paquete_recibido);
	}
}


char* ejecutar_API(command_api operacion, char** argumentos){
	switch(operacion){

		case SELECT:
			printf("hacer SELECT\n");
			break;
		case INSERT:
			printf("hacer INSERT");
			break;
		case CREATE:
			printf("hacer CREATE");
			break;
		case DESCRIBE:
			printf("hacer DESCRIBE");
			break;
		case DROP:
			printf("hacer DROP");
			break;
		default:
			log_info(logger, "Paquete no reconocido\n");
	}
	return NULL;
}
