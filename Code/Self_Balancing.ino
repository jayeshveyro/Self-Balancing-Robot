#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// MOTOR PINS 
#define ENA 5
#define IN1 7
#define IN2 8

#define ENB 6
#define IN3 9
#define IN4 10

//PID

float Kp = 22.0;
float Ki = 0.8;
float Kd = 0.7;


float targetAngle = 0.0;

float error;
float previousError = 0;
float integral = 0;


float angle = 0;
float gyroRate;

unsigned long previousTime;


float gyroAngle = 0;

void setup() {
  Serial.begin(115200);

  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  //MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 connected.");

  // MPU configuration
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(1000);

  previousTime = micros();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  angle = atan2(a.acceleration.y,
                a.acceleration.z) * 180.0 / PI;

  gyroAngle = angle;
}

void loop() {


  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  unsigned long currentTime = micros();

  float dt = (currentTime - previousTime) / 1000000.0;
  previousTime = currentTime;

  if (dt <= 0 || dt > 0.1) {
    return;
  }

  float accelAngle =
    atan2(accel.acceleration.y,
          accel.acceleration.z) * 180.0 / P

  gyroRate = gyro.gyro.x * 180.0 / PI;

  gyroAngle += gyroRate * dt;

  angle = 0.98 * (angle + gyroRate * dt)
        + 0.02 * accelAngle;

  
  error = targetAngle - angle;


  integral += error * dt;

  
  integral = constrain(integral, -100, 100);

 
  float derivative =
    (error - previousError) / dt;

  previousError = error;

  
  float output =
    Kp * error +
    Ki * integral +
    Kd * derivative;


  output = constrain(output, -255, 255);

  if (abs(angle) > 35) {
    stopMotors();
    integral = 0;
    previousError = 0;
    return;
  }

  if (abs(angle) < 0.5) {
    output = 0;
  }

  setMotors(output);

  Serial.print("Angle: ");
  Serial.print(angle);

  Serial.print(" | PID: ");
  Serial.println(output);
}

void setMotors(float power) {

  int pwm = abs((int)power);
  pwm = constrain(pwm, 0, 255);

  if (power > 0) {

    
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

  } 
  else if (power < 0) {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

  } 
  else {
    stopMotors();
    return;
  }

  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);
}


void stopMotors() {

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}