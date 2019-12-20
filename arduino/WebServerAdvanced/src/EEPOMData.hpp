#ifndef __EEPROM_DATA_HPP__
#define __EEPROM_DATA_HPP__
#include "EEPROM.h"
#define STRING_LEN 512
class EEPROMData {
    #define VERSION 1
    bool dirty = true;

    int version;
    char mqserver[STRING_LEN];
    int  mqport;
    char mquser[STRING_LEN];
    char mqpassword[STRING_LEN];
    bool mqssl;
     bool mqtt_status;
    char temp_topic[STRING_LEN];
    char humidity_topic[STRING_LEN];
    char name[STRING_LEN];
    bool dht_status;

public:
    EEPROMData(int signature) {
        version = signature;
        bzero(mqserver, sizeof(mqserver));
        mqport = 0;
        bzero(mquser, sizeof(mquser));
        bzero(mqpassword, sizeof(mqpassword));        
        mqssl = false;
        mqtt_status = false;
        bzero(temp_topic, sizeof(temp_topic));        
        bzero(humidity_topic, sizeof(humidity_topic));       
        bzero(name, sizeof(name));       
        dht_status = false; 
    }
    String getMQServer() {
        return String(mqserver);
    }
    void setMQServer(const String& str) {
        if(getMQServer() == str) {
            return;
        }
        dirty = true;
        bzero(mqserver, sizeof(mqserver));
        memcpy(mqserver, str.c_str(), str.length());
    }

     int getMQPort() {
        return mqport;
    }
    void setMQPort(int value) {
         if(mqport == value) {
            return;
        }
        mqport = value;
        dirty = true;
    }

     String getMQUser() {
        return String(mquser);
    }

    void setMQUser(const String& str) {
        if(getMQUser() == str) {
            return;
        }
        bzero(mquser, sizeof(mquser));
        memcpy(mquser, str.c_str(), str.length());
        dirty = true;
    }
     String getMQPassword() {
        return String(mqpassword);
    }

    void setMQPassword(const String& str) {
        if(getMQPassword() == str) {
            return;
        }
        bzero(mqpassword, sizeof(mqpassword));
        memcpy(mqpassword, str.c_str(), str.length());
        dirty = true;
    }

    bool getMQSSL() {
        return mqssl;
    }
    void setMQSSL(bool ssl) {
         if(ssl == mqssl) {
            return;
        }
        mqssl = ssl;
        dirty = true;
    }
  bool getMQTTStatus() {
        return mqtt_status;
    }
    void setMQTTStatus(bool value) {
         if(value == mqtt_status) {
            return;
        }
        mqtt_status = value;
        dirty = true;
    }



    String getTempTopic() {
        return String(temp_topic);
    }

    void setTempTopic(const String& str) {
        if(getTempTopic() == str) {
            return;
        }
        bzero(temp_topic, sizeof(temp_topic));
        memcpy(temp_topic, str.c_str(), str.length());
        dirty = true;
    }

    String getHumidityTopic() {
        return String(humidity_topic);
    }

    void setHumidityTopic(const String& str) {
        if(getHumidityTopic() == str) {
            return;
        }
        bzero(humidity_topic, sizeof(humidity_topic));
        memcpy(humidity_topic, str.c_str(), str.length());
        dirty = true;
    }


    String getName() {
        return String(name);
    }

    void setName(const String& str) {
        if(getName() == str) {
            return;
        }
        bzero(name, sizeof(name));
        memcpy(name, str.c_str(), str.length());
        dirty = true;
    }

     bool getDHTStatus() {
        return dht_status;
    }
    void setDHTStatus(bool value) {
         if(dht_status == value) {
            return;
        }
        dht_status = value;
        dirty = true;
    }


    void marshall() {
        if(!dirty) {
            return;
        }
        EEPROM.put(0, version);
        EEPROM.put(sizeof(version), mqserver);
        EEPROM.put(sizeof(version) + sizeof(mqserver),  mqport);
        EEPROM.put(sizeof(version) + sizeof(mqserver) + sizeof(mqport),  mquser);
        EEPROM.put(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser),  mqpassword);
        EEPROM.put(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword),  mqssl);
        EEPROM.put(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl),  mqtt_status);
        EEPROM.put(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl) + sizeof(mqtt_status),  temp_topic);
        EEPROM.put(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl) + sizeof(mqtt_status) + sizeof(temp_topic), humidity_topic);
        EEPROM.put(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl) + sizeof(mqtt_status) + sizeof(temp_topic) + sizeof(humidity_topic), name);
        EEPROM.put(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl) + sizeof(mqtt_status) + sizeof(temp_topic) + sizeof(humidity_topic) + sizeof(name), dht_status);
        EEPROM.commit();

    }

    
    void unmarshall() {        
        int signature = 0;
        EEPROM.get(0, signature);
        if(signature == version) {
            EEPROM.get(sizeof(version), mqserver);
            EEPROM.get(sizeof(version) + sizeof(mqserver),  mqport);
            EEPROM.get(sizeof(version) + sizeof(mqserver) + sizeof(mqport),  mquser);
            EEPROM.get(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser),  mqpassword);
            EEPROM.get(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword),  mqssl);
            EEPROM.get(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl),  mqtt_status);
            EEPROM.get(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl) + sizeof(mqtt_status),  temp_topic);
            EEPROM.get(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl) + sizeof(mqtt_status) + sizeof(temp_topic), humidity_topic);
            EEPROM.get(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl) + sizeof(mqtt_status) + sizeof(temp_topic) + sizeof(humidity_topic), name);
            EEPROM.get(sizeof(version) + sizeof(mqserver) + sizeof(mqport) + sizeof(mquser) + sizeof(mqpassword) + sizeof(mqssl) + sizeof(mqtt_status) + sizeof(temp_topic) + sizeof(humidity_topic) + sizeof(name), dht_status);

            // safety 
            if(strlen(name)>=sizeof(name)) {
                name[0] = 0;
            }
        }
    }

};
#endif