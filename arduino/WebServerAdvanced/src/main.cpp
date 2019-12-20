/* commands
pio run -e <env> -t upload
pio run -e <env> -t uploadfs
pio device list
pio device list --mdns --logical
pio device monitor --baud 115200

pio account login


python .\decoder.py -e .\.pio\build\esp12e\firmware.elf -s .\stack.txt 

*/
#include <config_with_dh1.h>
#if defined(SONOFF)
#undef HAS_DHT 
#endif
#define APP_VERSION "tags-2019.12.20"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <WiFiManager.h>
#include <ESP8266mDNS.h>
//#include <ESP8266WebServer.h>   // Include the WebServer library
#include "utils.hpp"
#include "EEPOMData.hpp"
#if HAS_DHT
#include <iostream>
#include <dht.h>
#endif

#ifdef MQTT_SUPPORT
#include <PubSubClient.h>
#endif
#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266OTA.h>

#define DEBUG_FAUXMO_VERBOSE_TCP 1
#define DEBUG_FAUXMO_VERBOSE_UDP 1
#include "fauxmoESP.h"
fauxmoESP fauxmo;


#define VIRTUAL_DEVICE "green boot"
#ifdef SONOFF
#define VIRTUAL_DEVICE_PIN0 12 // Relay
#else
#define VIRTUAL_DEVICE_PIN0 D1
#endif
bool  VIRTUAL_DEVICE0_STATE  = false;
//ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
WiFiManager wifiManager;

int port = 8000;
ESP8266WebServer server(port);    // Create a webserver object that listens for HTTP request on port 80
WiFiClient espClient;
#ifdef MQTT_SUPPORT
PubSubClient client(espClient);
#endif

#define DEBUG_1(arg)           Serial.print(__LINE__); Serial.print(": "); Serial.println(arg)
#define DEBUG_2(arg0, arg1)    Serial.print(__LINE__); Serial.print(": "); Serial.print(arg0);Serial.println(arg1)
#define DEBUG_3(arg0, arg1, arg2)    Serial.print(__LINE__); Serial.print(": "); Serial.print(arg0);Serial.print(arg1);Serial.println(arg2)
#define DEBUG_4(arg0, arg1, arg2, arg3)    Serial.print(__LINE__); Serial.print(": "); Serial.print(arg0);Serial.print(arg1);Serial.print(arg2);Serial.println(arg3)
#define DEBUG_5(arg0, arg1, arg2, arg3, arg4)    Serial.print(__LINE__); Serial.print(": "); Serial.print(arg0);Serial.print(arg1);Serial.print(arg2);Serial.print(arg3);Serial.print(arg4)
#define DEBUG_6(arg0, arg1, arg2, arg3, arg4, arg5)    Serial.print(__LINE__); Serial.print(": "); Serial.print(arg0);Serial.print(arg1);Serial.print(arg2);Serial.print(arg3);Serial.print(arg4);Serial.print(arg5)


const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";


ESP8266OTA otaUpdater;


void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();


void handleReset();
void handleAdmin();
void handleSystem();
void handleConfig();
void handleWifi();
//void handleWifi_old();
void handleSaveWifi();
bool handleFileRead(String path);
void handleStatus();
void handleAdmin_json();
void handleSubmitAdmin_json();
void handleConfigEdit();
void handleConfigEditPost();

EEPROMData eeprom(1);

#if HAS_DHT
#define DHT11_PIN D0
dht DHT; //(D0, DHT22, 15);
#endif


void set_state(bool state) {
  
   digitalWrite(VIRTUAL_DEVICE_PIN0,  state? HIGH : LOW);
   // opposite for LED
   digitalWrite(13,  state? LOW : HIGH);
   VIRTUAL_DEVICE0_STATE = state;

}


//MyPrintHandler handler; 
String MakeMine(const char *Template)
{
  uint16_t uChipId = ESP.getChipId();
  String Result = String(Template) + String(uChipId, HEX);
  return Result;
}

//std::map<String, String> remap;
 DynamicJsonDocument  remap(2048);
