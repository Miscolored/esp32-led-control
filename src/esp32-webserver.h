#ifndef WIFI.H
    #include <WiFi.h>
    #define WIFI.H
#endif
#ifndef WEBSERVER.H
    #include <WebServer.h>
    #define WEBSERVER.H
#endif

/* Put your SSID & Password */
const char* SSID = "ESP32_lucky";  // Enter SSID here
const char* PASSWORD = "12345678a";  //Enter Password here

/* Put IP Address details */
IPAddress LOCAL_IP(192,168,1,1);
IPAddress GATEWAY(192,168,1,1);
IPAddress SUBNET(255,255,255,0);

