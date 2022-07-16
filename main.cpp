#include <fstream>
#include <map>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

std::string getprop(char const prop[]){
    char value[255];
    if (__system_property_get(prop,value))
        printf("getprop: [%s]: [%s]\n",prop,value);
    else
        printf("prop [%s] does not exist\n",prop);
    return std::string(value);
}

static inline bool str_contains(std::string_view s, std::string_view ss) {
    return s.find(ss) != std::string::npos;
}


bool setprop(char const prop[], char const value[], bool add = false) {
    auto pi = (prop_info *) __system_property_find(prop);

    if (pi != nullptr) {
        if (__system_property_update(pi, value, strlen(value))==0){
            printf("modify prop [%s]: [%s]\n",prop,value);
            return true;
        }
    } else if (add) {
        if (__system_property_add(prop, strlen(prop), value, strlen(value))==0){
            printf("create prop [%s]: [%s]\n",prop,value);
            return true;
        }
    }
    return false;
}

bool setprop(const std::vector<std::string> &props, char const value[], bool add = false) {
    for (const auto &prop : props) {
        setprop(prop.c_str(), value, add);
    }
    return true;
}


void reset_sensitive_props(){

    static const char *prop_key[] =
        {"ro.boot.vbmeta.device_state", "ro.boot.verifiedbootstate", "ro.boot.flash.locked",
         "ro.boot.veritymode", "ro.boot.warranty_bit", "ro.warranty_bit",
         "ro.debuggable", "ro.secure", "ro.build.type", "ro.build.tags",
         "ro.vendor.boot.warranty_bit", "ro.vendor.warranty_bit",
         "vendor.boot.vbmeta.device_state", "vendor.boot.verifiedbootstate", nullptr};

    static const char *prop_val[] =
        {"locked", "green", "1",
         "enforcing", "0", "0",
         "0", "1", "user", "release-keys",
         "0", "0",
         "locked", "green", nullptr};


    for (int i = 0; prop_key[i]; ++i) {
            setprop(prop_key[i], prop_val[i]);
    }

    // Hide that we booted from recovery when magisk is in recovery mode
    auto bootmode = getprop("ro.bootmode");
    if (!bootmode.empty() && str_contains(bootmode, "recovery"))
        setprop("ro.bootmode", "unknown", false);
    bootmode = getprop("ro.boot.mode");
    if (!bootmode.empty() && str_contains(bootmode, "recovery"))
        setprop("ro.boot.mode", "unknown", false);
    bootmode = getprop("vendor.boot.mode");
    if (!bootmode.empty() && str_contains(bootmode, "recovery"))
        setprop("vendor.boot.mode", "unknown", false);

    // Xiaomi cross region flash
    auto hwc = getprop("ro.boot.hwc");
    if (!hwc.empty() && str_contains(hwc, "CN"))
        setprop("ro.boot.hwc", "GLOBAL", false);
    auto hwcountry = getprop("ro.boot.hwcountry");
    if (!hwcountry.empty() && str_contains(hwcountry, "China"))
        setprop("ro.boot.hwcountry", "GLOBAL", false);

}



int main(int argc, char *argv[]) {
   if (argc==1){
       printf("\
inject system properties tool\n\
\n\
Usage: %s NAME VALUE\n\
   or: %s --sensitive-props\n", argv[0], argv[0]);
   return 0;
   }


   if (__system_properties_init()) {
        fprintf(stderr, "__system_properties_init error\n");
        return -1;
   }


   
   const auto is_sensitive_props = strcmp(argv[1], "--sensitive-props") == 0;

   if (is_sensitive_props) {
        printf("Reset sensitive props\n");
        reset_sensitive_props();
   }
   else if (argc<3){
        char buffer[1024];
        __system_property_get(argv[1],buffer);
        fprintf(stdout, "%s\n", buffer);
        
   }
   else if (!setprop(argv[1],argv[2], true)){
           fprintf(stderr, "setprop failed\n");
           return -1;
   }

   return 0;
}
