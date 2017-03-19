#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define DHT_PIN      2
#define OLED_MOSI    9
#define OLED_CLK    10
#define OLED_DC     11
#define OLED_CS     12
#define OLED_RESET  13

#define LENG        31
#define DHTTYPE     DHT22
#define DOUBLE_CHAR_WIDTH 12
#define DOUBLE_CHAR_HEIGHT 16
#define SINGLE_CHAR_HEIGHT 8

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
DHT dht(DHT_PIN, DHTTYPE);

unsigned char buf[LENG];

int PM01 = 0;
int PM25 = 0;
int PM100 = 0;
int ERROR_CODE = 0;
float HUMID = 0;
float TEMP = 0;
unsigned long LAST_DHT_READ_TIMESTAMP = 0;
unsigned long LAST_DISPLAY_REFRESH_TIMESTAMP = 0;

void setup()   {
  Serial.begin(9600);
  Serial.setTimeout(1500);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setRotation(1);
}

void loop()
{
  // checking and consuming PMS7003 buffer
  if (Serial.find(0x42)) {
    Serial.readBytes(buf,LENG);
    if (buf[0] == 0x4d && isValid(buf)) {
      PM01 = (buf[9]<<8) + buf[10];
      PM25 = (buf[11]<<8) + buf[12];
      PM100 = (buf[13]<<8) + buf[14];
      ERROR_CODE  = buf[28];
    }
  }
  // read DHT22 only every 2 seconds
  if ((millis() - LAST_DHT_READ_TIMESTAMP) > 2000) {
    HUMID = dht.readHumidity();
    TEMP = dht.readTemperature();
    LAST_DHT_READ_TIMESTAMP = millis();
  }
  // refresh display only every 1 second
  if ((millis() - LAST_DISPLAY_REFRESH_TIMESTAMP) > 1000) {
    refreshDisplay();
    LAST_DISPLAY_REFRESH_TIMESTAMP = millis();
  }
}

// little helper to right-align text
int getXOffset(int value, int charWidth)
{
  int nChars = 1;
  if (value > 0) {
    nChars = (int)log10(value) + 1;
  }
  // seems like one of the 64 pixels is not usable (?)
  return 63 - (nChars * charWidth);
}

// displays one record of data on the display
int displayRow(char *label, int value, int y, int decimal)
{
  y = y + DOUBLE_CHAR_HEIGHT + 1;
  int x = 0;
  display.setCursor(x,y);
  display.setTextSize(1);
  display.print(label);
  y = y + SINGLE_CHAR_HEIGHT + 1;
  x = getXOffset(value, DOUBLE_CHAR_WIDTH);
  if (decimal > -1) {
    x = x - (DOUBLE_CHAR_WIDTH * 2);
  }
  display.setCursor(x,y);
  display.setTextSize(2);
  display.print(value);
  if (decimal > -1) {
    display.print(".");
    display.print(decimal);
  }
  return y;
}

// little helper to get the first float's decimal as integer
int getFirstDecimal(float a)
{
  int b = (int)a;
  a = a - b;
  a = a * 10;
  return (int)a;
}

// regresh display function
void refreshDisplay()
{
  display.clearDisplay();
  int x = 0;
  int y = 0;
  display.setCursor(x,y);
  if (ERROR_CODE != 0) {
    display.setTextSize(2);
    display.println("ERROR CODE: ");
    display.println(ERROR_CODE);
  } else {
    y = displayRow("PM 1.0", PM01, -17, -1);
    y = displayRow("PM 2.5", PM25, y, -1);
    y = displayRow("PM 10", PM100, y, -1);
    y = displayRow("TEMP", (int)TEMP, y, getFirstDecimal(TEMP));
    displayRow("HUMID", (int)HUMID, y, getFirstDecimal(HUMID));
  }
  display.display();
}

// validates a PMS7003 32 bytes frame against its checksum
int isValid(unsigned char *buf)
{
  int checkSum = (buf[29]<<8) + buf[30];
  int sum = 0x42;
  for (int i = 0; i < 29; i++) {
    sum = sum + buf[i];
  }
  return sum == checkSum;
}
