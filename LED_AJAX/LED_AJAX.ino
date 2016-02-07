/*
 * Simple Web Server serving a webpage to toggle the LED on ESP12E
 * 
 * Copyright (c) 2016 Jeffrey Antony
 * 
 * Part of this example contains code copied from esp8266-arm-swd
 * https://github.com/scanlime/esp8266-arm-swd
 * Copyright (c) 2015 Micah Elizabeth Scott
 * 
 * Some parts of the code we copied from the CaptivePortal example 
 * of ESP8266 Arduino
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Following links were used for reference
 * http://babaawesam.com/2015/11/26/control-input-output-of-esp8266-with-ajax/
 * https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/examples/CaptivePortal/CaptivePortal.ino
 * https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/examples/FSBrowser/FSBrowser.ino
 */

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

//edit below to chnage wifi network name
String Wifi_Name = "LED Control";

bool streamFileIfExists(String path)
{
    if (SPIFFS.exists(path)) {
        String contentType = "text/html";
        File f = SPIFFS.open(path, "r");
        webServer.sendHeader("Cache-Control", "max-age=36000");
        webServer.streamFile(f, contentType);
        f.close();
        return true;
    }
    return false;
}

void toggleLED()
{
    int value;
    
    value = digitalRead(2);
    
    digitalWrite(2, !(value)); //toogle the led
    
    if(!(value)) //value == 0
    {
        webServer.send(200, "text/plain", "OFF");
    }
    else //value == 1
    {
        webServer.send(200, "text/plain", "ON");
    }

}

void responseHTML()
{
    String path = webServer.uri();
    path += "index"; //can we put index.html here ?
  
    if (!streamFileIfExists(path) &&
        !streamFileIfExists(path + ".html")) {
        webServer.send(404, "text/plain",
            "File not found, in ESP8266FS flash.\n\n"
            "Did you run Tools -> ESP8266 Sketch Data Upload?");
    }
}

void setup() {
    
  pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(2, HIGH);  //Switch Off  LED
    
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(Wifi_Name);

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  webServer.onNotFound(responseHTML);
  
  webServer.on("/toggle", HTTP_GET, toggleLED);
  
  webServer.begin();
  
  digitalWrite(2, LOW); //Switch On LED
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();  
} 
