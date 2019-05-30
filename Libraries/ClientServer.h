/*
 * ClientServer.h
 *
 *  Created on: 26 may. 2019
 *      Author: utnso
 */

#ifndef CLIENTSERVER_H_
#define CLIENTSERVER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <sys/time.h>
#include <commons/string.h>
#include <commons/error.h>
#include <unistd.h>
#include <ifaddrs.h>

enum codigos_de_operacion {
	cop_generico,
	codigo_error = -1,
	cop_ok 		 = 	0,
	cop_handshake = 99



	//Crear más tokens
};

typedef struct {
	int codigo_operacion;
	int tamanio;
	void * data;
} t_paquete;

typedef int un_socket;

/**	@NAME: liberar_paquete
 * 	@DESC: libera el paquete y su data.
 *
 */
void liberar_paquete(t_paquete * paquete);

/**	@NAME: iniciar_servidor
 *	@DESC: Inicia el hilo servidor
 *
 */
void iniciar_servidor(un_socket*);

/**	@NAME: conectar_a
 * 	@DESC: Intenta conectarse.
 * 	@RETURN: Devuelve el socket o te avisa si hubo un error al conectarse.
 *
 */
un_socket conectar_a(char* direccion_IP, char* Port);


/**	@NAME: socket_escucha
 * 	@DESC: Configura un socket que solo le falta hacer listen.
 * 	@RETURN: Devuelve el socket listo para escuchar o te avisa si hubo un error al conectarse.
 *
 */
un_socket socket_escucha(char* direccion_IP, char* Port);

/**	@NAME: enviar
 * 	@DESC: Hace el envio de la data que le pasamos. No hay que hacer más nada.
 *
 */
void enviar(un_socket socket_envio, int codigo_operacion, int tamanio,
		void * data);

/**	@NAME: recibir
 * 	@DESC: devuelve un paquete que está en el socket pedido
 *
 */
t_paquete* recibir(un_socket socket_para_recibir);

/**	@NAME: aceptar_conexion
 * 	@DESC: devuelve el socket nuevo que se quería conectar
 *
 */
un_socket aceptar_conexion(un_socket socket_escuchador);

/**	@NAME: realizar_handshake
 *	@DESC: el cliente realiza un handshake.
 */

bool realizar_handshake(un_socket socket_del_servidor);

/**	@NAME: esperar_handshake
 *	@DESC: el servidor espera el handshake del cliente.
 */

bool esperar_handshake(un_socket socket_del_cliente,
		t_paquete* inicio_del_handshake);


/**	@NAME: hacer_select
 *	@DESC: Realiza la función select con estructuras de control
 *	@RETURN: El resultado del select, ya sea error o no, como un int
 *
 */

int hacer_select(un_socket maxfd, fd_set* temp_set,fd_set* read_set);

/**	@NAME: obtener_mi_ip
 *	@DESC: Obtiene la IP (I guess)
 *	@RETURN: La IP propia como un string
 *
 */

char* obtener_mi_ip();

/**	@NAME: get_in_addr
 *	@DESC: ?
 *	@RETURN: Una estructura sockaddr_in con la IP y puerto propios
 *
 */

void *get_in_addr(struct sockaddr *sa);


#endif /* CLIENTSERVER_H_ */
