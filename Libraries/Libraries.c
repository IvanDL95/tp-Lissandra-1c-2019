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


char get_campo_config_char(t_config* archivo_configuracion, char* nombre_campo) {
	char* valor;
	if(config_has_property(archivo_configuracion, nombre_campo)){
		valor = config_get_string_value(archivo_configuracion, nombre_campo);
		printf("El %s es: %s\n", nombre_campo, valor);
		return *valor;
	}
	return 0;
}

int get_campo_config_int(t_config* archivo_configuracion, char* nombre_campo) {
	int valor;
	if(config_has_property(archivo_configuracion, nombre_campo)){
		valor = config_get_int_value(archivo_configuracion, nombre_campo);
		printf("El %s es: %i\n", nombre_campo, valor);
		return valor;
	}
	return -1;
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


size_t size_of_string(const char* string) {
	return (strlen(string) * sizeof(char)) + 1;
}

char* string_concat(int cant_strings, ...) {
	va_list list;
   int j = 0;
   va_start(list, cant_strings);
   char* result = string_new();
   for(j=0; j < cant_strings; j++)
   {
	 string_append(&result, (void*)va_arg(list, int));
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

void enviar_listado_de_strings(un_socket socket, t_list * listado_strings, command_api comando) {
	int tamanio_buffer = size_of_list_of_strings_to_serialize(listado_strings);
	void * buffer = malloc(tamanio_buffer);
	int desplazamiento = 0;
	serializar_lista_strings(buffer, &desplazamiento, listado_strings);
	enviar(socket, comando, tamanio_buffer, buffer);
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
	if(desplazamiento != NULL){
		buffer = buffer + *desplazamiento;
		*desplazamiento = *desplazamiento + sizeof(int);
	}
	memcpy(buffer, &valor, sizeof(int));
}

//int deserializar_int(void * buffer, int * desplazamiento) {
//	if(desplazamiento != NULL){
//		buffer = buffer + *desplazamiento;
//		*desplazamiento = *desplazamiento + sizeof(int);
//	}
//	int *valor = malloc(sizeof(int));
//	memcpy(valor, buffer, sizeof(int));
//	return *valor;
//}

int deserializar_int(void * buffer, int * desplazamiento) {
	int valor = 0;
	memcpy(&valor, buffer + *desplazamiento, sizeof(int));
	int nuevo_desplazamiento = *desplazamiento + sizeof(int);
	memcpy(desplazamiento, &nuevo_desplazamiento, sizeof(int));
	return valor;
}

//void serializar_string(void * buffer, int * desplazamiento, char* valor) {
//	int tamanio_valor = size_of_string(valor);
//	serializar_int(buffer, desplazamiento, tamanio_valor);
//	if(desplazamiento == NULL)
//		memcpy(buffer, valor, tamanio_valor);
//	else{
//		memcpy(buffer + *desplazamiento, valor, tamanio_valor);
//		*desplazamiento = *desplazamiento + tamanio_valor;
//	}
//	memcpy(buffer, valor, tamanio_valor);
//}

void serializar_string(void * buffer, int * desplazamiento, char* valor) {
	int tamanio_valor = size_of_string(valor);
	serializar_int(buffer, desplazamiento, tamanio_valor);
	memcpy(buffer + *desplazamiento, valor, tamanio_valor);
	int nuevo_desplazamiento = *desplazamiento + tamanio_valor;
	memcpy(desplazamiento, &nuevo_desplazamiento, sizeof(int));
}

//char* deserializar_string(void * buffer, int * desplazamiento) {
//	int tamanio_valor = deserializar_int(buffer, desplazamiento);
//	char* valor = malloc(tamanio_valor);
//	if(desplazamiento == NULL)
//		memcpy(valor, buffer, tamanio_valor);
//	else{
//		memcpy(valor, buffer + *desplazamiento, tamanio_valor);
//		*desplazamiento = *desplazamiento + tamanio_valor;
//	}
//	return valor;
//}

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
	void _serializar_valor(char* valor) {
		serializar_string(buffer, desplazamiento, valor);
	}
	list_iterate(lista, (void*)_serializar_valor);
}

//t_list * deserializar_lista_strings(void * buffer, int * desplazamiento_x) {
//	t_list * resultado = list_create();
//	int desplazamiento = 0;
//	if(desplazamiento_x != NULL)
//		desplazamiento = *desplazamiento_x;
//
//	int tamanio_lista = deserializar_int(buffer, &desplazamiento);
//	int i;
//	for(i = 0; i < tamanio_lista; i++) {
//		char* valor = deserializar_string(buffer, &desplazamiento);
//		list_add(resultado, valor);
//	}
//	if(desplazamiento_x != NULL)
//		desplazamiento_x = &desplazamiento;
//	return resultado;
//}

t_list * deserializar_lista_strings(void * buffer, int * desplazamiento) {
	t_list * resultado = list_create();
	int tamanio_lista = deserializar_int(buffer, desplazamiento);
	for(int i = 0; i < tamanio_lista; i++) {
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
	 result += size_of_string((char*)va_arg(list, int));
   }
   va_end(list);
   return result;
}

int size_of_list_of_strings_to_serialize(t_list * list) {
	int result = list_size(list) * sizeof(int) + sizeof(int);
	void _agregar_tamanio_string(char* string) {
		result += size_of_string(string);
	}
	list_iterate(list, (void*)_agregar_tamanio_string);
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

void terminar_programa(t_log* log_file, int *socket){
	close(*socket);

	log_info(log_file, "Finaliza el programa");
	log_destroy(log_file);

	exit(EXIT_SUCCESS);
}

/*
un_socket levantar_servidor(uint32_t IP, uint16_t PORT){

	un_socket socket_listener = socket_escucha(IP, PORT);
    if (listen(socket_listener, 10) == -1) {
        perror("listen");
        exit(1);
    }

    return socket_listener;
}
*/
