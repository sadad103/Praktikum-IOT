const int buttonPin = 4;  // Pin tombol
const int ledPin = 5;     // Pin LED

int buttonState = 0;      // Menyimpan kondisi tombol
bool ledState = false;    // Menyimpan kondisi LED

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT);   // Tombol sebagai input
  pinMode(ledPin, OUTPUT);     // LED sebagai output

  digitalWrite(ledPin, LOW);   // Awal LED mati
}

void loop() {
  buttonState = digitalRead(buttonPin);  // Membaca tombol

  // Jika tombol ditekan
  if (buttonState == HIGH) {

    // Membalik kondisi LED
    ledState = !ledState;

    // Menyalakan atau mematikan LED
    digitalWrite(ledPin, ledState);

    // Menampilkan kondisi LED
    if (ledState) {
      Serial.println("LED ON");
    } else {
      Serial.println("LED OFF");
    }

    // Debounce dan menunggu tombol dilepas
    delay(200);
    while (digitalRead(buttonPin) == HIGH) {
      delay(10);
    }
  }
}