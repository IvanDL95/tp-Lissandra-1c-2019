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

Para administrar el reemplazo de las distintas páginas en una memoria principal se utilizará **el algoritmo LRU (Least Recently Used)**. Se debe tener en cuenta que la página a reemplazar no debe tener el *Flag de Modificado* activado.
También se da la situación en que una memoria no dispone de páginas libres para los segmentos actuales, ya que se encuentran en estado Modificado. Cuando esto suceda se dice que una memoria está *FULL (completa)*. Una memoria en este estado no responderá ningún pedido que implique una operación que requiera el reemplazo de uno de los segmentos actuales. 
Se debe comenzar el *proceso de Journal*, el cual, una vez finalizado, permitirá a la memoria volverá a aceptar los pedidos que se le realicen. 


### Journal

El proceso de Journal es el encargado de informar al File System todos los cambios realizados en la memoria principal para luego vaciar completamente la misma. 
Este proceso tiene dos modos de ejecución: 
 - Por medio de la API
 - Automáticamente cada X unidades de tiempo (el parámetro X se obtiene desde el archivo de configuración)
 
 ### API
 
 * SELECT [NOMBRE_TABLA] [KEY]
 
    Valor de retorno -> char* value
 * INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
 
    Valor de retorno -> char* mensaje_fin_operacion
 * CREATE [TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
 
    Valor de retorno -> char* mensaje_fin_operacion
 * DESCRIBE [NOMBRE_TABLA]
 
    Valor de retorno -> char* descripción_metadata
 * DROP [NOMBRE_TABLA]
 
    Valor de retorno -> char* mensaje_fin_operacion
 * JOURNAL
 
    Valor de retorno -> char* mensaje_fin_operacion


### Proceso de Gossiping

 . TODO
