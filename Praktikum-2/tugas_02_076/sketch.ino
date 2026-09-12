#include <DHT.h>

const int ldrPin = A0;
const int dhtPin = 13;
const int relayPin = 12;
const int ledPin = 5;

#define DHTTYPE DHT22

DHT dht(dhtPin, DHTTYPE);

void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Membuat ADC ESP32 menjadi 10-bit (0-1023)
  analogReadResolution(10);

  // Kondisi awal: aktuator mati
  digitalWrite(relayPin, HIGH);
  digitalWrite(ledPin, LOW);

  Serial.println("=== SMART WAREHOUSE ===");
  Serial.println("Sistem mulai...");
}

void loop() {

  // Membaca sensor
  int ldrValue = analogRead(ldrPin);
  float suhu = dht.readTemperature();

  // Mengecek apakah DHT berhasil dibaca
  if (isnan(suhu)) {
    Serial.println("Gagal membaca sensor DHT!");
    delay(2000);
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" C | LDR: ");
  Serial.println(ldrValue);

  // Mesin aturan menggunakan OR (||)
  if (suhu > 34 || ldrValue < 300) {

    // Aktuator aktif
    digitalWrite(relayPin, LOW);
    digitalWrite(ledPin, HIGH);

    Serial.println("Peringatan: Aktuator Aktif!");

  } else {

    // Aktuator mati
    digitalWrite(relayPin, HIGH);
    digitalWrite(ledPin, LOW);

    Serial.println("Kondisi Aman");
  }

  Serial.println("-------------------------");

  delay(2000);
}