void setup(void){
  //ESP.reset();
  EEPROM.begin(sizeof(eeprom));

  delay(100);
  eeprom.unmarshall();
  if(eeprom.getName().isEmpty()) {
    eeprom.setName(MakeMine(VIRTUAL_DEVICE));
    eeprom.marshall();
    
  }
  //dht.begin();
  

  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  Serial.print("MQ Server:");Serial.println(eeprom.getMQServer().c_str());
  Serial.print("SSL:");Serial.println(eeprom.getMQSSL());
  Serial.println("Connecting ...");


  //WiFi.disconnect(true);
  wifiManager.autoConnect(MakeMine("TagsWiFI").c_str()); // will setup an ad-hoc network if required.
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
  WiFi.printDiag(Serial);
  delay(10);
  Serial.print("NAME:");Serial.println(MakeMine("").c_str());

  String name =  eeprom.getName();
  name.replace(" ", "");
  
  if (MDNS.begin(name.c_str(), WiFi.localIP())) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
     MDNSResponder::hMDNSService handle = MDNS.addService(name.c_str(), "http", "tcp", port);
     if(handle) {
      MDNS.addServiceTxt(handle, "path", update_path);
      MDNS.addServiceTxt(handle, "path", "index.html");
    }
    
    handle = MDNS.addService(name.c_str(), "api", "tcp", 80);
     if(handle) {      
      MDNS.addServiceTxt(handle, "path", "/api/lights");
    }

  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.printf("HTTPSUpdateServer ready!\nOpen http://%s.local%d:%s in "\
                "your browser and login with username '%s' and password "\
                "'%s'\n", name.c_str(), port, update_path, update_username, update_password);

  
  otaUpdater.setUpdaterUi("Firmware Upload",(String("OTA Firmware Upload ") + eeprom.getName().c_str()).c_str(),"Build : 0.01","Branch : master","Device info : ESp8266","Version:");
  otaUpdater.setup(&server, update_path, update_username, update_password);

  //server.on("/index.html", handleRoot); 
  //server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  //server.on("/css/app.72c36a24.css", handleCss_1); 
  

  //server.on("/submit_config", HTTP_POST, handleConfig);
  server.on("/submit_admin", HTTP_POST, handleAdmin);
  server.on("/admin_submit_json", HTTP_POST, handleSubmitAdmin_json);
  server.on("/config_edit_post_json", HTTP_POST, handleConfigEditPost);
  

  server.on(String(F("/system")), handleSystem);
  server.on(String(F("/admin")), handleAdmin_json);
  
  
  server.on(String(F("/reset")), handleReset);
  //server.on(String(F("/wifi_old")).c_str(), handleWifi_old );
  server.on(String(F("/wifi")).c_str(), handleWifi );
  server.on(String(F("/wifisave")).c_str(), handleSaveWifi );
  server.on(String(F("/mqtt_config")).c_str(), handleStatus );
  server.on(String(F("/status")).c_str(), handleStatus );
  server.on(String(F("/config_edit")).c_str(), handleConfigEdit );

  

  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");

#ifdef MQTT_SUPPORT
  if(!eeprom.getMQServer().empty()&& eeprom.getMQPort()>0 && eeprom.getMQTTStatus()) {
      Serial.print("Connection ");Serial.print(eeprom.getMQServer().c_str());Serial.print(":");Serial.println(eeprom.getMQPort());
      client.setServer(eeprom.getMQServer().c_str(), eeprom.getMQPort());
  }
#endif  
  SPIFFS.begin(); 
  File fp = SPIFFS.open("/map.json", "r");
  String json = "";
  char buffer[512];
  if(fp) {
    while(fp.available()) {
      int b = fp.readBytes(buffer, sizeof(buffer)-1);
      if(b>0) {
        buffer[b] = 0;
        json += buffer;
      }
    }
    fp.close();
   
    Serial.printf("rema: %s\n", json.c_str());
    deserializeJson(remap,  json);
  }
  
  fauxmo.createServer(true); // not needed, this is the default value
  fauxmo.setPort(80); // This is required for gen3 devices

    // You have to call enable(true) once you have a WiFi connection
    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo.enable(true);
    

    // You can use different ways to invoke alexa to modify the devices state:
    // "Alexa, turn yellow lamp on"
    // "Alexa, turn on yellow lamp
    // "Alexa, set yellow lamp to fifty" (50 means 50% of brightness, note, this example does not use this functionality)

    // Add virtual devices
    fauxmo.addDevice(eeprom.getName().c_str());
    pinMode(VIRTUAL_DEVICE_PIN0, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    set_state(false); // known state.
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        
        // Callback when a command from Alexa is received. 
        // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
        // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
        // Just remember not to delay too much here, this is a callback, exit as soon as possible.
        // If you have to do something more involved here set a flag and process it in your main loop.
        
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
        // Otherwise comparing the device_name is safer.

        if (strcmp(device_name, eeprom.getName().c_str())==0) {
           // DEBUG_5("Got a request for", device_name, "State:", state, " Value", value);
            set_state(state);          
        } 

    });

}


