// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

 
#include "Arduino.h"
#include "SystemWiFi.h"
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"
#include "NTPClient.h"
#include "Telemetry.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// WiFi related functions
NetworkInterface *network = NULL;
static char ssid[WIFI_SSID_MAX_LEN + 1] = { 0 };

bool InitSystemWiFi(void)
{
    if (network == NULL)
    {
        network = new EMW10xxInterface();
    }
    
    return (network != NULL);
}

bool SystemWiFiConnect(void)
{
    EEPROMInterface eeprom;
    
    uint8_t pwd[WIFI_PWD_MAX_LEN + 1] = { '\0' };
    
    int ret = eeprom.read((uint8_t*)ssid, WIFI_SSID_MAX_LEN, 0x00, WIFI_SSID_ZONE_IDX);
    if (ret < 0)
    {
        Serial.print("ERROR: Failed to get the Wi-Fi SSID from EEPROM.\r\n");
        return false;
    }
    else if(ret == 0)
    {
        Serial.print("INFO: the Wi-Fi SSID is empty, please set the value in configuration mode.\r\n");
        return false;
    }
    ret = eeprom.read(pwd, WIFI_PWD_MAX_LEN, 0x00, WIFI_PWD_ZONE_IDX);
    if (ret < 0)
    {
        Serial.print("ERROR: Failed to get the Wi-Fi password from EEPROM.\r\n");
        return false;
    }

    ((EMW10xxInterface*)network)->set_interface(Station);
    ret = ((EMW10xxInterface*)network)->connect( (char*)ssid, (char*)pwd, NSAPI_SECURITY_WPA_WPA2, 0 );
    if(ret != 0)
    {
      	Serial.printf("ERROR: Failed to connect Wi-Fi %s.\r\n", ssid);
        return false;
    }
    else
    {
        Serial.printf("Wi-Fi %s connected.\r\n", ssid);
        
        // Initialize the telemetry only after Wi-Fi established
        telemetry_init();

        // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
        // We collect data about the features you use, how often you use them, and how you use them.
        send_telemetry_data_sync("", "wifi", "Wi-Fi connected");

        return true;
    }
}

const char* SystemWiFiSSID(void)
{
    return ssid;
}

NetworkInterface* WiFiInterface(void)
{
    return network;
}

int SystemWiFiRSSI(void)
{
    return (int)((EMW10xxInterface*)network)->get_rssi();
}

int WiFiScan(WiFiAccessPoint *res, unsigned count)
{
    if (network != NULL)
    {
        return ((EMW10xxInterface*)network)->scan(res, count);
    }
    return 0;
}

bool SystemWiFiAPStart(const char *ssid, const char *passphrase)
{
    if (network != NULL)
    {
        ((EMW10xxInterface*)network)->set_interface(Soft_AP);
        int ret = ((EMW10xxInterface*)network)->connect( (char*)ssid, (char*)passphrase, NSAPI_SECURITY_WPA_WPA2, 0 );
        if(ret != 0)
        {
            Serial.printf("ERROR: Failed to start AP for Wi-Fi %s.\r\n", ssid);
            return false;
        }
        else
        {
            Serial.printf("AP mode Wi-Fi %s started .\r\n", ssid);
            return true;
        }
    }

    return false;
}

NetworkInterface* WiFiAPInterface(void)
{
    return network;
}