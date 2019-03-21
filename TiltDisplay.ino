/*
   Tilt Hydrometer ESP32 Data Display

   Shows the temperature and the gravity of a Tilt Hydrometer in degrees Celsius and Plato on the display

   Written 2019 by Holger Eiboeck holger@eiboeck.de
   
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Wire.h>
#include "SSD1306Wire.h" // https://github.com/ThingPulse/esp8266-oled-ssd1306

#define MAX_MANUF_LEN 100
#define SLEEPTIME_SECS 60

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, 5, 4);

BLEScan* pBLEScan;
int scanTime = 5; //In seconds

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {

      if (advertisedDevice.haveManufacturerData() == true) {
        std::string strManufacturerData = advertisedDevice.getManufacturerData();

        if (strManufacturerData.length() >= MAX_MANUF_LEN) { // buffer overflow?
          return;
        }

        uint8_t cManufacturerData[MAX_MANUF_LEN];
        strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

        if (cManufacturerData[6] != 0xbb) { // isn't it a Tilt?
          return;
        }

        int tempF;
        tempF = cManufacturerData[20] * 256 + cManufacturerData[21];
        Serial.printf("TempF: %d\n", tempF);

        float tempC;
        tempC = ( tempF - 32) * 5 / 9;
        Serial.printf("TempC: %.1f\n", tempC);

        float gravity;
        gravity = (cManufacturerData[22] * 256 + cManufacturerData[23]) / 1000.0;
        Serial.printf("Gravity: %.1f\n", gravity);

        float gravityPlato;
        gravityPlato = 135.997 * (gravity * gravity * gravity) - 630.272 * (gravity * gravity) + 1111.14 * gravity - 616.868;

        String tempStr = String(tempC, 1) + "°C";
        String gravityStr = String(gravityPlato, 1) + "°P";

        display.clear();
        display.setColor(WHITE);
        display.setFont(ArialMT_Plain_24);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 2, tempStr);
        display.drawString(64, 28, gravityStr);
        display.display();
      }
    }
};

void setup() {
  Serial.begin(115200);

  Serial.println("Initialize...");
  display.init();
  display.flipScreenVertically();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster

  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 25, "Seaching Tilt Hydrometer...");
  display.display();
}

void loop() {
  Serial.println("Scanning for Tilt Hydrometer...");

  display.setColor(BLACK);
  display.fillRect(0, 0, 10, 10);
  display.display();

  BLEScanResults foundDevices = pBLEScan->start(scanTime);

  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, String(foundDevices.getCount()));
  display.display();

  delay(SLEEPTIME_SECS * 1000);
}
