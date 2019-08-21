
#include <LiquidCrystal_I2C.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include <dht11.h>
//#include <ESP8266WiFi.h>
//#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "LcdMenuMgr.h"
#include "ClickMgr.h"
#include "utils.h"
#include "Monitor.h"
#include "www_handler.h"
#include "BufferedData.h"

// F formatter tells compliler it's a floating point value
#define encoder0PinA D3 // CLK on rotary encoder
#define encoder0PinB D4 // DT on rotary encoder
#define CLICK_PIN D5
#define DHT11PIN D7
#define LED D6

volatile unsigned int encoder0Pos = 0;
//Encoder encoder;
int8_t clicks = 0;
char id = 0;
//LiquidCrystal_I2C lcd(0x3f, 20, 4); // SDA = D2, SCL=D1 (ESP8266E)
LiquidCrystal_I2C lcd(0x27, 16, 2); // SDA = D2, SCL=D1 (ESP8266E)
WiFiManager wifiManager;
WiFiServer server(80);
dht11  humidity_sensor;//(DHT11PIN, DHT11);
Encoder * encoder = NULL; //myEnc(D3, D4);
void (*reboot)(void) = 0;

//
//
// new
struct Callback: public LcdMenuCallback {
  void update_lcd(MenuDetails* menu, MenuItem* item) {
    lcd.setCursor(0, 0);
    //lcd.print("*MENU*");
    
  
    if (item != NULL) {
        //lcd.print(" < ");
        lcd.print(item->get_name());
    } else {
        lcd.print(menu->get_name());
        lcd.print(" < ");
    }
    if(item && item->is_editable()) {
        lcd.print( MENU_DOWN);
    }
    lcd.print("            ");
    lcd.setCursor(0, 1);
    
    if (item != NULL) {
      //lcd.print(item->get_name());
      lcd.print(item->get_edit_mode() ? "> " : "");
      lcd.print(item->get_status());
       lcd.print("            ");     
    } else {
      lcd.print("                ");
    }
  }

  void wakeup() {
    lcd.backlight();
  }
  void sleep() {
    lcd.noBacklight();
  }
};

Callback callback;
MenuMgr menu_mgr(&callback, 30000);

long led_expiry_time = 0;
bool desired = false;
Monitor::State led_state = Monitor::STOPPED;
Monitor led_monitor(30000, 5000);

const String get_led_runtime() {
  return String(led_monitor.get_runtime() / 1000);
}
void edit_led_runtime(bool clicked, bool longPressed, int direction, bool save) {
  led_monitor.set_runtime( led_monitor.get_runtime()  + (direction * 30000));
  Serial.print( led_monitor.get_runtime()  + (direction * 30000) );
  if (save) {
    Serial.print("Saving edit_led_runtime ");
    Serial.println(led_monitor.get_runtime());
  }
}
const String get_led_pausetime() {
  return String(led_monitor.get_pausetime() / 1000);
}
void edit_led_pausetime(bool clicked, bool longPressed, int direction, bool save) {
  led_monitor.set_pausetime( led_monitor.get_pausetime()  + (direction * 30000));
  if (save) {
    Serial.print("Saving led pause time ");
    Serial.println(led_monitor.get_pausetime());
  }
}
const String led_status() {
  String str(led_state == Monitor::STARTED ? "ON" : (led_state == Monitor::STOPPED ? "OFF" : "PAUSED"));
  double t = led_monitor.get_time();
  if ( t > 0) {
    return str + " " + String(t / 1000);
  }
  return str;
}


