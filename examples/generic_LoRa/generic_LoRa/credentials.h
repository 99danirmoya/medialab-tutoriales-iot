/* ***********************************************************************************************************************************************************
ARCHIVO DE CLAVES OTAA PARA TTN

Se debe tener especial cuidado copiando las claves desde la consola de TTN ya que algunas deben venir en formato LSB y, otras, en MSB
*********************************************************************************************************************************************************** */
#pragma once

// Only one of these settings must be defined
//#define USE_ABP
#define USE_OTAA

#ifdef USE_ABP  // UPDATE WITH YOUR TTN KEYS AND ADDR. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //static const u1_t PROGMEM APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //static const u4_t DEVADDR = 0x26010000 ;                                                   // <-- Change this address for every node!
#endif

#ifdef USE_OTAA  // UPDATE WITH YOUR TTN KEYS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  static const u1_t PROGMEM APPEUI[8]  = { 0x55, 0x55, 0x66, 0x76, 0x77, 0x88, 0x98, 0x99 };  // Debe ir en formato LSB (least-significant-byte first)
  static const u1_t PROGMEM DEVEUI[8]  = { 0x06, 0x78, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };  // Debe ir en formato LSB (least-significant-byte first)
  static const u1_t PROGMEM APPKEY[16] = { 0xD1, 0x82, 0x6B, 0x4C, 0x8A, 0xD0, 0xDE, 0x40, 0xD9, 0xF4, 0x13, 0x81, 0xCF, 0x4E, 0xDB, 0xCC };  // Debe ir en formato MSB (most-significant-byte first)
#endif