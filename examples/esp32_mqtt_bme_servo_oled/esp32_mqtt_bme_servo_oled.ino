/* ***********************************************************************************************************************************************************
EJEMPLO - PLANTILLA DE DISPOSITIVO PUBLISH/SUBSCRIBE MQTT: en este skecth sirve para programar un robot de dos ruedas que cuenta con un sensor climatico
BME280 que publica las medidas cada 5 segundos en el servidor MQTT https://emqx.broker.io para ser procesadas en NodeRED. Además, en NodeRED se crean los
topicos necesarios para controlar la pantalla OLED, el LED y los servos a bordo del robot, que reciben los parametros de un dashboard de NodeRED por medio de
estar suscrito a los topicos donde se envian dichos parametros.
*********************************************************************************************************************************************************** */

// ===========================================================================================================================================================
// INLCUSION DE LIBRERIAS
// ===========================================================================================================================================================
#include <WiFiManager.h>                                                                                  // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
// ===========================================================================================================================================================

// ===========================================================================================================================================================
// MACROS (de ser necesarias)
// ===========================================================================================================================================================
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define ledPin 40
#define pinServoRight 45
#define pinServoLeft 41
#define SDA_BME 38
#define SCL_BME 39
#define SDA_OLED 18
#define SCL_OLED 17

#define SEALEVELPRESSURE_HPA (1015)
// ===========================================================================================================================================================

// ===========================================================================================================================================================
// CONSTRUCTORES DE OBJETOS DE CLASE DE LIBRERIA, VARIABLES GLOBALES, CONSTANTES...
// ===========================================================================================================================================================
const char* mqtt_server = "broker.emqx.io";
WiFiClient espClient;                                                                                     // Objeto de la libreria WiFiManager
PubSubClient client(espClient);                                                                           // Objeto de la libreria MQTT

Adafruit_BME280 bme;                                                                                      // Objeto del sensor BME280
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);                                   // Objeto del OLED
Servo miServoRight;                                                                                       // Objeto del servo derecho
Servo miServoLeft;                                                                                        // Objeto del servo izquierdo

float temp, hum, alt, pres;                                                                               // Definicion de las variables del sensor BME280

