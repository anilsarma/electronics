
#include <LiquidCrystal_I2C.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

//#include <ESP8266WiFi.h>
//#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "LcdMenuMgr.h"


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


int counter = 0;
int aState;
int bState;
int aLastState;
Encoder * encoder = NULL; //myEnc(D3, D4);

String menu[] = {
  "Menu",                   //0
  "Menu>LED",               //1
  "Menu>LED>Off",           //2
  "Menu>LED>On",            //3
  "Menu>LED>Fade",          //4
  "Menu>LED>Blink",         //5
  "Menu>LCDlight",          //6
  "Menu>Nothing1",          //7
  "Menu>Nothing2",          //8
  "Menu>Nothing3"           //9
};

typedef const String (*status)();
typedef void (*action)(bool btnState, bool longPress);

struct ITEM {
  char *item;
  status s_fn;
  action a_fn;

  ITEM(char* i, status s, action a):
    item(i), s_fn(s), a_fn(a) {

  }
};
class Menu {
    Menu * parent = NULL;
    ITEM *items[20];

  public:
    char * name;
    int pos = 0;
    int count = 0;
    Menu() {
      name = "MENU";
      count = 0;
      pos = 0;
    }
    void add(char *item) {
      add(item, NULL, NULL);
    }
    void add(char* item, status func, action atn) {
      //strcpy(items[count], item); // = item;
      items[count] = new ITEM(item, func, atn);
      count ++;
      Serial.println(count);
      Serial.print("- ");
      Serial.println(item);
      Serial.println(items[count - 1]->item );
    }
    ITEM* get(int index) {
      Serial.print("index ");
      Serial.println(index, DEC);
      Serial.print(index, DEC);
      Serial.print("- ");
      Serial.println(items[index]->item);
      return items[index];
    }

};
void (*reboot)(void) = 0;
Menu mainmenu;
int menusize = 10;
Menu *currentMenu = &mainmenu;
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
String ip2Str(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++) {
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  }
  Serial.print(s);
  return s;
}
const String get_ip() {
  return ip2Str(WiFi.localIP());
}

const String get_humidity() {
  return String(millis() ) + "%";
}

const String get_uptime() {
  double t = millis()/1000;
  String u = "s";
  if( t > 1000) {
     t = t/60;
     u = "m";
  }
  if( t > 1000) {
     t = t/60.0;
     u = "h";
  }
   if( t > 1000) {
     t = t/24.0;
     u = "d";
  }
  return String(t) + "" + u;
}

void connect_to_wifi();
void setup() {
  Serial.begin(115200);
  mainmenu.add("HUMIDITY", get_humidity, NULL);
  mainmenu.add("IP", get_ip, NULL);
   mainmenu.add("UPTIME", get_uptime, NULL);
  mainmenu.add("RESET WIFI", NULL, reset_wifi);
  mainmenu.add("LED", led_status, led_toggle);
  mainmenu.add("SETTINGS");
  mainmenu.add("GOOD");
  mainmenu.add("BAD");
  mainmenu.add("UGLY");

  lcd.begin(2,16);
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
  updateMenu(false, false);


}
byte pressedButton, currentPos, currentPosParent, possiblePos[20], possiblePosCount, possiblePosScroll = 0;
bool isButtonPressed() {
  int val = digitalRead(D5);
  if ( val == HIGH) {
    return false;
  }
  return true;
}
int oldPosition = 0;
int t0  = 0;
int tbtn = 0;
bool btnPressed = false;
bool btnState = false;
bool longPress = false;
int lastMenuUpdate = 0;
void loop() {
  connect_to_wifi();
  // lcd.backlight();
  //doEncoder_Expanded();

  //lcd.setCursor(1, 0);
  //lcd.print("Capacitance ");

  // lcd.setCursor(1, 1);
  int val = digitalRead(D5);
  if ( val == HIGH) {
    // lcd.print("Working 0x3F HIGH");
  } else {
    //lcd.print("Working 0x3F LOW");
  }

  

  long newPosition = encoder->read();

  int mv = currentMenu->count - 1;
  int t1 = millis();
  newPosition = (newPosition > mv) ? mv : newPosition;
  newPosition = (newPosition < 0) ? 0 : newPosition;
  encoder->write(newPosition);

  if (isButtonPressed()) {
    if (btnPressed) {
      if ((t1 - tbtn) > 2000) {
        if (!longPress) {
          longPress = true;
          updateMenu(true, longPress);
        }
      }
    } else {
      tbtn = millis();
      btnPressed = true;
      updateMenu(true, longPress);
    }


  } else {
    if (btnPressed) {
      updateMenu(false, false);
    }
    longPress = false;
    btnPressed = false;
  }
  if (newPosition != oldPosition) {

    if ( (t1 - t0) < 500) {
      encoder->write(oldPosition);
      return;
    }
    {
      int diff = newPosition - oldPosition;
      Serial.print("diff ");
      Serial.println(diff, DEC);
      if ( diff < 0 ) {
        newPosition = oldPosition - 1;
      } else if (diff > 0) {
        newPosition = oldPosition + 1;
      }
      newPosition = (newPosition > mv) ? mv : newPosition;
      newPosition = (newPosition < 0) ? 0 : newPosition;
      encoder->write(newPosition);
    }
    t0 = t1;
    oldPosition = newPosition;
    Serial.print("Pos ");
    Serial.println(newPosition, DEC);
    currentPos = newPosition;
    updateMenu(isButtonPressed(), longPress);
  }


  //lcd.print(newPosition);
  //lcd.print("   ");
  aLastState = aState; // Updates the previous state of the outputA with the current state
  //delay(500);
  int now = millis();
  if(( now - lastMenuUpdate) > 500) {
      updateMenu(false, false);
  }
}

void updateMenu(bool isClick, bool longPress) {
  int mv = currentMenu->count - 1;
  
  lastMenuUpdate = millis();
  currentPos = (currentPos > mv) ? mv : currentPos;
  encoder->write(currentPos);
  lcd.setCursor(0, 0);
  lcd.print(currentMenu->name);

  lcd.setCursor(0, 1);
  ITEM* entry = currentMenu->get(currentPos);
  lcd.print(entry->item);

  //  if( strcmp(currentMenu->get(currentPos), "LED")==0) {
  //       if(isClick) {
  //          led_toggle(isClick, longPress);
  //       }
  //       lcd.print(" " );
  //       lcd.print(led_status());
  //  }
  if ( entry != NULL ) {
    if ( isClick && entry->a_fn != NULL) {
      entry->a_fn(isClick, longPress);
    }
    if (entry->s_fn != NULL) {
      lcd.print(" ");
      lcd.print(entry->s_fn());
    }
  }
  if (isClick) {
    if (longPress) {
      lcd.print(" LONG " );
    } else {
      lcd.print(" SHHORT " );
    }
  }
  lcd.print("            ");
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
              if(!led_state) {
                led_toggle(true, true );
              }         
            } else if (header.indexOf("GET /4/off") >= 0) {
              if(led_state) {
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
