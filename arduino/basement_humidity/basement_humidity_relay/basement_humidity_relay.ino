#include <dht11.h>
#define DHT11PIN 4
//require https://github.com/adidax/dht11/zipball/master
dht11 DHT11;
#define RELAY_PIN 3
#define DLH111_PIN 2

unsigned long  time =0;
char status = 0;
int first =1;

const unsigned long  MAX_RESTTIME=5*60L * 1000L;
const unsigned long MAX_RUNTIME = 20*60L*1000L;


float LOW_HUMIDITY=48;
float HIGH_HUMIDITY=50;

void setup()
{
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  time =0;
  status = 0;
}


void loop()
{
  Serial.println();

  unsigned long now = millis();
 
  unsigned long delayms=now - time;
  int chk = DHT11.read(2);

if(first) {
  first =1;
  delayms = 60* 60 * 10000L;
}
  Serial.print("Humidity (%): ");
  float h = (float)DHT11.humidity;
  Serial.println(h, 2);

  Serial.print("Temperature (C): ");
  Serial.println((float)DHT11.temperature, 2);

  
  float f = (float)DHT11.temperature*1.8 + 32.0;
  Serial.print("Temperature (F): ");
  Serial.println((float)f, 2);


  int desire = checkFanState(h);


  Serial.print("current state: ");
  Serial.println(status, DEC);


  Serial.print(" desire state: ");
  Serial.println(desire, DEC);

  Serial.print(" MAX rest");
  Serial.println(MAX_RESTTIME, DEC);

  Serial.print(" MAX run");
  Serial.println(MAX_RUNTIME, DEC);

  Serial.print(" decay run ");
  Serial.println(delayms*1.0, 2);

  if(desire==0 && status ==1) {
    status  =0;
    time = now;
    digitalWrite(RELAY_PIN, LOW);
  }
  // running for too long.
  if(status ==1) {
    // run time
    unsigned long runtime=now - time;
    if(runtime > MAX_RUNTIME) {
        digitalWrite(RELAY_PIN, LOW); // turn off
        time=now; // 
        status = 0;
    }
  } 

  if(desire==1) {
    if(status ==0) {
      //unsigned long delay = now - time;
      if( delayms > MAX_RESTTIME) {
        digitalWrite(RELAY_PIN, HIGH); // turn off
        time=now; // 
        status = 1;
      }else {
        Serial.print("rest time ");
        Serial.print(delayms * 1.0, 2);
      }
    }
  } else  {
    if( status ==1) {
      digitalWrite(RELAY_PIN, LOW); // turn off
      time=now; // 
      status = 0;
    }
  }
  
//  if(( now - time) > 10000) {
//     status = !status;
//     time = now;
//    Serial.print("Switching ");
//    Serial.println(status, DEC);
//    digitalWrite(RELAY_PIN, status?HIGH:LOW);
//  }
  
  delay(2000);

  
}

int checkFanState(float humidity)  {
  if(humidity >= HIGH_HUMIDITY ) {
    Serial.print("high humidity ");
    Serial.println(humidity, 2);
    return 1; // turn on
  }
  if( status ==0 ) {
    Serial.print("status off humidity ");
    Serial.println(humidity, 2);
    return 0; // turn off
  }
  if( humidity <= LOW_HUMIDITY) {
    Serial.print("low humidity ");
    Serial.println(humidity, 2);
    return 0;
  }
  Serial.print("high humidity defult ");
  Serial.println(humidity, 2);
  return 1;
}
