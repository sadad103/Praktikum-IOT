#include <DHT.h>

// Pin NodeMCU ESP8266
const byte dhtPin = 13;     // D7
const byte ldrPin = A0;     // A0
const byte relayPin = 12;  // D6
const byte ledPin = 5;     // D1

#define DHTTYPE DHT22
DHT dht(dhtPin, DHTTYPE);

// Relay Active-High
#define RELAY_ON HIGH
#define RELAY_OFF LOW

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Kondisi awal: relay dan LED mati
  digitalWrite(relayPin, RELAY_OFF);
  digitalWrite(ledPin, LOW);

  Serial.println("=== SMART WAREHOUSE ===");
  Serial.println("Sistem siap...");
}

void loop() {
  delay(2000);

  // Membaca sensor
  float temp = dht.readTemperature();
  int rawLdr = analogRead(ldrPin);

  // Membalik nilai LDR
  int ldrValue = 1023 - rawLdr;

  // Cek sensor DHT
  if (isnan(temp)) {
    Serial.println("DHT gagal dibaca!");
    return;
  }

  // Menampilkan data sensor
  Serial.print("Suhu  : ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Cahaya: ");
  Serial.println(ldrValue);

  // Aturan sistem
  if (temp > 34.0 || ldrValue < 300) {

    digitalWrite(relayPin, RELAY_ON);
    digitalWrite(ledPin, HIGH);

    Serial.println("Status: AKTIF");
    Serial.println("Penyebab: Panas atau gelap");

  } else {

    digitalWrite(relayPin, RELAY_OFF);
    digitalWrite(ledPin, LOW);

    Serial.println("Status: AMAN");
  }

  Serial.println("--------------------");
}