// Copy this file to the /include directory and rename it to apSecrets.h. The 
// .gitignore file has a line to prevent the Access Points information from
// being copied up to the cloud repo.
#ifndef secrets_h
#define secrets_h

#include <Arduino.h> // Arduino Core for ESP32. Comes with PlatformIO.

// Define a structure with key value pair Strings.
typedef struct apKeyAndValue_ 
{
   String ssid;
   String pwd;
} apKeyAndValue_t;

// Declare an array of key values paris containg Access Point IDs and 
// passwords. Change the values in this array to match your network. 
apKeyAndValue_t apSecrets[] = 
{
   {"SSID1", "PASSWORD1"},
   {"SSID2", "PASSWORD2"},
   {"SSID3", "PASSWORD3"},
   {"SSID4", "PASSWORD4"},
   {"SSID5", "PASSWORD5"},
   {"SSID6", "PASSWORD6"}
};

#endif