#include <Arduino.h>
#include <time.h>
#include <iostream>
#include <esp_task_wdt.h>

#include "WiFi.h"
#include <WiFiMulti.h>

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>

#define LED_PIN 32
#define SW_PIN 33

#define JST 3600 * 9

bool isValidRoop;
bool isValidWifi;
bool isValidInitialized = false;
int _T;
int scanTime = 2;      // In seconds
int scanInterval = 10; // In mili-seconds
int errorBleCounter = 0;

static void log(String message)
{
  Serial.println(message);
}

String translateEncryptionType(wifi_auth_mode_t encryptionType)
{
  switch (encryptionType)
  {
  case (WIFI_AUTH_OPEN):
    return "Open";
  case (WIFI_AUTH_WEP):
    return "WEP";
  case (WIFI_AUTH_WPA_PSK):
    return "WPA_PSK";
  case (WIFI_AUTH_WPA2_PSK):
    return "WPA2_PSK";
  case (WIFI_AUTH_WPA_WPA2_PSK):
    return "WPA_WPA2_PSK";
  case (WIFI_AUTH_WPA2_ENTERPRISE):
    return "WPA2_ENTERPRISE";
  default:
    return "Unknown";
  }
}

const char *deviceName = "BLEScanner";
BLEScan *pBLEScan;

