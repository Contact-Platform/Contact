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

<a href="https://youtu.be/MLwHU5oP0mU" target="_blank">Click here see a video with a demo.<img src="images/contact-logo.png" 
alt="Contact Video" width="100" height="auto" border="10" /></a>

## Contact Architecture
The main functionality of Contact is the ability to communicate between different devices. Contact uses WiFi communication to receive the data that it wants to transmit and in turn so that the user can request the data. The Contact code allows each unit to work as a Hotspot and Web Server at the same time. Through the Web Server in Contact the user can access different functions, such as a chat for communication between the Contact units, configuration screens for the units and screens that allow reading data sent through the Contact units and sending that data to other networks or the Internet.

![Contact Detail Architecture](/Platform/images/contact-detail-architecture.png)


## Contact's Back-end - Code

### Communication and Instruction Process 

The device has two hardware components. A LoRa module that allows the transmission of data over long distances and an ESP32 microcontroller that allows the process of instructions, create a web server, accept Wi-Fi communications and other functions such as adding a GPS module.

In order to receive and send messages, the unit uses the LoRa module and the WiFi network in the ESP32 at the same time through asynchronous functions and the elaboration of queues. Each received message is stored in a queue until the contact web communication interface requests the information. It is an advantage, because the user can access the last 50 messages received to the unit immediately connect through the web browser.

#### [Hardware Code]

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
## Contact's Front-end - Code

### Application Interfaces

The main 




[Hardware Code]: https://github.com/Contact-Platform/Contact/blob/master/Platform/device/device.ino "Contact Main Code"

