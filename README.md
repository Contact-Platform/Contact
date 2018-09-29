# CONTACT

## What is CONTACT?
An open platform that brings together software and hardware to allow everyone to communicate up to eight mile radius.

![How does Contact work](/Platform/images/contact-how-work-no-title.png)

## Why did we create CONTACT?
During and after an emergency the communication communities is critical. Helping people at all levels to communicate empowers them to recognize important issues and find common grounds for action, and builds a sense of identity and participation in order to implement decisions.

Well-conceived and effectively delivered emergency messages can help ensure public safety, protect property, facilitate response efforts, elicit cooperation, instill public confidence, and help families reunite.

> Delivering effective emergency communications is an essential part of emergency management. We have learned through the experience of Hurricane Maria first that the people around you respond more quickly than the government. Second, although government assistance is available, coordination between people and agencies is needed, and communication is necessary for this.

In emergency situations it is important to know the needs and emotions of all involved, but process all the field data its practically imposible.  For that reason, our solution, it is also designed to help you understand what your users are saying by usign an Artificially Inteligence system. Our solution can receives messages and analyzes them for sentiment,  emotional tone and identify the keywords in the conversations. Storing all the data in a database and visualize it through charts, graphs and maps can help you make appropriate and informed decisions on emergency situations and logistics understanding what people need and their emotion.

## Contact as a Project 
![Solution Roadmap](/Platform/images/solution-roadmap.png)
*Solution Roadmap*

## How does it Work?
- Contact combines the use of an IoT device that includes a LoRa transmitter and an ESP32 microcontroller to provide communication access up to eight miles away.

![Contact Detail Architecture](/Platform/images/contact-detail-architecture.png)

- It's the same as using the Internet in a Hotspot. It is not necessary to download additional applications, you only need a device with WiFi and a Web Browser.

![Selecting Locations to communicate](/Platform/images/sending-msg.png)

<a href="https://youtu.be/MLwHU5oP0mU" target="_blank">
<p>Click here to see a demo video.</p></a>

- Contact code allows each unit to extend the distance by replicating any message up to two more units, creating a simple MESH by using a lifetime for each data packet that has been sent.

![Contact](/Platform/images/simple-mesh.png)

## Features
### Communication Reach
- It is possible to reach *up to 8 miles* away in a communication. Within cities with buildings or structures allows a communication of up to two miles, even within structures. But it is possible to double or triple the distance when using the units with a simple MESH.

![Contact Reach](/Platform/images/contact-reach.png)

![Selecting Locations to communicate](/Platform/images/msg-units-selection.png)
*Online Locations Units*

- Is an alternative way of *taking communications trough hard places*. In most emergency incidents, mainly in natural disasters, geography or lack of access do not allow communication (forwarding the messages).

### Ease of Use
- To use contact it is not necessary to download any application. The unit works just like a hostpot, you use a WiFi connection and then you use the tools through any web browser.

![Main Screen](/Platform/images/main-interface.png)
*All interactions are through Web interfaces.*

![Sending Messages](/Platform/images/main-interface-use.png)

### Ease of Access
- The price is very affordable, the cost is $11.00 to produce a unit. But it can be less by producing more units in a bigger scale.

### Hardware Flexibility
- It is *Portable*, it's measure only 3.1 inches (8cm) x 2 inches (5cm). But we know that it can be smaller.

![Unit](/Platform/images/units-cell.png)

- It uses *rechargeable batteries* (3.7v 3.6aH, 18650) that can extend its use up to *30 continuous hours*. It *can be recharged and used with any usb micro "b" power source*, such as power banks for cell phones, small solar panels or the Eton Red Cross Charger.

![Contact Hardware](/Platform/images/hardware.png)
*Inside a Contact Box*

### Scalability
- It uses a *frequency free of licenses* for Industry, Science and Medicine (900-Mhz frequencies, ISM band) in the United States. But it is possible to select **other LoRa license-free frequencies for other Countries** through a configuration screen.