int t0=0;

#if  HAS_DHT

float humidity = 0;
float tempreature = 0;
float c_to_f(float c) {
  return (c * 9/5.0) + 32;
}

void read_dh11() {
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:  
		//Serial.print("OK,\t"); 
    humidity = DHT.humidity;
    tempreature = c_to_f(DHT.temperature);
		break;
    case DHTLIB_ERROR_CHECKSUM: 
    DEBUG_1("Checksum error, reading DH11"); 
		break;
    case DHTLIB_ERROR_TIMEOUT: 
		DEBUG_1("Time out error, reading DH11"); 
		break;
    default: 
	DEBUG_1("Unkown error, reading DH11"); 
		break;
  }
}
#endif
String name = MakeMine("TAGS");
int mt0=-10000000;
int wt0=-10000000;
int timeout = 10; // seconds
#ifdef MQTT_SUPPORT
void mqtt_reconnect(PubSubClient client, const String& user, const String& password) {
  // Loop until we're reconnected
 if(!client.connected()) {
    int t1 = millis();
    int elasped = t1-mt0;
    if(elasped < timeout* 1000) {
      return; // will attemp later.
    }
    DEBUG_1("Attempting MQTT connection ");
  
    Serial.print(user);Serial.print("!");Serial.print(password);Serial.print("@");Serial.print(eeprom.getMQServer().c_str());Serial.print(":");Serial.println(eeprom.getMQPort());
    Serial.print("  ");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
      
    if (client.connect(name.c_str(), user.c_str(), password.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.print(" try again in ");Serial.print(timeout); Serial.println(" seconds");
      // Wait 5 seconds before retrying
      mt0=t1;
     // delay(5000);
    }
  }
}
#endif
void loop(void){
  fauxmo.handle();
  server.handleClient();                    // Listen for HTTP requests from clients
  MDNS.update();

  int t1=millis();
  int elasped = t1-t0;
  if(elasped >5000) {
    t0 = t1;
    //Serial.println("in loop");
#if  HAS_DHT
    if(eeprom.getDHTStatus()) {
        read_dh11();
    }
#endif
    
     
  #ifdef MQTT_SUPPORT
    if( eeprom.getMQTTStatus() && eeprom.getDHTStatus() ) {
        int elasped = t1-wt0;
        //Serial.printf("Checking Status %s Elasped %d\n", eeprom.getMQTTStatus()?"ON":"OFF", elasped);
        if(elasped >60000*5) {
          if(!eeprom.getMQServer().empty()&& eeprom.getMQPort()>0) {
            DEBUG_1("Checking  MQTT");
              //client.setServer(eeprom.getMQServer().c_str(), eeprom.getMQPort());
          
              if(!client.connected()){
                mqtt_reconnect(client, String(eeprom.getMQUser().c_str()), String(eeprom.getMQPassword().c_str())  );
              }

              if(client.connected()){
                //client.publish(topic_humidity, String(h.get(),0).c_str(), true);
                //client.publish(topic_temp, String(t.get(),0).c_str(), true);
                Serial.println("Publising to MPTT.");   
                
                Serial.println("Publising to MPTT.");   
                if(  eeprom.getTempTopic().length()<64) {
                  Serial.print("temp ");Serial.print(eeprom.getTempTopic().c_str()); Serial.print(" "); Serial.println(String(tempreature, 3));
                  client.publish(eeprom.getTempTopic().c_str(), String(tempreature,0).c_str(), true);
                }
                if(  eeprom.getHumidityTopic().length()<64) {
                  Serial.print("humidity ");Serial.print(eeprom.getHumidityTopic().c_str()); Serial.print(" "); Serial.println(String(humidity, 3));
                  client.publish(eeprom.getHumidityTopic().c_str(), String(humidity,0).c_str(), true);
                }

                wt0 = t1;  
               // client.disconnect();
              }
          }
        }
    }

  #endif
   }
  //delay(10);
}

#ifdef UNUSED
void handleRoot() {                          // When URI / is requested, send a web page with a button to toggle the LED
  //server.send(200, "text/html", "<form action=\"/login\" method=\"POST\"><input type=\"text\" name=\"username\" placeholder=\"Username\"></br><input type=\"password\" name=\"password\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Login\"></form><p>Try 'John Doe' and 'password123' ...</p>");
  String str(index_dothtml);
  Serial.print(__LINE__); Serial.print(":check Response bytes:");Serial.println(String(str.length()));
  String ip = utils::tostr(WiFi.localIP()) + String("/") +  utils::tostr(WiFi.subnetMask()) + " gateway: " +  utils::tostr(WiFi.gatewayIP());
  Serial.print(__LINE__); Serial.print(":check Response bytes:");Serial.println(String(str.length()));


  str = utils::replace(str, "{ipaddress}", utils::tostr(WiFi.localIP()) );
  //str = utils::replace(str, "{subnet}", utils::tostr(WiFi.subnetMask()) );
  str = utils::replace(str, "{gateway}", utils::tostr(WiFi.gatewayIP()) );
  str = utils::replace(str, "{ssid}", WiFi.SSID());
  str = utils::replace(str, "{mac}", WiFi.macAddress() );
  str = utils::replace(str, "{protocol}", (eeprom.getMQSSL()?"ssl":"tcp"));
  // check..
  
  str = utils::replace(str, "{mqtt_status}", eeprom.getMQTTStatus()?"checked":"");
  str = utils::replace(str, "{server}", eeprom.getMQServer().c_str());
  str = utils::replace(str, "{port}", String(eeprom.getMQPort()).c_str());
  str = utils::replace(str, "{username}", eeprom.getMQUser().c_str());
  str = utils::replace(str, "{password}", eeprom.getMQPassword().c_str());
  str = utils::replace(str, "{ssl}", eeprom.getMQSSL()?"checked":"");

  str = utils::replace(str, "{temp_topic}", eeprom.getTempTopic().c_str());
  str = utils::replace(str, "{humidity_topic}", eeprom.getHumidityTopic().c_str());
  

  Serial.print(__LINE__); Serial.print(":check Response bytes:");Serial.println(String(str.length()));
  str = utils::replace(str, "{humidity}", String(humidity, 3));
  str = utils::replace(str, "{temp}", String(tempreature, 3));
  Serial.print(__LINE__); Serial.print(":check Response bytes:");Serial.println(String(str.length()));
  //yield(); // be very scared of the watchdog timer.

  Serial.print("Sending Response bytes:");Serial.println(String(str.length()));
  
  server.send(200, "text/html", str.c_str());
  Serial.println(str.substring(0, 5));
  
} 
#endif

void handleStatus() {
  DynamicJsonDocument  doc(2048);

  // create an entry in the wifi array.     
  JsonObject entry = doc.createNestedObject();

  String name = eeprom.getName();
  entry["name"] =  "NAME";
  entry["value"] =  eeprom.getName();

  entry = doc.createNestedObject();
  entry["name"] =  "version";
  entry["value"] =  APP_VERSION;


  entry = doc.createNestedObject();
  entry["name"] =  "IP Address";
  entry["value"] =  utils::tostr(WiFi.localIP()) + String("/") +  utils::tostr(WiFi.subnetMask());

  entry = doc.createNestedObject();
  entry["name"] =  "Gateway";
  entry["value"] =  utils::tostr(WiFi.gatewayIP());


  entry = doc.createNestedObject();
  entry["name"] =  "SSID";
  entry["value"] =  WiFi.SSID();

  entry = doc.createNestedObject();
  entry["name"] =  "MAC";
  entry["value"] =  WiFi.macAddress();


  entry = doc.createNestedObject();
  entry["name"] =  "mqtt_status";
  entry["value"] =  eeprom.getMQTTStatus();


  entry = doc.createNestedObject();
  entry["name"] =  "server";
  entry["value"] =  String(eeprom.getMQServer().c_str());


  entry = doc.createNestedObject();
  entry["name"] =  "port";
  entry["value"] =  eeprom.getMQPort();


  entry = doc.createNestedObject();
  entry["name"] =  "username";
  entry["value"] =  String(eeprom.getMQUser().c_str());


  entry = doc.createNestedObject();
  entry["name"] =  "password";
  entry["value"] = String(eeprom.getMQPassword().c_str());


  entry = doc.createNestedObject();
  entry["name"] =  "ssl";
  entry["value"] = eeprom.getMQSSL();

  #if defined(HAS_DHT)
  entry = doc.createNestedObject();
  entry["name"] =  "tempreature";
  entry["value"] = String(tempreature) + "F";
  
  entry = doc.createNestedObject();
  entry["name"] =  "humidity";
  entry["value"] = String(humidity, 2) + "%";


  entry = doc.createNestedObject();
  entry["name"] =  "DHT Enabled";
  entry["value"] = eeprom.getDHTStatus();


  #endif
  entry = doc.createNestedObject();
  entry["name"] =  "uptime";
  long t0 = millis();
  long seconds = t0/1000; 
  long sec = seconds%(24*60*60);
  long minutes = seconds/(24*60*60);
  
  long hours = minutes%(24*60);
  long days = hours/(24*60);
  

  entry["value"] = String(days) + " days, " + String(hours) + ":" + String(minutes) + ":" + String(sec) + "." + String(t0%1000);

  delay(0);

  entry = doc.createNestedObject();
  entry["name"] =  "uptimems";
  entry["value"] = String(t0) + "(ms)";

  entry = doc.createNestedObject();
  entry["name"] =  "Device ID";
  entry["value"] = MakeMine("");


  //int len = measureJson(doc);
  String data;
  serializeJson(doc, data);
  //Serial.printf("sending scans %s\n", data.c_str());
  server.sendHeader("Access-Control-Allow-Origin", "*");
  //server.sendHeader("Content-Length", String(len));
  server.send(200, "text/html", data);

}
void handleSystem() {
  DynamicJsonDocument  doc(2048);

  // create an entry in the wifi array.     
  JsonObject entry = doc.createNestedObject();

  entry["name"] =  "NAME";
  entry["value"] = eeprom.getName(); // for now

  entry = doc.createNestedObject();
  entry["name"] =  "IP Address";
  entry["value"] =  utils::tostr(WiFi.localIP()) ;


  entry = doc.createNestedObject();
  entry["name"] =  "Gateway";
  entry["value"] =  utils::tostr(WiFi.gatewayIP());


  entry = doc.createNestedObject();
  entry["name"] =  "SSID";
  entry["value"] =  WiFi.SSID();

  entry = doc.createNestedObject();
  entry["name"] =  "MAC";
  entry["value"] =  WiFi.macAddress();


 #if HAS_DHT
  entry = doc.createNestedObject();
  entry["name"] =  "tempreature";
  entry["value"] = String(tempreature) + "F";
  
  entry = doc.createNestedObject();
  entry["name"] =  "humidity";
  entry["value"] = String(humidity, 2) + "%";


  entry = doc.createNestedObject();
  entry["name"] =  "DHT Enabled";
  entry["value"] = eeprom.getDHTStatus();


  #endif
  entry = doc.createNestedObject();
  entry["name"] =  "uptime";
  entry["value"] = String(millis()) + "(ms)";

  entry = doc.createNestedObject();
  entry["name"] =  "Device ID";
  entry["value"] = MakeMine("");

  //int len = measureJson(doc);
  String data;
  serializeJson(doc, data);
  //Serial.printf("sending scans %s\n", data.c_str());
  server.sendHeader("Access-Control-Allow-Origin", "*");
  //server.sendHeader("Content-Length", String(len));
  server.send(200, "text/html", data);
}

void handleConfigEdit()
{
  DynamicJsonDocument  doc(2048);

  // create an entry in the wifi array.     
  JsonObject entry = doc.createNestedObject();

  entry["name"] =  "ServerName";
  entry["value"] =  eeprom.getName();


  entry = doc.createNestedObject();
  entry["name"] =  "MQTT Enabled";
  entry["value"] = eeprom.getMQTTStatus();

  entry = doc.createNestedObject();
  entry["name"] =  "MQSSL";
  entry["value"] = eeprom.getMQSSL();

#if HAS_DHT
  entry = doc.createNestedObject();
  entry["name"] =  "DHT Enabled";
  entry["value"] = eeprom.getDHTStatus();
#endif
 // int len = measureJson(doc);
  String data;
  serializeJson(doc, data);
  //Serial.printf("sending scans %s\n", data.c_str());
  server.sendHeader("Access-Control-Allow-Origin", "*");
  //server.sendHeader("Content-Length", String(len));
  server.send(200, "text/html", data);
}


void handleConfigEditPost() {
   
    Serial.printf("handleConfigEditPost: \n");
    for(int i=0; i <server.args(); i ++ ) {
      Serial.printf("%s=%s\n", server.argName(i).c_str(), server.arg(i).c_str());
    }
    if( server.args()>0) {
       DynamicJsonDocument  doc(2048);
        String json = server.argName(0);
        if(json=="plain") {
          json = server.arg(0);
        }
        deserializeJson(doc,  json);

        if( doc.containsKey("name") && doc.containsKey("value") ) {
         String name   = doc["name"];
         String  value = doc["value"];
       
          
          if(value.length() > 0) {      
            if(name == "ServerName") {
              String oldname = eeprom.getName();
              fauxmo.renameDevice(oldname.c_str(), value.c_str());
              eeprom.setName(value);
            } else if( name == "MQSSL") {
              if(value == "true") {
                eeprom.setMQSSL(true);
              } else if(value == "false") {
                eeprom.setMQSSL(false);
              }
            }
            else if( name == "MQTT Enabled") {
              if(value == "true") {
                eeprom.setMQTTStatus(true);
              } else if(value == "false") {
                eeprom.setMQTTStatus(false);
              }
            }
              else if( name == "DHT Enabled") {
              if(value == "true") {
                eeprom.setDHTStatus(true);
              } else if(value == "false") {
                eeprom.setDHTStatus(false);
              }
            }
        }

        eeprom.marshall();
    }
  }

 delay(0);
 handleConfigEdit();

}
void handleAdmin_json() {
  DynamicJsonDocument  doc(2048);

  // create an entry in the wifi array.     
  JsonObject entry = doc.createNestedObject();
  entry["name"] =  "Light";
  entry["value"] =  VIRTUAL_DEVICE0_STATE;



  int len = measureJson(doc);
  String data;
  serializeJson(doc, data);
  //Serial.printf("sending scans %s\n", data.c_str());
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Content-Length", String(len));
  server.send(200, "text/html", data);

}
void handleSubmitAdmin_json() {
   
    // Serial.printf("handleSubmitAdmin_json: \n");
    // for(int i=0; i <server.args(); i ++ ) {
    //   Serial.printf("%s=%s\n", server.argName(i).c_str(), server.arg(i).c_str());
    // }
    if( server.args()>0) {
       DynamicJsonDocument  doc(2048);
        String json = server.argName(0);
        if(json=="plain") {
          json = server.arg(0);
        }
        deserializeJson(doc,  json);

        String name = doc["name"];
        bool value = doc["value"];
        if( name.length() >0) {
      
          
          if(name == "Light") {      
              Serial.printf("handleSubmitAdmin_json: in %s  %d\n", name.c_str(), value);  
              fauxmo.setState((unsigned char)0, value, 255);               
               set_state(value);
          }


          else if(name == "Reboot") {      
              Serial.printf("handleSubmitAdmin_json: in %s  %d\n", name.c_str(), value);  
              delay(0);
              handleAdmin_json();
              delay(200);
              ESP.restart();

          }
          else if(name == "Wifi Reset") {      
              Serial.printf("handleSubmitAdmin_json: in %s  %d\n", name.c_str(), value);  
              delay(0);
              handleAdmin_json();
              delay(200);
              wifiManager.resetSettings();
          }
      }
    }
 delay(0);
 handleAdmin_json();

}

//css/app.72c36a24.css
void handleAdmin() {                          // When URI / is requested, send a web page with a button to toggle the LED
  if( server.hasArg("action")) {
    String action = server.arg("action");
    Serial.print("admin request ");Serial.println(action);
    if(action == "reset_wifi") {
      
      server.send(200, "text/html", "<h1>Success, !</h1><p>Reset successful</p>");
      delay(1000);
      wifiManager.resetSettings();
      return;
    } else  if(action == "reboot") {
     
      server.send(200, "text/html", "<h1>Success, !</h1><p>reboot successful</p>");
      delay(1000);
     
       ESP.restart();
     return;
    }
  }  
    server.send(401, "text/plain", "401: Unauthorized");
  //server.send(200, "text/html", css_dirapp_dot72c36a24_dotcss);
}




void handleNotFound(){
   if (handleFileRead(server.uri())) {
     return;
  }
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


void handleWifi() {
  DynamicJsonDocument  doc(2048);
  
    int n = WiFi.scanNetworks();
   // DEBUG_WM(F("Scan done"));
    Serial.printf("number scans %d\n", n);
    if (n == 0) {
      
    } else {

      //sort networks
      int indices[n];
      for (int i = 0; i < n; i++) {
        indices[i] = i;
      }

      // RSSI SORT

      // old sort
      for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
          }
        }
      }
      // remove duplicates ( must be RSSI sorted )
      {
        String cssid;
        for (int i = 0; i < n; i++) {
          if (indices[i] == -1) continue;
          cssid = WiFi.SSID(indices[i]);
          for (int j = i + 1; j < n; j++) {
            if (cssid == WiFi.SSID(indices[j])) {
             // DEBUG_WM("DUP AP: " + WiFi.SSID(indices[j]));
              indices[j] = -1; // set dup aps to index -1
            }
          }
        }
      }

      //display networks in page
      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue; // skip dups      
        int quality = wifiManager.getRSSIasQuality(WiFi.RSSI(indices[i]));

        // create an entry in the wifi array.     
        JsonObject entry = doc.createNestedObject();
        entry["SSID"] =  WiFi.SSID(indices[i]);
        entry["Strengh"] =  quality;
        entry["encrypted"] =  WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE;        
        //erial.printf("adding SSID %s scans \n", entry["SSID"]);
        delay(0);       
      }  
    }

  
  //int len = measureJson(doc);
  String data;
  serializeJson(doc, data);
   //Serial.printf("sending scans %s\n", data.c_str());
   server.sendHeader("Access-Control-Allow-Origin", "*");
  //server.sendHeader("Content-Length", String(len));
  server.send(200, "text/html", data);
  //delete data;


  //DEBUG_WM(F("Sent config page"));
}



/** Handle the WLAN save form and redirect to WLAN config page again */
void handleSaveWifi() {
  //DEBUG_WM(F("WiFi save"));

  //SAVE/connect here
  String _ssid = server.arg("s").c_str();
  String _pass = server.arg("p").c_str();

  server.send(200, "text/html", "<h1>SSID will be be set. " + server.arg("username") + "!</h1><p>Login successful</p>");
  delay(5000);
  WiFi.begin(_ssid.c_str(), _pass.c_str());

  //delay(5000);
  ESP.restart();
  //DEBUG_WM(F("Sent wifi save page"));

  //connect = true; //signal ready to connect/reset
}

void handleReset() {
  Serial.println("Reseting system");
  server.send(200, "text/html", "<h1>Reseting system !</h1><p>Login successful</p>");
  delay(200);
  WiFi.disconnect(true);
  ESP.reset();
}
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}




bool handleFileRead(String path){  // send the right file to the client (if it exists)

  if( remap.containsKey(path)) {
    String p = remap[path];
    path = p;
  }

  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";          
                                   // Use the compressed version
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    if(sent);
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}