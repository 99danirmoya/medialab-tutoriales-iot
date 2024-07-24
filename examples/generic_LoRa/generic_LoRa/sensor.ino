/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ARCHIVO PARA INCLUIR FUNCIONES DE SENSORES Y AÑADIR MEDICIONES A "txBuffer"

Este archivo ha sido modificado de manera considerable para implementar el sensor climático BME280 junto con el sensor de particulas de materia SDS011. La
librería de este último es una versión modificada de la original: https://github.com/misan/SDS011/tree/master 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Librerias de sensores
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "lvlbat.h"                                                                               // Se llama a la funcion de medicion de bateria para añadirlo al txBuffer
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SDS011.h>
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Macros, constructores, variables... de sensores
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Adafruit_BME280 bme;                                                                              // Constructor del BME280
SDS011 sds;                                                                                       // Constructor del SDS011
HardwareSerial SerialSDS(2);                                                                      // Se crea el segundo puerto serie para el sensor de particulas

int temp, hum, alt, pres, pm10_int, pm25_int, error;                                              // Variables int y float de los sensores
float pm10, pm25;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Funciones setup de los sensores - Se crean aqui para tenerlas en el scope correcto, luego se llaman en el 'setup()' principal
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void bme280_setup(){
  Wire.begin();                                                                                   // Inicializa I2C
  if(!bme.begin()){
    Serial.println(F("No encuentro un sensor BME280 valido!"));
    while (1);
  }
}

void sds011_setup(){
  SerialSDS.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);                                              // Inicializa el puerto serie del SDS
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ===========================================================================================================================================================
// FUNCION PRINCIPAL DE "sensor.ino" - Construir el paquete de datos que se enviara por LoRa, SE DEBE EDITAR PARA INCLUIR LAS VARIABLES DE OTROS SENSORES, PERO MANTENER LA ESTRUCTURA DE DESCOMPOSICION EN BYTES PARA SER AÑADIDOS AL "txBuffer"
// ===========================================================================================================================================================
void build_packet(uint8_t txBuffer[TX_BUFFER_SIZE]){                                              // Uso 'uint8_t' para que todos los datos sean del tamaño de 1byte (8bits)  
  // read the temperature from the BME280 -----------------------------------------------------------------------------------------------------------------------
  temp = (bme.readTemperature()) * 100;
  Serial.print("Temperature: "); Serial.print(temp / 100); Serial.println(" *C");
  txBuffer[0] = lowByte(temp);
  txBuffer[1] = highByte(temp);

  // read the humidity from the BME280 -----------------------------------------------------------------------------------------------------------------------
  hum = (bme.readHumidity()) * 100;
  Serial.print("RH: "); Serial.print(hum / 100); Serial.println(" %");
  txBuffer[2] = lowByte(hum);
  txBuffer[3] = highByte(hum);

  // read the pressure from the BME280 -----------------------------------------------------------------------------------------------------------------------
  pres = bme.readPressure();
  Serial.print("Pressure: "); Serial.print(pres); Serial.println(" hPa");
  txBuffer[4] = (byte) ((pres & 0X000000FF)       );
  txBuffer[5] = (byte) ((pres & 0x0000FF00) >> 8  );
  txBuffer[6] = (byte) ((pres & 0x00FF0000) >> 16 );
  txBuffer[7] = (byte) ((pres & 0xFF000000) >> 24 );

  // read the altitude from the BME280 -----------------------------------------------------------------------------------------------------------------------
  alt = (bme.readAltitude(SEALEVELPRESSURE_HPA)) * 100;
  Serial.print("Altitude: "); Serial.print(alt / 100); Serial.println(" m");
  txBuffer[8] = lowByte(alt);
  txBuffer[9] = highByte(alt);

  error = sds.read(&pm25,&pm10);
  if(!error){
    // read the PM2.5 from the SDS011 ------------------------------------------------------------------------------------------------------------------------
    Serial.print("PM2.5"); Serial.print(pm25); Serial.println(" ug/m3");
    pm25_int = pm25 * 10;
    txBuffer[10] = lowByte(pm25_int);
    txBuffer[11] = highByte(pm25_int);

    // read the PM10 from the SDS011 -------------------------------------------------------------------------------------------------------------------------
    Serial.print("PM10"); Serial.print(pm10); Serial.println(" ug/m3");
    pm10_int = pm10 * 10;
    txBuffer[12] = lowByte(pm10_int);
    txBuffer[13] = highByte(pm10_int);
  }

  // Bateria -------------------------------------------------------------------------------------------------------------------------------------------------
  uint8_t txBatLvl = battery_level();
  txBuffer[14] = lowByte(txBatLvl);                                                               // Como txBatlvl es un porcentaje entero, entra en el "lowByte"

  Serial.print(F("Nivel de la batería: ")); Serial.print(txBatLvl); Serial.println(F(" %"));
  Serial.println(F("==================="));                                                      // Separador visual entre bloques de datos debug
}
// ===========================================================================================================================================================