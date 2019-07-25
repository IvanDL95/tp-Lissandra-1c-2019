/*
 * MemoriaPrincipal.c
 *
 *  Created on: 23 jul. 2019
 */

#include "MemoriaPrincipal.h"

/******************* PRIVATE FUNCTIONS DECLARE ********************************/

static t_frame* copiar_a_memoria(int frame, const int key, const time_t, const char* value);
static char* copiar_desde_memoria(char* posicion_en_memoria);
static void page_destroyer(t_pagina*);
static void segment_destroyer(t_segmento*);
static void actualizar_cola(int key, t_segmento* tabla);
static t_frame* algoritmo_reemplazo(const int key, const char* valor);

/******************************************** PUBLIC *************************************/

t_segmento* crear_nuevo_segmento(const char* nombre_tabla){
	t_segmento* segmento_nuevo = malloc(sizeof(t_segmento));

	segmento_nuevo->nombre_tabla = malloc(size_of_string(nombre_tabla));
	strcpy(segmento_nuevo->nombre_tabla, nombre_tabla);
	//segmento_nuevo.nombre_tabla = nombre_tabla;
	log_debug(logger, "Tabla: %s\n",segmento_nuevo->nombre_tabla);
	segmento_nuevo->tabla = list_create();


	list_add(tabla_segmentos,segmento_nuevo);
	log_debug(logger, "Nuevo segmento creado\n");
	return segmento_nuevo;
}

void destruir_segmento(char* nombre_tabla){
	int _is_equal_segmento(t_segmento* segmento){
		// la lógica del list_find está al revés
		log_trace(logger,"Compara %s con %s\n", nombre_tabla, segmento->nombre_tabla);
		return !strcmp(nombre_tabla,segmento->nombre_tabla);
	}

	list_remove_and_destroy_by_condition(tabla_segmentos,(void*)_is_equal_segmento,(void*)segment_destroyer);
}

t_segmento* buscar_segmento(const char* nombre_tabla){
	int _is_equal_segmento(t_segmento* segmento){
		// la lógica del list_find está al revés
		log_trace(logger,"Compara %s con %s\n", nombre_tabla, segmento->nombre_tabla);
		return !strcmp(nombre_tabla,segmento->nombre_tabla);
	}

	return (t_segmento*)list_find(tabla_segmentos,(void*)_is_equal_segmento);
}

t_pagina* buscar_key(t_segmento* segmento_encontrado, int key){
	log_debug(logger,"Buscando key\n");
	/*
	 Ahora busca por índice en la tabla de páginas ya que key == índice lista

	int _is_equal_key(t_pagina* registro){
		// la lógica del list_find está al revés
		log_trace(logger,"Compara %d con %d\n", key, registro->pagina->key);
		return (key == registro->pagina->key);
	}*/
	t_cola_LRU* last_element = (t_cola_LRU*)queue_peek(cola_LRU);
	if(last_element->nro_pagina == key && strcmp(last_element->segmento->nombre_tabla, segmento_encontrado->nombre_tabla))
		return (t_pagina*)list_get(segmento_encontrado->tabla,key);

	if(!list_is_empty(segmento_encontrado->tabla)){
		t_pagina* current_page = list_get(segmento_encontrado->tabla, key);

		//((t_pagina*)list_find(segmento_encontrado->tabla,(void*)_is_equal_key));
		actualizar_cola(key,segmento_encontrado);
		queue_push(cola_LRU,current_page);
		return current_page;
	}
	else
		return NULL;
}

t_frame* solicitar_pagina(t_segmento* current_segmento, const char* valor, int key, flag flag_state){
	log_debug(logger,"Solcitando página para value: %s\n", valor);
	int i;

	t_frame* frame_modificado = malloc(sizeof(t_frame));

	for(i=0;i<CANTIDAD_FRAMES;i++){
		if(bitarray_test_bit(frame_bitarray,i)){
			log_trace(logger,"Frame libre!: %d\n",i);
			char* new_valor = malloc(tamanio_value);
			strcpy(new_valor,valor);

			//te devuelve el puntero a la pagina asignada
			frame_modificado = copiar_a_memoria(i, key,time(NULL),new_valor);
			break;
		}
		log_trace(logger,"Frame ocupado: %d\n",i);
	}

	if(i == CANTIDAD_FRAMES){
		log_info(logger,"Todas las páginas están ocupadas. Ejecutando algoritmo de reemplazo\n");

		frame_modificado = algoritmo_reemplazo(key, valor);
	}

	t_pagina* nuevo_registro = malloc(sizeof(t_pagina));
	nuevo_registro->pagina = frame_modificado;
	nuevo_registro->nro_frame = i;
	nuevo_registro->modificado = flag_state;

	list_add_in_index(current_segmento->tabla,key,nuevo_registro);
	actualizar_cola(key,current_segmento);

	return frame_modificado;
}

