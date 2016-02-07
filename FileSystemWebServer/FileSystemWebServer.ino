/*
 * Simple Web Server serving files from the Flash chip using SPIFFS
 * 
 * Copyright (c) 2016 Jeffrey Antony
 * 
 * Part of this example contains code copied from esp8266-arm-swd
 * https://github.com/scanlime/esp8266-arm-swd
 * Copyright (c) 2015 Micah Elizabeth Scott
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

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

// Edit these in the WIFI-DETAILS.ino file
extern const char *host, *ssid, *password;
 
ESP8266WebServer server(80);

String getContentType(String filename)
{
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js"))  return "application/javascript";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".gif")) return "image/gif";
    else if (filename.endsWith(".jpg")) return "image/jpeg";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".xml")) return "text/xml";
    else if (filename.endsWith(".pdf")) return "application/x-pdf";
    else if (filename.endsWith(".zip")) return "application/x-zip";
    else if (filename.endsWith(".gz"))  return "application/x-gzip";
    return "text/plain";
}

bool streamFileIfExists(String path)
{
    if (SPIFFS.exists(path)) {
        String contentType = getContentType(path);
        File f = SPIFFS.open(path, "r");
        server.sendHeader("Cache-Control", "max-age=36000");
        server.streamFile(f, contentType);
        f.close();
        return true;
    }
    return false;
}

void handleStaticFile()
{
    String path = server.uri();
    if (path.endsWith("/")) {
        path += "index";
    }

    if (!streamFileIfExists(path) &&
        !streamFileIfExists(path + ".html")) {
        server.send(404, "text/plain",
            "File not found, in ESP8266FS flash.\n\n"
            "Did you run Tools -> ESP8266 Sketch Data Upload?");
    }
}

void setup()
{
    pinMode(2, OUTPUT);     // Initialize the LED(GPIO2) pin as an output
    digitalWrite(2, HIGH);  //Switch Off  LED
    
    Serial.begin(115200);
    Serial.println("\n\n~ Starting up ~\n");

    SPIFFS.begin();
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
        String fileName = dir.fileName();
        Serial.printf("FS File: %s\n", fileName.c_str());
    }
    Serial.println();

    do {
        Serial.println("\nGetting the wifi going...");
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(ssid, password);
    } while (WiFi.waitForConnectResult() != WL_CONNECTED);

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.onNotFound(handleStaticFile);
    server.begin();

    MDNS.begin(host);
    MDNS.addService("http", "tcp", 80);

    Serial.printf("Server is running at http://%s.local/\n", host);

    digitalWrite(2, LOW); //Switch On LED
}

void loop()
{
    server.handleClient();
    delay(1);
}
