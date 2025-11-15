#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// Configuración WiFi
const char* ssid = "DESKTOP-889TK1R 6398";  // Nombre de tu red WiFi
const char* password = "9215hip152";        // Contraseña de tu red
const char* host = "192.168.137.1";         // IP del servidor (tu PC)

// Multiplexor TCA9548A
#define TCA_ADDR 0x70

Adafruit_MPU6050 mpu;
float yaw_1 = 0;
unsigned long lastTime = 0;

// Selección del canal del multiplexor
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a red WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a red WiFi!");
  Serial.print("IP cliente: ");
  Serial.println(WiFi.localIP());

  // Selección de canal y sensor
  tcaSelect(0);
  if (!mpu.begin()) {
    Serial.println("MPU6050 no detectado.");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
  Serial.println("MPU6050 inicializado.");
  lastTime = millis();
}

void loop() {
  tcaSelect(0);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float roll_1  = atan2(ay, az) * 180.0 / PI;
  float pitch_1 = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  yaw_1 += g.gyro.z * dt * 180.0 / PI;

  // Imprimir por Serial cada 500 ms
  static unsigned long lastPrint = 0;
  if (currentTime - lastPrint > 500) {
    Serial.print("Pitch: "); Serial.print(pitch_1, 1);
    Serial.print(" | Roll: "); Serial.print(roll_1, 1);
    Serial.print(" | Yaw: "); Serial.println(yaw_1, 1);
    lastPrint = currentTime;
  }

  // Enviar al servidor
  WiFiClient client;
  if (client.connect(host, 80)) {
    String url = "/datos?pitch_1=" + String(pitch_1, 1) +
                 "&roll_1=" + String(roll_1, 1) +
                 "&yaw_1=" + String(yaw_1, 1);

    Serial.print("Enviando: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    delay(10); // pequeña pausa
    client.stop();
  } else {
    Serial.println(">> Error conectando al servidor");
  }

  delay(100); // Envío cada 100ms (~10Hz)
}
