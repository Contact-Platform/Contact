/*******************************************************************************************   
    CONTACT     
    A real time Open and Portable Communication Platform.    

    Author: Javier A. Dastas
  
    Contact is an open platform that brings together software and 
    hardware to allow everyone to communicate. t use an IoT device 
    that includes a LoRa transmitter and an ESP32 microcontroller 
    to provide communication access and forms bridges to the 
    Internet if it necessary. It is possible to reach a radius of 
    up to eight miles between units or up to two miles away 
    through efficient structures. The range can be doubled or 
    tripled when the units are used as a simple MESH. It's an 
    alternative way of providing communications in disaster 
    situations or to communities in need.

 *******************************************************************************************/

#include <SPI.h>
#include <LoRa.h>
#include "EEPROM.h"
//#include "SSD1306.h" // OLED - It's not necessary


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

// LoRa Frequency Bands
#define BAND_0 915E6 // USA Only - We use as Default
                     // till change on Setup Screen
#define BAND_1 920E6 // USA/Asia/Australia
#define BAND_2 865E6 // Europe/India
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

AsyncWebServer server(80);

String MESH_MSG = "";

String message = ""; // char message[]

boolean sendLoRaMsg = false;
boolean devNotification = false;
boolean sendLoRaMsgToOther = false;

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

int nTTL_MAX = 2;                 // Time-To-Live Package (MESH)

#define MAX_TX_MESSAGES 15
#define MAX_RX_MESSAGES 50
#define MAX_UNITS 10
#define DATA_SIZE 500

typedef struct MESSAGE_TYPE {
  int  packetId;  
  char TxID[12];
  char RxID[12];
  char data[DATA_SIZE];
  int  packetsQty;
  int  packetCtr;
  int  packetType; // 1-Txt; 2-Sound; 3-Image; 4-Hand Shake; 5-Alert; 6-JSON
  int  ttl;
  int  packetQId;
  byte  sender; // 0 - Me; 1 - Him
};
MESSAGE_TYPE rx_message_queue[MAX_RX_MESSAGES];
MESSAGE_TYPE tx_message_queue[MAX_TX_MESSAGES];

typedef struct UNIT_TYPE {
  char TxID[12];
  char lastDate[10];
  char lastTime[18];
};
UNIT_TYPE unit_queue[MAX_UNITS];

int rx_MSG_CTR_Q = 0;
int tx_MSG_CTR_Q = 0;
int UNITS_CTR_Q = 0;
int tx_MSG_ID = 0;

int LAST_MSG_ID = 0;
char LAST_UNIT = 0;

class FLASHvariables {
public:
  char unitName[14];
  char unitDesc[30];

  uint8_t unitFreq; // Unit Area Frequency

  double lat;
  double lng;

  uint16_t isGateway; // 0 - No; 1 - Yes
 
  char ssid[20];
  char ssidPwd[20];  

  char smsServer[300];
  char dataServer[300];  

  char helpMessage[300];

  uint16_t tstVar; // 

  // METHODS
  void save();
  void get();
  void initialize();  
  
}  CONTACTvars;

void FLASHvariables::save()
{
  EEPROM.put(0, CONTACTvars);
  EEPROM.commit();
}
void FLASHvariables::get() 
{
  EEPROM.begin(sizeof(CONTACTvars));
  EEPROM.get(0, CONTACTvars);
}
void FLASHvariables::initialize() 
{
  // If data not found on flash RAM initialize all vars
  strcpy(unitName, "CONTACT");
  strcpy(unitDesc, "Physical Location");

  unitFreq = 0;

  lat = 0.00; lng = 0.00;

  isGateway = 0; // Default is SMS Unit 
  
  strcpy(ssid,    "not-defined");
  strcpy(ssidPwd, "not-defined");  
  
  strcpy(smsServer, "not-defined");
  strcpy(dataServer, "not-defined");

  strcpy(helpMessage, "I need Help, immedialty!");

  tstVar = 111;

  EEPROM.put(0, CONTACTvars);
  EEPROM.commit();  
}

