#ifndef __WWW_HANDLER_H__
#define __WWW_HANDLER_H__
String header;
void connect_to_wifi(WiFiServer server) {
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
//              if (!led_state) {
//                led_toggle(true, true );
//              }
            } else if (header.indexOf("GET /4/off") >= 0) {
//              if (led_state) {
//                led_toggle(true, true );
//              }
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
//            if (led_state) {
//              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
//            } else {
//              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
//            }


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
#endif
