/* ***********************************************************************************************************************************************************
ARCHIVO DE FUNCIONES PARA EL TIEMPO DE DEEP SLEEP

Se calcula el tiempo de deep sleep en "sleep_for", ajustandose con la resta de ese exceso de tiempo si el "SEND_INTERVAL" es mayor que el tiempo actual.
*********************************************************************************************************************************************************** */
#include <esp_sleep.h>
#include "configuration.h"                                                                       // Se usan macros declaradas en dicho archivo
#include "vsi.h"                                                                                 // Se usa el "SEND_INTERVAL"

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para dormir el ESP32 durante el tiempo de duty cycle
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void sleep(){
  #if SLEEP_BETWEEN_MESSAGES == 1
    Serial.println(F("Iniciando deep sleep...")); Serial.println(F("~~~~~~~~~~~~~~~~~~~"));      // Mostrar mensaje de que se ha iniciado el deep sleep
    Serial.println();
    delay(MESSAGE_TO_SLEEP_DELAY);                                                               // Esperar por "MESSAGE_TO_SLEEP_DELAY" para dormir

    uint32_t sleep_for = (millis() < SEND_INTERVAL) ? SEND_INTERVAL - millis() : SEND_INTERVAL;  // La sintaxis del operador ternario es "variable = (if condition) ? true : else"
    esp_sleep_enable_timer_wakeup(sleep_for * 1000);                                             // Temporizar el deep sleep durante el tiempo establecido
    esp_deep_sleep_start();
  #endif
}