void setup()
{
  pinMode(25,OUTPUT); //Send success, LED will bright 1 second
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH);
    
  Serial.begin(115200);
  GPSSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  CONTACTvars.get();  
  if (CONTACTvars.tstVar != 111) // all data is correct
  {
    Serial.println("No Data Exist on Flash RAM ... Initializing to defaults." + micros());
    CONTACTvars.initialize();
  }
  else
    Serial.println("Success retrieved persistent variables." + micros());

  Serial.print("UNIT Name: ");
  Serial.println(CONTACTvars.unitName);   

  Serial.print("Device Type: ");
  Serial.print(CONTACTvars.isGateway);
  Serial.print(" - ");
  Serial.println((CONTACTvars.isGateway==1?"Gateway":"Communication"));

  if (CONTACTvars.isGateway == 0) // Use Only for Communication
  {
      WiFi.mode(WIFI_AP);
      
      hostName = CONTACTvars.unitName;
      
      WiFi.softAP(hostName); // Use as OPEN HOT SPOT Communication Device
      // WiFi.softAP(hostName, CONTACTvars.unitPwd); 
      delay(100); 
      Serial.println("Connected!");
    
      Serial.println("Set softAPConfig");
      IPAddress Ip(1, 2, 3, 4);
      IPAddress NMask(255, 255, 255, 0);
      WiFi.softAPConfig(Ip, Ip, NMask);
    
      Serial.print("IP address: ");
      Serial.println(WiFi.softAPIP());      
  } 
  else  // Use Unit as Internet Gateway
  {
      int nets = WiFi.scanNetworks();
      Serial.println("scan done");
      if (nets == 0) {
          Serial.println("no networks found");
      } else {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
  
        ssid = CONTACTvars.ssid;      
        password = CONTACTvars.ssidPwd;
        WiFi.begin(ssid, password);
        Serial.print("WiFi Status: ");
        Serial.println(WiFi.status());
        int conxTries = 0;
        while ((WiFi.status() != WL_CONNECTED) and (conxTries++ < 40)) {
          delay(500);
          Serial.print(".");
        }
        Serial.println("");        
        if ( WiFi.status() != WL_CONNECTED) { 
            Serial.println("Couldn't Get a WiFi connection");
            Serial.println("as Gateway.");
            Serial.println("Please check the Settings Page.");
            
            CONTACTvars.isGateway = 0; // Set Unit to Default Again
            CONTACTvars.save();
            
            ESP.restart();
        } 
        else {  
          Serial.print("Connected to ");
          Serial.print(ssid);
          Serial.println(" Network.\n\n   Ready to Send data as Gateway.");
          
          Serial.println("WiFi connected!");
          Serial.print("  IP address: ");
          Serial.println(WiFi.localIP());
          Serial.print("  ESP Mac Address: ");
          Serial.println(WiFi.macAddress());
          Serial.print("  Subnet Mask: ");
          Serial.println(WiFi.subnetMask());
          Serial.print("  Gateway IP: ");
          Serial.println(WiFi.gatewayIP());
          Serial.print("  DNS: ");
          Serial.println(WiFi.dnsIP());    

          // Set up mDNS responder:
          // - first argument is the domain name, in this example
          //   the fully-qualified domain name is "esp8266.local"
          // - second argument is the IP address to advertise
          //   we send our IP address on the WiFi network
          //if (MDNS.begin(CONTACTvars.unitName)) {
          //    Serial.println("MDNS Responder Started!");
          //} else {
          //    Serial.println("Error setting up MDNS responder!");
          //}
        }  
      }
  }
  delay(100);

  // Not for all Devices, only based on Chrome
  // char mdnsHostName [12+1];
  // mdnsHostName = CONTACTvars.unitName
  // strcpy (mdnsHostName,"chat");

  // if (!MDNS.begin(mdnsHostName)) {
  //   Serial.println("Error setting up MDNS responder!");
  //   while(1){
  //     delay(1000);
  //   }
  // }
  // Serial.println("MDNS Name Setted.");

  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
  Serial.println("Starting LoRa.");
  Serial.print("   Frequency used 0-USA Only, 1-USA/Asia/Australia, 2-Europe/Indida: ");
  Serial.println(CONTACTvars.unitFreq);
  double BAND = (CONTACTvars.unitFreq == 0? BAND_0 : (CONTACTvars.unitFreq == 1? BAND_1 : BAND_2));
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }  

  // ********* Set Unit Web Pages and RESTs 
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      // Chat Page
      request->send(200, "text/html", main_page());
  });

  server.on("/deletechat", HTTP_GET, [](AsyncWebServerRequest *request){
    // Reset Chat Content
    String jsonData = "";
    
    int paramsNmbr = request->params();
    if (paramsNmbr > 0) {
      AsyncWebParameter* p1 = request->getParam(0);      
      int nKey = String(p1->value()).toInt();
      String page = "";
      if (nKey = 20172009) 
         page = "<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"> <title>Document</title> <script>if (localStorage.getItem('chat') !=null) localStorage.setItem('chat', ''); var devInfo='{\"uname\":\"\", \"location\":\"Puerto Rico.\", \"lat\": 18.2208, \"long\":66.5901}'; if (localStorage.getItem('location')==null) localStorage.setItem('location', devInfo); document.getElementById('deleted').innerHTML='<h1>DELETED</h1>'; </script></head><body> <div id='deleted'></div></body></html>";
      else
         page = "<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"> <title>Document</title> <script>if (localStorage.getItem('chat') !=null) localStorage.setItem('chat', ''); var devInfo='{\"uname\":\"\", \"location\":\"Puerto Rico.\", \"lat\": 18.2208, \"long\":66.5901}'; if (localStorage.getItem('location')==null) localStorage.setItem('location', devInfo); document.getElementById('borrado').innerHTML='<h1>BORRADO</h1>'; </script></head><body> <h1>Communication is everyone's right.</h1></body></html>";
      request->send(200, "text/html", page);
    }      
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    /*if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str()); */

    request->send(404, "text/html", "<html><body><h1>The content you are looking for was not found.</h1></body></html>");
      
  });

   server.on("/getgeo", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Get Geolocation from the GPS - if Available on Unit
    int paramsNmbr = request->params();
    if (paramsNmbr > 0) {
      
       String jsonData = "{\"data\":[";
       jsonData += strGeo;
       jsonData += "]}\n\n";    
       
       request->send(200, "text/json",  jsonData);

       Serial.println(jsonData);
    }
   });

   server.on("/getdev", HTTP_GET, [](AsyncWebServerRequest *request) {
   // Get Available Units Online to Communicate
    int paramsNmbr = request->params();
    if (paramsNmbr > 0) {
       Serial.println("Dev List Request ...");
       AsyncWebParameter* p = request->getParam(0);
       String jsonData = "{\"data\":[";
             
       for(int i = 0; i < UNITS_CTR_Q; i++) {    
          jsonData += ("{\"devunit\":\"" + ((String)unit_queue[i].TxID) + "\"}" );
          if (i < (UNITS_CTR_Q - 1)) {
            jsonData += ",";
          }
       }
       jsonData += "]}\n\n";    
       
       request->send(200, "text/json",  jsonData);

       Serial.println(jsonData);
    } 
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Get Received Messages from STACK
    int paramsNmbr = request->params();
    if ((paramsNmbr > 0) && (rx_MSG_CTR_Q > 0)){
       AsyncWebParameter* p1 = request->getParam(0);
       AsyncWebParameter* p2 = request->getParam(1);

       int nLastReaded = String(p1->value()).toInt();
       int bFirstTime = String(p2->value()).toInt();
       
       Serial.print(String(bFirstTime));
       Serial.print(" - ");
       Serial.print(String(nLastReaded));
       Serial.print(" - ");
       Serial.print(rx_MSG_CTR_Q);       
       Serial.print(" - ");
       Serial.println(nLastReaded == (rx_MSG_CTR_Q - 1));
       if (bFirstTime == 0)
           if (nLastReaded == (rx_MSG_CTR_Q - 1))
               return;
       Serial.println("Creating JSON Messages Data.");
       String jsonData = "{\"data\":[";
       String msgdata = "";
       int comma = (rx_MSG_CTR_Q - 1);
       int nQueInit = 1;
       int nStart = (bFirstTime == 1 ? nLastReaded : nLastReaded + 1); // nStart = 0 cambiado para no leer desde el inicio
       
       if (nLastReaded > rx_MSG_CTR_Q)
          nQueInit = 2;
       
       for(int n = 1; n <= nQueInit; n++) {
         if (nQueInit == 2) {
            if (n == 1)
              nStart = (nLastReaded + 1);
            else
              nStart = 0;
         }         
         for(int i = nStart; i < (nStart > rx_MSG_CTR_Q ? MAX_RX_MESSAGES : rx_MSG_CTR_Q); i++) {    
            msgdata = rx_message_queue[i].data;    
            jsonData += ("{\"pid\":" + ((String)rx_message_queue[i].packetId) + ", ");
            jsonData += ("\"message\":\"" + msgdata + "\", ");
            jsonData += ("\"txid\":\"" + ((String)rx_message_queue[i].TxID) + "\", " );
            jsonData += ("\"pqty\":" + ((String)rx_message_queue[i].packetsQty) + ", " );
            jsonData += ("\"pctr\":" + ((String)rx_message_queue[i].packetCtr) + ", " );
            jsonData += ("\"ptype\":" + ((String)rx_message_queue[i].packetType) + ", " );
            jsonData += ("\"pqid\":" + ((String)rx_message_queue[i].packetQId) + ", " );
            jsonData += ("\"sender\":" + ((String)rx_message_queue[i].sender) + ", " );
            jsonData += ("\"ttl\":" + ((String)rx_message_queue[i].ttl) + "}" );
            if (i < comma) {
              jsonData += ",";
            }
         }   
         if ((nQueInit == 2) && (n < 2)) {
            jsonData += ",";
         }          
       }
       jsonData += "]}\n\n";    
       
       Serial.println(jsonData);

       String jsonHeader = "HTTP/1.0 200 OK\nContent-Type: application/json;charset=utf-8\nAccept: application/json\nAccess-Control-Allow-Credentials: true\nAccess-Control-Allow-Origin: *\nConnection: close\n\n";

       request->send(200, "text/json",  jsonData);
    } // end if
  });
  
  server.on("/notify", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Send Handshake - Unit Broadcast or Availability Notification
    int paramsNumbr = request->params();
    Serial.println("Unit Registration Request ...");
    if (paramsNumbr > 0) {
       Serial.println("Creating Registration Message for LoRa ... ");
       //AsyncWebParameter* p = request->getParam(0);
       //strcpy(message, p->value().c_str());
       message = ""; //strcpy(message,"");
       message = deviceName; // strcpy(message, deviceName.c_str());

       Serial.println("Notify...");
       Serial.println(message);

       request->send(200, "text/json",  "OK");

       devNotification = true;
       sendLoRaMsg = false;
    }
  });
  
  server.on("/put", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Send Messages from Chat to the Stack.
  // It's possible to send files or images fragmented on small pieces.
  // Also an initial TIME-TO-LIVE Field is created for MESH the Package and
  // send again, the default TTL MESH is One. 
  // The message will be sent up to twice before reaching the destination.
    int paramsNumbr = request->params();
    if (paramsNumbr > 0) {
       AsyncWebParameter* p = request->getParam(0);
       message = String(p->value()); //strcpy(message, p->value().c_str());

       AsyncWebParameter* u = request->getParam(1);
       String rxUnit = String(u->value()); 

       AsyncWebParameter* usr = request->getParam(2);
       String usuario = String(usr->value());        

       String msg = "";
       int msgLen = 0;
       int nInit = 0;
       int nEnd = 0;          

       msg = message;
       msgLen = msg.length();
      
       if (msgLen < 1)
           return;      
      
       Serial.println("WiFi Message Sended from Unit.");
       Serial.print("Msg Original Size: ");
       Serial.println(msgLen);

      int npackets = round((msgLen / DATA_SIZE) + 0.5);
               
      for(int i = 1; i <= npackets; i++) {
        tx_MSG_ID++;
        if (npackets > 1) {
          nInit = ((DATA_SIZE - 1) * (i - 1)) + i;
          nEnd = DATA_SIZE * i;
          if (nEnd > msgLen)
            nEnd = msgLen;
          msg = String(message).substring( nInit, nEnd);
        }
        else
          msg = message;

        
        tx_message_queue[tx_MSG_CTR_Q].packetId = tx_MSG_ID;        
        deviceName.toCharArray(tx_message_queue[tx_MSG_CTR_Q].TxID, 12);
        rxUnit.toCharArray(tx_message_queue[tx_MSG_CTR_Q].RxID, 12);

        //msgLen = (msg.length() + 1);        
        //Serial.print("Msg Size: ");Serial.println(msgLen);
        
        msg.toCharArray(tx_message_queue[tx_MSG_CTR_Q].data, msgLen + 1); 
        Serial.print("Msg on Queue: ");
        Serial.print(tx_message_queue[tx_MSG_CTR_Q].data);
        Serial.println("||");
        
        tx_message_queue[tx_MSG_CTR_Q].packetsQty = npackets;
        tx_message_queue[tx_MSG_CTR_Q].packetCtr = i;
        tx_message_queue[tx_MSG_CTR_Q].packetType = 1;
        //if (strcmp(user, "JSON") != 0) tx_message_queue[tx_MSG_CTR_Q].packetType = 6;
        tx_message_queue[tx_MSG_CTR_Q].ttl = nTTL_MAX;      
        tx_message_queue[tx_MSG_CTR_Q].packetQId = tx_MSG_CTR_Q;
        tx_message_queue[tx_MSG_CTR_Q].sender = 0;

        tx_MSG_CTR_Q++; 
        if (tx_MSG_CTR_Q == MAX_TX_MESSAGES)
          tx_MSG_CTR_Q = 0;
      }
      
       request->send(200, "text/json",  "OK");
       devNotification = false;
       sendLoRaMsg = true;
    }
    else
    {
      Serial.println("NOT Parameter Value Received.");
    }
  });

  server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request){
      // Unit Settings Page
      request->send(200, "text/html", settings_page());
  });
  
  server.on("/putsetup", HTTP_GET, [](AsyncWebServerRequest *request){
  // Save Unit Settings to RAM (EEPROM).
    int paramsNumbr = request->params();    
    if (paramsNumbr > 0) {
       Serial.println("* Dev Settings Saved.");
      // dataServer=&smsServer=&ssidPwd=&ssid=&isGateway=0&unitDesc=LOCATION&unitName=CONTACT
       AsyncWebParameter* ds = request->getParam(0); 
       AsyncWebParameter* ss = request->getParam(1);
       AsyncWebParameter* spw = request->getParam(2);       
       AsyncWebParameter* sid = request->getParam(3);
       AsyncWebParameter* ig = request->getParam(4);
       AsyncWebParameter* uf = request->getParam(5);
       AsyncWebParameter* ud = request->getParam(6);
       AsyncWebParameter* un = request->getParam(7);
       
       strcpy(CONTACTvars.dataServer, ds->value().c_str()); 
       strcpy(CONTACTvars.smsServer, ss->value().c_str()); 
       
       strcpy(CONTACTvars.ssid, sid->value().c_str()); 
       strcpy(CONTACTvars.ssidPwd, spw->value().c_str()); 

       CONTACTvars.isGateway = String(ig->value()).toInt();
       Serial.print("Is Gateway? ");
       Serial.println(CONTACTvars.isGateway);

       CONTACTvars.unitFreq = String(uf->value()).toInt();
       
       strcpy(CONTACTvars.unitDesc, ud->value().c_str());  
       strcpy(CONTACTvars.unitName, un->value().c_str()); 
   
       CONTACTvars.save();

       String jsonData = "{\"data\":[{\"status\":\"saved\"}]}\n\n";
       request->send(200, "text/json",  jsonData);
       //request->send(200, "text/html", settings_page());
    }
  });

 server.on("/getsetup", HTTP_GET, [](AsyncWebServerRequest *request) {
 // Get the CONTACT vars Settings from RAM (EEPROM).
    int paramsNmbr = request->params();
    if (paramsNmbr > 0) {
       Serial.println("* Dev Settings Request ...");
       AsyncWebParameter* p = request->getParam(0);
       String jsonData = "{\"data\":[";
             
       jsonData += ("{\"unitname\":\"" + String(CONTACTvars.unitName) + "\",");
       jsonData += ("\"unitdesc\":\"" + String(CONTACTvars.unitDesc) + "\",");
       jsonData += ("\"unitfreq\":\"" + String(CONTACTvars.unitFreq) + "\",");
       jsonData += ("\"isgateway\":\"" + String(CONTACTvars.isGateway) + "\",");
       jsonData += ("\"ssid\":\"" + String(CONTACTvars.ssid) + "\",");
       jsonData += ("\"ssidpwd\":\"" + String(CONTACTvars.ssidPwd) + "\",");
       jsonData += ("\"smsserver\":\"" + String(CONTACTvars.smsServer) + "\",");
       jsonData += ("\"dataserver\":\"" + String(CONTACTvars.dataServer) + "\"");       
       jsonData += "}]}\n\n";    
        Serial.println("**********"); Serial.println(jsonData);
       request->send(200, "text/json",  jsonData);

       Serial.println(jsonData);
    } // end if
  });  

 server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
 // Restar Unit to Get New Settings 
    int paramsNmbr = request->params();
    if (paramsNmbr > 0) {
       Serial.println("* Dev Restart Request ...");
       ESP.restart();
    } 
  });   

 server.on("/putsomevars", HTTP_GET, [](AsyncWebServerRequest *request){
  // Save Chat Settings to RAM (EEPROM).
    int paramsNumbr = request->params();    
    if (paramsNumbr > 0) {
       Serial.println("* Dev Settings on Chat.");
       AsyncWebParameter* h = request->getParam(0); // help message
       AsyncWebParameter* ud = request->getParam(1);

       strcpy(CONTACTvars.helpMessage, h->value().c_str());        
       strcpy(CONTACTvars.unitDesc, ud->value().c_str());  
        
       CONTACTvars.save();

       String jsonData = "{\"data\":[{\"status\":\"saved\"}]}\n\n";
       request->send(200, "text/json",  jsonData);      
    }
  });  

  server.on("/getsomevars", HTTP_GET, [](AsyncWebServerRequest *request) {
 // Get the CONTACT vars Settings from RAM (EEPROM).
    int paramsNmbr = request->params();
    if (paramsNmbr > 0) {
       Serial.println("* Dev Some Settings Request ...");
       AsyncWebParameter* p = request->getParam(0);
       String jsonData = "{\"data\":[";             
       jsonData += ("{\"location\":\"" + String(CONTACTvars.unitDesc) + "\",");
       jsonData += ("\"helpmsg\":\"" + String(CONTACTvars.helpMessage) + "\"");
       jsonData += "}]}\n\n";    
        Serial.println("**********"); Serial.println(jsonData);
       request->send(200, "text/json",  jsonData);

       Serial.println(jsonData);
    } // end if
  });   

  server.on("/gateway", HTTP_GET, [](AsyncWebServerRequest *request){
      // Gateway Page
      request->send(200, "text/html", gateway_page());
  });  


  server.on("/delsmscache", HTTP_GET, [](AsyncWebServerRequest *request){
      // Gateway Page
      request->send(200, "text/html", delete_sms_cache());
  });  

  server.on("/geo", HTTP_GET, [](AsyncWebServerRequest *request) {
    int paramsNumbr = request->params();
    if (paramsNumbr > 0) {
       AsyncWebParameter* p = request->getParam(0);

       message = String(p->value());          

       String msg = message;
       int msgLen = msg.length();
      
       if (msgLen < 1)
           return;      
      
       Serial.println("WiFi Geo Sended from Unit.");

      int npackets = round((msgLen / DATA_SIZE) + 0.5);
               
      tx_MSG_ID++;
      msg = message;

        
      tx_message_queue[tx_MSG_CTR_Q].packetId = tx_MSG_ID;        
      deviceName.toCharArray(tx_message_queue[tx_MSG_CTR_Q].TxID, 11);

      msg.toCharArray(tx_message_queue[tx_MSG_CTR_Q].data, msgLen + 1); 
      Serial.print("Msg on Queue (GEO): ");
      Serial.println(tx_message_queue[tx_MSG_CTR_Q].data);

        tx_message_queue[tx_MSG_CTR_Q].packetsQty = 1;
        tx_message_queue[tx_MSG_CTR_Q].packetCtr = 1;
        tx_message_queue[tx_MSG_CTR_Q].packetType = 1;
        tx_message_queue[tx_MSG_CTR_Q].ttl = 5;      
        tx_message_queue[tx_MSG_CTR_Q].packetQId = tx_MSG_CTR_Q;
        tx_message_queue[tx_MSG_CTR_Q].sender = 0;

        tx_MSG_CTR_Q++; 
        if (tx_MSG_CTR_Q == MAX_TX_MESSAGES)
          tx_MSG_CTR_Q = 0;

       request->send(200, "text/json",  "OK GEO");
       devNotification = false;
       sendLoRaMsg = true;
       Serial.println("GEO WiFi setted.");
    }
    else
    {
      Serial.println("NOT Parameter Value Received.");
    }
  });  
  
  server.begin();
  Serial.println("Webserver Started");
  
  deviceName = CONTACTvars.unitName;
}