char* obtener_valor(t_segmento* segmento, int nro_pag){
	t_pagina* pagina = list_get(segmento->tabla, nro_pag);
	return copiar_desde_memoria(pagina->pagina->memAddr);
}

void actualizar_pagina(t_segmento* tabla,t_pagina* pagina_encontrada,const int key, const char* nuevo_valor){
	copiar_a_memoria(pagina_encontrada->nro_frame, 0, time(NULL), nuevo_valor);
	pagina_encontrada->modificado = MODIFICADO;
	actualizar_cola(key,tabla);
}
/*
  int asignar_key(tabla_paginas page_table){
	if(list_is_empty(page_table))
		return 1;

	int _is_bigger_key(t_pagina* x, t_pagina* y){
		return x->pagina->key > y->pagina->key;
	}

	t_list* tabla_ordenada_segun_key = list_sorted(page_table,(void*)_is_bigger_key);
	int key_anterior = (((t_pagina*)list_get(tabla_ordenada_segun_key, 0))->pagina->key);

	return key_anterior+1;
}
*/


static t_frame* algoritmo_reemplazo(const int key, const char* valor){

	t_cola_LRU* cabeza_cola = malloc(sizeof(t_cola_LRU));
	t_cola_LRU* primer_elemento = queue_peek(cola_LRU);

	while(!queue_is_empty(cola_LRU)){
		cabeza_cola = queue_pop(cola_LRU);
		if(primer_elemento->nro_pagina == cabeza_cola->nro_pagina &&
			!strcmp(primer_elemento->segmento->nombre_tabla, cabeza_cola->segmento->nombre_tabla))
			break;

		t_pagina* ultima_pagina = (t_pagina*)list_get(primer_elemento->segmento->tabla, cabeza_cola->nro_pagina);

		if(ultima_pagina->modificado == MODIFICADO){
			queue_push(cola_LRU,cabeza_cola);
			continue;
		}

		free(cabeza_cola);
		return copiar_a_memoria(ultima_pagina->nro_frame,key, time(NULL), valor);
	}

	free(cabeza_cola);
	printf("Memoria está FULL, iniciando proceso de journal\n");
	esta_full_memoria = true;
	hacer_journal();
	return NULL;
}

//TODO rehacer JOURNAL

void hacer_journal(){
	log_debug(logger, "Acá hay que bloquear todo\n");
	if(esta_full_memoria){
		//enviar todas las paginas al FS y vaciar todoo
		t_list* listado_a_enviar = list_create();
		t_segmento* current_segmento = malloc(sizeof(t_segmento));

		for(int i=0;!list_is_empty(tabla_segmentos);i++){
			current_segmento = (t_segmento*)list_remove(tabla_segmentos,i);
			list_add_all(listado_a_enviar,current_segmento->tabla);
		}
		for(int i=0;i<CANTIDAD_FRAMES;i++)
			bitarray_clean_bit(frame_bitarray, i);

		enviar_listado_de_strings(socket_FS,listado_a_enviar,INSERT);
		queue_clean(cola_LRU);
		esta_full_memoria = false;
	}else{
		t_list* modified_pages_list = list_create();
		t_segmento* current_segmento = malloc(sizeof(t_segmento));
		int key;

		/*	Matchea el elemento que tenga el mismo nombre tabla y key que la pagina filtrada	*/

		int _has_same_key_and_table(t_cola_LRU* elemento){
			if(!strcmp(current_segmento->nombre_tabla,elemento->segmento->nombre_tabla) && elemento->nro_pagina == key)
				return 1;
			else
				return 0;
		}

		/* Filtrar elementos y luego quitarlos de la cola */

		int _not_modified(t_pagina* pagina_actual){
			if(pagina_actual->modificado)
				return 0;
			else{
				key = *(int*)pagina_actual->pagina->memAddr;
				list_remove_by_condition(cola_LRU->elements,(void*)_has_same_key_and_table);
				bitarray_clean_bit(frame_bitarray, pagina_actual->nro_frame);
				return 1;
			}
		}

		t_list* tabla_segmentos_aux = list_create();
		tabla_segmentos_aux = list_duplicate(tabla_segmentos);


		for(int i=0;!list_is_empty(tabla_segmentos_aux);i++){
			current_segmento = (t_segmento*)list_remove(tabla_segmentos_aux,i);
			list_add_all(modified_pages_list,list_filter(current_segmento->tabla,(void*)_not_modified));
		}

		enviar_listado_de_strings(socket_FS,modified_pages_list,INSERT);
	}
	return;
}

