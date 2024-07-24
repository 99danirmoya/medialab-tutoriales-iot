/* ***********************************************************************************************************************************************************
SENSOR COOPER V3.3 Release Candidate - MediaLab_ IoT, UNIVERSIDAD DE OVIEDO

Este archivo ha sido modificado de manera considerable para implementar el sensor ultrasónico de distancia JSN-SR04T en vez del original BME280.
El enlace para consultar el proyecto original del usuario de GitHub "rwanrooy" es el siguiente:
https://github.com/rwanrooy/TTGO-PAXCOUNTER-LoRa32-V2.1-TTN.git

A LO LARGO DEL PROYECTO, SE INDICA CON LINEAS HECHAS CON "~" AQUELLAS SECCIONES DE CODIGO QUE DEBEN SER EDITADAS A GUSTO, EL RESTO NO SE DEBE DE MODIFICAR.
*********************************************************************************************************************************************************** */
#include "configuration.h"                                               // Libreria de macros 
#include "sensor.h"                                                      // Libreria de funciones de sensores y construccion del "txBuffer" con las medidas a enviar
#include "sleep.h"                                                       // Libreria de funciones para activar el deep sleep
#include "vsi.h"                                                         // Libreria de funciones para activar el Variable Send Interval
#include "rom/rtc.h"                                                     // Libreria para usar la memoria RTC del ESP32, donde se pueden guardar variables cuyos valores sobreviven al deep sleep

RTC_DATA_ATTR uint32_t bootCount = 0;                                    // Contador de despertares tras deep sleep, almacenado en la memoria RTC para sobrevivir deep sleep. Se usa para el control de los mensajes confirmados
RTC_DATA_ATTR uint16_t bufferCircular[TARGET_ARRAY_LENGTH];              // Lista donde guardo los últimos 5 valores enviados a TTN. LA ALOJO EN LA MEMORIA RTC PARA QUE SOBREVIVA AL DEEP SLEEP

static uint8_t txBuffer[TX_BUFFER_SIZE];                                 // Como el array de bytes que se envía a TTN se calcula en otro archivo, lo declaro como static para que sea visible. AJUSTAR EL NUMERO DE BYTES AL DEL PAYLOAD

