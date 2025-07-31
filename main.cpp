#include "cJSON.h"
#include "WiFi.h"
#include <WebServer.h>

String wifi_ssid = "";
String wifi_password = "";

WebServer server(80)

void setup() {
    Serial.begin(115200);
    WiFi.begin(wifi_ssid, wifi_password);
    if (testWifi()) {
      Serial.println("WiFi connected OK");
      Serial.print("Local IP: ");
      Serial.println(WiFi.localIP());
      createWebServer();
      server.begin();
      Serial.println("Server started");
    }
    else {
      Serial.println("WiFi connected NG");
    }
      
  }

void createWebServer(){
    server.on("/", []() {
 
        content = "<!DOCTYPE HTML>\r\n<html>";
        content += "<p>";
        content += "<ol>";
        content += room_id;
        content += "</ol>";
        content += "</p><form method='get' action='setuid'><label>UID</label><input name='uid' length=64><input type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);
    });
}