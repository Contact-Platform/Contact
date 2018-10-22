/*******************************************************************************************   
    CONTACT    
    
    A real time Open Communication Platform.    
    Because communication is a right of all and a need of all.
 *******************************************************************************************/

#include <SPI.h>
#include <LoRa.h>
#include "EEPROM.h"
//#include "SSD1306.h" // OLED - Avaible only on Some Units


#include <WiFi.h>
//#include <ESPmDNS.h>    // Avaible only on Some Units
//#include <WiFiClient.h> // 

#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


#include "Arduino.h"    // Needed by ArduinoJSON Lib
#include <ArduinoJson.h>

#define SS      18
#define RST     14
#define DI0     26

#define BAND_0 915E6 // USA Only
#define BAND_1 902E6 // USA/Asia/Australia
#define BAND_2 868E6 // Europe/India
#define PABOOST true

/********** GPS UNIT SETTINGS *************/
#include <TinyGPS++.h>

HardwareSerial GPSSerial(2);

#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;
String strGeo = "";
/********** GPS *************/

const char* hostName   = "CONTACT";
String      deviceName = "CONTACT";
const char* ssid     = "wifi-gateway";
const char* password = "";
