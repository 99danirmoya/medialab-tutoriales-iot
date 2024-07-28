/* ***********************************************************************************************************************************************************
ARCHIVO DE FUNCIONES DEL VARIABLE SEND INTERVAL

El objetivo de este archivo es crear una pila FiFo en la que se carguen los últimos 5 valores de distancia enviados a TTN. Si la desviación estándar es
"pequeña", los datos se envían cada "más tiempo" y, si es "grande", ya que implica cambios significativos en el sistema a medir, los datos se envía cada
"menos tiempo".
*********************************************************************************************************************************************************** */
#include "configuration.h"                                   // Se usan macros declarados en dicho archivo

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para construir el array circular
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void carga_valores(int medidaNueva){
  for(uint8_t i = TARGET_ARRAY_LENGTH - 1; i > 0; i--){      // El indice del bucle va desde la última posición del array a la primera
    bufferCircular[i] = bufferCircular[i - 1];               // El indice del array es igual al índice anterior, "chocan" los valores desde la izquierda y se "barren" a la derecha
  }
  bufferCircular[0] = medidaNueva;                           // Cada vez que quiero meter un valor nuevo, lo cargo en la primera posición del array
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para imprimir el array
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void print_array(){
  Serial.print("Buffer circular: ");
  for(uint8_t i = 0; i < TARGET_ARRAY_LENGTH; i++) {
    Serial.print(bufferCircular[i]); Serial.print(" ");
  }
  Serial.println();
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para obtener la media de los valores de la lista
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
float obten_media(){
  int suma = 0;
  for(uint8_t i = 0; i < TARGET_ARRAY_LENGTH; i++){
    suma += bufferCircular[i];                               // Sumas acumuladas
  }
  return ((float)suma) / TARGET_ARRAY_LENGTH;                // Divido la suma total entre el número de valores de la lista
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para obtener la desviacion estandar de los valores de la lista
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
float obten_desviacion_estandar(){
  float media = obten_media();
  float total = 0.0;
  float varianza, dvstd;
  for(uint8_t i = 0; i < TARGET_ARRAY_LENGTH; i++){
    total = total + sq((bufferCircular[i] - media));         // En cada iteración del bucle, calculo la suma total de los cuadrados de la resta de cada uno de los valores de la lista menos la media
  }

  varianza = total / TARGET_ARRAY_LENGTH;                    // La varianza resulta ser el total entre el número de valores de la lista
  dvstd = sqrt(varianza);                                    // La desviación estandar resulta ser la raíz cuadrada de la varianza
  Serial.print("La media del buffer es "); Serial.print(media); Serial.print(" y la desviación estandar es "); Serial.println(dvstd);
  return dvstd;
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// ===========================================================================================================================================================
// Funcion para decidir que "SEND_INTERVAL" usar en función de la desviación estandar
// ===========================================================================================================================================================
uint32_t variable_send_interval(){
  float desviacionEstandarListaMedidasMedias = obten_desviacion_estandar();
  uint32_t variableSendInterval = (desviacionEstandarListaMedidasMedias <= 1) ? SEND_INTERVAL_RELAXED : SEND_INTERVAL_INTENSIVE;

  Serial.print("El send interval es: "); Serial.println(variableSendInterval); Serial.println(F("==================="));
  return variableSendInterval;
}
// ===========================================================================================================================================================

// ===========================================================================================================================================================
// Funcion para obligar a que el SEND_INTERVAL sea siempre el definido como STATIC
// ===========================================================================================================================================================
uint32_t static_send_interval(){
  uint32_t staticSendInterval = SEND_INTERVAL_STATIC;

  Serial.print("El send interval es: "); Serial.println(staticSendInterval); Serial.println(F("==================="));
  return staticSendInterval;
}
// ===========================================================================================================================================================