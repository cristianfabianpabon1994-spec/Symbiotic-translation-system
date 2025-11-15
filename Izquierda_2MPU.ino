#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// ==============================
// CONFIGURACIÓN DE RED
// ==============================
const char* ssid = "DESKTOP-889TK1R 6398";
const char* password = "9215hip152";
const char* host = "192.168.137.1";  // IP del servidor

// ==============================
// CONFIGURACIÓN DEL MULTIPLEXOR
// ==============================
#define TCA_ADDR 0x70

// Objeto único para ambos sensores (se usa cambiando el canal)
Adafruit_MPU6050 mpu;

// Función para seleccionar canal del TCA9548A
void tcaSelect(uint8_t canal) {
  if (canal > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << canal);
  Wire.endTransmission();
}

// Función para inicializar una MPU6050 en un canal
bool inicializarMPU(uint8_t canal) {
  tcaSelect(canal);
  if (!mpu.begin()) {
    Serial.print("MPU6050 no detectado en canal ");
    Serial.println(canal);
    return false;
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);
  return true;
}

// ==============================
// VARIABLES DE YAW
// ==============================
float yaw_0 = 0;
float yaw_1 = 0;
unsigned long lastTime = 0;

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

  // Inicializar ambos sensores
  if (inicializarMPU(0)) Serial.println("MPU6050 canal 0 inicializado.");
  if (inicializarMPU(1)) Serial.println("MPU6050 canal 1 inicializado.");

  lastTime = millis();
}

void loop() {
  float pitch_0, roll_0;
  float pitch_1, roll_1;

  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  // === Sensor en canal 0 ===
  tcaSelect(0);
  sensors_event_t a0, g0, temp0;
  mpu.getEvent(&a0, &g0, &temp0);

  pitch_0 = atan2(-a0.acceleration.x, sqrt(a0.acceleration.y * a0.acceleration.y + a0.acceleration.z * a0.acceleration.z)) * 180.0 / PI;
  roll_0 = atan2(a0.acceleration.y, a0.acceleration.z) * 180.0 / PI;
  yaw_0 += g0.gyro.z * dt * 180.0 / PI;

  // === Sensor en canal 1 ===
  tcaSelect(1);
  sensors_event_t a1, g1, temp1;
  mpu.getEvent(&a1, &g1, &temp1);

  pitch_1 = atan2(-a1.acceleration.x, sqrt(a1.acceleration.y * a1.acceleration.y + a1.acceleration.z * a1.acceleration.z)) * 180.0 / PI;
  roll_1 = atan2(a1.acceleration.y, a1.acceleration.z) * 180.0 / PI;
  yaw_1 += g1.gyro.z * dt * 180.0 / PI;

  // === Mostrar por serial ===
  static unsigned long lastPrint = 0;
  if (currentTime - lastPrint > 500) {
    //Serial.println("Sensor P:");
    Serial.print("Pitch_IP: "); Serial.print(pitch_0, 1);
    Serial.print(" | Roll_IP: "); Serial.print(roll_0, 1);
    Serial.print(" | Yaw_IP: "); Serial.println(yaw_0, 1);

    //Serial.println("Sensor 1:");
    Serial.print("Pitch_II: "); Serial.print(pitch_1, 1);
    Serial.print(" | Roll_II: "); Serial.print(roll_1, 1);
    Serial.print(" | Yaw_II: "); Serial.println(yaw_1, 1);

    lastPrint = currentTime;
  }

  // === Enviar al servidor ===
  WiFiClient client;
  if (client.connect(host, 80)) {
    String url = "";
    url += "pitch_IP=" + String(pitch_0, 1) + "&roll_IP=" + String(roll_0, 1) + "&yaw_IP=" + String(yaw_0, 1);
    url += "&pitch_II=" + String(pitch_1, 1) + "&roll_II=" + String(roll_1, 1) + "&yaw_II=" + String(yaw_1, 1);

   // Serial.print("Enviando: ");
    Serial.println(url);

    client.print(String("GET /datos?") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    delay(10);
    client.stop();
  } else {
    Serial.println(">> Error conectando al servidor");
  }

  delay(100); // 10 Hz
}
