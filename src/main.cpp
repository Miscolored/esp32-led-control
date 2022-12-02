/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 * updated for NimBLE by H2zero
 */
 
/** NimBLE differences highlighted in comment blocks **/


#include <Arduino.h>
#include "NimBLEDevice.h"

uint8_t OFF[] = {0xCC, 0x24, 0x33};
uint8_t ON[] =  {0XCC, 0x23, 0x33};

// SINGLE COLOR                R     G     B
uint8_t SOLID_RED[] =   {0x56, 0xFF, 0x00, 0x00, 0x00, 0xF0, 0xAA};
uint8_t SOLID_GREEN[] = {0x56, 0x00, 0xFF, 0x00, 0x00, 0xF0, 0xAA};
uint8_t DIM_GREEN[] =   {0x56, 0x00, 0x01, 0x00, 0x00, 0xF0, 0xAA};

// WHITES intensity(high bright)                 INTENSITY
uint8_t SOLID_WHITE[] = {0x56, 0x00, 0x00, 0x00, 0x7F, 0x0F, 0xAA};

// BUILT IN md(0x25-38), sp (low fast) MODE  SPEED
uint8_t PURPLE_BUILT_IN[] =     {0xBB, 0x2B, 0x01, 0x44};
uint8_t RAINBOW[] =             {0xBB, 0x38, 0x10, 0x44};

// String serverUUID = "a4:c1:38:59:a1:d9";
static BLEUUID serviceUUID("FFD5");
static BLEUUID    charUUID("FFD9");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

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

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
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
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = advertisedDevice; /** Just save the reference now, no need to copy the object */
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(9600);
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

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    Serial.println(millis());
  
    switch(millis() % 4) {
      default:
      pRemoteCharacteristic->writeValue(ON, sizeof(ON));
        Serial.println("Setting SOILD RAINBOW");
        pRemoteCharacteristic->writeValue(RAINBOW, sizeof(RAINBOW));
        break;
      case 4:
        pRemoteCharacteristic->writeValue(ON, sizeof(ON));
        Serial.println("Setting SOLID_RED");
        pRemoteCharacteristic->writeValue(SOLID_RED, sizeof(SOLID_RED));
        break;
      case 5:
        pRemoteCharacteristic->writeValue(ON, sizeof(ON));
        Serial.println("Setting SOLID_WHITE");
        pRemoteCharacteristic->writeValue(SOLID_WHITE, sizeof(SOLID_WHITE));
        break;
      case 1:
        pRemoteCharacteristic->writeValue(ON, sizeof(ON));
        Serial.println("Setting DIM_GREEN");
        pRemoteCharacteristic->writeValue(DIM_GREEN, sizeof(DIM_GREEN));
        break;
      case 6:
        Serial.println("Setting PURPLE_BUILT_IN");
        pRemoteCharacteristic->writeValue(PURPLE_BUILT_IN, sizeof(PURPLE_BUILT_IN));
        break;    
    }
    String newValue = "Time since boot: " + String(millis()/1000);
    //Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    /*** Note: write / read value now returns true if successful, false otherwise - try again or disconnect ***/
    //pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }
  delay(1500);
  
} // End of loop