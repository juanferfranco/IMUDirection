
#define M5STACK_MPU6886
#include <M5Core2.h>
#include "password.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <string.h>

WiFiUDP udpDevice;
uint16_t localUdpPort = 3302;
uint16_t UDPPort = 3301;

float accX = 0.0F;  // Define variables for storing inertial sensor data
float accY = 0.0F;  //定义存储惯性传感器相关数据的相关变量
float accZ = 0.0F;

float offsetArr[3] = {0.0F};

void getOffset(float *offset) {
  float offsetX, offsetY, offsetZ;
  M5.IMU.getAccelData(&offsetX, &offsetY, &offsetZ);
  offset[0] = offsetX;
  offset[1] = offsetY;
  offset[2] = offsetZ;
}

void drawGrid() {
  M5.Lcd.drawLine(41, 120, 279, 120, CYAN);
  M5.Lcd.drawLine(160, 1, 160, 239, CYAN);
  M5.Lcd.drawCircle(160, 120, 119, CYAN);
  M5.Lcd.drawCircle(160, 120, 60, CYAN);
}

void drawSpot(int ax, int ay) {
  static int prevx = 0;
  static int prevy = 0;
  int x, y;
  x = map(constrain(ax, -300, 300), -300, 300, 40, 280);
  y = map(constrain(ay, -300, 300), -300, 300, 240, 0);

  //M5.Lcd.fillScreen(BLACK);
  M5.Lcd.fillCircle(prevx, prevy, 7, BLACK);
  drawGrid();
  M5.Lcd.fillCircle(x, y, 7, WHITE);
  prevx = x;
  prevy = y;

}


void setup() {
  M5.begin();     //Init M5Core.
  M5.IMU.Init();  //Init IMU sensor
  getOffset(offsetArr);
  M5.Axp.SetLed(0);

  /*
    M5.Lcd.setTextColor(
    GREEN,
    BLACK);
    M5.Lcd.setTextSize(2);
  */
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
  udpDevice.begin(localUdpPort);

}

void loop() {

  static uint32_t imuReadTime = 0;
  static uint32_t printIMUTime = 0;

  uint32_t currentTime = millis();

  if ( (currentTime - imuReadTime) > 100) {
    imuReadTime = currentTime;
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    drawSpot((int)((accX - offsetArr[0]) * 1000), (int)((accY - offsetArr[1]) * 1000));
  }



  //M5.Axp.LightSleep(SLEEP_SEC(1));


  currentTime = millis();
  if ( (currentTime - printIMUTime ) > 1000) {
    printIMUTime = currentTime;

    M5.IMU.getAccelData(&accX, &accY, &accZ);

    int x, y;
    x = (int)((accX - offsetArr[0]) * 1000);
    y = (int)((accY - offsetArr[1]) * 1000);
    printf("accX: %d, accY: %d\n", x, y);
    printf("Vbat:%f/Cbat:%f\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());
    if (y >= 150) {
      auto m = (accX - offsetArr[0]) / (accY - offsetArr[1]);
      printf("m: %f\n", m);

      //M5.Lcd.setCursor(0, 20);
      //M5.Lcd.printf("m: %f", m);

      // send back a reply, to the IP address and port we got the packet from
      udpDevice.beginPacket("192.168.1.12", UDPPort);
      udpDevice.write((uint8_t *)&m, 4);
      udpDevice .endPacket();

      udpDevice.beginPacket("192.168.1.12", UDPPort);
      char buff[64];
      sprintf(buff, "Vbat:%f/Cbat:%f\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());
      udpDevice.write((const uint8_t*)buff, strlen(buff));
      udpDevice .endPacket();


    }
  }

  /*
    uint8_t packetSize = udpDevice.parsePacket();
    if (packetSize) {
      Serial.print("Data from: ");
      Serial.print(udpDevice.remoteIP());
      Serial.print(":");
      Serial.print(udpDevice.remotePort());
      Serial.print(' ');
      for (uint8_t i = 0; i < packetSize; i++) {
        Serial.write(udpDevice.read());
      }
    }
  */

}