![Contact Setup Screen](/Platform/images/setup.png)

- It can be *used as an open network platform to send any form data*. It has sending or receiving functions through specific RESTful technology.

![Data Form](/Platform/images/data-form.png)

**Customized Data Form Code**

```javascript
  function sendData() {
            var jsonData = jsonForm(document.getElementById('form'));
            SendToServer(jsonData, 'GATEWAY'); /* Your Receiving Contact Unit */
            document.getElementById('datos').innerHTML += 
                '<pre>'  + jsonData + '</pre><br>';            
        }

        function jsonForm(form) {  
             
            var jsonString = "{data[{";
            if (typeof form == 'object' && form.nodeName.toLowerCase() == "form") {  
                var fields = form.getElementsByTagName("input");  
                for(var i=0;i<fields.length;i++){ 
                    var n = fields[i].getAttribute('name'); 
                    var v = document.getElementById(n).value;

                    jsonString += "'" + n + "':'" + v + "',";
                   
                }  
                jsonString = jsonString.substr(0, jsonString.length - 1);
                jsonString += "]}";
            }  
            return jsonString;  
        }      
        function SendToServer(msg, unit) {         
            fetch('http://1.2.3.4/put?message=' + msg + '&unit=' + unit + '&usr=JSON'
                , {mode: 'no-cors'})
                .then(response => response.json())
                .then(json => console.log(json))
                .catch(function(error) {  
                    console.log('Request failed', error)  
                });
        }       
```
[Customized Data Form]

- It **can be used as a Network or Internet Gateway** to send all received messages to another network, the Internet,  [Contact API] or another platform such as Twilio.

![Gateway Confirmation Screen](/Platform/images/gateway.png)

- *Beacon and Geolocation Integration*. It allowing it to be used as a Beacon to locate people in other rescue situations. By using a powerful open-technology microcontroller (ESP32) you can extend the capacity of the device while maintaining low cost and performance. 

![Beacon Screen](/Platform/images/beacon-option.png)

![Beacon On](/Platform/images/beacon-active.png)

## CALL FOR CODE + CONTACT
We believe that everyone has creative ideas. We, in response to [Call for Code], develop the **Contact** capabilities around the powerful tools of **IBM Cloud** to demonstrate that it is a powerful platform that can grow.

![Contac Api Architecture](/API/images/architecture.png)

- [Contact API - Demo]
- [Contact API - Github]

![Contact Api Running](/Platform/images/contact-api-01.png)

The Contact API is **designed to help _you_ understand what your users are saying.** The API receives messages and analyzes them for sentiment and emotional tone. We store all the analysis data in a database and visualize it through charts, graphs and maps to help you make appropriate and informed decisions. 

Visit the [Contact API] to see it up and running.

That is a reason why we provide a complete *Open Communication Platform* that allows others to control the hardware and the data directly; the people can create custom interfaces and even send the data to other platforms.
- [Platform Code on Github]

[Call for Code]: (http://callforcode.org) "Call for Code"

[inside]: https://github.com/jdastas/contact-platform/Platform/images/unit-inside.jpg "Unit Inside"
[setup]: https://github.com/jdastas/contact-platform/Platform/images/setup.png "Contact Setup Screen"
[gateway]: https://github.com/jdastas/contact-platform/Platform/images/gateway.png "Internet Gateway"
[Customized Data Form]: https://github.com/jdastas/contact-platform/Platform/interfaces/data-form.html "Data Form Demo"

[Contact API]: https://contact-app.mybluemix.net/ "Contact API"

[Contact API - Demo]: https://contact-app.mybluemix.net/ "Contact API"
[Contact API - Github]: https://github.com/javierdastas/Contact/tree/master/API "Contact API Code"


[Platform Code on Github]: https://github.com/Contact-Platform/Contact/tree/master/Platform "Contact Unit Code"
