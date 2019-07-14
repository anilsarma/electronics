/*
  Created by Yvan / https://Brainy-Bits.com

  This code is in the public domain...
  You can: copy it, use it, modify it, share it or just plain ignore it!
  Thx!

*/


// NRF24L01 Module Tutorial - Code for Receiver using Arduino UNO

//Include needed Libraries at beginning
#include "nRF24L01.h" // NRF24L01 library created by TMRh20 https://github.com/TMRh20/RF24
#include "RF24.h"
#include "SPI.h"
//nclude "FastLED.h" // FastLED library for WS2812 RGB Stick http://fastled.io/
//#include <printf.h>
#define NUM_LEDS 8 // Number of leds on stick
#define LED_PIN 8 // Digital In (DI) of RGB Stick connected to pin 8 of the UNO


const int payload=32;
char ReceivedMessage[payload] = {000, }; // Used to store value received by the NRF24L01

RF24 radio(9, 10); // NRF24L01 used SPI pins + Pin 9 and 10 on the UNO

const uint64_t pipe = 0xE6E6E6E6E6E6; // Needs to be the same for communicating between 2 NRF24L01


void setup(void) {
  Serial.begin(9600);

  radio.begin(); // Start the NRF24L01
  radio.setChannel(115);
  radio.setPayloadSize(payload);
  radio.setPALevel(RF24_PA_MAX);
  //radio.setDataRate(RF24_250KBPS);
  
  radio.openReadingPipe(1, pipe); // Get NRF24L01 ready to receive
  radio.startListening(); // Listen to see if information received
  pinMode(LED_PIN, OUTPUT); // Set RGB Stick UNO pin to an OUTPUT
  
}
const int index = 31;
void loop(void) {
  //radio.printDetails();
 
  //bool c = radio.isChipConnected();
  //Serial.print("Connected ");
  //Serial.print(c);
   //Serial.print( " ");
   // Serial.print(radio.available());
  //Serial.println();
  while (radio.available())
  {
    radio.read(ReceivedMessage, payload); // Read information from the NRF24L01
    Serial.print("receiver radio available.");
    Serial.print( " index=");Serial.print(index, DEC);  Serial.print(" ");
    Serial.print(ReceivedMessage[index], DEC);
     
    Serial.print("\n");
    if (ReceivedMessage[index] == 111) // Indicates switch is pressed
    {
      digitalWrite(LED_PIN, HIGH);
    }
    else
    {
      digitalWrite(LED_PIN, LOW);
    }
    //digitalWrite(LED_PIN, HIGH);
  }
  delay(20); 
}
