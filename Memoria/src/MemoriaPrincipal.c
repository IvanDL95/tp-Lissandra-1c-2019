/*
 * MemoriaPrincipal.c
 *
 *  Created on: 23 jul. 2019
 */

#include "MemoriaPrincipal.h"

/******************* PRIVATE FUNCTIONS DECLARE ********************************/

static t_frame* copiar_a_memoria(t_frame* frame, const KEY_T, const TIME_T, const char* value);
static struct mem_struct* copiar_desde_memoria(t_frame*);
static void page_destroyer(t_pagina*);
static void segment_destroyer(t_segmento*);
static void actualizar_cola(int nro_pag, t_segmento* tabla);
static t_frame* algoritmo_reemplazo(const KEY_T, const char* valor);
static int _misma_tabla(t_cola_LRU* ultima_pagina,const char* nombre_tabla);

/******************************************** PUBLIC *************************************/

void inicializar_memoria(){
	memoria_principal = calloc(1,config_MP.TAM_MEM);
	bitarray_block = malloc(CANTIDAD_FRAMES);
	frame = malloc(sizeof(t_frame)*CANTIDAD_FRAMES);
	frame_bitarray = bitarray_create_with_mode(bitarray_block, CANTIDAD_FRAMES/8, LSB_FIRST);

	frame[0].base = memoria_principal;
	frame[0].nro_frame = 0;
	frame[0].offset = TAMANIO_PAGINA;

	for(int i=1;i<CANTIDAD_FRAMES;i++){
	    frame[i].base = frame[i-1].base + frame[i-1].offset;
	    frame[i].nro_frame = i;
	    frame[i].offset = TAMANIO_PAGINA;
	}
	log_debug(logger, "Malloc memoria exitoso\n");


	tabla_segmentos = list_create();
}

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

t_pagina* buscar_key(t_segmento* segmento_encontrado, KEY_T key){
	log_debug(logger,"Buscando key\n");
	/*
	 Ahora busca por índice en la tabla de páginas ya que key == índice lista
	*/
	int _is_equal_key(t_pagina* pagina){
		KEY_T key_actual = obtener_key(pagina);
		// la lógica del list_find está al revés
		log_trace(logger,"Compara %d con %d\n", key, key_actual);
		return (key == key_actual);
	}

	if(!list_is_empty(segmento_encontrado->tabla)){
		t_pagina* current_page = ((t_pagina*)list_find(segmento_encontrado->tabla,(void*)_is_equal_key));
		if(current_page != NULL){
			actualizar_cola(current_page->nro,segmento_encontrado);
			return current_page;
		}else
			return NULL;
	}
	else
		return NULL;
}

t_pagina* solicitar_pagina(t_segmento* current_segmento, const char* valor, const KEY_T key){
	log_debug(logger,"Solcitando página para value: %s\n", valor);
	int i;

	t_frame* frame_modificado;

	for(i=0;i<CANTIDAD_FRAMES;i++){
		if(!bitarray_test_bit(frame_bitarray,i)){
			log_trace(logger,"Frame libre!: %d\n",i);
			char* new_valor = malloc(tamanio_value);
			strcpy(new_valor,valor);

			//te devuelve el puntero a la pagina asignada
			frame_modificado = copiar_a_memoria(&frame[i], key,(TIME_T)time(NULL),new_valor);
			break;
		}
		log_trace(logger,"Frame ocupado: %d\n",i);
	}

	if(i == CANTIDAD_FRAMES){
		log_info(logger,"Todas las páginas están ocupadas. Ejecutando algoritmo de reemplazo\n");

		frame_modificado = algoritmo_reemplazo(key, valor);
		if(frame_modificado == NULL && esta_full_memoria){
			printf("Memoria está FULL, iniciando proceso de journal\n");
			char* nombre_tabla = current_segmento->nombre_tabla;
			hacer_journal();
			t_segmento* nuevo_segmento = crear_nuevo_segmento(nombre_tabla);
			return solicitar_pagina(nuevo_segmento, valor, key);
		}
	}

	t_pagina* nuevo_registro = malloc(sizeof(t_pagina));
	nuevo_registro->frame = frame_modificado;
	//nuevo_registro->modificado = flag_state;

	list_add(current_segmento->tabla,nuevo_registro);
	nuevo_registro->nro = current_segmento->tabla->elements_count - 1;
	actualizar_cola(nuevo_registro->nro,current_segmento);

	return nuevo_registro;
}

