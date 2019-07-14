/*
  Created by Yvan / https://Brainy-Bits.com

  This code is in the public domain...
  You can: copy it, use it, modify it, share it or just plain ignore it!
  Thx!

*/

// NRF24L01 Module Tutorial - Code for Transmitter using Arduino NANO

//Include needed Libraries at beginning
#include "nRF24L01.h" //NRF24L01 library created by TMRh20 https://github.com/TMRh20/RF24
#include "RF24.h"
#include "SPI.h"

#define SwitchPin 8 // Arcade switch is connected to Pin 8 on NANO
const int payload=32;
char SentMessage[payload] = {000,}; // Used to store value before being sent through the NRF24L01

RF24 radio(9, 10); // NRF24L01 used SPI pins + Pin 9 and 10 on the NANO

const uint64_t pipe = 0xE6E6E6E6E6E6; // Needs to be the same for communicating between 2 NRF24L01


void setup(void) {
  Serial.begin(9600);
  pinMode(SwitchPin, INPUT_PULLUP); // Define the arcade switch NANO pin as an Input using Internal Pullups
  digitalWrite(SwitchPin, HIGH); // Set Pin to HIGH at beginning
  delay(1000);
  radio.begin(); // Start the NRF24L01

  radio.setChannel(115);
  radio.setPayloadSize(payload);
  radio.setPALevel(RF24_PA_MAX);
  //radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(pipe); // Get NRF24L01 ready to transmit
}

const int index = 9;
void loop(void) {
  bool c = radio.isChipConnected();
  Serial.print(c);
  Serial.print( " index="); Serial.print(index, DEC);  Serial.print(" ");
  if (digitalRead(SwitchPin) == LOW) { // If Switch is Activated
    for (int i = 0; i < payload; i ++ ) {
      SentMessage[i] = 111;
    }
    SentMessage[index] = 111;
    Serial.print("Sending ");
    Serial.print(SentMessage[index], DEC);
    Serial.print(" ");
    radio.write(SentMessage, payload); // Send value through NRF24L01

  }
  else {
    Serial.print("Sending ");
    for (int i = 0; i < payload; i ++ ) {
      SentMessage[i] = 000;
    }
    Serial.print(SentMessage[index], DEC);
    radio.write(SentMessage, payload);
  }
  Serial.println();
  delay(100);
}
