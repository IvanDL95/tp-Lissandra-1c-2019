/*
 * ClientServer.c
 *
 *  Created on: 26 may. 2019
 *      Author: utnso
 */

#include "ClientServer.h"

un_socket _crear_socket(struct addrinfo*);
struct addrinfo* _configurar_addrinfo(char* IP, char* Port);
//int _hacer_select(un_socket , fd_set* , struct timeval*);


void liberar_paquete(t_paquete * paquete) {
	paquete->data = NULL;
	free(paquete->data);
	free(paquete);
}

un_socket conectar_a(char* IP, char* Port) {
	struct addrinfo *server_info = _configurar_addrinfo(IP, Port);
	un_socket socket_cliente = _crear_socket(server_info);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		perror("connect");
		return -1;
	}

	freeaddrinfo(server_info);
	return(socket_cliente);
}

un_socket socket_escucha(char* IP, char* Port) {
	struct addrinfo* serverInfo = _configurar_addrinfo(IP, Port);

	struct addrinfo *p;
	un_socket socketEscucha;

    for (p=serverInfo; p != NULL; p = p->ai_next)
    {
    	socketEscucha = _crear_socket(p);
        if (socketEscucha == -1)
            continue;

        int option = 1;
        if(setsockopt(socketEscucha,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(int)) == -1)
        	perror("setsockopt");

        if (bind(socketEscucha, p->ai_addr, p->ai_addrlen) == -1) {
        	perror("bind");
            close(socketEscucha);
            continue;
        }
        break;
    }

    freeaddrinfo(serverInfo);
	listen(socketEscucha, SOMAXCONN);
	return socketEscucha;
}

un_socket aceptar_conexion(un_socket socket_servidor) {
	/*
	struct sockaddr_storage remoteaddr;

	socklen_t addrlen;
	addrlen = sizeof remoteaddr;
    if ((new_fd = accept(socket_listener, (struct sockaddr *)&their_addr,
                                                   &sin_size)) == -1) {
        perror("accept");
        continue;
    }
    printf("server: got connection from %s\n",
                                       inet_ntoa(their_addr.sin_addr));
	*/
	struct sockaddr_in dir_cliente;
	unsigned int tam_direccion = sizeof(struct sockaddr_in);
	un_socket socket_cliente;

    socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
    if(socket_cliente == -1)
    	perror("Accept");

	return socket_cliente;
}


bool realizar_handshake(un_socket socket_del_servidor) {

	//TODO no me está dejando liberar esta memoria. A veces Valgrind tira "invalid free()" más abajo.
	char * mensaje = malloc(21);
	mensaje = "Inicio autenticacion";
	enviar(socket_del_servidor, cop_handshake, 21, mensaje);
	mensaje = NULL;
	free(mensaje);

	t_paquete * resultado_del_handhsake = recibir(socket_del_servidor);
	bool resultado = string_equals_ignore_case(
			(char *) resultado_del_handhsake->data, "Autenticado");

	liberar_paquete(resultado_del_handhsake);

	return resultado;
}

bool esperar_handshake(un_socket socket_del_cliente, t_paquete* inicio_del_handshake) {

	bool resultado = string_equals_ignore_case(
			(char *) inicio_del_handshake->data, "Inicio autenticacion");

	char * respuesta;
	if (resultado) {
		respuesta = malloc(12);
		respuesta = "Autenticado";
		enviar(socket_del_cliente, cop_handshake, 12, respuesta);
	} else {
		respuesta = malloc(6);
		respuesta = "Error";
		enviar(socket_del_cliente, cop_handshake, 6, respuesta);
	}
	respuesta = NULL;
	free(respuesta);
	return resultado;
}

void enviar(un_socket socket_para_enviar, int codigo_operacion, int tamanio,
		void * data) {
	//int tamanio = sizeof((char*)data);
	int tamanio_paquete = 2 * sizeof(int) + tamanio;
	void * buffer = malloc(tamanio_paquete);

	memcpy(buffer, &codigo_operacion, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanio, sizeof(int));
	memcpy(buffer + 2 * sizeof(int), data, tamanio);

	int ok;

	//TODO testear
	do{
		ok = send(socket_para_enviar, buffer, tamanio_paquete, MSG_DONTWAIT);
	//	if(ok==-1){
	//		perror("send");
			break;
	//	}
	}while(ok != tamanio_paquete);

	free(buffer);
}

t_paquete* recibir(un_socket socket_para_recibir) {

	t_paquete * paquete_recibido = malloc(sizeof(t_paquete));
	int retorno = 0;
	retorno = recv(socket_para_recibir, &paquete_recibido->codigo_operacion, sizeof(int),
	MSG_WAITALL);

	if(retorno==0){
		paquete_recibido->codigo_operacion=-1;
		void * informacion_recibida = malloc(sizeof(int));
		paquete_recibido->data = informacion_recibida;
		return paquete_recibido;

	}
	recv(socket_para_recibir, &paquete_recibido->tamanio, sizeof(int),
	MSG_WAITALL);

	if(paquete_recibido->tamanio > 0)
	{
		void * informacion_recibida = malloc(paquete_recibido->tamanio);

			recv(socket_para_recibir, informacion_recibida, paquete_recibido->tamanio,
			MSG_WAITALL);

			paquete_recibido->data = informacion_recibida;
	}

	return paquete_recibido;
}


char* obtener_mi_ip(){
	 struct ifaddrs *ifaddr, *ifa;
	   int family, s, n;
	   char* host = malloc(NI_MAXHOST);

	   if (getifaddrs(&ifaddr) == -1) {
		   return NULL;
	   }

	   /* Walk through linked list, maintaining head pointer so we
		  can free list later */

	   for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
		   if (ifa->ifa_addr == NULL)
			   continue;

		   family = ifa->ifa_addr->sa_family;


		   if (family == AF_INET) {
			   s = getnameinfo(ifa->ifa_addr,
					   (family == AF_INET) ? sizeof(struct sockaddr_in) :
											 sizeof(struct sockaddr_in6),
					   host, NI_MAXHOST,
					   NULL, 0, NI_NUMERICHOST);
			   if (s != 0) {
				   printf("getnameinfo() failed: %s\n", gai_strerror(s));
				   exit(EXIT_FAILURE);
			   }

			   freeifaddrs(ifaddr);
			   return host;

		   }
	   }
	   return NULL;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr); //IPv6 en Argentina, lulz
}

/************************ ****************************/

un_socket _crear_socket(struct addrinfo *p){
	int sockfd;
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("socket");
        return -1;
    }
	return sockfd;
}

struct addrinfo* _configurar_addrinfo(char* IP, char* Port) {
	struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(IP, Port, &hints, &servinfo);
	return servinfo;
}

int hacer_select(un_socket maxfd, fd_set* temp_set,fd_set* read_set, struct timeval* tv){
	memcpy(temp_set, read_set, sizeof(*temp_set));

	int result = select(maxfd + 1, temp_set, NULL, NULL, tv);

	if (result == 0) {
		printf("select() timed out!\n");
		return -1;
	}
	else if (result < 0 && errno != EINTR) {
		printf("Error in select(): %s\n", strerror(errno));
		return -1;
	}
	return result;
}

