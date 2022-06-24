#include <fstream>
#include <map>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_

#include <_system_properties.h>

#include <system_properties/prop_area.h>
#include <system_properties/system_properties.h>

#include "private/bionic_defs.h"

static SystemProperties system_properties;
static_assert(__is_trivially_constructible(SystemProperties),
              "System Properties must be trivially constructable");

// This is public because it was exposed in the NDK. As of 2017-01, ~60 apps reference this symbol.
// It is set to nullptr and never modified.
__BIONIC_WEAK_VARIABLE_FOR_NATIVE_BRIDGE
prop_area* __system_property_area__ = nullptr;

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_properties_init() {
  return system_properties.Init(PROP_FILENAME) ? 0 : -1;
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_set_filename(const char*) {
  return -1;
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_area_init() {
  bool fsetxattr_failed = false;
  return system_properties.AreaInit(PROP_FILENAME, &fsetxattr_failed) && !fsetxattr_failed ? 0 : -1;
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
uint32_t __system_property_area_serial() {
  return system_properties.AreaSerial();
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
const prop_info* __system_property_find(const char* name) {
  return system_properties.Find(name);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_read(const prop_info* pi, char* name, char* value) {
  return system_properties.Read(pi, name, value);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
void __system_property_read_callback(const prop_info* pi,
                                     void (*callback)(void* cookie, const char* name,
                                                      const char* value, uint32_t serial),
                                     void* cookie) {
  return system_properties.ReadCallback(pi, callback, cookie);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_get(const char* name, char* value) {
  return system_properties.Get(name, value);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_update(prop_info* pi, const char* value, unsigned int len) {
  return system_properties.Update(pi, value, len);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_add(const char* name, unsigned int namelen, const char* value,
                          unsigned int valuelen) {
  return system_properties.Add(name, namelen, value, valuelen);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_delete(const char *name, bool trim_node) {
  return system_properties.Delete(name, trim_node);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
uint32_t __system_property_serial(const prop_info* pi) {
  return system_properties.Serial(pi);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
uint32_t __system_property_wait_any(uint32_t old_serial) {
  return system_properties.WaitAny(old_serial);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
bool __system_property_wait(const prop_info* pi, uint32_t old_serial, uint32_t* new_serial_ptr,
                            const timespec* relative_timeout) {
  return system_properties.Wait(pi, old_serial, new_serial_ptr, relative_timeout);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
const prop_info* __system_property_find_nth(unsigned n) {
  return system_properties.FindNth(n);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_foreach(void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  return system_properties.Foreach(propfn, cookie);
}

std::string getprop(char const prop[]){
    char value[255];
    __system_property_get(prop,value);
    return std::string(value);
}

static inline bool str_contains(std::string_view s, std::string_view ss) {
    return s.find(ss) != std::string::npos;
}


void setprop(char const prop[], char const value[], bool add = false) {
    auto pi = (prop_info *) __system_property_find(prop);

    if (pi != nullptr) {
        __system_property_update(pi, value, strlen(value));
    } else if (add) {
        __system_property_add(prop, strlen(prop), value, strlen(value));
    }
}

void setprop(const std::vector<std::string> &props, char const value[], bool add = false) {
    for (const auto &prop : props) {
        setprop(prop.c_str(), value, add);
    }
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
    __system_property_delete("ro.build.selinux");

}



int main(int argc, char *argv[]) {
    if (__system_properties_init()) {
        return -1;
    }


   if (argc==1){
       printf("\
inject system properties tool\n\
\n\
Usage: %s NAME VALUE\n\
   or: %s --sensitive-props\n", argv[0], argv[0]);
   return 0;
   }

   const auto is_delete = strcmp(argv[1], "--delete") == 0;

   const auto is_sensitive_props = strcmp(argv[1], "--sensitive-props") == 0;

   if (is_sensitive_props) {
        printf("Reset sensitive props\n");
        reset_sensitive_props();
    }
    else if (is_delete){
        if (argc==3) __system_property_delete(argv[2]);
        else return 1;
    }
    else
        setprop(argv[1],argv[2], true);

    return 0;
}
