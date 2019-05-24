# Proceso Memoria

El proceso memoria cumplirá la función de disponibilizar los datos requeridos para la obtención y actualización que los distintos request requieran de manera concurrente. Todas aquellas solicitudes que le podíamos pedir al LFS también se las podremos solicitar a la memoria. Cada segmento estará **identificado por el path de la tabla a la cual pertenece**.
Las tablas pueden ser modificadas dentro de la memoria. Para identificar aquellas páginas que fueron modificadas utilizaremos el concepto *Flag de Modificado*. Se sabrá entonces que aquellas páginas cuyo *Flag de Modificado* esté activado tienen un estado más actualizado que la tabla del FS.

Cada memoria estará definida de la siguiente manera:
- Se asumirá el concepto de Localidad Temporal.
- Para administrar el contenido de los datos se utilizará el concepto de Segmentación Paginada:
  - Tabla Segmentos: Lista de Segmentos
  - Segmento:       Tabla paginada + Identificador de Tabla
  - Tabla paginada: Lista de Registros
  - Registros:      Nro. de página + Página + Flag de modificado  
- Bajo estos conceptos:
  - Cada segmento es una tabla de la base de datos
  - Cada página es un registro de una tabla del LFS
  
 Los cambios realizados sobre las tablas que se encuentren en memoria, por los distintos request, luego deberán verse impactados en el LFS, mediante el proceso al que llamamos Journaling.
 
### Start Up

Al momento de iniciar el proceso memoria se deben realizar los siguientes pasos:
1. Conectarse al proceso File System y realizar handshake necesario para obtener los datos requeridos. Esto incluye el tamaño máximo del Value configurado para la administración de las páginas.
2. Inicializar la memoria principal.
   - Crear la Tabla de segmentos
   - Se reservará espacio para la Memoria Principal. La misma se define como un espacio de memoria (física) contigua donde se alojan las distintas páginas de los segmentos utilizados.
3. Iniciar el proceso de Gossiping que consiste en la comunicación de cada proceso memoria con otros procesos memorias, o seeds, para intercambiar y descubrir otros procesos memorias que se encuentren dentro del pool (conjunto de memorias).

**Todos los componentes deben ser inicializados por única vez al iniciar el proceso, reservando en este momento la memoria necesaria para la memoria principal.**

### Algoritmo de reemplazo

Se utilizará **el algoritmo LRU (Least Recently Used)** :
 - La página a reemplazar no debe tener el *Flag de Modificado* activado.
 - Si todas las páginas/frames se encuentran en estado Modificado, se dice que una memoria está **FULL (completa)**. Una memoria en este estado no responderá ningún pedido que implique el reemplazo de uno de los segmentos actuales. Se debe comenzar el *proceso de Journal*, el cual, una vez finalizado, permitirá a la memoria volver a aceptar los pedidos que se le realicen.
 
 ### API
 
 Cada función se define de la siguiente manera:
 
 OPERACION *argumento[0] argumento[1] argumento[2]* ...
 
 Siendo argumento un char**, es decir un array de strings y argumento[i] el argumento en la posición i. 
 
 * SELECT [NOMBRE_TABLA] [KEY]
 
    Valor de retorno -> char* value
    
   - Verificar si existe el segmento de la tabla solicitada y busca en las páginas del mismo si contiene key solicitada. 
      - Si la contiene, devuelve el *Value* y finaliza el proceso.
      - Si no la contiene:
        1. Envía la solicitud a FileSystem para obtener el valor solicitado y almacenarlo
        2. Se debe solicitar una nueva página libre para almacenar el mismo. 
           En caso de no disponer de una página libre:
           - Se debe ejecutar el algoritmo de reemplazo 
           - En caso de no poder efectuarlo por estar la memoria full, ejecutar el Journal de la memoria.

 * INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
 
    Valor de retorno -> char* mensaje_fin_operacion
    
   - Verificar si existe el segmento de la tabla en la memoria principal 
     - En caso de que exista:
       1. Buscar en sus páginas si contiene la *Key* solicitada  
       2. Al retornar:
          - De contenerla actualiza el valor insertando el Timestamp actual. 
          - En caso que no contenga la Key, se solicita una nueva página para almacenar la misma. Se deberá tener en cuenta que si no se disponen de páginas libres aplicar el algoritmo de reemplazo y en caso de que la memoria se encuentre full iniciar el proceso Journal.

     - En caso que no se encuentre el segmento:
       1. Se debe generar el nuevo segmento y solicitar una nueva página (aplicando para esto la misma lógica que el punto anterior)
       2. Se creará y se agregará la nueva Key con el Timestamp actual, junto con el nombre de la tabla en el segmento.

 * CREATE [TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
 
    Valor de retorno -> char* mensaje_fin_operacion
 * DESCRIBE [NOMBRE_TABLA]
 
    Valor de retorno -> char* descripción_metadata
 * DROP [NOMBRE_TABLA]
 
    Valor de retorno -> char* mensaje_fin_operacion
 * JOURNAL
 
    Valor de retorno -> char* mensaje_fin_operacion

### Journal

El proceso de *Journal* es el encargado de informar al File System todos los cambios realizados en la memoria principal para luego vaciar completamente la misma. Identificamos todas aquellas escrituras a ser notificadas al FS con el “Flag de Modificado”.
Este proceso tiene dos modos de ejecución: 
 - Por medio de la API
 - Automáticamente cada X unidades de tiempo (el parámetro X se obtiene desde el archivo de configuración)
 - Forzosamente debido a que se entró en un estado *FULL* y se requieren nuevas páginas para asignar (realizado por pedido del Kernel)

Se debe:
1. Encontrar todas aquellas páginas cuyas *Key* deben ser actualizadas dentro del FS:
   1. Buscar páginas con *Flag de Modificado* activado
      - La tabla de Segmentos tiene Segmentos
      - Cada Segmento tiene una Tabla de Páginas
      - Cada Tabla de Páginas tiene un registro que contiene el *Flag de Modificado*
   2. Retornar las *Key* con el *Value* a actualizar de las páginas que cumplan dicha condición
2. Enviar, por cada una de estas, una petición de *INSERT* al FileSystem indicando los datos adecuados

Una vez efectuados estos envíos se procederá a 
 - Desactivar el *Flag de Modificado* de todas las páginas 
 - **Eliminar los segmentos actuales.**

**En caso de una caída global del sistema, los Flags de Modificado se perderán, pudiendo estos cambios nunca ser impactados finalmente en el FS. Este es un riesgo que estamos dispuestos a tomar.**

### Proceso de Gossiping

 . TODO
