
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
#include "Monitor.h"
#include "www_handler.h"


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

bool desired=false;
Monitor::State led_state = Monitor::STOPPED;
Monitor led_monitor(30000, 5000);

const String get_led_runtime() {
  return String(led_monitor.get_runtime()/1000);
}
void edit_led_runtime(bool clicked, bool longPressed, int direction, bool save) {
  led_monitor.set_runtime( led_monitor.get_runtime()  + (direction * 5000));
  Serial.print( led_monitor.get_runtime()  + (direction * 5000) );
  if(save) {
    Serial.print("Saving edit_led_runti "); 
    Serial.println(led_monitor.get_runtime());
  }
}
 const String get_led_pausetime() {
  return String(led_monitor.get_pausetime()/1000);
}
void edit_led_pausetime(bool clicked, bool longPressed, int direction, bool save) {
  led_monitor.set_pausetime( led_monitor.get_pausetime()  + (direction * 5000));
  if(save) {
    Serial.print("Saving led pause time ");
    Serial.println(led_monitor.get_pausetime());
  }
}
const String led_status() {
  String str(led_state==Monitor::STARTED ? "ON" : (led_state==Monitor::STOPPED?"OFF":"PAUSED"));
  double t = led_monitor.get_time();
  if( t>0) {
    return str + " " + String(t/1000);
  }
  return str;
}


void set_led_state(Monitor::State state) {
  if( led_state != state ) {
      led_state = state;
      digitalWrite(LED, led_state==Monitor::STARTED ? HIGH : LOW);
  }
}
void led_toggle(bool btn, bool lp) {
  desired = !desired;
  led_monitor.monitor(desired);
  set_led_state(desired?Monitor::STARTED:Monitor::STOPPED);
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

void edit_high_humidity(bool clicked, bool longPressed, int direction, bool save) {
  high_humidity += direction;
  if(save) {
    Serial.print("Saving HIGH ");
    Serial.println(high_humidity);
  }
}
int low_humidity = 50;
const String get_low_humidity() {
  return  String(low_humidity);
}
void edit_low_humidity(bool clicked, bool longPressed, int direction, bool save) {
  low_humidity += direction;
  if(save) {
    Serial.print("Saving LOW ");
    Serial.println(low_humidity);
  }
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

void connect_to_wifi(WiFiServer);
void setup() {
  Serial.begin(9600);
 
  menu_mgr.get_menu().add("HUMIDITY", get_humidity, NULL);
  menu_mgr.get_menu().add("HUM HIGH", get_high_humidity, NULL, &edit_high_humidity);
  menu_mgr.get_menu().add("HUM LOW", get_low_humidity, NULL,   &edit_low_humidity);
  menu_mgr.get_menu().add("IP", get_ip, NULL);
  menu_mgr.get_menu().add("UPTIME", get_uptime, NULL);
  menu_mgr.get_menu().add("RESET WIFI", NULL, reset_wifi);
  menu_mgr.get_menu().add("LED", led_status, led_toggle);
  menu_mgr.get_menu().add("LED RUN", get_led_runtime, NULL,   &edit_led_runtime);
  menu_mgr.get_menu().add("LED PAUSE", get_led_pausetime, NULL,   &edit_led_pausetime);
  
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
//byte pressedButton, currentPos, currentPosParent, possiblePos[20], possiblePosCount, possiblePosScroll = 0;
bool isButtonPressed() {
  int val = digitalRead(D5);
  if ( val == HIGH) {
    return false;
  }
  return true;
}


void loop() { 
  connect_to_wifi(server);
  long newPosition = encoder->read();
  menu_mgr.loop(newPosition, isButtonPressed());


   set_led_state(led_monitor.check());
 
}
