#include <Wire.h>
#include <CRC8.h>
#include <CRC.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

struct __attribute__((__packed__)) COUNTER {
  uint32_t ONLINE;
  uint32_t RESULT;
};

struct __attribute__((__packed__)) SEND {
  uint32_t SECONDS = 1;
  uint16_t MILLIVOLTS;
  COUNTER CBM20;
  COUNTER CBT11A;
  COUNTER CI3BG;
  uint8_t CRC;
};

SEND SEND;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

void setup() {
 Wire.begin(8);
 Wire.onReceive(receiveEvent);
 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 display.setRotation(2);

 display.clearDisplay();
 display.display();

 display.setTextSize(3);
 display.setTextColor(WHITE);  

 //Serial.begin(115200);           /* инициализация UART. */
}

void receiveEvent(int COUNT) {
  Wire.readBytes((uint8_t*)&SEND, sizeof(SEND));
}

void loop() {
  if (calcCRC8((uint8_t*)&SEND, sizeof(SEND)) == 0)
  { 
      /*Serial.print(SEND.SECONDS);
      Serial.print(" | ");
      Serial.print(SEND.MILLIVOLTS);
      Serial.print(" || ");
      Serial.print(SEND.CBM20.ONLINE);
      Serial.print(" = ");
      Serial.print(SEND.CBM20.RESULT);
      Serial.print(" : ");
      Serial.print(SEND.CBT11A.ONLINE);
      Serial.print(" = ");
      Serial.print(SEND.CBT11A.RESULT);
      Serial.print(" : ");
      Serial.print(SEND.CI3BG.ONLINE);
      Serial.print(" = ");
      Serial.println(SEND.CI3BG.RESULT);*/

      
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(SEND.CBM20.ONLINE);
  display.print(" || ");
  display.println(SEND.CBM20.RESULT);
  display.display();

}