unsigned long lastTime = 0;                                                                               // Se inicializa la variable en la que se guarda el tiempo en milisegundos tras cada iteracion del loop
const unsigned long timerDelay = 5000;                                                                    // Se inicializa el intervalo de publicacion de datos
// ===========================================================================================================================================================

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCION SETUP - SOLO SE EJECUTA UNA VEZ
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // =======================================================================================================================================================
  // INICIALIZACION DE I/O
  // =======================================================================================================================================================
  pinMode(ledPin, OUTPUT);
  miServoRight.attach(pinServoRight);
  miServoLeft.attach(pinServoLeft);

  Wire.begin(SDA_BME, SCL_BME);                                                                           // Initialize I2C for the BME280. SDA, SCL
  Wire1.begin(SDA_OLED, SCL_OLED);                                                                        // Initialize I2C for the OLED. SDA, SCL

  oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  oled.display();                                                                                         // Display initialization - LOGO ADAFRUIT
  delay(1000);
  oled.clearDisplay();                                                                                    // Clear the display
  oled.setTextSize(3);
  oled.setTextColor(SSD1306_WHITE);
  oled.display();

  if (!bme.begin(0x76)){
    Serial.println("No encuentro un sensor BME280 valido!");
    while (1);
  }
  // =======================================================================================================================================================

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // ---------------------------------------------------------------------------------------------------------------------------------------------------------
  // CONFIGURACION DEL HOTSPOT QUE CREA EL ESP. 3 MODOS: NOMBRE AUTOMATICO CON ID DEL ESP, NOMBRE A ELEGIR Y NOMBRE A ELEGIR CON CONTRASEÑA A ELEGIR
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------
  WiFiManager wm;                                                                                         //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around

  bool res;

  res = wm.autoConnect("AP_Moya","m3di4l4b");                                                             // password protected ap

  if(!res) {
    Serial.println("Failed to connect");
  } 
  else { 
    Serial.println("connected...yeey :)");                                                              // if you get here you have connected to the WiFi   
  }
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCION LOOP - TRAS HABERSE COMPLETADO LA CONFIGURACION INICIAL, ESTE ES EL ALGORITMO PARA EL MQTT PARA EL PUBLISHING
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  if (!client.connected()) {                                                                              // Si no hay conexion
    reconnect();                                                                                          // Entra la funcion de reconexion
  }
  client.loop();

  if ((millis() - lastTime) > timerDelay) {                                                               // Send an MQTT publish request every 5 secs
    if(WiFi.status()== WL_CONNECTED){                                                                     // Check WiFi connection status
      // ===================================================================================================================================================
      // PREPARACION DE LAS MEDICIONES DEL SENSOR Y CONVERSION A STRING DE CARACTERES PARA ENVIARSE POR MQTT
      // =====================================================================================================================================================
      char dataStr[40];                                                                                   // Se crea un string de caracteres para guardar ambas medidas, se reservan 20 espacios
      temp = bme.readTemperature();                                                                       // Variable que guarda la temperatura del sensor BME280
      pres = bme.readPressure() / 100.0F;                                                                 // Variable que guarda la presion del sensor BME280
      alt = bme.readAltitude(SEALEVELPRESSURE_HPA);                                                       // Variable que guarda la altitud del sensor BME280
      hum = bme.readHumidity();                                                                           // Variable que guarda la humedad del sensor BME280

      sprintf(dataStr, "%5.2f, %6.2f, %4.2f, %4.2f", temp, pres, alt, hum);                               // La funcion 'sprintf' de C++ se usa para poder introducir las medidas y elegir el ancho de numero y su precision. Las medidas se separan con una coma ','
      Serial.println(dataStr);                                                                            // Muestra en el serial de Arduino el string

      client.publish("moya/sensores", dataStr);                                                           // Se publica el string con los datos de los sensores en el topico 'moya/sensores'
      // =====================================================================================================================================================

    }else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();                                                                                  // Se refresca el valor de 'lastTime' al tiempo actual tras enviar el 'dataStr'
  }
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCION CALLBACK - FUNCION A LA QUE LLEGAN LOS MENSAJES DE LOS TOPICOS SUSCRITOS PARA HACER CAMBIOS EN EL MICRO, ES COMO UN SEGUNDO LOOP
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void callback(char* topic, byte* message, unsigned int length) {                                          // Funcion que recibe el topico MQTT, el mensaje y la longitud del mismo
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);                                                                                    // En 'topic' se guarda el nombre del topic. Por ejemplo 'moya/luces'
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {                                                                      // Bucle para printear el mensaje (caracter a caracter)
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];                                                                      // En 'messageTemp' se carga el contenido de 'message', el cual se reinicia tras cada iteracion del 'loop'
  }
  Serial.println();

  // ===================================================================================================================================================
  // Feel free to add more if statements to control more GPIOs with MQTT
  // ===================================================================================================================================================
  if(String(topic) == "moya/luces"){                                                                      // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
    Serial.print("Changing output to ");                                                                  // Changes the output state according to the message
    if(messageTemp == "on"){
      Serial.println("on LED");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off LED");
      digitalWrite(ledPin, LOW);
    }
  }

  if(String(topic) == "moya/servoRight"){                                                                 // Funcion para mover servo recibiendo en formato string el valor numerico
    Serial.print("Moving servo to ");
    int sliderValueRight = messageTemp.toInt();                                                           // Se transforma el string con el numero en un numero entero con 'toInt()'
    int servoRightAngle = map(sliderValueRight, 0, 100, 45, 135);                                         // Con un slider range 0-100, se mapean los angulos correspondientes
    Serial.print(servoRightAngle);
    Serial.println(" degrees");
    miServoRight.write(servoRightAngle);                                                                  // Se mueve el servo a dicho angulo
  }

  if(String(topic) == "moya/servoLeft"){
    Serial.print("Moving servo to ");
    int sliderValueLeft = messageTemp.toInt();
    int servoLeftAngle = map(sliderValueLeft, 0, 100, 135, 45);
    Serial.print(servoLeftAngle);
    Serial.println(" degrees");
    miServoLeft.write(servoLeftAngle);
  }

  if(String(topic) == "moya/oled"){
    if(messageTemp == "onOLED"){
      Serial.println("on OLED");    
      oled.clearDisplay();                                                                                // Se limpia el buffer del OLED
      oled.setTextSize(1);                                                                                // Se selecciona el tamaño de la letra

      oled.setCursor(10,0);                                                                               // Se indica el pixel donde se quiere escribir (X,Y)
      oled.print("Temp: ");                                                                               // Se escribe un string
      oled.setCursor(40,0);
      oled.print(bme.readTemperature());                                                                  // Se escribe el valor actual de la temperatura del BME280

      oled.setCursor(10,15);
      oled.print("Pres: ");
      oled.setCursor(40,15);
      oled.print(bme.readPressure() / 100.0F);

      oled.setCursor(10,30);
      oled.print("Alti: ");
      oled.setCursor(40,30);
      oled.print(bme.readAltitude(SEALEVELPRESSURE_HPA));

      oled.setCursor(10,45);
      oled.print("Hume: ");
      oled.setCursor(40,45);
      oled.print(bme.readHumidity());

      oled.display();
    }
    else if(messageTemp == "offOLED"){                                                                    // Si se recibe un OFF
      Serial.println("off OLED");
      oled.clearDisplay();                                                                                // Se limpia el buffer del OLED

      oled.display();                                                                                     // Se printea el buffer que, como no es nada, se apaga
    }
  }
  // ===================================================================================================================================================

}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCION RECONNECT - FUNCION QUE ESTABLECE LA CONNEXION POR MQTT PARA RECIBIR LOS MENSAJES DE LOS TOPICOS A LOS QUE SE ESTA SUSCRITO
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void reconnect() {
  while (!client.connected()) {                                                                           // Loop until we're reconnected (OR JUST CONNECTED)
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_Moya")) {                                                                   // Conexion establecida con el dispositivo MQTT. CUIDADO QUE AQUI SE PUEDE AÑADIR ,"USER", "PASSWORD"
      Serial.println("connected");

      // ===================================================================================================================================================
      // Feel free to add more if statements to control more GPIOs with MQTT
      // ===================================================================================================================================================
      client.subscribe("moya/luces");
      client.subscribe("moya/servoRight");
      client.subscribe("moya/servoLeft");
      client.subscribe("moya/oled");
      // ===================================================================================================================================================

    } else {
      Serial.print("failed, rc=");                                                                        // Si no se establece conexion
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");                                                          // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
