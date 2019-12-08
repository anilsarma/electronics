#ifndef __MPTT_H__
#define __MPTT_H__

int mt0=0;
void mptt_reconnect(PubSubClient client, char* user, char* password) {
  // Loop until we're reconnected
 if(!client.connected()) {
    int t1 = millis();
    int elasped = t1-mt0;
    if(elasped < 5000) {
      return; // will attemp later.
    }
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client_BASEMENT", user, password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      mt0=t1;
     // delay(5000);
    }
  }
}
#endif