void loop(){
  
  String jsonString = "";

  strGeo = geoLocation(); 
 // Serial.println(strGeo);
  
  if (devNotification) {
     Serial.println("Sending Hand Shake Notification ...");
     jsonString = "{\"pid\": 777, ";
     jsonString += "\"txid\": \"" + deviceName + "\", ";
     jsonString += "\"rxid\": \"ALL\", ";
     jsonString += "\"msg\": \"" + deviceName + "\", ";
     jsonString += "\"pqty\": 1, ";
     jsonString += "\"pctr\": 1, ";
     jsonString += "\"ptype\": 4, ";
     jsonString += "\"ttl\": nTTL_MAX, ";
     jsonString += "\"pqid\": 777}";
     
      LoRa.beginPacket();
      LoRa.print(jsonString);
      LoRa.endPacket();
      
      Serial.println("Hand Shake Sended ...");
      devNotification = false;
  }
  if (sendLoRaMsg) { 
    Serial.println("Sending message ...");
    // Send Messages on Stack
    for(int i = 0; i < tx_MSG_CTR_Q; i++) {     
        Serial.println(tx_message_queue[i].data);
        jsonString = "{\"pid\": " + String(tx_message_queue[i].packetId) + ", ";
        jsonString += "\"txid\": \"" + String(tx_message_queue[i].TxID) + "\", ";
        jsonString += "\"rxid\": \"" + String(tx_message_queue[i].RxID) + "\", ";
        jsonString += "\"msg\": \"" + String(tx_message_queue[i].data) + "\", ";
        jsonString += "\"pqty\": " + String(tx_message_queue[i].packetsQty) + ", ";
        jsonString += "\"pctr\": " + String(tx_message_queue[i].packetCtr) + ", ";
        jsonString += "\"ptype\": 1, ";      
        jsonString += "\"ttl\": " + String(tx_message_queue[i].ttl) + ", ";
        jsonString += "\"pqid\": " + String(tx_message_queue[i].packetQId) + "} ";
        Serial.println(jsonString);
        delay(10);
        LoRa.beginPacket();
        Serial.print("Begin LoRa Packet");
        //LoRa.write(jsonString.length()); 
        Serial.print("Setting Size Packet");
        LoRa.print(jsonString);
        Serial.print("Print LoRa Packet");
        LoRa.endPacket();
        Serial.print("End LoRa Packet");
        Serial.println(jsonString);
        Serial.println("Message was sended.");
    }    
    tx_MSG_CTR_Q = 0; 
    sendLoRaMsg = false;
  }

  if (sendLoRaMsgToOther) { 
    // MESH Messaging
    Serial.println("Sending message ...");
      jsonString = MESH_MSG;     
      Serial.println(jsonString);
      delay(10);
      LoRa.beginPacket();
      Serial.print("Begin LoRa Packet");
      //LoRa.write(jsonString.length()); 
      Serial.print("Setting Size Packet");
      LoRa.print(jsonString);
      Serial.print("Print LoRa Packet");
      LoRa.endPacket();
      Serial.print("End LoRa Packet");
      Serial.println(jsonString);
      Serial.println("Message was sended.");
     
    sendLoRaMsgToOther = false;
  }

  onReceive(LoRa.parsePacket());

}

void charcopy(char* src, char* dst, int len) {
    memcpy(dst, src, sizeof(src[0])*len);
}

