#include <TinyXML.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid      = "ZyXEL_8975";
const char* password  = "LGT47GP7BP";
const char* host      = "http://luasforecasts.rpa.ie/xml/get.ashx?action=forecast&stop=phi&encrypt=false";

TinyXML xml;
uint8_t buffer[150]; // For XML decoding

void setup () {

  WiFi.begin(ssid, password);
  xml.init((uint8_t *)buffer, sizeof(buffer), &XML_callback);

  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..");
  }
  Serial.println("");
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient
    
    //Specify request destination
    http.begin(host);
    int httpCode = http.GET();  //Send the request

    if (httpCode > 0) { //Check the returning code
      xml.reset();
      String payload = http.getString();   //Get the request response payload
      for (int i = 0; i < payload.length(); i++) {
        xml.processChar(payload[i]);
      }
    } else {
      Serial.println("HTTP Get request - got nothing...");
    }

    http.end();   //Close connection

  }

  delay(5000);    //Send a request every 5 seconds

}

void XML_callback(uint8_t statusflags, char* tagName,
                  uint16_t tagNameLen, char* data, uint16_t dataLen) {

  char msg[50];
  bool newMsg = false;

  if (statusflags & STATUS_TAG_TEXT) {
    if (!strcasecmp(tagName, "/stopInfo/message")) {
      Serial.println("--------------------------------------");
      sprintf(msg, "Status: \n %s\n", data);
      newMsg = true;
    }
    if (!strcasecmp(tagName, "/stopInfo/direction")) {
      sprintf(msg, "%s\n", data);
      newMsg = true;
    }
  }

  if (statusflags & STATUS_ATTR_TEXT) {
    static int dueMins = 0U;
    if (!strcasecmp(tagName, "dueMins")) {
      if (!strcasecmp(data, "DUE")) {
        dueMins = -1;
      } else {
        dueMins = atoi(data);
      }
    }
    if (!strcasecmp(tagName, "destination")) {
      if (dueMins > 0) {
        sprintf(msg, "\tLuas to %s due in %d minutes\n", data, dueMins);
      } else {
        sprintf(msg, "\tLuas to %s is due NOW\n", data);
      }
      newMsg = true;
    }
  }

  if (newMsg == true) {
    Serial.print(msg);
  }

}
