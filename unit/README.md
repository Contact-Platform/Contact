# CONTACT-UNIT

## What is CONTACT?
An open platform that brings together software and hardware to allow everyone to communicate up to eight mile radius.

## Why we create CONTACT?
During and after an emergency, communication with communities is critical.

Helping people at all levels to communicate empowers them to recognize important issues and find common grounds for action, and builds a sense of identity and participation in order to implement decisions.

Well-conceived and effectively delivered emergency messages can help ensure public safety, protect property, facilitate response efforts, elicit cooperation, instill public confidence, and help families reunite.

> Delivering effective emergency communications is an essential part of emergency management. We have learned through the experience of Hurricane Maria first that the people around you respond more quickly than the government. Second, although government assistance is available, coordination between people and agencies is needed, and communication is necessary for this.
[imagen unidades contact]


## How it's Work?
- Contact combines the use of an IoT device that includes a LoRa transmitter and an ESP32 microcontroller to provide communication access up to eight miles away.
[imagen de distancia]
[enlace a fotos]
- It's the same as using the Internet in a Hotspot. It is not necessary to download additional applications, you only need a device with WiFi and a Web Browser.
[una imagen]
[enlace a pantallas]
- Contact code allows each unit to extend the distance by replicating any message up to two more units, creating a simple MESH by using a lifetime for each data packet that has been sent.
[ejemplo del código]


## Features
### Communication Reach
- It is possible to reach *up to 8 miles* away in a communication. Within cities with buildings or structures allows a communication of up to two miles, even within structures. But it is possible to double or triple the distance when using the units with a simple MESH.
[imagen lista online communities]
- Is an alternative way of *taking communications trough hard places*. In most emergency incidents, mainly in natural disasters, geography or lack of access do not allow communication.

### Ease of Access
- The price is very affordable, the cost is $11.00 to produce a unit. But it can be less by producing more units in a bigger scale.
[imagen por dentro]

### Hardware Flexibility
- It is *Portable*, it's measure only 3.1 inches (8cm) x 2 inches (5cm). But we know that it can be smaller.
[imagenes o fotos]
- It uses *rechargeable batteries* (3.7v 3.6aH, 18650) that can extend its use up to *30 continuous hours*. It *can be recharged and used with any power source*, such as power banks for cell phones, small solar panels or the Eton Red Cross Charger.
[imagen conectado]

### Scalability
- It uses a *frequency free of licenses* for Industry, Science and Medicine (900-Mhz frequencies, ISM band) in the United States. But it is possible to select **other LoRa license-free frequencies for other Countries** through a configuration screen.
[setup]
- It can be *used as an open network platform to send any form data*. It has sending or receiving functions through specific RESTful technology.
[imagen formulario]
[enlace código]
- It **can be used as a Network or Internet Gateway** to send all received messages to another network, the Internet,  [Contact API]() or another platform such as Twilio.
[imagen]
- *Beacon and Geolocation Integration*. It allowing it to be used as a Beacon to locate people in other rescue situations. By using a powerful open-technology microcontroller (ESP32) you can extend the capacity of the device while maintaining low cost and performance. 
[imagen]

## CALL FOR CODE + CONTACT
We believe that everyone has creative ideas. We, in response to **Call for Code**, develop the **Contact** capabilities around the powerful tools of **IBM Cloud** to demonstrate that it is a powerful platform that can grow.
[enlace to dashboard]
[enlace to github Janiel]


That is a reason why we provide a complete *Open Communication Platform* that allows others to control the hardware and the data directly; the people can create custom interfaces and even send the data to other platforms.
[enlace to github Contact Unit]
[enlace a pantallas]

<Añadir información de Janiel de como trabaja>


[setup]: https://github.com/jdastas/contact-platform/setup.png "Contact Setup Screen"


[setup]: https://github.com/jdastas/contact-platform/setup.png "Contact Setup Screen"
[gateway]: https://github.com/jdastas/contact-platform/gateway.png "Internet Gateway"