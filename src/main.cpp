/**
 * BLE connection to Triones device, controlled via SoftAP ESP32
 */
 
/**
 * 
 * WIFI Section
 * 
*/

#include <Arduino.h>
#include "NimBLEDevice.h"

#include <esp32-webserver.h>
#include <sp105-ble.h>
#include <triones-ble.h>



WebServer wifiserver(80);

String ble_mode = "off";

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

/**
 * 
 * WIFI Handlers
 * 
*/

String SendHTML(String ble_mode) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  if (ble_mode == "off") {
    ptr +="<p>BLE Status: OFF</p><a class=\"button button-on\" href=\"/off\">OFF</a>\n";
    ptr +="<p>OFF</p><a class=\"button button-off\" href=\"/disco\">DISCO</a>\n";
    ptr +="<p>OFF</p><a class=\"button button-off\" href=\"/solid_red\">Solid Red</a>\n";
  } else if (ble_mode == "disco") {
    ptr +="<p>BLE Status: OFF</p><a class=\"button button-off\" href=\"/off\">OFF</a>\n";
    ptr +="<p>OFF</p><a class=\"button button-on\" href=\"/disco\">DISCO</a>\n";
    ptr +="<p>OFF</p><a class=\"button button-off\" href=\"/solid_red\">Solid Red</a>\n";
  } else if (ble_mode == "solid_red") {
    ptr +="<p>BLE Status: OFF</p><a class=\"button button-off\" href=\"/off\">OFF</a>\n";
    ptr +="<p>OFF</p><a class=\"button button-off\" href=\"/disco\">DISCO</a>\n";
    ptr +="<p>OFF</p><a class=\"button button-on\" href=\"/solid_red\">Solid Red</a>\n";
  }
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void handle_OnConnect() {
  Serial.println("handle_OnConnect");
  wifiserver.send(200, "text/html", SendHTML(ble_mode));
}

void handle_ble_mode_off() {
  Serial.println("handle_ble_mode_off");
  ble_mode = "off";
  wifiserver.send(200, "text/html", SendHTML(ble_mode));
  Serial.println("Setting OFF");
  pRemoteCharacteristic->writeValue(TRIONES_OFF, sizeof(TRIONES_OFF));

}

void handle_ble_mode_disco() {
  Serial.println("handle_ble_mode_disco");
  ble_mode = "disco";
  wifiserver.send(200, "text/html", SendHTML(ble_mode));
  Serial.println("Setting DISCO");
  pRemoteCharacteristic->writeValue(TRIONES_ON, sizeof(TRIONES_ON));
  pRemoteCharacteristic->writeValue(TRIONES_DISCO, sizeof(TRIONES_DISCO));  
}

void handle_ble_mode_solid_red() {
  Serial.println("handle_ble_mode_solid_green");
  ble_mode = "solid_red";
  wifiserver.send(200, "text/html", SendHTML(ble_mode));
  Serial.println("Setting SOLID_RED");
  pRemoteCharacteristic->writeValue(TRIONES_ON, sizeof(TRIONES_ON));
  pRemoteCharacteristic->writeValue(TRIONES_SOLID_RED, sizeof(TRIONES_SOLID_RED));
}

void handle_404() {
  Serial.println("page not found");
  wifiserver.send(404, "text/plain", "Not found");
}

/**
 * 
 * BLE Callbacks
 * 
*/
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */  
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("onConnect");
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
/***************** New - Security handled here ********************
****** Note: these are the same return values as defaults ********/
  uint32_t onPassKeyRequest(){
    Serial.println("Client PassKeyRequest");
    return 123456; 
  }
  bool onConfirmPIN(uint32_t pass_key){
    Serial.print("The passkey YES/NO number: ");Serial.println(pass_key);
    return true; 
  }
  void onAuthenticationComplete(ble_gap_conn_desc desc){
    Serial.println("Starting BLE work!");
  }
/*******************************************************************/
};

bool connectToBLEServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created BLE client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to BLE server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(TRIONES_SERVICE_UUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(TRIONES_SERVICE_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(TRIONES_CHAR_UUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(TRIONES_CHAR_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      Serial.println("The charactistic value is notifiable");
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */

  void onResult(BLEAdvertisedDevice* advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice->toString().c_str());
    Serial.println((int)advertisedDevice->haveServiceUUID());
    Serial.println(advertisedDevice->getServiceUUID(0).toString().c_str());
    Serial.println(advertisedDevice->getServiceUUID(1).toString().c_str());
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(TRIONES_SERVICE_UUID)) {

      BLEDevice::getScan()->stop();
      myDevice = advertisedDevice; /** Just save the reference now, no need to copy the object */
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {

  Serial.begin(9600);

  /**
   * 
   * WIFI
   * 
  */
  WiFi.softAP(SSID, PASSWORD);
  WiFi.softAPConfig(LOCAL_IP, GATEWAY, SUBNET);
  delay(100);
  wifiserver.on("/", handle_OnConnect);
  wifiserver.on("/solid_red", handle_ble_mode_solid_red);
  wifiserver.on("/disco", handle_ble_mode_disco);
  wifiserver.on("/off", handle_ble_mode_off);
  wifiserver.onNotFound(handle_404);
  wifiserver.begin();
  Serial.println("HTTP server started");

  /**
   * 
   * BLE
   * 
  */
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  /**
   * 
   * WIFI
   * 
  */
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToBLEServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }



  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    Serial.println("BLE connected, handling wifi client");
    wifiserver.handleClient();
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }
  delay(1500);
  
} // End of loop