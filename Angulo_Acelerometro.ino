//////////////////////////////////////////////////
////Angulo inclinacion con acelerometro///////////
//////////////////////////////////////////////////

#include<Wire.h>
#include<Adafruit_MPU6050.h>
#include<Adafruit_Sensor.h>
#include<math.h>

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  while(!Serial);
  Serial.println("Iniciando MPU6050");
  if(!mpu.begin()){
    Serial.println("Verificar conexion");
    while(1)delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  delay(100);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;
  //Calcular los angulos de inclinacion:
  float accel_ang_x=atan(ay/sqrt(pow(ax,2) + pow(az,2)))*(180.0/3.14);
  float accel_ang_y=atan(ax/sqrt(pow(ay,2) + pow(az,2)))*(180.0/3.14);
  //Mostrar los angulos separadas por un [tab]
  Serial.print("Inclinacion en X: ");
  Serial.print(accel_ang_x); 
  Serial.print("tInclinacion en Y:");
  Serial.println(accel_ang_y);
  delay(10);
}
