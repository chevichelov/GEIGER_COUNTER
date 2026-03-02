#include <ESP8266WiFi.h>
#include <Wire.h>
#include <CRC8.h>
#include <CRC.h>

#define  NUMBER_CBM20   34
#define  NUMBER_SBT11A  34
#define  NUMBER_CI3BG   34

#define R1 467590
#define R2 33290 


#define SLAVE_ADDR 8                                 //I2C адрес ведомого устройства 0x8

volatile uint32_t SECONDS             = 0;
volatile uint8_t  SECONDS_CBM20       = 0;
volatile uint32_t DOSE_CBM20_ONLINE   = 0;
volatile uint32_t DOSE_CBM20[NUMBER_CBM20];

volatile uint8_t  SECONDS_CBT11A      = 0;
volatile uint32_t DOSE_CBT11A_ONLINE  = 0;
volatile uint32_t DOSE_CBT11A[NUMBER_SBT11A];

volatile uint8_t  SECONDS_CI3BG      = 0;
volatile uint32_t DOSE_CI3BG_ONLINE  = 0;
volatile uint32_t DOSE_CI3BG[NUMBER_CI3BG];

struct __attribute__((__packed__)) COUNTER {
  uint32_t ONLINE;
  uint32_t RESULT;
};

struct __attribute__((__packed__)) SEND {
  uint32_t SECONDS;
  uint16_t MILLIVOLTS;
  COUNTER CBM20;
  COUNTER CBT11A;
  COUNTER CI3BG;
  uint8_t CRC;
};

struct DATA {
  uint32_t SUMM     = 0;
};

DATA DATA_CBM20;
DATA DATA_CBT11A;
DATA DATA_CI3BG;
SEND SEND;

IRAM_ATTR void timerISR() 
{
  DOSE_CBM20_ONLINE = DOSE_CBM20[SECONDS_CBM20];
  ++SECONDS_CBM20;
  if (SECONDS_CBM20 > NUMBER_CBM20)
    SECONDS_CBM20 = 0;
  DOSE_CBM20[SECONDS_CBM20] = 0;

  DOSE_CBT11A_ONLINE = DOSE_CBT11A[SECONDS_CBT11A];
  ++SECONDS_CBT11A;
  if (SECONDS_CBT11A > NUMBER_SBT11A)
    SECONDS_CBT11A = 0;
  DOSE_CBT11A[SECONDS_CBT11A] = 0;

  DOSE_CI3BG_ONLINE = DOSE_CI3BG[SECONDS_CI3BG];
  ++SECONDS_CI3BG;
  if (SECONDS_CI3BG > NUMBER_CI3BG)
    SECONDS_CI3BG = 0;
  DOSE_CI3BG[SECONDS_CI3BG] = 0;

  ++SECONDS;
  timer0_write(ESP.getCycleCount() + 80000000L);      //Тактовая частота 80MHz, получаем секунду
}

IRAM_ATTR void CBM20() 
{
  ++DOSE_CBM20[SECONDS_CBM20];
}

IRAM_ATTR void CBT11A() 
{
  ++DOSE_CBT11A[SECONDS_CBT11A];
}

IRAM_ATTR void CI3BG() 
{
  ++DOSE_CI3BG[SECONDS_CI3BG];
}

void setup() {
  noInterrupts();
  WiFiMode(WIFI_STA);
  WiFi.disconnect(); 
  WiFi.mode(WIFI_OFF);
  Wire.begin(D1, D2);                               //Настройка I2C с использованием определенных пинов
  
  timer0_isr_init(); //
  timer0_attachInterrupt(timerISR);                 //Настраиваем прерывание (привязка к функции)
  timer0_write(ESP.getCycleCount() + 80000000L);    //Тактовая частота 80MHz, получаем секунду
  
  attachInterrupt(14, CBM20, FALLING);
  attachInterrupt(12, CBT11A, FALLING);
  attachInterrupt(13, CI3BG, FALLING);
  interrupts();
}


void loop() {
  if (SEND.SECONDS != SECONDS)
  {
    SEND.SECONDS      = SECONDS;
    SEND.MILLIVOLTS   = (analogRead(A0) / 1024.0f * (( R1 + R2 ) / R2)) * 1000;
    
    DATA_CBM20.SUMM   = 0;
    for (uint32_t DOSE : DOSE_CBM20)
      DATA_CBM20.SUMM = DATA_CBM20.SUMM + DOSE;
    SEND.CBM20.ONLINE = DOSE_CBM20_ONLINE;
    SEND.CBM20.RESULT = DATA_CBM20.SUMM / (1 - NUMBER_CBM20 * 0.00019);
  
    DATA_CBT11A.SUMM  = 0;
    for (uint32_t DOSE : DOSE_CBT11A)
      DATA_CBT11A.SUMM = DATA_CBT11A.SUMM + DOSE;
    SEND.CBT11A.ONLINE = DOSE_CBT11A_ONLINE;
    SEND.CBT11A.RESULT = DATA_CBT11A.SUMM / (1 - NUMBER_SBT11A * 0.00044);
  
    DATA_CI3BG.SUMM  = 0;   
    for (uint32_t DOSE : DOSE_CI3BG)
      DATA_CI3BG.SUMM = DATA_CI3BG.SUMM + DOSE;
    SEND.CI3BG.ONLINE = DOSE_CI3BG_ONLINE;
    SEND.CI3BG.RESULT = DATA_CI3BG.SUMM / (1 - NUMBER_CI3BG * 0.00019);
  
    SEND.CRC = calcCRC8((uint8_t*)&SEND, sizeof(SEND) - sizeof(SEND.CRC));
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write((uint8_t*)&SEND, sizeof(SEND));
    Wire.endTransmission();
  }
}
