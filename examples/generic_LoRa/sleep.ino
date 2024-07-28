/* ***********************************************************************************************************************************************************
ARCHIVO DE FUNCIONES PARA EL TIEMPO DE DEEP SLEEP

Se calcula el tiempo de deep sleep en "sleep_for", ajustandose con la resta de ese exceso de tiempo si el "SEND_INTERVAL" es mayor que el tiempo actual.
*********************************************************************************************************************************************************** */
#include <esp_sleep.h>
#include "configuration.h"                                                                       // Se usan macros declaradas en dicho archivo

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para dormir el ESP32 durante el tiempo de duty cycle
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void sleep(){
  #if SLEEP_BETWEEN_MESSAGES == 1
    Serial.println(F("Iniciando deep sleep...")); Serial.println(F("~~~~~~~~~~~~~~~~~~~"));      // Mostrar mensaje de que se ha iniciado el deep sleep
    Serial.println();
    delay(MESSAGE_TO_SLEEP_DELAY);                                                               // Esperar por "MESSAGE_TO_SLEEP_DELAY" para dormir

    uint32_t sleepFor = (millis() < sendInterval) ? sendInterval - millis() : 0;                 // La sintaxis del operador ternario es "variable = (if condition) ? true : else". SE CORRIGE EL TIEMPO SI EL TIEMPO ACTUAL ES INFERIOR AL TIEMPO DE TRANSMISION PARA EVITAR QUE VARIE
    esp_sleep_enable_timer_wakeup(sleepFor * 1000);                                              // Temporizar el deep sleep durante el tiempo establecido
    esp_deep_sleep_start();
  #endif
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------