String main_page() {
  String page = ("<!DOCTYPE html><html><head> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, height=device-height, target-densitydpi=high-dpi, initial-scale=1.0'> <meta name='apple-mobile-web-app-capable' content='yes'/> <meta http-equiv='X-UA-Compatible' content='ie=edge'> <title>Contact</title> <style>html, body{width: 100%; height: 100%; max-height: 100vh; margin: 0; padding: 0; box-sizing: border-box; font-family: Helvetica, Geneva, Verdana, sans-serif;}*, *:before, *:after{box-sizing: inherit;}#container{height: 100%; max-height: 100%; margin: 0 auto;}#content{width: 100%; height: 100%; padding: 8px; background: #fafafa;}#user-set{position: absolute; z-index: 3500; margin: 0px; background: rgba(100,100,100, .8); width: 100%;height: 100%; top: 0px; left: 0px; overflow: hidden;}.user-set-hide{top: -110%; -webkit-transition: left 1s ease-in-out; transition: left 1s ease-in-out;}.user-set-show{top: 0px; visibility: visible;}#user-content{background: white; position: relative; margin: 15% auto; color: white; width: 95%; text-align: center; vertical-align: middle; border: 3px solid darkgray; box-shadow: 0 0 6px #2f2f2f;}#user-content div{padding: 2px;}#user-content h3{font-size: x-large; color: #455a64;}#user{color: tomato; padding: 6px 4px; border-radius: 3px; box-shadow: none; outline:none; border: 1px solid #90A4AE; font-weight: bold; font-size: medium; text-align: center; width: 95%;}#register{font-size: medium; color: ghostwhite; background: #4284f4; border-radius: 3px; border: none; padding: 8px; margin: 4px 8px;}#register{margin: 20px 0px 24px 0px;}#register:hover, #register:active{background: #0d47a1;}#user-info{position: absolute; z-index: 2200; bottom: 0px; opacity: .5; color: #3f3f3f; text-align: center;}#user-info h3{display: inline-block; font-size: medium; font-family: consolas;}#user-info h3:first-child{font-weight: normal; visibility: hidden;}#id-user{margin: 98% 50%; font-size: .02em;}#panel{position: absolute; z-index: 3000; top: 0px; height: 99.8%; width: 97%;}.panel-hide{left: -90%; -webkit-transition: left 1s ease-in-out; transition: left 1s ease-in-out;}.panel-show{left: 0px;}#panel > div{position: relative; display: block; float:left;}span{font-size: large; color: whitesmoke; visibility: hidden;}#list{width: 94%; background: #212121; height: 100%; padding: 0px 0px;}#list-header{width: 100%; height: 5%; padding: 2px 0px 40px; color: lightgrey; border-bottom: 1px solid #2e2e2e; line-height: 1.4; vertical-align: middle;}#list-title{height: inherit; width: inherit; margin: 0px;}#list-title > h4{margin: 6px 10px 10px; display: block; font-weight: normal;}#list-content{margin-top: 4px; height: 94%; display: flex; flex-direction: column; overflow: hidden; overflow-y: auto; font-size: large; width: 100%;}.dev-unit{display: block; padding: 4px 10px 8px; border-left: 10px solid #272727; background: inherit; height: fit-content; margin-left: 6px; margin-bottom: 6px; padding-top: 8px;}#dev-unit-text, #dev-unit-box{display: block; float: left;}#dev-unit-text{width: 92%; padding-left: 4px;}#dev-unit-box{color: white; cursor: pointer; font-size: large;}#dev-unit-text > div.title{display: block; color: rgb(201, 147, 12);}#dev-unit-text > div.desc{color: gray; display: block; padding-top: 4px; font-size: small;}#dev-unit-box > div.checked{border-left: 10px solid rgb(68, 67, 38);}#icon-area{width: 5%; padding-left: 4px; padding-top: 4px;}#icon{background: #263238; color: white; cursor: pointer; width: 34px; height: 34px; border: 1px solid #263238; border-radius: 100%; font-weight: bolder; font-size: 2em; box-shadow: 0px 0px 4px black; line-height: 29px;}#settings{margin-top: 8px; padding-left: 4px; margin-left: 3px; padding-top: 3px; background: #455a64; color: white; cursor: pointer; width: 34px; height: 34px; border: 1px solid #455a64; border-radius: 100%; font-weight: bolder; font-size: 1.75em; box-shadow: 0px 0px 4px black; line-height: 29px;}@keyframes help-flash{0%{background-color: red;}25%{background-color: #ff9999;}50%{background-color: red;}100%{background-color: #ff9999;}}#help-me{margin-top: 10px; padding-left: 4px; margin-left: 2px; padding-top: 3px; background-color: var(--help-bg); background-image: var(--help-bg-image); background-repeat: var(--no-repeat); background-position: 50% 35%; background-size:60%; color: white; cursor: pointer; width: 34px; height: 34px; border: 1px solid red; border-radius: 100%; font-weight: bolder; font-size: 1.75em; box-shadow: 0px 0px 4px black; line-height: 29px; animation-name: help-flash; animation-duration: 1s; animation-iteration-count: infinite;}#help-me:hover::after{position:absolute; z-index: inherit; margin-left: 25px; margin-top: 20px; padding: 4px 9px; width: 130px; border-radius: 5px; background: #333; font-size: small; content: 'Requesting Help';}.icon-unselect{-webkit-transition: -webkit-transform .8s ease-in-out; transition: transform .8s ease-in-out;}.icon-select{-webkit-transform:rotate(225deg); transform:rotate(225deg);}#fab{position: fixed; display: block; bottom: 25px; left: 50%; animation: bot-to-top 1s ease-out;}#fab-icon{z-index: 2000; background: #00AA8D; box-shadow: 0px 0px 6px black; width: 66px; height: 66px; text-align: center; border-radius: 100%; font-size: 2em; position: relative; left: -50%; display: inline-block; color: white; vertical-align: center; cursor: pointer; line-height: 1.98; animation: rotate-in 3s;}.fabicon-select{-webkit-transition: all 1s; -webkit-transform: rotateZ(225deg); transform: rotateZ(225deg); transition: all 1s;}.fabicon-unselect{-webkit-transition: all 1s; -webkit-transform: rotateZ(0deg); transform: rotateZ(0deg); transition: all 1s;}.fab-menu-init{opacity: 0; margin-bottom: -35px; -webkit-transition: all .5s ease-in-out; transition: all .5s ease-in-out;}.fab-menu-toggle{opacity: 1; margin-bottom: 20px; visibility: visible;}ul{position: relative; left: -50%; list-style: none; padding: 0px; margin-bottom: -5px;}ul li{display: block; width: 42px; height: 42px; margin: 0 auto; padding-top: 12px; text-align: center; color: white; border-radius: 100%; cursor: pointer; box-shadow: 0px 0px 6px black;}ul li:first-child{background: #4284f4; background-image: url(data:image/svg+xml;utf8;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iaXNvLTg4NTktMSI/Pgo8IS0tIEdlbmVyYXRvcjogQWRvYmUgSWxsdXN0cmF0b3IgMTkuMC4wLCBTVkcgRXhwb3J0IFBsdWctSW4gLiBTVkcgVmVyc2lvbjogNi4wMCBCdWlsZCAwKSAgLS0+CjxzdmcgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayIgdmVyc2lvbj0iMS4xIiBpZD0iQ2FwYV8xIiB4PSIwcHgiIHk9IjBweCIgdmlld0JveD0iMCAwIDQ5MCA0OTAiIHN0eWxlPSJlbmFibGUtYmFja2dyb3VuZDpuZXcgMCAwIDQ5MCA0OTA7IiB4bWw6c3BhY2U9InByZXNlcnZlIiB3aWR0aD0iMTZweCIgaGVpZ2h0PSIxNnB4Ij4KPGc+Cgk8cGF0aCBkPSJNMTEwLjA3Myw0NjUuNzN2LTk5LjUxNkgwVjI0LjI3aDQ5MHYzNDEuOTQ0SDIzMi4zMTlMMTEwLjA3Myw0NjUuNzN6IE0zMC42MjUsMzM1LjU4OWgxMTAuMDczdjY1LjcyMWw4MC43MzUtNjUuNzIxICAgaDIzNy45NDJWNTQuODk1SDMwLjYyNVYzMzUuNTg5eiIgZmlsbD0iI0ZGRkZGRiIvPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+CjxnPgo8L2c+Cjwvc3ZnPgo=); background-repeat: no-repeat; background-position:50% 60%; background-size:60%;}:root{--help-bg: #ff4444; --help-bg-image: url(data:image/svg+xml;utf8;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iaXNvLTg4NTktMSI/Pgo8IS0tIEdlbmVyYXRvcjogQWRvYmUgSWxsdXN0cmF0b3IgMTkuMC4wLCBTVkcgRXhwb3J0IFBsdWctSW4gLiBTVkcgVmVyc2lvbjogNi4wMCBCdWlsZCAwKSAgLS0+CjxzdmcgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayIgdmVyc2lvbj0iMS4xIiBpZD0iQ2FwYV8xIiB4PSIwcHgiIHk9IjBweCIgdmlld0JveD0iMCAwIDQ2OS4zMzMgNDY5LjMzMyIgc3R5bGU9ImVuYWJsZS1iYWNrZ3JvdW5kOm5ldyAwIDAgNDY5LjMzMyA0NjkuMzMzOyIgeG1sOnNwYWNlPSJwcmVzZXJ2ZSIgd2lkdGg9IjE2cHgiIGhlaWdodD0iMTZweCI+CjxnPgoJPGc+CgkJPHBhdGggZD0iTTIzNC42NjcsMzJMMCw0MzcuMzMzaDQ2OS4zMzNMMjM0LjY2NywzMnogTTI1NiwzNzMuMzMzaC00Mi42Njd2LTQyLjY2N0gyNTZWMzczLjMzM3ogTTIxMy4zMzMsMjg4di04NS4zMzNIMjU2VjI4OCAgICBIMjEzLjMzM3oiIGZpbGw9IiNGRkZGRkYiLz4KCTwvZz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8L3N2Zz4K); --no-repeat: no-repeat;}ul li:nth-child(2){background: var(--help-bg); background-image: var(--help-bg-image) ; background-repeat: var(--no-repeat); background-position:50% 40%; background-size:60%;}ul li:nth-child(3){background: #00c851; background-image: url(data:image/svg+xml;utf8;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iaXNvLTg4NTktMSI/Pgo8IS0tIEdlbmVyYXRvcjogQWRvYmUgSWxsdXN0cmF0b3IgMTguMS4xLCBTVkcgRXhwb3J0IFBsdWctSW4gLiBTVkcgVmVyc2lvbjogNi4wMCBCdWlsZCAwKSAgLS0+CjxzdmcgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayIgdmVyc2lvbj0iMS4xIiBpZD0iQ2FwYV8xIiB4PSIwcHgiIHk9IjBweCIgdmlld0JveD0iMCAwIDMyIDMyIiBzdHlsZT0iZW5hYmxlLWJhY2tncm91bmQ6bmV3IDAgMCAzMiAzMjsiIHhtbDpzcGFjZT0icHJlc2VydmUiIHdpZHRoPSIxNnB4IiBoZWlnaHQ9IjE2cHgiPgo8Zz4KCTxnIGlkPSJtYXBfeDVGX3Bpbl94NUZfYWx0Ij4KCQk8cGF0aCBkPSJNMTYsMGMtNC40MTgsMC04LDMuNTgyLTgsOHM4LDI0LDgsMjRzOC0xOS41ODIsOC0yNFMyMC40MTgsMCwxNiwweiBNMTYsMTJjLTIuMjA5LDAtNC0xLjc5MS00LTQgICAgczEuNzkxLTQsNC00czQsMS43OTEsNCw0UzE4LjIwOSwxMiwxNiwxMnoiIGZpbGw9IiNGRkZGRkYiLz4KCTwvZz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8L3N2Zz4K); background-repeat: no-repeat; background-position:50% 40%; background-size:60%;}ul li:hover:after{position: absolute; padding: 6px; font-size: small; border-radius: 5px; margin-left: 40px; margin-top: -6px; background: #4f4f4f;}ul li:first-child:hover:after{content: 'SMS';}ul li:nth-child(2):hover:after{content: 'Beacon';}ul li:nth-child(3):hover:after{content: 'Location';}.hide{visibility: hidden;}#help-req{position: absolute; z-index: 4000; margin: 0px; background: rgb(185, 5, 5); width: 100%;height: 100%; top: 0px; left: 0px; overflow: hidden;}#help-content{position: relative; margin: 0 auto; color: white; height: 100%; width: 90%; text-align: center; vertical-align: middle;}#help-content > h1{margin-top: 10%;}#help-content > h3{font-weight: normal; color: #999999; text-shadow: 0px 0px 3px black;}#help-close{position: absolute; display: inline-block; right: 10px; top: 10px; width: 40px; height: 40px; padding-top: 5px; font-size: 1.25em; border-radius: 100%; text-align: center; vertical-align: middle; color: red; background: darkred; cursor: pointer;}#onoff-switch{margin-top: 50px; padding: 0px; text-align: center; display: block; width: 100%;}.onoffswitch{margin: 0 auto; position: relative; width: 228px; -webkit-user-select:none; user-select: none;}.onoffswitch-checkbox{display: none;}.onoffswitch-label{display: block; overflow: hidden; cursor: pointer; border: 1px solid #999999; border-radius: 50px; box-shadow: 0px 0px 10px black;}.onoffswitch-inner{width: 305%; margin-left: -200%; -webkit-transition: margin 0.3s ease-in 0s; transition: margin 0.3s ease-in 0s;}.onoffswitch-inner:before, .onoffswitch-inner:after{float: left; width: 50%; height: 104px; padding: 0; line-height: 30px; font-size: 44px; color: white; font-family: Trebuchet, Arial, sans-serif; font-weight: bold; -webkit-box-sizing: border-box; box-sizing: border-box;}.onoffswitch-inner:before{content: \"ON\"; text-shadow: 0px 0px 3px black; padding-top: 36px; padding-left: 20px; margin-left: -111px; background-color: white; color: red;}.onoffswitch-inner:after{content: \"OFF\"; text-shadow: 0px 0px 5px white; margin-left: 100px; padding-top: 36px; padding-right: 20px; background-color: #EEEEEE; color: #999999; text-align: right;}.onoffswitch-switch{z-index: 4100; width: 96px; margin: 4px; background: #FFFFFF; border: 2px solid #999999; border-radius: 50px; position: absolute; top: 0; bottom: 0; right: 120px; box-shadow: 0px 0px 4px black; -webkit-transition: all 0.3s ease-in 0s; transition: all 0.3s ease-in 0s;}.no-display{display: none;}.onoffswitch-checkbox:checked + .onoffswitch-label .onoffswitch-inner{margin-left: 0;}.onoffswitch-checkbox:checked + .onoffswitch-label .onoffswitch-switch{right: 0px; border: 2px solid #ff9999; animation-duration: 0.25s; animation-name: changeBackground; animation-iteration-count: infinite; animation-direction: alternate;}#msg-panel{position: absolute; z-index: 2500; width: 100%;height: 100%; background: rgba(100,100,100, .5); top: 0px; left: 0px; overflow: hidden;}#msg-box{display: block; margin-top: 15%; margin-left: 2%; height: 175px; width: 90%; padding: 8px; border: 2px solid lightgrey; border-radius: 3px; box-shadow: 0px 0px 10px black; background: ghostwhite;}#msg-box > div:first-child{border: 1px solid whitesmoke; padding: 6px; height: 75%;}#msg-box > div:last-child{text-align: right; padding: 8px;}#msg-box > div:last-child div{display: inline-block; height: 32px; padding: 3px 7px; border-radius: 3px; margin: 5px 4px 4px; font-size: 1.35; line-height: 1.65; cursor: pointer; background:#455a64; color: white; box-shadow: 0px 0px 4px #333; border: none;}#msg-box > div:last-child div:active, #msg-box > div:last-child div:hover{box-shadow: none; background: #263238;}#msg-box > div:last-child div:last-child{display: inline-block; border-radius: 3px; height: 32px; padding: 3x 7px; margin: 4px; font-size: 1.35; line-height: 1.65; background: #4284f4; color: white; box-shadow: 0px 0px 4px #333; border: none;}#msg-box > div:last-child div:last-child:active, #msg-box > div:last-child div:last-child:hover{box-shadow: none; background: #0d47a1;}textarea{border: 0px; box-shadow: 0px; outline: none; font-size: 1.25em; width: 100%; height: 100%; padding: 6px; background: white; border-radius: 3px;}#chat-content{width: 70%; height: 100%; overflow: hidden; overflow-y: auto; margin: 0 auto;}#chat{max-height: 90%; width: 100%; top: 0px; left: 0px; list-style: none; padding: 0px;}#chat li{display: inline-block; clear: both; text-align: initial; width: initial; cursor: initial; height: initial; margin: 4px 2px; padding: 10px; border-radius: 8px; font-size: 1.25; box-shadow: none; max-width: 85%; color: white;}#chat .me{float: right; background: rgb(2, 131, 252); color: white;}#chat .me .devrx{text-align: right; font-size: x-small; color: lightblue;}#chat .me:last-of-type{border-top-right-radius: 0px; border-bottom-right-radius: 15px;}#chat .him{float: left; background: gray;}#chat .need-help{background: darkred; color: white;}#chat .me.need-help:before, #chat .him.need-help:after{content: ' \\26A0';}#chat .him:last-of-type{border-top-left-radius: 0px; border-bottom-left-radius: 15px;}#chat #dev-usr{font-size: x-small; color: lightgray; display: block; text-align: right; margin-bottom: 2px;}#chat .him #dev-usr{text-align: left;}#forma-panel{position: absolute; z-index: 9999; margin: 0px; background: rgba(100,100,100, .8); width: 100%;height: 100%; top: 0px; left: 0px; overflow: hidden;}.forma-show{top: 0px; visibility: visible;}#forma{margin: 10% auto; width: 90%; height: fit-content; font-family: sans-serif; background: ghostwhite; border: 2px solid lightgray; box-shadow: 0px 0px 4px black; font-size: medium; border-radius: 3px;}#forma-header{font-size: small; color: gray; text-align: left; border-bottom: 1px solid lightgray; padding: 6px 12px;}#forma div:nth-child(2){padding: 4px; margin: 2px; text-align: left; vertical-align: top;}#forma > div:nth-child(2) label{display: inline-block; color: #2e2e2e; padding: 2px 8px;}#forma div:nth-child(2) > input[type='text']{display: block; box-lines: 0px; outline: none; box-shadow: 0px; border: 1px solid lightgray; padding: 6px 8px; border-radius: 3px; margin: 4px 4px 10px; font-size: large;}#save{padding: 8px; border-radius: 3px; background: #4284f4; color: white; margin: 10px 4px; font-size: medium; border: none; box-shadow: 0px 0px 4px gray;}#save:active, #save:hover{background: #0d47a1; box-shadow: none;}#cancel{padding: 8px; background: #455a64; color: ghostwhite; margin: 10px 4px; font-size: medium; border-radius: 3px; border: none; box-shadow: 0px 0px 4px gray;}#cancel:hover, #cacel:active{background: #263238; box-shadow: none;}#forma > div:last-child{width: 100%; text-align: right; padding: 6px 10px 4px; margin-top: 8px; border-top: 1px solid lightgray;}#location, #emergency{width: 97%;}#btnReset{float: left; margin: 10px 4px; background: orangered; color: white; border: none; border-radius: 5px; padding: 8px; font-size: medium;}#btnReset:active{background: orange;}@keyframes changeBackground{from{background: white;}to{background: red;}}@keyframes rotate-in{from{transform: rotate(0deg);}to{transform: rotate(360deg);}}@keyframes rotate-out{from{transform: rotate(360deg);}to{transform: rotate(0deg);}}@keyframes bot-to-top{0%{bottom:-100px}50%{bottom: 0px}}@media (max-width: 812px){#panel{width: 91%;}.panel-hide{left: -85%;}.panel-show{left: 0px;}#icon{margin-top: 3px; margin-left: 3px; padding-left: 3px; padding-top: 2px; width: 32px; height: 32px; font-size: 1.5em;}#fab-icon{background: #008975; opacity: 0.6; padding-left: 1px; padding-top: 1px; line-height: 2.37; font-size: 1.75em;}#help-content{margin-top: 30%;}#msg-box{width: 97vw; left: -4px; padding: 4px 2px 2px;}#msg-box > div:first-child{padding: 2px;}#msg-box > div:last-child{padding: 0px;}textarea{padding: 2px; font-size: 1.2em; line-height: 1.5;}#chat-content{width: 100%;}}/* Larger than desktop */@media (min-width: 813px){#panel{width: 45%;}.panel-hide{left: -35%;}.panel-show{left: 0px;}h1{font-size: 3em;}h3{font-size: 2em;}ul li:first-child{padding-top: 10px;}ul li:last-child{padding-top: 13px;}#msg-box{margin-left: 15%; height: 30%; width: 70%; padding: 4px;}#user-content{width: 35%;}#user-content div{padding: 10px 10px 27px;}#user{width: auto;}#forma{width: 336px;}#forma-header{font-size: large; padding: 10px 12px;}#forma > div:last-child{padding: 8px 10px 6px;}#forma > div:nth-child(2) label{padding: 10px 8px 2px;}#settings{margin-left: 1px; padding-top: 1px;}}</style></head><body> <div id='container'> <div id='content'> <div id='chat-content'> <ul id='chat'> </ul> <div id='user-info'> <h3>User:</h3> <h3 id='id-user'></h3> </div></div><div id='fab'> <ul id='fab-menu'> <li class='hide fab-menu-init' onclick='ToggleMsg();'>&nbsp;</li><li class='hide fab-menu-init' onclick='ToggleHelp();'>&nbsp;</li><li class='hide fab-menu-init' onclick='ToggleHelp();'>&nbsp;</li></ul> <div id='fab-icon' class='fabicon-unselect' onclick='ToggleFab();'>&#65291;</div></div></div></div><div id='panel' class='panel-hide'> <div id='list'> <div id='list-header'> <div id='list-title'> <h4><span>&#9967;</span>&nbsp;ONLINE LOCATIONS</h4> </div></div><div id='list-content'> </div></div><div id='icon-area' > <div id='icon' class='icon-unselect' onclick='TogglePanel();'>&#65291;</div><div id='settings' style='display:block;' onclick='openForm();'>&#8943;</div><div id='help-me' class='help-me hide'></div></div></div><div id='user-set' class='hide'> <div id='user-content'> <div> <h3>How do people call you?</h3> <input type='text' name='user' id='user' size='30' maxlength='20' placeholder='Your Nickname or Username' autofocus> <input type='button' id='register' value='Continue' onclick='setUser()'> </div></div></div><div id='help-req' class='hide'> <div id='help-content'> <h1>Emergency Request Notification</h1> <h3>This Notification Service reach all communities networks.</h3> <div id='onoff-switch' > <div id='toogle-alert'> <div class=\"onoffswitch\"> <input type=\"checkbox\" name=\"onoffswitch\" class=\"onoffswitch-checkbox\" id=\"myonoffswitch\"> <label id='onoff-label' class=\"onoffswitch-label\" for=\"myonoffswitch\"> <div class=\"onoffswitch-inner\"></div><div id='round-switch' class=\"onoffswitch-switch no-display\"></div></label> </div></div></div></div><div id='help-close' onclick='ToggleHelp();'>&#x2715;</div></div><div id='msg-panel' class='hide'> <div id='msg-box'> <div> <textarea id='msg' autofocus maxlength='280' rows='3' placeholder='Your message here ...'></textarea> </div><div> <div id='cancel-msg' onclick='ToggleMsg();'>Cancel</div><div id='snd-msg' onclick='SendMessage();'>Send</div></div></div></div><div id='forma-panel' class='hide'> <div id='forma'> <form id='data-form' name='data-form'> <div id='forma-header'> Settings </div><div> <label>Location:</label> <input type='text' size='30' maxlength='30' id='location' name='location' placeholder='How others can found you.' > <label>Message on Emergency:</label> <input type='text' size='30' maxlength='30' id='emergency' name='emergency' placeholder='What others need to know.' > </div><div> <input type='button' id='btnReset' value='Reset Chat' onclick='resetChat();'> <input type='button' value='Cancel' id='cancel' onclick='closeForm();'> <input type='button' value='Save' id='save' onclick='saveForm();'> </div></form> </div></div><script>var lastMsgReaded=0; var bFirstTime=true; var bResquestingHelp=false; var user=''; var unitLocation=''; var chkedUnits=\"\"; var aUnits=new Array(); var nReqHelpId=0; if (sessionStorage.getItem('bFirstTime') !=null){bFirstTime=false; lastMsgReaded=sessionStorage.getItem('lastMsgReaded'); document.getElementById('id-user').innerText=sessionStorage.getItem('user'); user=sessionStorage.getItem('user');}else{document.getElementById('user-set').classList.toggle('hide'); document.getElementById('user-set').classList.toggle('user-set-show'); document.getElementById('user').focus(); sessionStorage.setItem('bFistTime', false);}DeviceNotify(); GetDeviceList(); GetMsgs(); setInterval(GetMsgs, 7000); setInterval(DeviceNotify, 10000); setInterval(GetDeviceList, 10000); function setUser(){user=document.getElementById('user').value; if (user.length > 0){document.getElementById('user-set').classList.toggle('user-set-show'); document.getElementById('user-set').classList.toggle('hide'); /*document.getElementById('id-user').innerText=user;*/ sessionStorage.setItem('user', user);}else document.getElementById('user').focus();}function TogglePanel(){document.getElementById('panel').classList.toggle('panel-show'); document.getElementById('icon').classList.toggle('icon-select');}function ToggleFab(){document.getElementById('fab-icon').classList.toggle('fabicon-select'); document.getElementById('fab-icon').classList.toggle('fabicon-unselect'); var items=Array.from(document.getElementById('fab-menu').getElementsByTagName('li')); items.forEach(function(li, i, items){if (li.className.includes('hide')) li.classList.toggle('hide'); li.classList.toggle('fab-menu-toggle');});}function ToggleHelp(){if (document.getElementById('fab-icon').className.includes('fabicon-select')) ToggleFab(); document.getElementById('help-req').classList.toggle('hide'); document.getElementById('round-switch').classList.toggle('no-display'); if (document.getElementById('myonoffswitch').checked==true){document.getElementById('help-me').classList.remove('hide'); RequestHelp(); nReqHelpId=setInterval(RequestHelp, 10000);}else{document.getElementById('help-me').classList.add('hide'); clearInterval(nReqHelpId);}}function RequestHelp(){console.log('** Request Help'); aUnits.forEach(function(val,i){console.log('Dev #' + i + ' ' + val);});}function SendMessage(){if (document.getElementById('dev-select')==null){alert('There is no other online community for communication.'); ToggleMsg(); return;}var chat=document.getElementById('chat'); var msg=document.getElementById('msg'); var aUnits=chkedUnits.split(','); aUnits.pop(); if (aUnits.length < 1){alert('Please select at least one of the online communities from the list in the left panel.'); return;}if (msg.value.length < 2){alert('You dont enter any message.');}if (msg.value.length >=2){var tmp=chkedUnits.slice(0, -1); chat.innerHTML +=('<li class=\"me\">' + msg.value + '<br><div class=\"devrx\">' + 'sended to [' + tmp + ']</div></li>'); var newMsgs=''; newMsgs=(localStorage.getItem('chat') !=null? localStorage.getItem('chat'):'') + ('<li class=\"me\">' + msg.value + '<br><div class=\"devrx\">' + 'sended to [' + tmp + ']</div></li>'); localStorage.setItem('chat', newMsgs); aUnits.forEach(function(devUnit, i, aUnits){SendToServer(msg.value, devUnit);});}var ChatDiv=document.getElementById('chat-content'); ChatDiv.scrollTop=ChatDiv.scrollHeight; ToggleMsg();}function ToggleMsg(){var settings=document.getElementById('settings'); settings.style.display=(settings.style.display=='block'? 'none': 'block'); if (document.getElementById('fab-icon').className.includes('fabicon-select')) ToggleFab(); document.getElementById('msg-panel').classList.toggle('hide');}function SendToServer(msg, unit){console.log('Send'); fetch('put?message=' + msg + '&unit=' + unit + '&usr=' + user ,{mode: 'no-cors'}) .then(response=> response.json()) .then(json=> console.log(json)) .catch(function(error){console.log('Request failed', error);});}function DeviceNotify(){console.log('** Device Broadcast'); var devInfo='{\"uname\":\"\", \"location\":\"Physical Location\", \"lat\": 18.38, \"long\":66.16}'; if (localStorage.getItem('location')===null) localStorage.setItem('location', devInfo); else{/* TO-DO: Get Form Info */}fetch('notify?location={\"uname\":\"\", location\":\"Physical Location\", \"lat\": 18.38, \"long\":66.16}',{mode: 'no-cors'}) .then(response=> response.json()) .then(json=> console.log(json)) .catch(function(error){console.log('Sending failed', error);});}function GetDeviceList(){console.log('** Request Dev List'); var devs=''; fetch('getdev?a=1',{mode: 'no-cors'}) .then(response=> response.json()) .then(json=>{if (json.data.length > 0){aUnits=new Array(); for(i=0; i < json.data.length; i++){aUnits.push(json.data[i]['devunit']); devs +='<div class=\"dev-unit\"><div id=\"dev-unit-text\"><div class=\"title\">' + json.data[i]['devunit'] + '</div>' + '<div class=\"desc\">&nbsp;</div></div>' + `<div id=\"dev-unit-box\"><div id=\"dev-select\" onclick=\"check(this,'` + json.data[i]['devunit'] + `')\">` + (chkedUnits.includes(json.data[i]['devunit']) ? '&#x2611;' : '&#x2610;') + `</div></div></div>`;}document.getElementById('list-content').innerHTML=devs;}}) .catch(function(error){console.log('Sending failed', error);}); document.getElementById('chat')}function GetMsgs(){var msgs=''; if ((localStorage.getItem('chat') !=null) && (bFirstTime)){document.getElementById('chat').innerHTML=localStorage.getItem('chat'); console.log('Readed from LocalStorage.'); var ChatDiv=document.getElementById('chat-content'); ChatDiv.scrollTop=ChatDiv.scrollHeight;}console.log('Reading from Device.'); fetch('get?lastMsg=' + lastMsgReaded + '&fistTime=' + (bFirstTime?1:0),{mode: 'no-cors'}) .then(response=> response.json()) .then(json=>{var msgs=\"\"; if (json.data.length > 0){console.log('Messages Readed. JSON Data on Page.'); for(i=0; i < json.data.length; i++){msgs +='<li class=\"him\"><div id=\"dev-usr\">' + json.data[i]['txid'] + '</div>' + json.data[i]['message'] + '</li>'; lastMsgReaded=json.data[i]['pqid'];}sessionStorage.setItem('lastMsgReaded', lastMsgReaded); var newMsgs=''; if (localStorage.getItem('chat')===null) newMsgs=msgs; else newMsgs=localStorage.getItem('chat') + msgs; console.log('Message Structure: '); console.log(newMsgs); localStorage.setItem('chat', newMsgs); document.getElementById('chat').innerHTML +=msgs; bFirstTime=false; var ChatDiv=document.getElementById('chat-content'); ChatDiv.scrollTop=ChatDiv.scrollHeight;}}) .catch(function(error){console.log('Request failed', error);});}function check(sender, devUnit){if (sender.innerHTML.charCodeAt(0)==9745){sender.innerHTML='&#x2610;'; var tmp=(devUnit + ','); chkedUnits=chkedUnits.replace(tmp, '');}else{sender.innerHTML='&#x2611;'; chkedUnits +=(devUnit + ',');}sender.parentNode.parentNode.classList.toggle('checked');}function openForm(){document.getElementById('forma-panel').classList.toggle('hide'); document.getElementById('forma-panel').classList.toggle('forma-show'); getSomeVars();}function saveForm(){var loc=document.getElementById('location').text; /*var devInfo='{\"uname\":\"\", \"location\":' + '\"' + loc + '\", \"lat\": 18.38, \"long\":66.16}'; */ var devInfo=''; localStorage.setItem('location', devInfo); localStorage.setItem('emergency', document.getElementById('emergency').text); saveFormData();}function closeForm(){document.getElementById('forma-panel').classList.toggle('hide'); document.getElementById('forma-panel').classList.toggle('forma-show');}function resetChat(){console.log('** Reset Chat'); if (localStorage.getItem('chat') !=null) localStorage.setItem('chat', ''); var devInfo='{\"uname\":\"\", \"location\":\"Physical Location\", \"lat\": 18.2208, \"long\":66.5901}'; if (localStorage.getItem('location')==null) localStorage.setItem('location', devInfo); document.getElementById('chat').innerHTML(''); console.log(' Chat Reseted');}function getSomeVars(){console.log('Getting Unit Setup Values.'); fetch('getsomevars?id=1234',{mode: 'no-cors'}) .then(response=>response.json()) .then(json=>{console.log('JSON: ', json.data[0]); if (json.data.length > 0){unitLocation=json.data[0]['location']; document.getElementById('location').value=json.data[0]['location']; document.getElementById('emergency').value=json.data[0]['helpmsg'];}}) .catch(function(error){console.log('Request failed', error);});}function saveFormData(){var dataVars=('emergency=' + document.getElementById('emergency').value); dataVars +=('&location=' + document.getElementById('location').value); console.log(dataVars); fetch('putsomevars?' + dataVars,{mode: 'no-cors'}) .then(response=> response.json()) .then(json=> console.log(json)) .catch(function(error){console.log('Request failed', error);}); alert('New Settings Saved.');}</script></body></html>");
  return (page);
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  //int recipient = LoRa.read();          // recipient address
  // byte sender = LoRa.read();            // sender address
  // byte incomingMsgId = LoRa.read();     // incoming msg ID
  // byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  /*  Another LoRa Reader
  while (LoRa.available()) {
    incoming = LoRa.readString();
  }
  */
  if (incoming.length() >= 40) // If possible message for our unit is processed.
  { 
      String content = incoming; //.substring(1, incoming.length());
      Serial.println(content);
      
      // Allocate JsonBuffer
      // Use arduinojson.org/assistant to compute the capacity.
      const size_t capacity = JSON_OBJECT_SIZE(8) + 600; // = JSON_OBJECT_SIZE(8) + JSON_ARRAY_SIZE(2) + 60;
      DynamicJsonBuffer jsonBuffer; //(capacity); 
      // Parse JSON object
      JsonObject& root = jsonBuffer.parseObject(content);
      if (!root.success()) {
        Serial.println(F("Parsing failed!"));
        return;
      }
      Serial.println("JSON Text on LoRa: ");
      root.prettyPrintTo(Serial);

      if (strcmp(root["txid"].as<char*>(), hostName) == 0) {
          Serial.println("Unidad que transmite recibiendo el mismo.");
          Serial.println(root["txid"].as<char*>());
          return;
      }
      char* all = "ALL";
      if (CONTACTvars.isGateway == 1) // If Unit is Gateway Receive Any Message
          root["rxid"] =  "ALL"; 
      if ((strcmp(root["rxid"].as<char*>(), all) == 0) && ((root["ttl"].as<int>() - 1) > 0)
          && (strcmp(root["rxid"].as<char*>(), hostName) == 0))
        { // Mesh Network
          Serial.print("** Msg to Mesh ...");
          Serial.println(Serial.println(root["rxid"].as<char*>()));
          root["ttl"] = String(root["ttl"].as<int>() - 1);
          root.printTo(MESH_MSG);
          sendLoRaMsgToOther = true;
          return;
        }
          
      if ((strcmp(root["rxid"].as<char*>(), hostName) != 0) && 
          (strcmp(root["rxid"].as<char*>(), all) != 0)) {
            Serial.println("Ninguna");
            Serial.println(root["rxid"].as<char*>());
          return;
      }
      
      if ((root["pid"].as<int>() == 0) || (root["ptype"].as<int>() == 0) || (String(root["msg"].as<char*>()).length() < 1)
        || (String(root["txid"].as<char*>()).length() < 1)) {
        Serial.println(F("Packet Data Error."));
        return;
      }      
      if (root["ptype"].as<int>() == 1) {        
        Serial.print("Text Message Received ...");
                  
        rx_message_queue[rx_MSG_CTR_Q].packetId = root["pid"].as<int>();
        strcpy( rx_message_queue[rx_MSG_CTR_Q].data, root["msg"].as<char*>()); //,sizeof(root["message"].as<char*>())); //strcpy( ..., p->value().c_str());
        strcpy( rx_message_queue[rx_MSG_CTR_Q].TxID, root["txid"].as<char*>());
        strcpy( rx_message_queue[rx_MSG_CTR_Q].RxID, root["rxid"].as<char*>());
        rx_message_queue[rx_MSG_CTR_Q].packetsQty = root["pqty"].as<int>();
        rx_message_queue[rx_MSG_CTR_Q].packetCtr = root["pctr"].as<int>();
        rx_message_queue[rx_MSG_CTR_Q].packetType = root["ptype"].as<int>();
        rx_message_queue[rx_MSG_CTR_Q].ttl = (root["ttl"].as<int>() - 1);     
        rx_message_queue[rx_MSG_CTR_Q].packetQId = rx_MSG_CTR_Q;
        Serial.println("All MSG Content Readed.");
        rx_MSG_CTR_Q++;
        if (rx_MSG_CTR_Q == MAX_RX_MESSAGES)
          rx_MSG_CTR_Q = 0;

      } 
      if (root["ptype"].as<int>() == 4) {
          bool unitExist = false;
          int i = 0;
          Serial.println("Hand Shake Received ...");
          //Serial.println("Compare ...");
          for(i = 0; i < UNITS_CTR_Q; i++) {
            
            //Serial.println(unit_queue[i].TxID);
            //Serial.println(root["msg"].as<char*>());
            //Serial.println(strcmp(unit_queue[i].TxID, root["msg"]));
            // TO-DO: Verificar si la Unidad es un Nombre Valido
            
            if (strcmp(unit_queue[i].TxID, root["msg"]) == 0) {
              unitExist = true;
              break;
            }
          }
          if (!unitExist) {
            strcpy( unit_queue[UNITS_CTR_Q].TxID ,  root["msg"].as<char*>() );
            strcpy( unit_queue[UNITS_CTR_Q].lastDate , "01/01/2018" ); // Date is used from Device
            strcpy( unit_queue[UNITS_CTR_Q].lastTime , "01:00:00 am" );
      
            Serial.println(unit_queue[UNITS_CTR_Q].TxID);
            Serial.println("New Unit Registered on Queue");
            UNITS_CTR_Q++;          
          }
          else
            Serial.println("Unit Reported Exist on Queue");
      }
      // Extract values
     
      Serial.println("LoRa Data Readed!"); 
  } // if (incoming.length() >= 40) - If possible message for our unit is processed. 
}

