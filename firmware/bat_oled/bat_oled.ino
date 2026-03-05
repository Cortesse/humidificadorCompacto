#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define pinADC 3
float maxBat = 4.05;
float minBat = 1.55; 

/* =======================
   MEDICIÓN DE BATERÍA
   ======================= */
float leerBateria() {
  long suma = 0;

  for (int i = 0; i < 100; i++) {
    suma += analogRead(pinADC);
    delay(3);
  }

  float adcProm = suma / 100.0;

  // ADC a 11dB ≈ 3.6V reales
  float voltADC = (adcProm / 4095.0) * 3.6;

  // Divisor + factor empírico
  float voltBat = (voltADC * (122.0 / 22.0)) * 0.831;

  return voltBat;
}

/* =======================
   VOLTAJE → PORCENTAJE BASE
   ======================= */
int voltPor(float v) {
  float porcentaje;

  // Limites absolutos
  if (v >= maxBat) porcentaje = 100;
  else if (v <= minBat) porcentaje = 0;
  else {
    // Escalado lineal entre 1.55 V y 4.05 V
    porcentaje = (v - minBat) * 100.0 / (maxBat - minBat);
  }

  // Cuantización a pasos de 5 %
  int por5 = round(porcentaje / 5.0) * 5;

  // Saturación de seguridad
  if (por5 > 100) por5 = 100;
  if (por5 < 0) por5 = 0;

  return por5;
}


/* =======================
   PORCENTAJE ESTABLE (CON HISTÉRESIS)
   ======================= */
int voltPorEstable(float v) {
  static int porcentajeActual = -1;
  static float vReferencia = 0;

  int nuevoPorcentaje = voltPor(v);

  // Primera medición
  if (porcentajeActual < 0) {
    porcentajeActual = nuevoPorcentaje;
    vReferencia = v;
    return porcentajeActual;
  }

  // Histéresis por voltaje real (60 mV)
  if (abs(v - vReferencia) > 0.06) {
    porcentajeActual = nuevoPorcentaje;
    vReferencia = v;
  }

  return porcentajeActual;
}

/* =======================
   OLED
   ======================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define pinSDA 8
#define pinSCL 9

/* =======================
   PARPADEO BATERÍA BAJA
   ======================= */
bool estadoParpadeo = true;
unsigned long tBlink = 0;

/* =======================
   DIBUJO BATERÍA FULL SCREEN
   ======================= */
void dibujarBateriaFull(int porcentaje) {
  int x = 0;
  int y = 4;
  int ancho = 120;
  int alto = 24;

  display.drawRect(x, y, ancho, alto, SSD1306_WHITE);
  display.fillRect(x + ancho, y + 6, 6, 12, SSD1306_WHITE);

  bool mostrar = true;

  // Parpadeo si < 20 %
  if (porcentaje < 20) {
    if (millis() - tBlink > 500) {
      tBlink = millis();
      estadoParpadeo = !estadoParpadeo;
    }
    mostrar = estadoParpadeo;
  }

  if (mostrar) {
    int anchoFill = map(porcentaje, 0, 100, 0, ancho - 4);
    display.fillRect(x + 2, y + 2, anchoFill, alto - 4, SSD1306_WHITE);
  }
}

/* =======================
   SETUP
   ======================= */
void setup() {
  analogReadResolution(12);
  analogSetPinAttenuation(pinADC, ADC_11db);

  Serial.begin(115200);
  delay(1000);
  Serial.println("BOOT OK");

  Wire.begin(pinSDA, pinSCL);
  Wire.setClock(100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  display.clearDisplay();
  display.display();
}

/* =======================
   LOOP
   ======================= */
void loop() {
  float vBat = leerBateria();
  int porcentaje = voltPorEstable(vBat);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  dibujarBateriaFull(porcentaje);

  display.display();
  delay(200);
}