char* obtener_valor(t_segmento* segmento, int nro_pag){
	t_pagina* pagina = list_get(segmento->tabla, nro_pag);
	struct mem_struct* key_ts_val = copiar_desde_memoria(pagina->frame);
	return key_ts_val->value;
}

TIME_T obtener_timestamp(t_segmento* segmento, int nro_pag){
	t_pagina* pagina = list_get(segmento->tabla, nro_pag);
	struct mem_struct* key_ts_val = copiar_desde_memoria(pagina->frame);
	return key_ts_val->timestamp;
}

KEY_T obtener_key(t_pagina* pagina){
	struct mem_struct* key_ts_val = copiar_desde_memoria(pagina->frame);
	return key_ts_val->key;
}

void actualizar_pagina(t_segmento* tabla,t_pagina* pagina_encontrada, const KEY_T key, const char* nuevo_valor){
	copiar_a_memoria(pagina_encontrada->frame, key, (TIME_T)time(NULL), nuevo_valor);
	pagina_encontrada->modificado = MODIFICADO;
	actualizar_cola(pagina_encontrada->nro,tabla);
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
		KEY_T key;

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
				key = *(int*)pagina_actual->frame->base;
				list_remove_by_condition(cola_LRU->elements,(void*)_has_same_key_and_table);
				bitarray_clean_bit(frame_bitarray, pagina_actual->frame->nro_frame);
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

static t_frame* copiar_a_memoria(t_frame* frame, const KEY_T key, const TIME_T timestamp, const char* value){
	bitarray_set_bit(frame_bitarray,frame->nro_frame);

	memcpy(frame->base, &key, KEY_OFFSET);
	memcpy(frame->base+KEY_OFFSET, &timestamp, TIME_OFFSET);
	strcpy(frame->base+KEY_OFFSET+TIME_OFFSET, value);

	log_trace(logger,"Key: %d, Timestamp: %d, Value: %s",
			*(KEY_T*)(frame->base),
			*(TIME_T*)(frame->base+KEY_OFFSET),
			(char*)(frame->base+KEY_OFFSET+TIME_OFFSET));

	return frame;
}

static struct mem_struct* copiar_desde_memoria(t_frame* frame_a_copiar){
	struct mem_struct* copy_mem = malloc(sizeof(struct mem_struct));

	memcpy(&copy_mem->key, frame_a_copiar->base, KEY_OFFSET);
	memcpy(&copy_mem->timestamp,frame_a_copiar->base+KEY_OFFSET, TIME_OFFSET);
	copy_mem->value = malloc(tamanio_value);
	strcpy(copy_mem->value,frame_a_copiar->base+KEY_OFFSET+TIME_OFFSET);

	return copy_mem;
}

t_cola_LRU* revisar_cola(const char* nombre_tabla, KEY_T key){
	if(queue_is_empty(cola_LRU))
		return NULL;

	t_cola_LRU* ultima_pagina = list_get(cola_LRU->elements,cola_LRU->elements->elements_count - 1);

	if(_misma_tabla(ultima_pagina,nombre_tabla)){
		t_pagina* pagina_cola = list_get(ultima_pagina->segmento->tabla,ultima_pagina->nro_pagina);
		if(obtener_key(pagina_cola) == key)
			return ultima_pagina;
		else
			return NULL;
	}
	else
		return NULL;

}

static void actualizar_cola(int nro_pagina, t_segmento* tabla){

	int is_seeked_queue_element(t_cola_LRU* cola_element){
		if(!strcmp(tabla->nombre_tabla,cola_element->segmento->nombre_tabla)){
			t_pagina* pag_actual = list_get(cola_element->segmento->tabla, cola_element->nro_pagina);
			return pag_actual->nro == nro_pagina;
		}
		return false;
	}

	t_cola_LRU* elemento = (t_cola_LRU*)list_remove_by_condition(cola_LRU->elements,(void*)is_seeked_queue_element);

	if(elemento != NULL)
		queue_push(cola_LRU,elemento);
	else{
		t_cola_LRU* new_element = malloc(sizeof(t_cola_LRU));
		new_element->nro_pagina = nro_pagina;
		new_element->segmento = tabla;

		queue_push(cola_LRU,new_element);
	}
}

static t_frame* algoritmo_reemplazo(const KEY_T key, const char* valor){

	t_cola_LRU* cabeza_cola = malloc(sizeof(t_cola_LRU));
	t_cola_LRU* primer_elemento = queue_peek(cola_LRU);

	while(!queue_is_empty(cola_LRU)){
		cabeza_cola = queue_pop(cola_LRU);
		if(primer_elemento->nro_pagina == cabeza_cola->nro_pagina &&
			!strcmp(primer_elemento->segmento->nombre_tabla, cabeza_cola->segmento->nombre_tabla))
			break;

		t_pagina* ultima_pagina = (t_pagina*)list_remove(cabeza_cola->segmento->tabla, cabeza_cola->nro_pagina);

		if(ultima_pagina->modificado == MODIFICADO){
			list_add_in_index(cabeza_cola->segmento->tabla,cabeza_cola->nro_pagina, ultima_pagina);
			queue_push(cola_LRU,cabeza_cola);
			continue;
		}

		t_pagina* _cambia_nro_pagina(t_pagina* pagina_a_modificar){
			if(pagina_a_modificar->nro > ultima_pagina->nro){
				pagina_a_modificar->nro -= 1;
			}
			return pagina_a_modificar;
		}
		tabla_paginas aux_tabla_paginas = list_duplicate(cabeza_cola->segmento->tabla);
		cabeza_cola->segmento->tabla = list_map(aux_tabla_paginas,(void*)_cambia_nro_pagina);

		t_frame* frame_reemplazado = copiar_a_memoria(ultima_pagina->frame,key, time(NULL), valor);
		list_destroy(aux_tabla_paginas);
		free(ultima_pagina);
		free(cabeza_cola);
		return frame_reemplazado;
	}

	free(cabeza_cola);
	esta_full_memoria = true;
	return NULL;
}

static void segment_destroyer(t_segmento* segmento_a_destruir){
	t_pagina* pagina_a_destruir = malloc(sizeof(t_pagina));


	//TODO ver como hacer coincidir el numero de página con la página
	int is_seeked_key(t_cola_LRU* cola_element){
		KEY_T key;
		memcpy(&key,pagina_a_destruir->frame->base,KEY_OFFSET);
		return cola_element->nro_pagina == key;
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
	//memmove(pagina_a_destruir->frame, &x, sizeof(t_frame));
	bitarray_clean_bit(frame_bitarray, pagina_a_destruir->frame->nro_frame);
	//memcpy(cola_element->registro->frame->value,NULL,0);
	free(pagina_a_destruir->frame);
	//free(pagina_a_destruir);?
}

static int _misma_tabla(t_cola_LRU* ultima_pagina, const char* nombre_tabla){
	return (!strcmp(ultima_pagina->segmento->nombre_tabla, nombre_tabla));
}

/*
  static int asignar_key(tabla_paginas page_table){
	if(list_is_empty(page_table))
		return 1;

	int _is_bigger_key(t_pagina* x, t_pagina* y){
		return x->frame->key > y->frame->key;
	}

	t_list* tabla_ordenada_segun_key = list_sorted(page_table,(void*)_is_bigger_key);
	int key_anterior = (((t_pagina*)list_get(tabla_ordenada_segun_key, 0))->frame->key);

	return key_anterior+1;
}
*/