void sendMessage() //(String outgoing) {
{
  LoRa.beginPacket();                     // start packet
 // LoRa.write(destination);              // add destination address
 // LoRa.write(localAddress);             // add sender address
 // LoRa.write(msgCount);                 // add message ID
 // LoRa.write(outgoing.length());        // add payload length
 // LoRa.print(outgoing);                 // add payload
  LoRa.print(message);
  LoRa.endPacket();                       // finish packet and send it
  msgCount++;                             // increment message ID
}

String geoLocation() {
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  return ("{\"lat\":\"" + String(gps.location.lat(),5) + "\"," +
          "\"log\":\"" + String(gps.location.lng(),5) + "\"," +
          "\"sat\":\"" + String(gps.satellites.value()) + "\"," +
          "\"date\":\"" + 
          (gps.date.day() < 10 ? "0" : "") + String(gps.date.day()) + "/" + 
          (gps.date.month() < 10 ? "0" : "") + String(gps.date.month()) + "/" + 
          String(gps.date.year()) + "\"," +
          "\"time\":\"" + (gps.time.hour() < 10 ? "0" : "") + String(gps.time.hour()) + 
          ":" + (gps.time.minute() < 10 ? "0" : "") + String(gps.time.minute()) + "\"}");
}

String settings_page() {
  return ("<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <meta http-equiv='X-UA-Compatible' content='ie=edge'> <title>Device Setup</title> <style>html, body, #content{font-family: Helvetica, Geneva, Verdana, sans-serif; background: rgb(247, 213, 194); height: 100%; width: 100%; margin: 0; padding:0; overflow: hidden; border-top: 1px solid orange;}#content{overflow-y: auto;}#form{margin: auto; border: 1px solid lightgray; border-radius: 3px; background: white; width: 90%; overflow-y: auto; box-shadow: 0px 0px 6px grey;}header{border-bottom: 1px solid lightgray; padding: 4px 8px; margin-bottom: 10px; background: rgb(37, 40, 43);}section{border-top: 1px solid lightgray; border-bottom: 1px solid lightgray; margin: 8px 0px;}aside{margin: 0px 22px 8px;}footer{text-align: right; padding: 5px 28px 12px 8px;}h1{font-weight: bolder; font-size: x-large; margin: 4px; color: white;}h3{font-weight: thin; font-size: medium; margin: 6px; color: darkgrey;}label{display: block; margin: 10px 14px 4px; font-size: small; color: grey;}input[type='text'], input[type='password'], textarea{border: 1px solid lightgray; box-shadow: none; outline: none; padding: 4px 8px; margin: 4px 16px; border-radius: 3px; font-size: medium; color: black;}input[type='button']{font-size: medium; padding: 8px; border: 0px; border-radius: 3px; color: white; box-shadow: 0px 0px 4px gray;}#save{background: #4284f4;}#save:active, #save:hover{background: #0d47a1; box-shadow: none;}#restart{background: rgb(255, 81, 0);}#restart:active, #restart:hover{background: rgb(255, 0, 0);}p{font-size: small; margin: 14px 0px 8px; padding: 0px 18px 8px 16px;}#checkbox{color: blue; margin-bottom: 12px; font-size: medium;}.freq{margin-bottom: 0px;}.unitFreq{display: inline-block;}.hide{display: none;}@media (max-width: 812px){#form{width: 98%; overflow-x: hidden; margin-top: 0;}h3{margin: 4px 6px;}aside{margin: 0px;}label{margin: 6px 14px 2px;}textarea{width: 85%;}#checkbox{font-size: small;}footer{margin-right: 0px; padding: 2px 12px 10px 0px;}}</style></head><body> <div id='content'> <header> <h1>CONTACT</h1> <h3>DEVICE SETTINGS</h3> </header> <div id='form'> <form> <label>Device Name (Home, Shelter, other):</label> <input type='text' maxlength='12' name='unitName' id='unitName' placeholder='Device Name' size='20' value=''> <Label>Location (if not mobile):</Label> <input type='text' maxlength='30' name='unitDesc' id='unitDesc' placeholder='Location' size='30' value=''> <Label class='freq'>Unit Frequency:</Label> <Label class='unitFreq'> <input type='radio' name='unitFreq' id='unitFreq1' value='1'>USA/Asia/Australia</Label> <Label class='unitFreq'> <input type='radio' name='unitFreq' id='unitFreq2' value='2'>Europe/India</Label> <section> <label id='checkbox'> <input type='checkbox' value='1' name='isGateway' id='isGateway' onchange='chkSelected();' name='isGateway'>&nbsp;Check to Use this Device as Gateway </label> <aside id='aside' class='hide'> <p>Please complete the following information to connect the CONTACT Device as Internet Gateway.</p><label>WiFi Connection to Network:</label> <input type='text' maxlength='20' name='ssid' id='ssid' placeholder='WiFi Network' value='' > <label>WiFi Password (if needed): </label> <input type='password' maxlength='20' name='ssidPwd' id='ssidPwd' placeholder='WiFi Password' value='' > <label>Server URL to Post the Messages:</label> <textarea name='smsServer' id='smsServer' row='3' cols='50' maxlength='200' placeholder='Rest URL for Messages' ></textarea> <label>Server URL to Post the Data:</label> <textarea name='dataServer' id='dataServer' row='3' cols='50' maxlength='200' placeholder='Rest URL for Data' ></textarea> </aside> </section> <footer> <input type='button' id='save' onclick='saveVars();' value='Save'>&nbsp; <input type='button' id='restart' onclick='restartDev();' value='Restart Unit'> </footer> </form> </div></div><script>getUnitSetup(); function chkSelected(){var lbl=document.getElementById('checkbox'); var chk=document.getElementById('isGateway'); document.getElementById('aside').classList.toggle('hide'); if (chk.checked==true){lbl.style.fontWeight='bold';}else{lbl.style.fontWeight='normal';}}function getUnitSetup(){console.log('Getting Unit Setup Values.'); fetch('getsetup?id=1234',{mode: 'no-cors'}) .then(response=>response.json()) .then(json=>{console.log('JSON Len: ', json.data); console.log('JSON: ', json.data[0]); if (json.data.length > 0){document.getElementById('unitName').value=json.data[0]['unitname']; document.getElementById('unitDesc').value=json.data[0]['unitdesc']; document.getElementById('unitFreq1').checked=(json.data[0]['unitfreq']=='0' ? false : (json.data[0]['unitfreq']=='1' ? true : false)); document.getElementById('unitFreq2').checked=(json.data[0]['unitfreq']=='0' ? false : (json.data[0]['unitfreq']=='2' ? true : false)); document.getElementById('isGateway').checked=(json.data[0]['isgateway']=='1' ? true : false); if (json.data[0]['isgateway']=='1') chkSelected(); document.getElementById('ssid').value=json.data[0]['ssid']; document.getElementById('ssidPwd').value=json.data[0]['ssidpwd']; document.getElementById('smsServer').value=json.data[0]['smsserver']; document.getElementById('dataServer').value=json.data[0]['dataserver'];}}) .catch(function(error){console.log('Request failed', error);});}function saveVars(){var dataVars=serialize(document.forms[0]); console.log(dataVars); fetch('putsetup?' + dataVars,{mode: 'no-cors'}) .then(response=> response.json()) .then(json=> console.log(json)) .catch(function(error){console.log('Request failed', error);}); alert('New Settings Saved.');}function restartDev(){var opt=confirm('Are you sure?'); if (opt==true){fetch('restart?id=1234',{mode: 'no-cors'}) .then(response=> response.json()) .then(json=> console.log(json)) .catch(function(error){console.log('Request failed', error);}); alert('Unit Restarted...\\nConnect again, if necessary!');}}function serialize(form){var lastClassName=''; var bEvaluated=false; if (!form || form.nodeName !=='FORM'){return;}var i, j, q=[]; for (i=form.elements.length - 1; i >=0; i=i - 1){var n=form.elements[i].name; var v=form.elements[i].value; var t=form.elements[i].type; if (n===''){continue;}switch (form.elements[i].nodeName){case 'INPUT': switch (t){case 'text': case 'hidden': case 'password': q.push(n + \"=\" + encodeURIComponent(v)); break; case 'checkbox': case 'radio': var nodeList=document.querySelectorAll('input[name=\"'+ n + '\"]'); if (nodeList.length > 1){if (n !==lastClassName){nodeList.forEach(function(node, i){if (node.checked){q.push(n + \"=\" + encodeURIComponent(node.value)); bEvaluated=true;}}); if (!bEvaluated) q.push(n + \"=\" + encodeURIComponent(0)); lastClassName=n;}}else if (form.elements[i].checked){q.push(n + \"=\" + encodeURIComponent(v));}else q.push(n + \"=\" + encodeURIComponent('0')); break;}break; case 'TEXTAREA': q.push(n + \"=\" + encodeURIComponent(v)); break;}}return q.join(\"&\");}</script></body></html>");
}

