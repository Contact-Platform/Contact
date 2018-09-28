# Contact's Unit

Contact combines the use of an IoT device that includes a LoRa transmitter and an ESP32 microcontroller to provide communication access up to eight miles away.

To be able to use the capacity of this device to the maximum, an elaborated coding is necessary.

This is why the integration of multiple programming languages ​​such as C++, HTML, Javascript and CSS is necessary. In addition to the use of tools that facilitate the integration of all these.

The Contact code is divided into two layers. 
- The first layer is used to give instructions to the IoT device and is developed in C++. 
- The second layer allows all the communication functions between the devices and the interaction with the device by the user. This second layer is encoded in HTML, Javascript and CSS.

## Contact's Code
### Communication and Instruction Process 

The device has two hardware components. A LoRa module that allows the transmission of data over long distances and an ESP32 microcontroller that allows the process of instructions, create a web server, accept Wi-Fi communications and other functions such as adding a GPS module.

In order to receive and send messages, the unit uses the LoRa module and the WiFi network in the ESP32 at the same time through asynchronous functions and the elaboration of queues or stacks. Each received message is stored in a queue until the contact web communication interface requests the information. It is an advantage, because the user can access the last 50 messages received to the unit immediately connect through the web browser.

##### [Hardware Code]

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



[Hardware Code]: https://github.com/Contact-Platform/Contact/blob/master/Platform/device/device.ino "Contact Main Code"

