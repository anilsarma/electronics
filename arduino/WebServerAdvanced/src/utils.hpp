#ifndef __UTILS_HPP__
#define __UTILS_HPP__
namespace utils {
    String replace(String& haystack, const String& needle, const String& replacement){

        //String  str2(haystack);
        haystack.replace(needle, replacement);
        yield();
        return haystack; 
    }

    String tostr(const IPAddress& addr) {
        return String(addr[0]) + String(".") + String(addr[1]) + String(".") +  String(addr[2]) + String(".") +  String(addr[3]);
    }
}

#endif