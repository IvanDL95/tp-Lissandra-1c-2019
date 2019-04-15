#include "Libraries.h"

#include "Libraries.h"

//prueba libraries
void imprimir(char* filename){
	FILE *fptr = NULL;

	if((fptr = fopen(filename,"r")) == NULL)
	{
		fprintf(stderr,"error opening %s\n",filename);
		return;
	}

	print_image(fptr);

	fclose(fptr);

	return;
}

void print_image(FILE *fptr)
{
	char read_string[MAX_LEN];

	while(fgets(read_string,sizeof(read_string),fptr) != NULL)
		printf("%s",read_string);
}

struct sockaddr_in* _configurar_addrinfo(char *IP, char* Port) {
	struct sockaddr_in *my_addr = malloc(sizeof(struct sockaddr_in));
    my_addr->sin_family = AF_INET;         // Ordenación de bytes de la máquina
    my_addr->sin_port = htons(Port);     // short, Ordenación de bytes de la red
    my_addr->sin_addr.s_addr = IP; // Rellenar con mi dirección IP
    memset(&(my_addr->sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	return my_addr;
}

un_socket conectar_a(char *IP, char* Port) {
	struct sockaddr_in* serverInfo = _configurar_addrinfo(IP, Port);
	if (serverInfo == NULL) {
		exit(EXIT_FAILURE);
	}
	int serverSocket = crear_socket(serverInfo);

	if (connect(serverSocket, (struct sockaddr *)&serverInfo, sizeof(struct sockaddr))
			== -1) {
		puts("\n");
		error_show(
				"No se pudo conectar con el proceso que hace de servidor, error: %d\n",
				errno);
		close(serverSocket);
		exit(EXIT_FAILURE);
	}
	free(serverInfo);
	return serverSocket;
}


un_socket crear_socket(){
	int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
	return sockfd;
}

un_socket socket_escucha(char* IP, char* Port) {
	struct sockaddr_in* serverInfo = _configurar_addrinfo(IP, Port);

	if (serverInfo == NULL) {
		exit(EXIT_FAILURE);
	}



	/* no descomentar por favor, no hace absoultamente nada útil para este TP
	socketEscucha = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	bind(socketEscucha, serverInfo->ai_addr, serverInfo->ai_addrlen);
	*/
	int socketEscucha, i;

	//for(i = 1; i == 5000; i++) {
		socketEscucha = crear_socket(serverInfo);
		if (socketEscucha < 0) {
			//continue;
		}
		// lose the pesky "address already in use" error message
		int enable = 1;
	    if (setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) { //forzar la reutilizacion del socket
	        perror("setsockopt");
	        exit(1);
	    }
		if (bind(socketEscucha, (struct sockaddr *) serverInfo, sizeof(struct sockaddr))== -1) {
			close(socketEscucha);
			//continue;
	    }
		//break;
	//}

	free(serverInfo);

	return socketEscucha;
}

char* obtener_mi_ip(){
	 struct ifaddrs *ifaddr, *ifa;
	   int family, s, n;
	   char host[NI_MAXHOST];

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

void enviar(un_socket socket_para_enviar, int codigo_operacion, int tamanio,
		void * data) {

	int tamanio_paquete = 2 * sizeof(int) + tamanio;
	void * buffer = malloc(tamanio_paquete);

	memcpy(buffer, &codigo_operacion, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanio, sizeof(int));
	memcpy(buffer + 2 * sizeof(int), data, tamanio);


	send(socket_para_enviar, buffer, tamanio_paquete, MSG_WAITALL);

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

un_socket aceptar_conexion(un_socket socket_escuchador) {
	/*
    if ((new_fd = accept(socket_listener, (struct sockaddr *)&their_addr,
                                                   &sin_size)) == -1) {
        perror("accept");
        continue;
    }
    printf("server: got connection from %s\n",
                                       inet_ntoa(their_addr.sin_addr));
	*/
	struct sockaddr_storage remoteaddr;

	socklen_t addrlen;

	addrlen = sizeof remoteaddr;

	un_socket nuevo_socket;

	if ((nuevo_socket = accept(socket_escuchador,
			(struct sockaddr *) &remoteaddr, &addrlen)) == -1)
        perror("accept");
        //continue;


	return nuevo_socket;
}

void liberar_paquete(t_paquete * paquete) {
	free(paquete->data);
	free(paquete);
}

bool realizar_handshake(un_socket socket_del_servidor) {

	char * mensaje = malloc(18);
	mensaje = "Inicio autenticacion";

	enviar(socket_del_servidor, cop_handshake, 21, mensaje);

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
	return resultado;
}

char get_campo_config_char(t_config* archivo_configuracion, char* nombre_campo) {
	char* valor;
	if(config_has_property(archivo_configuracion, nombre_campo)){
		valor = config_get_string_value(archivo_configuracion, nombre_campo);
		printf("El %s es: %s\n", nombre_campo, valor);
		return &valor;
	}
	return NULL;
}

int get_campo_config_int(t_config* archivo_configuracion, char* nombre_campo) {
	int valor;
	if(config_has_property(archivo_configuracion, nombre_campo)){
		valor = config_get_int_value(archivo_configuracion, nombre_campo);
		printf("El %s es: %i\n", nombre_campo, valor);
		return valor;
	}
	return NULL;
}


char* get_campo_config_string(t_config* archivo_configuracion, char* nombre_campo) {
	char* valor;
	if(config_has_property(archivo_configuracion, nombre_campo)){
		valor = config_get_string_value(archivo_configuracion, nombre_campo);
		printf("El %s es: %s\n", nombre_campo, valor);
		return valor;
	}
	return NULL;
}


char** get_campo_config_array(t_config* archivo_configuracion, char* nombre_campo) {
	char** valor;
	if(config_has_property(archivo_configuracion, nombre_campo)){
		valor = config_get_array_value(archivo_configuracion, nombre_campo);
		printf("Los %s son: [", nombre_campo);

		char* valor_actual = valor[0];
		printf("%s", valor_actual);
		for(int i = 1;valor[i] != NULL;i++){
			valor_actual = valor[i];
			printf(",%s", valor_actual);
		}
		printf("]\n");
		return valor;
	}
	return NULL;
}

char** str_split(char* a_str, const char a_delim) {
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;
	/* Count how many elements will be extracted. */
	while (*tmp) {
		if (a_delim == *tmp) {
			count++;
			last_comma = tmp;
		}
		tmp++;
	}
	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);
	/* Add space for terminating null string so caller
	 knows where the list of returned strings ends. */
	count++;
	result = malloc(sizeof(char*) * count);
	if (result) {
		size_t idx = 0;
		char* token = strtok(a_str, delim);
		while (token) {
			assert(idx < count);
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
		assert(idx == count - 1);
		*(result + idx) = 0;
	}
	return result;
}

char *str_replace(char *orig, char *rep, char *with) {
	char *result; // the return string
	char *ins;    // the next insert point
	char *tmp;    // varies
	int len_rep;  // length of rep (the string to remove)
	int len_with; // length of with (the string to replace rep with)
	int len_front; // distance between rep and end of last rep
	int count;    // number of replacements

	// sanity checks and initialization
	if (!orig || !rep)
		return NULL;
	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL; // empty rep causes infinite loop during count
	if (!with)
		with = "";
	len_with = strlen(with);

	// count the number of replacements needed
	ins = orig;
	for (count = 0; (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}
	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);
	if (!result)
		return NULL;

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of rep in orig
	//    orig points to the remainder of orig after "end of rep"
	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);
	return result;
}

int countOccurrences(char * str, char * toSearch) {
	int i, j, found, count;
	int stringLen, searchLen;
	stringLen = strlen(str);      // length of string
	searchLen = strlen(toSearch); // length of word to be searched
	count = 0;
	for (i = 0; i <= stringLen - searchLen; i++) {
		/* Match word with string */
		found = 1;
		for (j = 0; j < searchLen; j++) {
			if (str[i + j] != toSearch[j]) {
				found = 0;
				break;
			}
		}
		if (found == 1) {
			count++;
		}
	}
	return count;
}

char *randstring(size_t length) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *randomString = NULL;
    if (length){
        randomString = malloc(sizeof(char) * (length +1));
        if (randomString){
        	int n;
            for ( n = 0;n < length;n++) {
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }
            randomString[length] = '\0';
        }
    }
    return randomString;
}

unsigned long int lineCountFile(const char *filename){
    FILE *fp = fopen(filename, "r");
    unsigned long int linecount = 0;
    int c;
    if(fp == NULL){
        fclose(fp);
        return 0;
    }
    while((c=fgetc(fp)) != EOF ){
        if(c == '\n')
            linecount++;
    }
    fclose(fp);
    return linecount;
}

char* generarDirectorioTemporal(char* carpeta){
	char* dirTemp= string_new();
	string_append(&dirTemp, carpeta);
	string_append(&dirTemp, randstring(5));
	return dirTemp;
}

void crear_subcarpeta(char* nombre) {
	struct stat st = { 0 };
	if (stat(nombre, &st) == -1) {
		mkdir(nombre, 0777);
	}
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr); //IPv6 en Argentina, lulz
}

int size_of_string(char* string) {
	return strlen(string) * sizeof(char) + 1;
}

char* string_concat(int cant_strings, ...) {
	va_list list;
   int j = 0;
   va_start(list, cant_strings);
   char* result = string_new();
   for(j=0; j < cant_strings; j++)
   {
	 string_append(&result, va_arg(list, int));
   }
   va_end(list);
   return result;
}

void log_and_free(t_log* logger, char* mensaje) {
	log_info(logger, mensaje);
	free(mensaje);
}

void log_error_and_free(t_log* logger, char* mensaje) {
	log_error(logger, mensaje);
	free(mensaje);
}

void free_array(char** array, int array_size) {
	int i;
	for(i = 0;i < array_size;i++) {
		free(array[i]);
	}
	free(array);
}

char* copy_string(char* value) {
	char* pointer = malloc(size_of_string(value));
	strcpy(pointer, value);
	return pointer;
}

void enviar_listado_de_strings(un_socket socket, t_list * listado_strings) {
	int tamanio_buffer = size_of_list_of_strings_to_serialize(listado_strings);
	void * buffer = malloc(tamanio_buffer);
	int desplazamiento = 0;
	serializar_lista_strings(buffer, &desplazamiento, listado_strings);
	enviar(socket, cop_generico, tamanio_buffer, buffer);
	free(buffer);
}

t_list * recibir_listado_de_strings(un_socket socket) {
	int desplazamiento = 0;
	t_paquete* paqueteListado = recibir(socket);
	t_list * listado = deserializar_lista_strings(paqueteListado->data, &desplazamiento);
	liberar_paquete(paqueteListado);
	return listado;
}

void serializar_int(void * buffer, int * desplazamiento, int valor) {
	memcpy(buffer + *desplazamiento, &valor, sizeof(int));
	int nuevo_desplazamiento = *desplazamiento + sizeof(int);
	memcpy(desplazamiento, &nuevo_desplazamiento, sizeof(int));
}

int deserializar_int(void * buffer, int * desplazamiento) {
	int valor = NULL;
	memcpy(&valor, buffer + *desplazamiento, sizeof(int));
	int nuevo_desplazamiento = *desplazamiento + sizeof(int);
	memcpy(desplazamiento, &nuevo_desplazamiento, sizeof(int));
	return valor;
}

void serializar_string(void * buffer, int * desplazamiento, char* valor) {
	int tamanio_valor = size_of_string(valor);
	serializar_int(buffer, desplazamiento, tamanio_valor);
	memcpy(buffer + *desplazamiento, valor, tamanio_valor);
	int nuevo_desplazamiento = *desplazamiento + tamanio_valor;
	memcpy(desplazamiento, &nuevo_desplazamiento, sizeof(int));
}

char* deserializar_string(void * buffer, int * desplazamiento) {
	int tamanio_valor = deserializar_int(buffer, desplazamiento);
	char* valor = malloc(tamanio_valor);
	memcpy(valor, buffer + *desplazamiento, tamanio_valor);
	int nuevo_desplazamiento = *desplazamiento + tamanio_valor;
	memcpy(desplazamiento, &nuevo_desplazamiento, sizeof(int));
	return valor;
}

void serializar_lista_strings(void * buffer, int * desplazamiento, t_list * lista) {
	serializar_int(buffer, desplazamiento, list_size(lista));
	void serializar_valor(char* valor) {
		serializar_string(buffer, desplazamiento, valor);
	}
	list_iterate(lista, serializar_valor);
}

t_list * deserializar_lista_strings(void * buffer, int * desplazamiento) {
	t_list * resultado = list_create();
	int tamanio_lista = deserializar_int(buffer, desplazamiento);
	int i;
	for(i = 0; i < tamanio_lista; i++) {
		char* valor = deserializar_string(buffer, desplazamiento);
		list_add(resultado, valor);
	}
	return resultado;
}

int size_of_strings(int cant_strings, ...) {
	va_list list;
   int j = 0;
   va_start(list, cant_strings);
   int result = cant_strings * sizeof(int);
   for(j=0; j < cant_strings; j++)
   {
	 result += size_of_string(va_arg(list, int));
   }
   va_end(list);
   return result;
}

int size_of_list_of_strings_to_serialize(t_list * list) {
	int result = list_size(list) * sizeof(int) + sizeof(int);
	void agregar_tamanio_string(char* string) {
		result += size_of_string(string);
	}
	list_iterate(list, agregar_tamanio_string);
	return result;
}

void destruir_lista_strings(t_list * lista) {
	void string_destroyer(void * string) {
		free(string);
	}
	list_destroy_and_destroy_elements(lista, string_destroyer);
}

bool strings_equal(char* string1, char* string2) {
	return strcmp(string1, string2) == 0 ? true : false;
}

t_list * copy_list(t_list * lista) {
	t_list * nueva_lista = list_create();
	void agregar_elemento(void * e) {
		list_add(nueva_lista, e);
	}
	list_iterate(lista, agregar_elemento);
	return nueva_lista;
}

int array_of_strings_length(char** array) {
	int i = 0;
	char* item;
	while(array[i] != NULL) {
		printf("Item: %s \n", array[i]);
		i++;
	}
	return i;
}

t_list * list_remove_all_by_condition(t_list * lista, bool(*condicion)(void*)) {
	t_list * result = list_filter(lista, condicion);
	while (list_find(lista, condicion) != NULL) {
		list_remove_by_condition(lista, condicion);
	}
	return result;
}

void terminar_programa(t_log* log_file, un_socket *socket){
	close(&socket);

	log_info(log_file, "Finaliza el programa");
	log_destroy(log_file);

	exit(EXIT_SUCCESS);
}

un_socket levantar_servidor(char* IP, char* PORT){

	un_socket socket_listener = socket_escucha(IP, PORT);
    if (listen(socket_listener, 10) == -1) {
        perror("listen");
        exit(1);
    }

    return socket_listener;
}
