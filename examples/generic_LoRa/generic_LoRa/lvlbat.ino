/* ***********************************************************************************************************************************************************
ARCHIVO DE MEDICION DEL NIVEL DE BATERIA

Archivo para medir el nivel de bateria teniendo en cuenta que la descarga de una Li-ion 18650 no es lineal.
SE DEBE HACER UN DIVISOR RESISTIVO ENTRE LA BATERÍA Y EL "VBAT_PIN" CON UNA RESISTENCIA DE 22K y otra de 47K.
*********************************************************************************************************************************************************** */
#include "configuration.h"                                                           // Se usan macros declaradas en dicho archivo

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion medir el nivel de batería
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t battery_level(){
  uint16_t analogValue = analogRead(VBAT_PIN);
  uint8_t bat_percent;
  if(analogValue >= 3500) bat_percent = 100;                                         // Cualquier cosa por encima, 100%
  else if(analogValue >= 2960) bat_percent = map(analogValue, 2960, 3500, 90, 100);  // Rango 90-100%, 3.7-4.2V
  else if(analogValue >= 2525) bat_percent = map(analogValue, 2525, 2960, 10, 90);   // Rango 10-90%, 3.2-3.7V
  else if(analogValue >= 1940) bat_percent = map(analogValue, 1940, 2525, 0, 10);    // Rango 0-10%, 2.5-3.2V
  else bat_percent = 0;                                                              // Cualquier cosa por debajo, 0%
  
  return bat_percent;
}