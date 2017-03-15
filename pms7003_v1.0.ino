#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUTTON       2
#define OLED_MOSI    9
#define OLED_CLK    10
#define OLED_DC     11
#define OLED_CS     12
#define OLED_RESET  13
#define LENG        31   //0x42 + 31 bytes equal to 32 bytes

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

unsigned char buf[LENG];

int PM01_CF1    = 0;
int PM25_CF1    = 0;
int PM100_CF1   = 0;
int PM01_ATM    = 0;
int PM25_ATM    = 0;
int PM100_ATM   = 0;
int PM03_COUNT  = 0;
int PM05_COUNT  = 0;
int PM10_COUNT  = 0;
int PM25_COUNT  = 0;
int PM50_COUNT  = 0;
int PM100_COUNT = 0;
int BUTTON_STATE  = 0;
int DISPLAY_STATE = 0;
int ERROR_CODE = 0;

void setup()   {                
  Serial.begin(9600);
  Serial.setTimeout(1500);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  pinMode(BUTTON, INPUT);
}

void loop()
{
  BUTTON_STATE = digitalRead(BUTTON);
  if (BUTTON_STATE != DISPLAY_STATE) {
    DISPLAY_STATE = BUTTON_STATE;
    refreshDisplay();
  }
  if (Serial.find(0x42)) {    //start to read when detect 0x42
    Serial.readBytes(buf,LENG);

    if (buf[0] == 0x4d) {
      if (isValid(buf)) {
        PM01_CF1    = (buf[3]<<8) + buf[4];
        PM25_CF1    = (buf[5]<<8) + buf[6];
        PM100_CF1   = (buf[7]<<8) + buf[8];
        PM01_ATM    = (buf[9]<<8) + buf[10];
        PM25_ATM    = (buf[11]<<8) + buf[12];
        PM100_ATM   = (buf[13]<<8) + buf[14];
        PM03_COUNT  = (buf[15]<<8) + buf[16];
        PM05_COUNT  = (buf[17]<<8) + buf[18];
        PM10_COUNT  = (buf[19]<<8) + buf[20];
        PM25_COUNT  = (buf[21]<<8) + buf[22];
        PM50_COUNT  = (buf[23]<<8) + buf[24];
        PM100_COUNT = (buf[25]<<8) + buf[26];
        ERROR_CODE  = buf[28];
      }           
    } 
  }

  static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis();
      refreshDisplay();
    }
  
}

void refreshDisplay()
{
  display.clearDisplay();
  display.setCursor(0,0);

  if (ERROR_CODE != 0) {
    display.print("ERROR CODE: ");
    display.print(ERROR_CODE);
  } else if (DISPLAY_STATE == 0) {
    display.print("Std. Particles CF-1: ");
    display.println();
    display.print(" PM 1.0: ");
    display.print(PM01_CF1);
    display.print(" ug/m3");
    display.println();
    display.print(" PM 2.5: ");
    display.print(PM25_CF1);
    display.print(" ug/m3");
    display.println();
    display.print(" PM  10: ");
    display.print(PM100_CF1);
    display.print(" ug/m3");
    display.println();
    display.print("Std. Atmosphere: ");
    display.println();
    display.print(" PM 1.0: ");
    display.print(PM01_ATM);
    display.print(" ug/m3");
    display.println();
    display.print(" PM 2.5: ");
    display.print(PM25_ATM);
    display.print(" ug/m3");
    display.println();
    display.print(" PM  10: ");
    display.print(PM100_ATM);
    display.print(" ug/m3");
  } else {
    display.print("Particle Counts: ");
    display.println();
    display.print("PM  0.3: ");
    display.print(PM03_COUNT);
    display.println();
    display.print("PM  0.5: ");
    display.print(PM05_COUNT);
    display.println();
    display.print("PM  1.0: ");
    display.print(PM10_COUNT);
    display.println();
    display.print("PM  2.5: ");
    display.print(PM25_COUNT);
    display.println();
    display.print("PM  5.0: ");
    display.print(PM50_COUNT);
    display.println();
    display.print("PM 10.0: ");
    display.print(PM100_COUNT);
  }
  display.display();
}

int isValid(unsigned char *buf)
{  
  int checkSum = (buf[29]<<8) + buf[30];
  int sum = 0x42;
  for (int i = 0; i < 29; i++) {
    sum = sum + buf[i];
  }
  return sum == checkSum;
}