void setup()
{
  Serial.begin(115200);
  log("Setup!");
  BLEDevice::init(deviceName);
  pBLEScan = BLEDevice::getScan(); // create new scan
  pBLEScan->setActiveScan(true);   // active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

  esp_task_wdt_init(30, true); // enable panic so ESP32 restarts, interrupt when task executed for more than 3 secons
  esp_task_wdt_add(NULL);      // add current thread to WDT watch
  _T = millis();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  pinMode(LED_PIN, OUTPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
}

void loop()
{
  time_t t;
  struct tm *tm;
  static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
  bool isValidBle = false;

  String mitsunami = "1C:6B:CA:00:0C:9A";
  String addrx = "24:0a:c4:f9:8c:d2";
  // String errorBleCharacter = "24:0a:c4:f9:8c:d2";
  String errorBleCharacter = "scan_evt timeout";
  clock_t start = clock();
  int initializedTimer = 0;

  t = time(NULL);
  tm = localtime(&t);
  Serial.printf("%04d/%02d/%02d(%s) %02d:%02d:%02d\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                wd[tm->tm_wday],
                tm->tm_hour, tm->tm_min, tm->tm_sec);
  Serial.println("");
  delay(1000);

  log(addrx);
  if (isValidInitialized)
  {
    for (int i = 0; i < 10; i++)
    {
      time_t t;
      clock_t start = clock();
      t = time(NULL);
      tm = localtime(&t);
      Serial.printf("%04d/%02d/%02d(%s) %02d:%02d:%02d\n",
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    wd[tm->tm_wday],
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
      Serial.println("");
      Serial.printf("Scan<%d>\n", i);
      Serial.println("");
      // Serial.printf("<%d>\n");
      delay(scanInterval);
      if (!isValidRoop)
      {
        if (WiFi.status() != WL_CONNECTED)
        {
          std::cout << "Wifi Scan start"
                    << "\n";
          // log("Wifi Scan start");

          int n = WiFi.scanNetworks(false, true, false, 200);
          if (n == 0)
          {
            log("---no networks found---");
          }
          else
          {
            Serial.print(n);
            Serial.println("---networks found---");
            for (int i = 0; i < n; ++i)
            {

              String str2 = WiFi.BSSIDstr(i);
              str2 = str2.substring(0, 18);
              // Print SSID and RSSI for each network found
              if (mitsunami == str2)
              {
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.print(") ");
                Serial.print(WiFi.BSSIDstr(i));
                Serial.print(" ");
                Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
                delay(10);
                isValidWifi = true;
                log("---Wifi is Enable---");

                // static int status = WL_IDLE_STATUS; // the Wifi radio's status
                const char *ssid = "SecurityCamera";
                const char *password = "ProcTOSTECkey";

                while (WiFi.status() != WL_CONNECTED)
                {
                  // Serial.printf("Trying to connect to %s\n", ssid);
                  std::cout << "---"
                            << "Trying to connect to " << ssid << "---"
                            << "\n";
                  bool done = true;
                  // WiFi.begin(ssid, password);
                  // delay(500);
                  // Serial.print('.');
                  WiFi.begin(ssid, password);
                  while (done)
                  {
                    Serial.print("WiFi connecting");
                    auto last = millis();
                    while (WiFi.status() != WL_CONNECTED && last + 5000 > millis())
                    {
                      delay(500);
                      Serial.print(".");
                    }
                    if (WiFi.status() == WL_CONNECTED)
                    {
                      done = false;
                    }
                    else
                    {
                      Serial.println("retry");
                      WiFi.disconnect();
                      WiFi.reconnect();
                    }
                  }
                }
                // Serial.println("Connected to network");
                // Serial.println(WiFi.dnsIP());
                std::cout << "---Connected to network---"
                          << "\n";
                // std::cout << WiFi.dnsIP() << "\n";
                Serial.printf("---IPAdress ");
                Serial.print(WiFi.dnsIP());
                Serial.println("---");
                delay(1000);
                Serial.println("");
              }
            }
          }
        }
        if (WiFi.status() == WL_CONNECTED)
        {
          // Serial.println("Already Connected to network");
          // Serial.println(WiFi.dnsIP());
          std::cout << "---Already Connected to network---"
                    << "\n";
          Serial.printf("---IPAdress ");
          Serial.print(WiFi.dnsIP());
          Serial.println("---");
          delay(1000);
          Serial.println("");
        }

        log("---Listing BLE Sensors---");

        BLEScanResults foundSensors = pBLEScan->start(2, false);
        int count = foundSensors.getCount();

        // initBLEScan();
        // esp_task_wdt_reset(); // reset the watchdog timer
        for (int j = 0; j < count; j++)
        {
          BLEAdvertisedDevice bleSensor = foundSensors.getDevice(j);
          String sensorName = bleSensor.getName().c_str();
          String address = bleSensor.getAddress().toString().c_str();
          log(address + " " + j + " " + sensorName);
          if (addrx == address)
          {
            log("---FIND!!!!---");
            isValidBle = true;
            if (isValidBle == true)
            {
              log("---BLE On---");
            }
            sleep(1);
            break;
          }
          if (errorBleCharacter == address)
          {
            log("---error!!!!---");
            errorBleCounter = errorBleCounter + 1;
            log(errorBleCounter);
            Serial.println(errorBleCounter);
            sleep(1);
            break;
          }
          isValidBle = false;
        }
        esp_task_wdt_reset(); // reset the watchdog timer
      }
      if (!isValidBle)
      {
        log("---BLE Off---");
      }

      if (isValidBle && (WiFi.status() != WL_CONNECTED))
      {
        digitalWrite(LED_PIN, 1);
        delay(3000);
        digitalWrite(LED_PIN, 0);
        delay(3000);
        // isValidWifi = true;
      }

      if (!isValidBle && (WiFi.status() == WL_CONNECTED))
      {
        digitalWrite(LED_PIN, 1);
        delay(3000);
        digitalWrite(LED_PIN, 0);
        delay(3000);
        isValidWifi = false;
      }

      if (isValidWifi)
      {
        log("---wifi On---");
      }

      if (!isValidWifi)
      {
        log("---wifi Off---");
      }

      Serial.println("");
      delay(5000);

      // log("Sleeping");
      std::cout << "-----Sleeping-----"
                << "\n";
      Serial.println("");
      Serial.println("");
      sleep(5);
      // esp_task_wdt_reset(); // reset the watchdog timer

      // checkSerialCmd();
      // checkWifi();
      // digitalWrite(LED_BUILTIN, LOW);
    } /* code */
  }
  else
  {
    initializedTimer = 10;
    while (initializedTimer > 0)
    {
      std::cout << initializedTimer << "\n";
      sleep(1);
      initializedTimer = initializedTimer - 1;
      esp_task_wdt_reset();
    }
    isValidInitialized = true;
  }
  delay(1);
}