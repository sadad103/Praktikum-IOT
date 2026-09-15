const byte ldrPin = A0;
const byte ledPin = 5;

void setup() {
  Serial.begin(115200);

  // Menentukan D1 sebagai output
  pinMode(ledPin, OUTPUT);
}

void loop() {

  // Membaca nilai ADC dari sensor LDR (0-1023)
  int ldrValue = analogRead(ldrPin);

  // Menampilkan nilai ADC
  Serial.print("Nilai ADC: ");
  Serial.println(ldrValue);

  // Jika nilai ADC di bawah 200, LED menyala
  if (ldrValue < 200) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Kondisi: Gelap - LED MENYALA");
  }

  // Jika nilai ADC 200 atau lebih, LED mati
  else {
    digitalWrite(ledPin, LOW);
    Serial.println("Kondisi: Tidak gelap - LED MATI");
  }

  delay(1000);
}