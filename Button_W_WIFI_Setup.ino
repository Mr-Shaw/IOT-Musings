//Simple button that can be setup via Access Point and saved to the EEPROM
//button will log "SW" to publish topic defined during setupAP


#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
/************************* WiFi Access Point *********************************/

char emqusr[64] = "User";
char emqpwd[64] = "MyPass";
char emqpub[64] = "MyTopic";

#define MQTT_SERVER      "##MQ-Server_Address##"  // give static address
#define MQTT_PORT         1883                    //1883 standard port
#define MQTT_USERNAME (emqusr)
#define MQTT_PASSWORD (emqpwd)
//#define MQTT_TOPIC (emqpub)
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
/****************************** Feeds ***************************************/

Adafruit_MQTT_Publish Status = Adafruit_MQTT_Publish(&mqtt, emqpub);
/*************************** Sketch Code ************************************/


ESP8266WebServer server(80);

const char* ssid = "Button-MQTEST1";
const char* passphrase = "";
String st;
String content;
int statusCode;
const int buttonPin = D1;
int buttonState = 0;

void before() {

Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Reading EEPROM MQTT Pub");
  String emqpub = "";
  for (int i = 96; i < 160; ++i)
    {
      emqpub += char(EEPROM.read(i));
    }
  Serial.print("MQTT Pub: ");
  Serial.println(emqpub);
  Serial.println("Reading EEPROM MQTT Sub");
  String emqsub = "";
  for (int i = 160; i < 224; ++i)
    {
      emqsub += char(EEPROM.read(i));
    }
  Serial.print("MQTT Sub: ");
  Serial.println(emqsub);
  Serial.println("Reading EEPROM MQTT User");
  String emqusr = "";
  for (int i = 224; i < 288; ++i)
    {
      emqusr += char(EEPROM.read(i));
    }
  Serial.print("MQTT User: ");
  Serial.println(emqusr);
  Serial.println("Reading EEPROM MQTT Pass");
  String emqpwd = "";
  for (int i = 288; i < 352; ++i)
    {
      emqpwd += char(EEPROM.read(i));
    }
  Serial.print("MQTT Pass: ");
  Serial.println(emqpwd);


}

void setup() {
  // put your setup code here, to run once:


  pinMode(buttonPin, INPUT);


  Serial.println("Startup");
  // read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  String esid;
  for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = 32; i < 96; ++i)
    {
      epass += char(EEPROM.read(i));
    }
  Serial.print("PASS: ");
  Serial.println(epass);
  Serial.println("Reading EEPROM Host Name");
  String ehost = "";
  for (int i = 352; i < 414; ++i)
    {
      ehost += char(EEPROM.read(i));
    }
  Serial.print("Host Name: ");
  Serial.println(ehost);
  if ( esid.length() > 1 ) {


      WiFi.hostname(ehost.c_str());
      WiFi.begin(esid.c_str(), epass.c_str());
      if (testWifi()) {
        launchWeb(0);
        return;
      }
  }
  setupAP();

}


bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; }
    delay(500);
    Serial.print(WiFi.status());
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  delay(100);
  WiFi.softAP(ssid, passphrase, 6);
  Serial.println("softap");
  launchWeb(1);
  Serial.println("over");
}

void createWebServer(int webtype)
{
  if ( webtype == 1 ) {
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
        content += ipStr;
        content += "<p>";
        content += st;
        content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32></br><label>PASS: </label><input name='pass' length=64></br><label>Host Name: </label><input name='host_n' length=64></br><label>MQTT Pub: </label><input name='mq_pub' length=64></br><label>MQTT Sub: </label><input name='mq_sub' length=64></br><label>MQTT User: </label><input name='mq_usr' length=64></br><label>MQTT Pass: </label><input name='mq_pwd' length=64></br><input type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        String mqpub = server.arg("mq_pub");
        String mqsub = server.arg("mq_sub");
        String mqusr = server.arg("mq_usr");
        String mqpwd = server.arg("mq_pwd");
        String hostn = server.arg("host_n");
        if (qsid.length() > 0 && qpass.length() > 0) {
          Serial.println("clearing eeprom");
          for (int i = 0; i < 414; ++i) { EEPROM.write(i, 0); }
          Serial.println(qsid);
          Serial.println("");
          Serial.println(qpass);
          Serial.println("");
          Serial.println(mqpub);
          Serial.println("");
          Serial.println(mqsub);
          Serial.println("");
          Serial.println(mqusr);
          Serial.println("");
          Serial.println(mqpwd);
          Serial.println("");
          Serial.println(hostn);
          Serial.println("");

          Serial.println("writing eeprom ssid:");
          for (int i = 0; i < qsid.length(); ++i)
            {
              EEPROM.write(i, qsid[i]);
              Serial.print("Wrote: ");
              Serial.println(qsid[i]);
            }
          Serial.println("writing eeprom pass:");
          for (int i = 0; i < qpass.length(); ++i)
            {
              EEPROM.write(32+i, qpass[i]);
              Serial.print("Wrote: ");
              Serial.println(qpass[i]);
            }
          Serial.println("writing eeprom mq pub:");
          for (int i = 0; i < mqpub.length(); ++i)
            {
              EEPROM.write(96+i, mqpub[i]);
              Serial.print("Wrote: ");
              Serial.println(mqpub[i]);
            }
          Serial.println("writing eeprom mq sub:");
          for (int i = 0; i < mqsub.length(); ++i)
            {
              EEPROM.write(160+i, mqsub[i]);
              Serial.print("Wrote: ");
              Serial.println(mqsub[i]);
            }
          Serial.println("writing eeprom mq usr:");
          for (int i = 0; i < mqusr.length(); ++i)
            {
              EEPROM.write(224+i, mqusr[i]);
              Serial.print("Wrote: ");
              Serial.println(mqusr[i]);
            }
          Serial.println("writing eeprom mq pwd:");
          for (int i = 0; i < mqpwd.length(); ++i)
            {
              EEPROM.write(288+i, mqpwd[i]);
              Serial.print("Wrote: ");
              Serial.println(mqpwd[i]);
            }
          Serial.println("writing eeprom mq pwd:");
          for (int i = 0; i < hostn.length(); ++i)
            {
              EEPROM.write(352+i, hostn[i]);
              Serial.print("Wrote: ");
              Serial.println(hostn[i]);
            }
          EEPROM.commit();
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {
    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.send(200, "application/json", "{\"IP\":\"" + ipStr + "\"}");
    });
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      for (int i = 0; i < 414; ++i) { EEPROM.write(i, 0); }
      EEPROM.commit();
      WiFi.disconnect();
    });
  }
}

void loop() {
  server.handleClient();
  // put your main code here, to run repeatedly:


 MQTT_connect();

 buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {

                if (! Status.publish("SW")) {
                  Serial.println(F("Failed"));
                } else {
                  Serial.println(F("OK!"));
                }


  delay(1500);
  }

}


// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {
 int8_t ret;
 // Stop if already connected.
 if (mqtt.connected()) {
   return;
 }
 Serial.print("Connecting to MQTT... ");
 uint8_t retries = 3;
 while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("Retrying MQTT connection in 5 seconds...");
      mqtt.disconnect();
      delay(5000);  // wait 5 seconds
      retries--;
      if (retries == 0) {
        // basically die and wait for WDT to reset me
        while (1);
      }
 }
}
