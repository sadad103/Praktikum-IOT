
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Kredensial Jaringan Wi-Fi
const char* ssid = "Aku suka kamu";
const char* password = "Kamusukasamayanglain";

// Konfigurasi Broker MQTT Publik
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* clientId = "ESP8266_LabIoT_25782076_Full";

// Pembagian Topik Mandiri
const char* topicPublish   = "kampus/labiot/25782076/telemetri";
const char* topicSubscribe = "kampus/labiot/25782076/kendali";
const char* topicAlert     = "kampus/labiot/25782076/alert";
const char* topicStatus    = "kampus/labiot/25782076/status";

// Alokasi Pin Perangkat Keras
const byte dhtPin = 2; // Pin D4 (GPIO 2)
const byte ledPin = 5; // Pin D1 (GPIO 5)
#define DHTTYPE DHT22

DHT dht(dhtPin, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastTelemetryTime = 0;
const unsigned long telemetryInterval = 4000;

// Variabel tambahan Tugas 6.7
const float BATAS_SUHU = 31.0;
bool kondisiBahaya = false;
bool alertSudahDikirim = false;
bool ledManual = false;

// Pengaturan LED otomatis
void perbaruiLED() {
  if (kondisiBahaya) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, ledManual ? HIGH : LOW);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char incomingBuffer[length + 1];
  memcpy(incomingBuffer, payload, length);
  incomingBuffer[length] = '\0';

  Serial.println("----------------------------------------");
  Serial.printf("[KONTROL DITERIMA] Topik: %s | Payload: %s\n", topic, incomingBuffer);

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, incomingBuffer);

  if (err) {
    Serial.println("Gagal membedah payload JSON!");
    return;
  }

  if (strcmp(topic, topicSubscribe) == 0 &&
      doc.containsKey("led")) {

    if (kondisiBahaya) {
      Serial.println("Kendali manual ditolak: suhu berbahaya!");
      Serial.println("----------------------------------------");
      return;
    }

    int val = doc["led"];

    if (val == 0 || val == 1) {
      ledManual = (val == 1);
      digitalWrite(ledPin, ledManual ? HIGH : LOW);

      Serial.printf(
        "--> [INDIKATOR] Status LED diubah: %s\n",
        ledManual ? "MENYALA" : "PADAM"
      );
    }
  }

  Serial.println("----------------------------------------");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan kembali ke Broker...");

    if (client.connect(
          clientId,
          topicStatus,
          1,
          true,
          "OFFLINE")) {

      Serial.println(" Sukses Terhubung!");

      client.subscribe(topicSubscribe);
      client.publish(topicStatus, "ONLINE", true);

      Serial.printf("Subskripsi Topik: %s\n", topicSubscribe);
      Serial.println("[STATUS] ONLINE");

    } else {
      Serial.printf(
        "Gagal (rc=%d). Coba lagi dalam 4 detik...\n",
        client.state()
      );
      delay(4000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Terkoneksi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  unsigned long now = millis();

  if (now - lastTelemetryTime >= telemetryInterval) {
    lastTelemetryTime = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      // Pengujian A: status suhu
      kondisiBahaya = (t > BATAS_SUHU);

      if (!kondisiBahaya) {
        alertSudahDikirim = false;
      }

      // Pengujian C: LED otomatis
      perbaruiLED();

      StaticJsonDocument<256> docOut;
      docOut["node_id"] = clientId;
      docOut["suhu"] = serialized(String(t, 2));
      docOut["kelembapan"] = serialized(String(h, 2));
      docOut["status_led"] = digitalRead(ledPin);
      docOut["uptime_detik"] = millis() / 1000;
      docOut["ip_lokal"] = WiFi.localIP().toString();
      docOut["status"] = kondisiBahaya ? "BAHAYA" : "NORMAL";

      char buffer[256];
      serializeJson(docOut, buffer);

      bool success = client.publish(topicPublish, buffer);

      if (success) {
        Serial.printf("[TELEMETRI TERKIRIM] %s\n", buffer);
      } else {
        Serial.println("[ERROR] Gagal memublikasikan telemetri!");
      }

      // Pengujian B: alert suhu
      Serial.printf(
        "[DEBUG ALERT] Suhu=%.2f | Bahaya=%d | SudahDikirim=%d | MQTT=%d\n",
        t, kondisiBahaya, alertSudahDikirim, client.connected()
      );

      if (kondisiBahaya && !alertSudahDikirim) {
        StaticJsonDocument<128> docAlert;

        docAlert["status"] = "OVERHEAT";
        docAlert["suhu"] = t;
        docAlert["batas_suhu"] = BATAS_SUHU;
        docAlert["node_id"] = clientId;

        char alertBuffer[128];
        serializeJson(docAlert, alertBuffer);

        Serial.printf("[DEBUG ALERT] Mengirim ke topik: %s\n", topicAlert);

        if (client.publish(topicAlert, alertBuffer)) {
          alertSudahDikirim = true;
          Serial.printf("[ALERT TERKIRIM] %s\n", alertBuffer);
        } else {
          Serial.println("[ERROR] Gagal mengirim alert!");
        }
      }

      if (kondisiBahaya) {
        Serial.println("[PERINGATAN] SUHU DI ATAS 31 C!");
      } else {
        Serial.println("[STATUS SUHU] NORMAL");
      }

    } else {
      Serial.println("[ERROR] Gagal membaca sensor DHT22!");
    }
  }
}