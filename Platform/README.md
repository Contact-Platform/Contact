# Contact's Unit

Contact combines the use of an IoT device that includes a LoRa transmitter and an ESP32 microcontroller to provide communication access up to eight miles away.

To be able to use the capacity of this device to the maximum, an elaborated coding is necessary.

This is why the integration of multiple programming languages ​​such as C++, HTML, Javascript and CSS is necessary. In addition to the use of tools that facilitate the integration of all these.

## How Contact's elements work in between?

There are technologies defined for particular developments. But the greatest benefit of technology is in the development of Open Information Systems. Contact was developed thinking about solving perhaps the biggest problem in a natural disaster, the lack of services that allow communication. 
Contact offers an embedded application to allow communication. But contact is not just an application and a communication device, it is a platform that allows increasing the support of communications between people in need and the rest of the planet. Contact allows to develop and integrate other interfaces for data capture in a simple way using technologies similar to RESTful or HTML injection

![Contact Platform](/Platform/images/contact-architecture.png)


The Contact code is divided into two layers. 
- The first layer, the main core of the **Contact Platform**, is used to give instructions to the IoT device and is developed in C++. It is our **Contact Platform Back-end**.
- The second layer, the **Contact Platform Front-end**, allows all the communication functions between the devices and the interaction with the device by the user. This second layer is encoded in HTML, Javascript and CSS.

All layers work as state of the art watch's parts to get the most powerfull and simple communication device.

<a href="https://youtu.be/MLwHU5oP0mU" target="_blank"><img src="images/contact-logo.png" 
alt="Contact Video" width="100" height="auto" border="10" /><p>&nbsp;Click here to see a demo video.</p></a>

## Contact Architecture
The main functionality of Contact is the ability to communicate between different devices. Contact uses WiFi communication to receive the data that it wants to transmit and in turn so that the user can request the data. The Contact code allows each unit to work as a Hotspot and Web Server at the same time. Through the Web Server in Contact the user can access different functions, such as a chat for communication between the Contact units, configuration screens for the units and screens that allow reading data sent through the Contact units and sending that data to other networks or the Internet.

![Contact Detail Architecture](/Platform/images/contact-how-work.png)

The contact functions are due to the understanding of many disciplines in technology related to hardware, software and communications. The following image presents in greater detail how the elements integrated in contact work.

![Contact Detail Architecture](/Platform/images/contact-detail-architecture.png)

The orchestration of all this technology allows it to be simple in its use and implementation, besides offering the opportunity to grow as a platform.


## Contact's Back-end - Code

### Communication and Instruction Process 

The device has two hardware components. A LoRa module that allows the transmission of data over long distances and an ESP32 microcontroller that allows the process of instructions, create a web server, accept Wi-Fi communications and other functions such as adding a GPS module.

In order to receive and send messages, the unit uses the LoRa module and the WiFi network in the ESP32 at the same time through asynchronous functions and the elaboration of queues. Each received message is stored in a queue until the contact web communication interface requests the information. It is an advantage, because the user can access the last 50 messages received to the unit immediately connect through the web browser.

#### [Hardware Code]

The code is developed in C ++ using a structure based on Arduino microcontrollers. When using a programming structure based on Arduino we obtain a device that from its bases is an Open Platform.

```cpp

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

```
#### The Web Server Process on ESP32

Through the creation of a Web Server in the Contact unit, simple interfaces can be encoded using HTML or other programming languages.

```cpp
server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Read Received Messages from STACK
    int paramsNmbr = request->params();
    if ((paramsNmbr > 0) && (rx_MSG_CTR_Q > 0)){
       AsyncWebParameter* p1 = request->getParam(0);
       AsyncWebParameter* p2 = request->getParam(1);

       int nLastReaded = String(p1->value()).toInt();
       int bFirstTime = String(p2->value()).toInt();
       );
       if (bFirstTime == 0)
           if (nLastReaded == (rx_MSG_CTR_Q - 1))
               return;
       
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

       request->send(200, "text/json",  jsonData);
    } // end if
  });
```

#### Forward Messages to Other Units - Our Simple Mesh
Another very important option is the ability to forward messages to other units. For this, each package is created with a specific structure using the JSON technology and it is checked if the message belongs to the unit or if it is necessary to send the package again.

