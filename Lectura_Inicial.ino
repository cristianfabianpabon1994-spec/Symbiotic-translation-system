#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

//objeto
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin();
//  mpu.initialize();
  while (!Serial); //Detectando el puerto serial
  Serial.println("°°°°Iniciando Indice Izquiero°°°°");
  if(!mpu.begin()){
    Serial.println(".....Verificar conexion....");
    while(1)delay(10);
  }
  // Configuración del sensor
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  delay(100);
}

void loop() {
  sensors_event_t a, g, temp; //Estructura donde se almacenan los datos
  mpu.getEvent(&a, &g, &temp);//Solicita la lectura y completa las estructuras  

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;

  Serial.print("a[x y z] g[x y z]:\t");
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.print(az); Serial.print("\t");
  Serial.print(gx); Serial.print("\t");
  Serial.print(gy); Serial.print("\t");
  Serial.println(gz);

}
