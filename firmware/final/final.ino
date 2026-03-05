#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* =======================
   PINES
   ======================= */
#define pinADC_BAT   3

#define pinSDA 4
#define pinSCL 5

#define pinTTP 2
#define pinHum 6

/* =======================
   RANGO BATERÍA
   ======================= */
float maxBat = 4.05;
float minBat = 1.55;

/* =======================
   OLED
   ======================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* =======================
   ESTADOS DEL SISTEMA
   ======================= */
enum EstadoSistema {
  EST_READY,
  EST_RUN,
  EST_FULL,
  EST_LOW
};

/* =======================
   VARIABLES DE CONTROL
   ======================= */
bool ttpAnterior = LOW;
bool humEncendido = false;

/* =======================
   MEDICIÓN BATERÍA
   ======================= */
float vBatFiltrada = 0;

float leerBateria() {
  long suma = 0;

  for (int i = 0; i < 100; i++) {
    suma += analogRead(pinADC_BAT);
    delay(2);
  }

  float adcProm = suma / 100.0;
  float voltADC = (adcProm / 4095.0) * 3.6;
  float voltBat = (voltADC * (122.0 / 22.0)) * 0.831;

  return voltBat;
}

/* =======================
   VOLTAJE → PORCENTAJE
   ======================= */
int voltPor(float v) {
  float p;

  if (v >= maxBat) p = 100;
  else if (v <= minBat) p = 0;
  else p = (v - minBat) * 100.0 / (maxBat - minBat);

  int por5 = round(p / 5.0) * 5;
  return constrain(por5, 0, 100);
}

/* =======================
   PORCENTAJE ESTABLE
   ======================= */
int voltPorEstable(float v) {
  static int porcentajeActual = -1;
  static float vRef = 0;

  int nuevo = voltPor(v);

  if (porcentajeActual < 0) {
    porcentajeActual = nuevo;
    vRef = v;
    return porcentajeActual;
  }

  if (abs(v - vRef) > 0.06) {
    porcentajeActual = nuevo;
    vRef = v;
  }

  return porcentajeActual;
}

/* =======================
   ESTADO DEL SISTEMA
   ======================= */
EstadoSistema obtenerEstadoSistema(int porcentaje) {

  if (porcentaje <= 30)
    return EST_LOW;

  if (porcentaje >= 95)
    return EST_FULL;

  if (humEncendido)
    return EST_RUN;

  return EST_READY;
}

/* =======================
   DISPARO HUMIDIFICADOR
   ======================= */
void dispararHumidificador() {
  digitalWrite(pinHum, LOW);
  delay(10);
  digitalWrite(pinHum, HIGH);
}

/* =======================
   ICONO DE BATERÍA
   ======================= */
void dibujarIconoBateria(int porcentaje) {
  int x = 4;
  int y = 6;
  int w = 48;
  int h = 20;

  display.drawRect(x, y, w, h, SSD1306_WHITE);
  display.fillRect(x + w, y + 6, 3, 8, SSD1306_WHITE);

  int margen = 2;
  int anchoUtil = w - 2 * margen;
  int fill = map(porcentaje, 0, 100, 0, anchoUtil);

  display.fillRect(
    x + margen,
    y + margen,
    fill,
    h - 2 * margen,
    SSD1306_WHITE
  );
}

/* =======================
   TEXTO DE ESTADO
   ======================= */
void dibujarTextoEstado(EstadoSistema estado) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  switch (estado) {

    case EST_READY:
      display.setCursor(68, 12);
      display.print("READY");
      break;

    case EST_RUN:
      display.setCursor(68, 12);
      display.print("RUN");
      break;

    case EST_FULL:
      display.setCursor(68, 6);
      display.print("BATTERY");
      display.setCursor(68, 18);
      display.print("FULL");
      break;

    case EST_LOW:
      display.setCursor(68, 6);
      display.print("BATTERY");
      display.setCursor(68, 18);
      display.print("LOW");
      break;
  }
}

/* =======================
   SETUP
   ======================= */
void setup() {

  analogReadResolution(12);
  analogSetPinAttenuation(pinADC_BAT, ADC_11db);

  pinMode(pinTTP, INPUT_PULLDOWN);
  pinMode(pinHum, OUTPUT);
  digitalWrite(pinHum, HIGH);

  Serial.begin(115200);

  Wire.begin(pinSDA, pinSCL);
  Wire.setClock(100000);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}

/* =======================
   LOOP
   ======================= */
void loop() {

  // ===== TTP223 =====
  bool ttpActual = digitalRead(pinTTP);

  // Flanco descendente
  if (ttpAnterior == HIGH && ttpActual == LOW) {
    humEncendido = !humEncendido;
    dispararHumidificador();
  }

  ttpAnterior = ttpActual;

  // ===== BATERÍA + OLED =====
  if (!humEncendido) {
    vBatFiltrada = leerBateria();
  }

  int porcentaje = voltPorEstable(vBatFiltrada);

  EstadoSistema estado = obtenerEstadoSistema(porcentaje);

  display.clearDisplay();
  dibujarIconoBateria(porcentaje);
  dibujarTextoEstado(estado);
  display.display();

  delay(200);
}
