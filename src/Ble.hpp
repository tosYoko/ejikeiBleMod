// module;
//  Example.ixx
// export module Example;
#include <esp_task_wdt.h>
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>

#include <iostream>
#include <Arduino.h>
#include <time.h>

String addrx = "24:0a:c4:f9:8c:d2";
String errorBleCharacter = "scan_evt timeout";
bool isValidBle = false;
int errorBleCounter = 0;

// #define ANSWER 42
const char *deviceName = "BLEScanner";
BLEScan *pBLEScan;
// namespace Example_NS
// {
void bleSetup()
{
    Serial.begin(115200);
    // log("Setup!");
    BLEDevice::init(deviceName);
    pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setActiveScan(true);   // active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99); // less or equal setInterval value
}

void bleScanner()
{
    BLEScanResults foundSensors = pBLEScan->start(2, false);
    int count = foundSensors.getCount();

    // initBLEScan();
    // esp_task_wdt_reset(); // reset the watchdog timer
    for (int j = 0; j < count; j++)
    {
        BLEAdvertisedDevice bleSensor = foundSensors.getDevice(j);
        String sensorName = bleSensor.getName().c_str();
        String address = bleSensor.getAddress().toString().c_str();
        // log(address + " " + j + " " + sensorName);
        Serial.print(address);
        Serial.print(j);
        Serial.print(" : ");
        Serial.println(sensorName);
        // std::cout << address << " " << j << " " << sensorName;

        if (addrx == address)
        {
            // log("---FIND!!!!---");
            Serial.println("---Find!!!---");
            isValidBle = true;
            if (isValidBle == true)
            {
                // log("---BLE On---");
                Serial.println("---BLE ON---");
            }
            sleep(1);
            break;
        }
        if (!pBLEScan)
        {
            // initBLEScan();
            esp_task_wdt_reset(); // reset the watchdog timer
        }
        if (errorBleCharacter == address)
        {
            // log("---error!!!!---");
            errorBleCounter = errorBleCounter + 1;
            log(errorBleCounter);
            Serial.println(errorBleCounter);
            sleep(1);
            break;
        }
        isValidBle = false;
    }
}

// export int f()
// {
//     return f_internal();
// }
// }