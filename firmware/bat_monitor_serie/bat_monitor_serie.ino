#define pinADC 3

const float divRe = 122.0 / 100.0;  // ≈ 1.22

float leerBat() {
  long suma = 0;

  for (int i = 0; i < 20; i++) {
    suma += analogRead(pinADC);
    delay(3);
  }

  float adcProm = suma / 20.0;
  float voltADC = (adcProm / 4095.0) * 3.3;

  return voltADC * divRe;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("BOOT OK");
}

void loop() {
  float voltBat = leerBat();

  Serial.print("Bateria: ");
  Serial.print(voltBat, 3);
  Serial.println(" V");

  delay(1000);
}

