/* ***********************************************************************************************************************************************************
PROGRAMA PLANTILLA PARA ENVIAR Y RECIBIR DATOS POR HTTP A NodeRED. CONSULTAR EL FLUJO DE NodeRED EN LA CARPETA DEL PROYECTO DONDE VIENE EL JSON CON EL FLUJO
QUE SIRVE COMO PLANTILLA. BASICAMENTE, SE USA UN NODO 'http in' CON LA URL 'data' QUE SE CONECTA A UN NODO 'change' QUE ENVIA 'recibido' A UN NODO
'http response'. LAS SECCIONES DE CODIGO QUE SE DEBEN EDITAR PARA INLCUIR OTROS SENSORES Y FUNCIONES VIENEN INDICADAS CON EL SEPARADOR '===' Y, LAS QUE DEBEN
QUEDAR COMO ESTAN EN ESTA PLANTILLA, CON EL SEPARADOR '---'
*********************************************************************************************************************************************************** */
// ===========================================================================================================================================================
// INLCUSION DE LIBRERIAS
// ===========================================================================================================================================================
#include <WiFiManager.h>                                                          // Libreria crear un hotspot al que conectarse desde otro dispositivo y conectar el ESP a una WiFi. UNA VEZ LA ESP SE CONECTE CORRECTAMENTE A LA WiFi, SE GUARDAN LAS CREDENCIALES AUN PULSANDO RESET
#include <HTTPClient.h>                                                           // Libreria para usar el protocolo HTTP
#include <Wire.h>                                                                 // Libreria para usar el bus I2C
#include "Adafruit_HTU21DF.h"                                                     // Libreria del sensor HTU21DF
#include <Adafruit_Sensor.h>                                                      // Libreria para sensores de Adafruit
#include <Adafruit_BME280.h>                                                      // Libreria del sensor BME280
// ===========================================================================================================================================================

// ===========================================================================================================================================================
// MACROS (de ser necesarias)
// ===========================================================================================================================================================
#define ledPin 13
#define SEALEVELPRESSURE_HPA (1015)
// ===========================================================================================================================================================
Adafruit_BME280 bme;                                                              // Creacion del objeto 'bme' de la clase 'Adafruit_BME280' para usar el sensor BME280
Adafruit_HTU21DF htu = Adafruit_HTU21DF();                                        // Creacion del objeto 'htu' de la clase 'Adafruit_HTU21DF' para usar el sensor HTU21DF

String serverNamePOST = "http://192.168.42.17:1880/data";                         // Nombre del enlace al servidor, puerto de NodeRED y nombre bajo el que se haran los metodos GET y POST

float temp_bme, temp_htu, hum_bme, hum_htu, alt, pres;                            // Declaracion de las variables de los sensores BME280 y HTU21DF

