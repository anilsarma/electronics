#ifndef __UTILS_H__
#define __UTILS_H__
String ip2Str(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++) {
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  }
 // Serial.print(s);
  return s;
}
const String get_ip() {
  return ip2Str(WiFi.localIP());
}
#endif