String gateway_page() {
  return ("<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"> <title>Push Messages to Server</title> <style>html, body, #content{font-family: Helvetica, Geneva, Verdana, sans-serif; background: rgb(247, 213, 194); height: 100%; width: 100%; margin: 0; padding:0; overflow: hidden; border-top: 1px solid orange;}#content{overflow: none;}header, footer{border-bottom: 1px solid lightgray; padding: 4px 8px; margin-bottom: 10px; background: rgb(37, 40, 43);}footer{position: fixed; left: 0; bottom: -14px; width: 100%;}h1{font-weight: bolder; font-size: x-large; margin: 4px; color: white;}h3{font-weight: lighter; font-size: medium; margin: 6px; color: darkgrey;}h4{font-weight: lighter; font-size: small; margin: 6px; color: lightgrey;}main{margin: 0px; width: 100%; height: 100%; overflow: hidden; overflow-y: auto; background: white;}#sms-list{overflow-y: auto;width: 100%; height: 100%;}#titleH, #row{display: block; border-bottom: 1px solid lightgrey;}#senderH, #msgH{display: inline-block; text-align: center; font-weight: bolder; vertical-align: middle; color: grey; height: 30px; line-height: 30px;}#msgH, #sms{padding-left: 8px;}#senderH, #unit{width: 15%; text-align: center; border-right: 1px solid whitesmoke;}#unit, #sms{display: inline-block;vertical-align: middle; font-family: 'Courier New', Courier, monospace; height: 30px; line-height: 30px; color: black}#sms{word-wrap: break-word;}#smsCtr{display: inline;}@media (max-width: 812px){#unit, #sms{height: auto; vertical-align: top;}#senderH span{display: none;}#senderH:after{content: 'Unit';}}</style></head><body> <div id='content'> <header> <h1>CONTACT</h1> <h3>MESSAGES SENDED FROM ONLINE LOCATIONS</h3> </header> <main> <div id='sms-list'> <div id='titleH'><div id='senderH'><span>LOCATION</span></div><div id='msgH'>MESSAGE</div></div></div></main> <footer> <h4><div id='smsCtr'></div>&nbsp;Messages received from Online Locations Units.</h4> </footer> </div><script>var smsCtr=0; var lastMsgReaded=0; var bFirstTime=true; if (sessionStorage.getItem('bFirstTime') !=null){bFirstTime=false; lastMsgReaded=sessionStorage.getItem('lastMsgReaded'); smsCtr=sessionStorage.getItem('smsCtr');}else{sessionStorage.setItem('bFistTime', false); sessionStorage.setItem('smsCtr', smsCtr);}document.getElementById('smsCtr').innerText=smsCtr; SendSMSToServer(); setInterval(SendSMSToServer, 7000); function SendSMSToServer(){var msgs=''; var aSMS=new Array(); var smsServer=''; if ((localStorage.getItem('sms') !=null) && (bFirstTime)){document.getElementById('sms-list').innerHTML=localStorage.getItem('sms'); console.log('Readed from LocalStorage.'); var smsDiv=document.getElementById('sms-list'); smsDiv.scrollTop=smsDiv.scrollHeight;}console.log('Reading from Device.'); fetch('get?lastMsg=' + lastMsgReaded + '&fistTime=' + (bFirstTime?1:0),{mode: 'no-cors'}) .then(response=> response.json()) .then(json=>{var msgs=\"\"; if (json.data.length > 0){console.log('Messages Readed. JSON Data on Page.'); for(i=0; i < json.data.length; i++){msgs +='<div id=\"row\"><div id=\"unit\">' + json.data[i]['txid'] + '</div><div id=\"sms\">' + json.data[i]['message'] + '</div></div>'; lastMsgReaded=json.data[i]['pqid']; aSMS.push(json.data[i]['message']); ++smsCtr; sessionStorage.setItem('smsCtr', smsCtr);}sessionStorage.setItem('lastMsgReaded', lastMsgReaded); var newMsgs=''; if (localStorage.getItem('sms')===null) newMsgs=msgs; else newMsgs=localStorage.getItem('sms') + msgs; console.log('Message Structure: '); console.log(newMsgs); localStorage.setItem('sms', newMsgs); document.getElementById('sms-list').innerHTML +=msgs; bFirstTime=false; var smsDiv=document.getElementById('sms-list'); smsDiv.scrollTop=ChatDiv.scrollHeight; console.log(\"SMS Count: \", smsCtr); document.getElementById('smsCtr').innerText=smsCtr;}}) .catch(function(error){console.log('Request failed', error);}); if (aSMS.length > 0){pushSMSs();}}</script></body></html>");
}

String delete_sms_cache() {
  return ("<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"> <title>Delete SMS Cache</title></head><body> <h1>Delete All SMS Cache from Browser</h1> <script>localStorage.setItem('sms',''); </script></body></html>");
}


String otherPage() {
  return ("<html><body><h1>The content you are looking for was not found.</h1></body></html>");
}

// BENYWHY
