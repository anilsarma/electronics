
#include <LiquidCrystal_I2C.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

//#include <ESP8266WiFi.h>
//#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "LcdMenuMgr.h"
#include "ClickMgr.h"
#include "utils.h"

// F formatter tells compliler it's a floating point value
#define encoder0PinA D3 // CLK on rotary encoder
#define encoder0PinB D4 // DT on rotary encoder
#define LED D6

volatile unsigned int encoder0Pos = 0;
//Encoder encoder;
int8_t clicks = 0;
char id = 0;
//LiquidCrystal_I2C lcd(0x3f, 20, 4); // SDA = D2, SCL=D1 (ESP8266E)
LiquidCrystal_I2C lcd(0x27, 16, 2); // SDA = D2, SCL=D1 (ESP8266E)
WiFiManager wifiManager;
WiFiServer server(80);

Encoder * encoder = NULL; //myEnc(D3, D4);
void (*reboot)(void) = 0;

//
//
// new
struct Callback: public LcdMenuCallback {
  void update_lcd(MenuItem* item) {
//    Serial.print(item->get_name());
//    if (item->is_editable()) {
//      Serial.print(item->get_value());
//    }

    lcd.setCursor(0, 0);
    lcd.print("*MENU*");

    lcd.setCursor(0, 1);
    lcd.print(item->get_name());
    lcd.print(item->get_edit_mode()?">":" ");
    lcd.print(item->get_status());
    lcd.print("            ");


  }
};

Callback callback;
MenuMgr menu_mgr(&callback);

bool led_state = false;

const String led_status() {
  return led_state ? "ON" : "OFF";
}

void led_toggle(bool btn, bool lp) {
  led_state = !led_state;
  digitalWrite(LED, led_state ? HIGH : LOW);
}

void reset_wifi(bool btn, bool lp) {
  if (btn && lp) {
    Serial.println("reset wifi and rebooting ... ");
    wifiManager.resetSettings();
    delay(5000);
    reboot();
  }
}

int high_humidity = 50;
const String get_high_humidity() {
  return  String(high_humidity);
}

int low_humidity = 50;
const String get_low_humidity() {
  return  String(low_humidity);
}

const String get_humidity() {
  return String(millis() ) + "%";
}
const String get_uptime() {
  double t = millis() / 1000;
  String u = "s";
  if ( t > 1000) {
    t = t / 60;
    u = "m";
  }
  if ( t > 1000) {
    t = t / 60.0;
    u = "h";
  }
  if ( t > 1000) {
    t = t / 24.0;
    u = "d";
  }
  return String(t) + "" + u;
}

void connect_to_wifi();
void setup() {
  Serial.begin(9600);
 
  menu_mgr.get_menu().add("HUMIDITY", get_humidity, NULL);
  menu_mgr.get_menu().add("HUM HIGH", get_high_humidity, NULL, true, &high_humidity);
  menu_mgr.get_menu().add("HUM LOW", get_low_humidity, NULL, true, &low_humidity);
  menu_mgr.get_menu().add("IP", get_ip, NULL);
  menu_mgr.get_menu().add("UPTIME", get_uptime, NULL);
  menu_mgr.get_menu().add("RESET WIFI", NULL, reset_wifi);
  menu_mgr.get_menu().add("LED", led_status, led_toggle);
  menu_mgr.get_menu().add("SETTINGS");
  menu_mgr.get_menu().add("GOOD");
  menu_mgr.get_menu().add("BAD");
  menu_mgr.get_menu().add("UGLY");
  lcd.begin();
  //lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("configuring WIFI .... ");
  wifiManager.autoConnect("NODEMCUHU");
  lcd.clear();
  server.begin();

  encoder = new Encoder(D3, D4);

  pinMode(D5, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  //attachInterrupt(D5, handleKey, RISING);
  // initialize serial transmission for debugging
  Serial.write("setup complete");
  lcd.setCursor(0, 0);
  lcd.print("Ready");

}
byte pressedButton, currentPos, currentPosParent, possiblePos[20], possiblePosCount, possiblePosScroll = 0;
bool isButtonPressed() {
  int val = digitalRead(D5);
  if ( val == HIGH) {
    return false;
  }
  return true;
}


void loop() { 
  connect_to_wifi();
  long newPosition = encoder->read();
  menu_mgr.loop(newPosition, isButtonPressed());
}



String mid(String str, int start, int len) {
  int t = 0;
  String u = "";
  for (t = 0; t < len; t++) {
    u = u + str.charAt(t + start - 1);
  }
  return u;
}

int inStrRev(String str, String chr) {
  int t = str.length() - 1;
  int u = 0;
  while (t > -1) {
    if (str.charAt(t) == chr.charAt(0)) {
      u = t + 1; t = -1;
    }
    t = t - 1;
  }
  return u;
}

int len(String str) {
  return str.length();
}


String header;
void connect_to_wifi() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /4/on") >= 0) {
              if (!led_state) {
                led_toggle(true, true );
              }
            } else if (header.indexOf("GET /4/off") >= 0) {
              if (led_state) {
                led_toggle(true, true );
              }
            } else if (header.indexOf("GET /4/reboot") >= 0) {
              Serial.println("REbooting");
              //delay(5000);
              // reboot();
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>HOME Humidity Web Server</h1>");

            // Display current state, and ON/OFF buttons for GPIO 5
            //client.println("<p>GPIO 5 - State " + led_state + "</p>");
            // If the output5State is off, it displays the ON button
            if (led_state) {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            }


            client.println("<p><a href=\"/4/reboot\"><button class=\"button button2\">Reboot</button></a></p>");
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
