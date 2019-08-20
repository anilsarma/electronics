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

#endif
