# ESP8266SignalkClient
Minimalistic sketch to send delta messages from an ESP8266, via Wifi, to a SignalK UDP socket. 

![image](https://github.com/marcobergman/ESP8266SignalkClient/assets/17980560/0003051d-cd51-4d8d-bb3b-ff6d800f7304)

To prepare the Arduino IDE, goto File->Preferences and add http://arduino.esp8266.com/stable/package_esp8266com_index.json as an Additional Board Manager URL. Then goto Tools->Board->Boards Manager, search for ESP8266 and install. For my Wemos D1 Mini board, I chose Generic ESP8266 Module. Google your way around. 

To set up SignalK, add a Data Connection with data type 'SignalK', Signalk Source 'UDP', port '30330'. 

The analog input of the ESP8266 chip has a range of 0-1.0V; the Wemos D1 Mini I used has an on-board voltage divider giving it a 0-3.3V range. Do your own research. The chip's AD converter is 10 bits only; for better resolution hook up an AD1115.

To test this type of things, this code was used:

```
echo '{"updates":[{"meta":[{"path":"cell.voltage","value":{"units":"V"}}]}]}' | netcat -N -u 10.10.10.1 30330
echo '{"updates":[{"values":[{"path":"cell.voltage","value":"87", "meta": {"units": "V", "Description": "cell voltage"}}]}]}' | netcat -N -u 10.10.10.1 30330
```
      