// perioidcally called in the loop
void set_led_state(Monitor::State state) {
  bool manual_override = (millis() < led_expiry_time);
  bool acutal_desired_state = desired || manual_override;
  state = acutal_desired_state ? state : Monitor::STOPPED;
  if( manual_override ) {
    state = Monitor::STARTED;
  }
  if ( led_state != state ) {
    led_state = state;
    digitalWrite(LED, led_state == Monitor::STARTED ? HIGH : LOW);
  }
}
void led_toggle(bool btn, bool lp) {
  desired = !desired;
  led_monitor.monitor(desired);
  set_led_state(desired ? Monitor::STARTED : Monitor::STOPPED);
}
const String manual_led_status() {
  Serial.print("manual_led_status ");
  Serial.print( led_expiry_time );
  Serial.print(" <-> ");
  long t1 = millis();
  long diff = led_expiry_time - millis();
  Serial.print( t1 );
  Serial.print(" diff  ");
  Serial.println( diff );
  return String( diff >0?"ON":"OFF") + String(" ") + ((diff>0)?String(diff/1000):String(""));
}
void on_click_manual_led(bool btn, bool lp) {
  led_expiry_time = millis() + 20000; // 20 seconds for now.
  Serial.print("onclock manual ");
  Serial.print( led_expiry_time );
  Serial.print(" <-> ");
  Serial.print( millis() );
  Serial.print(" diff  ");
  Serial.println( millis()-led_expiry_time );
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
  return String(high_humidity); 
}

void edit_high_humidity(bool clicked, bool longPressed, int direction, bool save) {
  high_humidity += direction;
  if (save) {
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
  if (save) {
    Serial.print("Saving LOW ");
    Serial.println(low_humidity);
  }
}
BufferedData<float> humidity(0, 1000);
BufferedData<float> temp(0, 1000); // piggy back

const String get_humidity() {
  if( humidity.is_stale()) {
    humidity_sensor.read(DHT11PIN);
    humidity.set( humidity_sensor.humidity );
    temp.set(humidity_sensor.temperature *1.8 + 32.0);
  }
  return String(humidity.get()) + "%  " + String(temp.get()) + "F";
}




void connect_to_wifi(WiFiServer);
void setup() {
  Serial.begin(9600);

  menu_mgr.get_menu().add("HUMIDITY", get_humidity, NULL);
  menu_mgr.get_menu().add("LED", led_status, led_toggle);
  menu_mgr.get_menu().add("MANUAL", manual_led_status, on_click_manual_led);
  menu_mgr.get_menu().add("IP", get_ip, NULL);
  menu_mgr.get_menu().add("UPTIME", get_uptime, NULL);
  
  MenuDetails *settings = new MenuDetails("Settings ...");
  settings->add("SET RUN TIME", get_led_runtime, NULL,   &edit_led_runtime);
  settings->add("SET PAUSE TIME", get_led_pausetime, NULL,   &edit_led_pausetime);
  
  settings->add("SET HUMIDITY ON", get_high_humidity, NULL, &edit_high_humidity);
  settings->add("SET HUMIDITY OFF", get_low_humidity, NULL,   &edit_low_humidity);
  settings->add_parent(&menu_mgr.get_menu());

  menu_mgr.get_menu().add(settings);
  
  menu_mgr.get_menu().add("RESET WIFI", NULL, reset_wifi);

  lcd.begin();
  //lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Setting up WIFI ... ");
  wifiManager.autoConnect("NODEMCUHU");
  lcd.clear();
  server.begin();
  //humidity_sensor.begin();
  encoder = new Encoder(D3, D4);

  pinMode(CLICK_PIN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  //attachInterrupt(D5, handleKey, RISING);
  // initialize serial transmission for debugging
  Serial.write("setup complete");
  lcd.setCursor(0, 0);
  lcd.print("Ready ");
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
  humidity_sensor.read( DHT11PIN );
  menu_mgr.loop(newPosition, isButtonPressed());
  set_led_state(led_monitor.check());

  // update the humidity cache.
  get_humidity();
  if( humidity.get() > 30.0) {
    if(!desired) {
      led_toggle(true, true); // enable
    }
  } else {
    if( desired) { // running
      if( humidity.get() < 29.0) {       
        led_toggle(true, true); //disable
      }
    }
  }
}