unsigned long lastTime = 0;                                                       // Variable para control del tiempo sin usar delays
unsigned long timerDelay = 5000;                                                  // Temporización para envio de datos automaticos

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCION SETUP - SOLO SE EJECUTA UNA VEZ
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void setup(){
  // WiFi.mode(WIFI_STA);                                                         // Especificar el modo de WiFi, por predeterminado es STA+AP. NO ES NECESARIO HABILITAR ESTA LINEA

  Serial.begin(115200);

  // =======================================================================================================================================================
  // INICIALIZACION DE I/O
  // =======================================================================================================================================================
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);                                                      // Por seguridad, siempre inicializar a LOW algo que se quiera apagado desde inicio aunque sea redundante

  if (!htu.begin()) {                                                             // Si no se encuentra el sensor HTU21DF,
    Serial.println("¡No encuentro un sensor HTU21DF valido!");                    // error
    while (1);                                                                    // y se bloquea el codigo
  }

  Wire.begin();

  if (!bme.begin(0x76)){                                                          // Si el sensor no se encuentra en su dirección I2C correspondiente,
    Serial.println("¡No encuentro un sensor BME280 valido!");                     // error
    while(1);                                                                     // y se bloquea el codigo
  }
  // =======================================================================================================================================================

  WiFiManager wm;                                                                 // Inicializacion del objeto 'wm' de la clase WiFiManager

  //wm.resetSettings();                                                           // Reset de las credenciales de la ultima WiFi a la que se conecto el ESP. NO ES NECESARIO HABILITAR ESTA LINEA

  // ---------------------------------------------------------------------------------------------------------------------------------------------------------
  // CONFIGURACION DEL HOTSPOT QUE CREA EL ESP. 3 MODOS: NOMBRE AUTOMATICO CON ID DEL ESP, NOMBRE A ELEGIR Y NOMBRE A ELEGIR CON CONTRASEÑA A ELEGIR
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------
  bool res;                                                                       // Booleano que determina el exito de la autoconexion a la WiFi
  // res = wm.autoConnect();                                                      // NOMBRE AUTOMATICO CON ID DEL ESP
  // res = wm.autoConnect("AutoConnectAP");                                       // NOMBRE A ELEGIR
  res = wm.autoConnect("AP_WiFi","password");                                     // NOMBRE A ELEGIR CON CONTRASEÑA A ELEGIR

  if(!res){                                                                       // 'res' en false significa que no se pudo conectar
    Serial.println("Failed to connect");
    // ESP.restart();
  }else{                                                                          // 'res' en true significa exito en la conexion
    Serial.println("connected...yeey :)");
  }
  // -------------------------------------------------------------------------------------------------------------------------------------------------------
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCION LOOP - TRAS HABERSE COMPLETADO LA CONFIGURACION INICIAL, ESTE ES EL ALGORITMO PARA EL HTTP
// ---------------------------------------------------------------------------------------------------------------------------------------------------------
void loop(){
  if((millis() - lastTime) > timerDelay){                                         // if que solo se ejecuta cada tiempo determinado por 'timerDelay'
    if(WiFi.status()== WL_CONNECTED){                                             // Se comprueba, de nuevo, que la conexion WiFi se mantenga, 'WL_CONNECTED' devuelve un true cuando hay conexion
      // ===================================================================================================================================================
      // PREPARACION DE LAS MEDICIONES DEL SENSOR Y CONVERSION A STRING DE CARACTERES PARA ENVIARSE POR HTTP
      // =====================================================================================================================================================
      char dataStr[60];                                                           // Se crea un string de caracteres para guardar ambas medidas, se reservan 20 espacios
      temp_bme = bme.readTemperature();                                           // Variable que guarda la temperatura del sensor BME280
      pres = bme.readPressure() / 100.0F;                                         // Variable que guarda la presion del sensor BME280
      alt = bme.readAltitude(SEALEVELPRESSURE_HPA);                               // Variable que guarda la altitud del sensor BME280
      hum_bme = bme.readHumidity();                                               // Variable que guarda la humedad del sensor BME280
      temp_htu = htu.readTemperature();                                           // Variable que guarda la temperatura del sensor HTU21DF
      hum_htu = htu.readHumidity();                                               // Variable que guarda la humedad del sensor HTU21DF
      
      sprintf(dataStr,"%5.2f, %6.2f, %4.2f, %4.2f, %5.2f,%6.2f", temp_bme, pres, alt, hum_bme, temp_htu, hum_htu);  // La funcion 'sprintf' de C++ se usa para poder introducir las medidas y elegir el ancho de numero y su precision. Las medidas se separan con una coma ','
      Serial.println(dataStr);                                                    // Muestra en el serial de Arduino el string
      // =====================================================================================================================================================

      // =====================================================================================================================================================
      // METODO HTTP POST - LOS DATOS SE INLCUYEN EN EL PAQUETE QUE SE ENVIA, MÁS SEGURO QUE GET DONDE APARECEN EN LA URL
      // =====================================================================================================================================================
      HTTPClient httpPOST;                                                        // Se crea el objeto 'http' de la clase HTTPClient
      String serverPathPOST = serverNamePOST;                                     // Se guarda 'serverName' en 'serverPath'
      httpPOST.begin(serverPathPOST.c_str());                                     // La conexion http se inicia con el nombre de la URL, puerto y nombre

      digitalWrite(ledPin, HIGH);                                                 // Encendemos el LED cuando se inicia la comunicacion HTTP
      delay(100);

      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");  // En caso de tener NodeRED o el servidor protegido, habilitar esta linea con las credenciales
        
      int httpPOSTResponseCode = httpPOST.POST(dataStr);                          // HTTP 'POST' request, SE ENVIAN LAS MEDIDAS AL NODO 'http in' DE NodeRED Y SE DEVUELVE UN CODIGO
      // =====================================================================================================================================================

      // -----------------------------------------------------------------------------------------------------------------------------------------------------
      // COMPROBACION DE QUE EL METODO POST PRODUCE UNA RESPUESTA
      // -----------------------------------------------------------------------------------------------------------------------------------------------------
      if(httpPOSTResponseCode > 0){                                               // Si 'httpResponseCode' es mayor que 0, es que hay un codigo de respuesta al postear 'dataStr'
        Serial.print("HTTP Response code: ");
        Serial.println(httpPOSTResponseCode);                                     // Mostramos el codigo de respuesta que genera el metodo POST
        String respuesta = httpPOST.getString();                                  // Guardamos en 'respuesta' el 'recibido' escrito en el nodo 'http response'
        Serial.println(respuesta);                                                // y lo mostramos en el monitor serial
      }else{                                                                      // Si 'httpResponseCode' es menor o igual a 0, es que no ha habido respuesta al enviar los datos a la URL
        Serial.print("Error code: ");                                             // y eso es un PROBLEMA
        Serial.println(httpPOSTResponseCode);
      }
      // -----------------------------------------------------------------------------------------------------------------------------------------------------

      httpPOST.end();                                                             // Liberamos recursos acabando la comunicacion http tras haber mandado 'dataStr'

      digitalWrite(ledPin, LOW);                                                  // Apagamos el LED cuando se acaba la comunicacion HTTP
      delay(100);                                                                 // Delay de cortesia tras acabar la comunicacion HTTP
    }else{                                                                        // Si 'WL_CONNECTED' devuelve false, ha habido una desconexion de la WiFi
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();                                                          // Se refresca el valor de 'lastTime' al tiempo actual tras enviar el 'dataStr'
  }
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------