/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ARCHIVO PARA INCLUIR FUNCIONES DE SENSORES Y AÑADIR MEDICIONES A "txBuffer"

Este archivo ha sido modificado de manera considerable para implementar el sensor ultrasónico de distancia JSN-SR04T en vez del original BME280.
Enlace para consultar el proyecto original del usuario de GitHub "rwanrooy": https://github.com/rwanrooy/TTGO-PAXCOUNTER-LoRa32-V2.1-TTN.git
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <QuickMedianLib.h>                                                                       // Librería para obtener la mediana de un array
#include "configuration.h"                                                                        // Se usan macros declarados en dicho archivo
#include "lvlbat.h"                                                                               // Se llama a la funcion de medicion de bateria para añadirlo al txBuffer

int lista_medidas[SENSOR_SAMPLES];                                                                // Array de medidas y su tamaño, al que se le va a aplicar la mediana

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// JSN-SR04T config - FUNCION PROPIA DEL SENSOR JSN-SR04T
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int obten_distancia(){
  uint32_t duracion;
  uint16_t distancia;
  
  digitalWrite(TRIG_PIN, LOW);                                                                    // Reestablecimiento de "TRIG_PIN" poniéndolo en LOW
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);                                                                   // Accionar el sensor poniendo el "TRIG_PIN" en HIGH durante 10 microsegundos
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duracion = pulseIn(ECHO_PIN, HIGH);                                                             // Leer "ECHO_PIN". "pulseIn()" devuelve la duración del pulso en microsegundos
  distancia = (duracion / 2) * 0.034;                                                             // Calcular la distancia. Se divide entre 2 la duración ya que es un viaje de ida y vuelta. Se divide entre 0.034, ya que la velocidad del sonido es 340 m/s, pero la distancia se calcula en cm
  return distancia;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Funcion para calcular la mediana de un numero n de muestras de distancia para hacer el dato del nivel del agua más robusto - FUNCION PROPIA DEL SENSOR JSN-SR04T
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint16_t obten_distancia_mediana(uint8_t muestras){                                               // Función para obtener el número deseado de medidas y calcular la mediana
  uint16_t distancia_sonar, distancia_mediana;

  digitalWrite(VOUT_PIN, HIGH);                                                                   // Lo primero es cambiar a HIGH el MOSFET que permite la alimentación al sensor
  delay(2000);                                                                                    // Delay de cortesía para estabilizar el cambio de estado del MOSFET

  // Generador del array de la mediana ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for(uint8_t i = 0; i < muestras; i++){                                                          // Bucle para iterar tantas veces como numero de medidas se haya establecido
    distancia_sonar = obten_distancia();

    uint32_t tiempoInicio = millis();                                                             // Variable para almacenar el tiempo actual en millisegundos
    float tiempoTranscurrido;                                                                     // Variable para almacenar el tiempo transcurrido en segundos en el que se intenta corregir el error, aquellas medidas menores de 24 y mayores de 400

    // Iterador temporizado para corregir medidas erroneas ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    uint8_t index = 0;                                                                            // Reseteador del indice del bucle "while"
    while((millis() - tiempoInicio < 5000) && (distancia_sonar < 25 || distancia_sonar > 400)){   // Bucle para iterar durante un máximo de tiempo establecido si esta habiendo error
      tiempoTranscurrido = (millis() - tiempoInicio) / 1000.0;                                    // Calculador de tiempo transcurrido

      Serial.print(F("Medida conflictiva ")); Serial.print(index + 1); Serial.print(F(": ")); Serial.println(distancia_sonar);
      Serial.print(F("Tiempo transcurrido ")); Serial.print(tiempoTranscurrido); Serial.println(F(" segundos"));
      Serial.println(F("-------------------"));

      distancia_sonar = obten_distancia();                                                        // Actualizador de medidas
      index++;
      delay(250);
    }
  
    Serial.print(F("La muestra ")); Serial.print(i + 1); Serial.print(F(" es ")); Serial.println(distancia_sonar);

    lista_medidas[i] = distancia_sonar;                                                           // Las medidas correctas se añaden al array de la mediana
    delay(250);
  }

  digitalWrite(VOUT_PIN, LOW);                                                                    // Despues de haberse cogido las medidas, el sensor se apaga poniendo a LOW el MOSFET
  delay(2000);                                                                                    // Delay de cortesia para el cambio de estado del MOSFET

  Serial.println(F("==================="));
  distancia_mediana = QuickMedian<int>::GetMedian(lista_medidas, muestras);                       // La mediana se calcula con las medidas del array
  return distancia_mediana;
}

// ===========================================================================================================================================================
// FUNCION PRINCIPAL DE "sensor.ino" - Construir el paquete de datos que se enviara por LoRa, SE DEBE EDITAR PARA INCLUIR LAS VARIABLES DE OTROS SENSORES, PERO MANTENER LA ESTRUCTURA DE DESCOMPOSICION EN BYTES PARA SER AÑADIDOS AL "txBuffer"
// ===========================================================================================================================================================
void build_packet(uint8_t txBuffer[TX_BUFFER_SIZE]){                                              // Uso 'uint8_t' para que todos los datos sean del tamaño de 1byte (8bits)  
  // Distancia ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint16_t txDistancia = obten_distancia_mediana(SENSOR_SAMPLES);
  txBuffer[0] = lowByte(txDistancia);                                                             // Para distancia, que puede estar entre 0 y 400, 'lowByte()' me da valores de 0 a 255. Si 'distance' supera n*(2^8), entonces 'distance' = highByte()*256 + lowByte()
  txBuffer[1] = highByte(txDistancia);                                                            // Si la distancia supera los 255, entonces se acumula en 'highByte()' una unidad por cada vez que pase. Por ello, en el Payload formatters de TTN, la misma fórmula en .json se representa ---------------------------------> data.distancia = (bytes[0] | (bytes[1] << 8))

  Serial.print(F("Distancia hasta el agua: ")); Serial.print(txDistancia); Serial.println(F(" cm"));

  // Bateria ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint8_t txBatLvl = battery_level();
  txBuffer[2] = lowByte(txBatLvl);                                                               // Como txBatlvl es un porcentaje entero, entra en el "lowByte"

  Serial.print(F("Nivel de la batería: ")); Serial.print(txBatLvl); Serial.println(F(" %"));
  Serial.println(F("==================="));                                                      // Separador visual entre bloques de datos debug
}