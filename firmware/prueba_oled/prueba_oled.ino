#include <U8g2lib.h>
#include <Wire.h>

#define SDA_PIN 8
#define SCL_PIN 9

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(
  U8G2_R0,
  /* reset=*/ U8X8_PIN_NONE
);

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  delay(200);

  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 15, "OLED OK");
  u8g2.sendBuffer();
}

void loop() {
}