static uint32_t SEND_INTERVAL;                                           // Tanto el "SEND_INTERVAL" como "txBuffer" se declaran como "static" ya que se usan en el "main", pero vienen de otros archivos

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para enviar el paquete de datos LoRa
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void do_send(){
  build_packet(txBuffer);
  ttn_send(txBuffer, sizeof(txBuffer), LORAWAN_PORT, false);

  ttn_cnt(bootCount);
  bootCount++;                                                           // Se le suma uno al contador de arranques tras cada ciclo de envío de datos por LoRa
    
  // Gestión del Send Interval (AQUÍ PARA EVITAR ENÉSIMAS ITERACIONES) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  #if ENABLE_VSI == 1
    uint16_t distanciaBuffer = txBuffer[0] + (txBuffer[1] << 8);         // Variable en la que se guarda el último valor de distancia enviado a TTN y almacenado en el buffer circular. SE DEBE EDITAR EL TIPO DE VARIABLE (p.e.: int, uint8_t, long, ...), TENIENDOSE EN CUENTA EL NUMERO DE BYTES
    carga_valores(distanciaBuffer); print_array();                       // Se cargan el valor actual del sensor al buffer circular y se imprime por pantalla el estado actual del buffer
    SEND_INTERVAL = variable_send_interval();                            // Se asigna el valor del "Variable Send Interval" a la variable "SEND_INTERVAL"
  #else
    SEND_INTERVAL = static_send_interval();                              // Se asigna el valor del "Static Send Interval" a la variable "SEND_INTERVAL"
  #endif
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para mostrar mensajes por monitor serial segun se interactue con TTN por medio de "ttn.ino"
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void callback(uint8_t message){
  if(EV_JOINING       == message){ Serial.println(F("Conectando con TTN..."));                       Serial.println(F("-------------------")); }
  if(EV_JOINED        == message){ Serial.println(F("¡Conectado con TTN!"));                         Serial.println(F("-------------------")); }
  if(EV_JOIN_FAILED   == message){ Serial.println(F("¡Conexion con TTN fallida!"));                  Serial.println(F("-------------------")); }
  if(EV_REJOIN_FAILED == message){ Serial.println(F("¡Nuevo intento de conexion con TTN fallido!")); Serial.println(F("-------------------")); }
  if(EV_RESET         == message){ Serial.println(F("Reseteo de conexion con TTN"));                 Serial.println(F("-------------------")); }
  if(EV_LINK_DEAD     == message){ Serial.println(F("¡Link con TTN muerto!"));                       Serial.println(F("-------------------")); }
  if(EV_ACK           == message){ Serial.println(F("¡ACK recibido!"));                              Serial.println(F("-------------------")); }
  if(EV_PENDING       == message){ Serial.println(F("Mensaje descartado"));                          Serial.println(F("-------------------")); }
  if(EV_QUEUED        == message){ Serial.println(F("Mensaje en cola..."));                          Serial.println(F("-------------------")); }
  if(EV_TXCOMPLETE    == message){                                       // Este mensaje indica que se ha podido realizar el envío de datos por LoRa
    Serial.println(F("EV_TXCOMPLETE (incluye espera para ventanas de RX)"));
    if(LMIC.txrxFlags & TXRX_ACK) Serial.println(F("¡ACK recibido!"));
    if(LMIC.dataLen){ Serial.print(F("Recibidos ")); Serial.print(LMIC.dataLen); Serial.println(F(" bytes de payload")); }
    Serial.println(F("-------------------"));
    sleep();                                                             // Me voy a dormir aquí, cuando se confirma el envío del payload
  }
}

// ===========================================================================================================================================================
// Setup main - MODIFICAR PIN MODES Y SUS CONDICIONES INICIALES
// ===========================================================================================================================================================
void setup(){
  #if ENABLE_DEBUG == 1                                                  // Activar o desactivar desde "configuration.h" el monitor serial para debugging
    DEBUG_PORT.begin(SERIAL_BAUD);
  #endif

  // Setup de los perifericos que envian por LoRa ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bme280_setup();
  sds011_setup();  
  
  // TTN setup -----------------------------------------------------------------------------------------------------------------------------------------------
  if(!ttn_setup()){
    Serial.println(F("[ERR] Chip de radio LoRa no encontrado, ¡ACTIVANDO DEEP SLEEP HASTA REINICIO MANUAL!"));
    delay(MESSAGE_TO_SLEEP_DELAY);
    esp_deep_sleep_start();                                              // Aquí me voy a dormir para siempre, como si fuese un while(1) pero de bajo consumo
  }

  // TTN register --------------------------------------------------------------------------------------------------------------------------------------------
  ttn_register(callback);                                                // Funcion de eventos de TTN
  ttn_join();
  ttn_sf(LORAWAN_SF);                                                    // Spreading Factor
  ttn_adr(LORAWAN_ADR);
}

// ===========================================================================================================================================================
// Loop main
// ===========================================================================================================================================================
void loop(){
  ttn_loop();                                                            // Función definida en "ttn.ino" en la que se ejecuta la función principal "os_runloop_once()" de la librería LMIC
  
  static uint32_t tiempoPrevio = 0;
  if(tiempoPrevio == 0 || millis() - tiempoPrevio > SEND_INTERVAL){      // Se espera el tiempo hasta que se cumpla el duty cycle, salvo si es la primera ejecución del programa, donde se usa la condición "tiempoPrevio == 0"
    tiempoPrevio = millis();                                             // Actualizamos el valor del "tiempoPrevio", que sería el momento en el que se envío el último payload
    Serial.print(F("TRANSMISION NUMERO ")); Serial.println(bootCount + 1); Serial.println(F("==================="));
    do_send();                                                           // Llamamos a la función "send()", encargada de enviar los datos por LoRa
  }
}