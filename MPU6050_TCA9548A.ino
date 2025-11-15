#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// Dirección del TCA9548A
#define TCA_ADDR 0x70

Adafruit_MPU6050 mpu;

float yaw = 0;
unsigned long lastTime = 0;

// Función para seleccionar canal del TCA9548A
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);  // canal 0 = 0x01, canal 1 = 0x02, etc.
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("Inicializando MPU6050 a través del TCA9548A...");

  tcaSelect(0); // Selecciona canal 0

  if (!mpu.begin()) {
    Serial.println("No se encontró el MPU6050 en canal 0. Verifica conexión.");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
  Serial.println("MPU6050 listo en canal 0.");

  lastTime = millis();
}

void loop() {
  tcaSelect(0);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float roll  = atan2(ay, az) * 180.0 / PI;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  yaw += g.gyro.z * dt * 180.0 / PI;

  // Datos para Serial Plotter: separados por tabs
  Serial.print(pitch); Serial.print("\t");
  Serial.print(roll);  Serial.print("\t");
  Serial.println(yaw);

  delay(20); // Frecuencia ~50 Hz
}