/******************* PRIVATE FUNCTIONS IMPLE ********************************/

static t_frame* copiar_a_memoria(int nro_frame, const int key, const time_t timestamp, const char* value){


	memmove(frame[nro_frame].memAddr, &key, sizeof(int));
	memmove(frame[nro_frame].memAddr+sizeof(int), &timestamp, sizeof(time_t));
	memmove(frame[nro_frame].memAddr+sizeof(int)+sizeof(time_t), value, tamanio_value);

	log_trace(logger,"Key: %d, Timestamp: %d, Value: %s",
			*(int*)(frame[nro_frame].memAddr),
			*(time_t*)(frame[nro_frame].memAddr+sizeof(int)),
			(char*)(frame[nro_frame].memAddr+sizeof(int)+sizeof(time_t)));

	bitarray_set_bit(frame_bitarray,nro_frame);

	return frame;
}

static char* copiar_desde_memoria(char* posicion_en_memoria){
	time_t timestamp;
	char* value = malloc(tamanio_value);

	memmove(&timestamp, posicion_en_memoria + sizeof(int), sizeof(time_t));
	memmove(value,posicion_en_memoria +sizeof(int) + sizeof(time_t), tamanio_value);

	return value;
}

static void actualizar_cola(int key, t_segmento* tabla){

	int is_seeked_queue_element(t_cola_LRU* cola_element){
		return (cola_element->nro_pagina == key && !strcmp(tabla->nombre_tabla,cola_element->segmento->nombre_tabla));
	}

	t_cola_LRU* elemento = (t_cola_LRU*)list_remove_by_condition(cola_LRU->elements,(void*)is_seeked_queue_element);

	if(elemento != NULL)
		queue_push(cola_LRU,elemento);
	else{
		t_cola_LRU* new_element = malloc(sizeof(t_cola_LRU));
		new_element->nro_pagina = key;
		new_element->segmento->nombre_tabla = malloc(size_of_string(tabla->nombre_tabla));
		strcpy(new_element->segmento->nombre_tabla,tabla->nombre_tabla);

		queue_push(cola_LRU,new_element);
	}

}

static void segment_destroyer(t_segmento* segmento_a_destruir){
	t_pagina* pagina_a_destruir = malloc(sizeof(t_pagina));


	//TODO ver como hacer coincidir el numero de página con la página
	int is_seeked_key(t_cola_LRU* cola_element){
		return cola_element->nro_pagina == pagina_a_destruir->nro_frame;
	}

	void cola_destroyer(t_cola_LRU* cola_element){
		free(cola_element->segmento->nombre_tabla);
		//free(cola_element);
	}

	for(int i=0;!list_is_empty(segmento_a_destruir->tabla);i++){
		pagina_a_destruir = list_remove(segmento_a_destruir->tabla, i);
		if(pagina_a_destruir == NULL)
			continue;
		list_remove_and_destroy_by_condition(cola_LRU->elements, (void*)is_seeked_key, (void*)cola_destroyer);
	}
	free(pagina_a_destruir);
	list_destroy_and_destroy_elements(segmento_a_destruir->tabla,(void*)page_destroyer);
	return;
}

static void page_destroyer(t_pagina* pagina_a_destruir){
	//int x = 0;
	//memmove(pagina_a_destruir->pagina, &x, sizeof(t_frame));
	bitarray_clean_bit(frame_bitarray, pagina_a_destruir->nro_frame);
	//memcpy(cola_element->registro->pagina->value,NULL,0);
	free(pagina_a_destruir->pagina);
	//free(pagina_a_destruir);?
}