```cpp
    String content = incoming; 
    Serial.println(content);
      
    // Allocate JsonBuffer
    // Use arduinojson.org/assistant to compute the capacity.
    const size_t capacity = JSON_OBJECT_SIZE(8) + 600; 
    DynamicJsonBuffer jsonBuffer; //(capacity); 
    /* Parse JSON object */
    JsonObject& root = jsonBuffer.parseObject(content);
    if (!root.success()) {
    return;
    }

    root.prettyPrintTo(Serial);

    if (strcmp(root["txid"].as<char*>(), hostName) == 0) {
        return;
    }

    char* all = "ALL";

    // If Unit is a Gateway, Receive Any Message
    if (CONTACTvars.isGateway == 1)     
        root["rxid"] =  "ALL"; 
    // If Receiver is other Unit foward the message
    if ((strcmp(root["rxid"].as<char*>(), all) == 0) && ((root["ttl"].as<int>() - 1) > 0)
        && (strcmp(root["rxid"].as<char*>(), hostName) == 0))
    { // Simple Mesh Network
        Serial.println(Serial.println(root["rxid"].as<char*>()));
        root["ttl"] = String(root["ttl"].as<int>() - 1);
        root.printTo(MESH_MSG);
        sendLoRaMsgToOther = true;
        return;
    }
         
```
[Click here to view the complete Code](
https://github.com/Contact-Platform/Contact/blob/master/Platform/device/device.ino "Unit Code")

## Contact's Front-end - Embedded Application Code
Is important remember that the Contact's Front-end interfaces are web pages, those codes are integrated with the Contact Back-end code. For that reason, every time you modify the Font-end code *you need to merge the code with the Back-end code* to get the results on the Contact's user web interface.

### Main Application Interface

The main screen of the web application integrated in the Contact web server allows users to send messages to the other units. The code also asks the unit, at intervals of time, for the information that the unit has received. Another function that allows is to send the messages written by the user. Also, to send messages requesting help, in time intervals. This happens when the beacon option is active. It also sends certain time intervals information related to the unit, this allows the other units to know what units are available to make contact. 

[Click here to view the Code](https://github.com/Contact-Platform/Contact/blob/master/Platform/interfaces/main/main-screen.html "Main Screen Code")

The code was written in HTML, Javascript and CSS.

![Selecting Locations to communicate](/Platform/images/msg-units-selection.png)
*Selecting Online Locations to Send Messages*

![Main Screen](/Platform/images/main-interface.png)
*Main Options and Interaction*

![Sending Messages](/Platform/images/main-interface-use.png)

*Sending a Message to selected Online Locations*

#### Beacon Alert
It allowing it to be used as a Beacon to locate people in other rescue situations. 
```cpp
// GPS UNIT SETTINGS FOR BEITIAN BN-180 GPS Module
#include <TinyGPS++.h>

HardwareSerial GPSSerial(2);

#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;
String strGeo = "";

```
```cpp
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
```

#### <a href='https://youtu.be/3B9AbOJu10c' target='_blank' alt='Beacon Demo'>Click here to see a demo video using the Beacon option.</a>

![Beacon Screen](/Platform/images/beacon-option.png)


### Setup Interface

This interface allows the user to specify options related to the unit and functions to connect the unit as a gateway to other platforms or the Internet. All data sent through this interface is recorded in the EEPROM or Flash Memory of the Unit.

```cpp

class FLASHvariables {
public:
  char unitName[14];
  char unitDesc[30];

  uint8_t unitFreq; // Unit Area Frequency
  // Default is 0, ISM Band will be selected.

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

```

The interface is divided into two parts to specify data. The first allows you to specify the name with which you want to identify or make known in the communications to the unit, in addition to the frequency of communications to be used according to the region or the country in which you live. The unit will always start working in the frequency of 900Mhz (ISM band), for this reason it is not necessary to be specific if you want to maintain that frequency.

The second part of the data allows you to specify that the unit connect to another network to read all the data sent to any unit and send them to another network, the Internet or another platform.

![Contact Setup Screen](/Platform/images/setup.png)

The code was written in HTML, Javascript and CSS.

[Click here to view the Code](https://github.com/Contact-Platform/Contact/blob/master/Platform/setup/unit-setting.html "Setup Screen Code")

### Gateway Interface

This interface allows the unit that has been configured as a Gateway to receive all the data sent between the Units in their range and send these to an external platform in a network or through the Internet. The code will read in time intervals the data received in the Gateway Unit and send them to the address indicated in the Setup screen.

![Gateway Confirmation Screen](/Platform/images/gateway.png)

The code was written in HTML, Javascript and CSS.

[Click here to view the Code](https://github.com/Contact-Platform/Contact/blob/master/Platform/interfaces/gateway/pushsms.html "Setup Screen Code")

### Data Form - Customized Interface

This code is an example of the capabilities of the Contact Platform. The example demonstrates how an external interface can be developed to send data through the Contact Platform.

![Other Data Form Screen](/Platform/images/data-form.png)


It is important that the code indicates to which unit you want to send the data, which can be a Contact Gateway.

```javascript
    function sendData() {
        var jsonData = jsonForm(document.getElementById('form'));
        SendToServer(jsonData, '<your-unit-name>'); 
        document.getElementById('datos').innerHTML += 
            '<pre>'  + jsonData + '</pre><br>';            
    }
```

The code was written in HTML, Javascript and CSS. 

[Click here to view the Code](https://github.com/Contact-Platform/Contact/blob/master/Platform/interfaces/other-data-example/form-data.html "Data Form Code")

[Hardware Code]: https://github.com/Contact-Platform/Contact/blob/master/Platform/device/device.ino "Contact Main Code"

