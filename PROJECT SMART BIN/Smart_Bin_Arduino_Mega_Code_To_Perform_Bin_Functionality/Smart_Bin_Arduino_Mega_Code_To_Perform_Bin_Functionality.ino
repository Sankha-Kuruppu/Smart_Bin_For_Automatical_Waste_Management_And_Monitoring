#include <Servo.h>
#include <AccelStepper.h>
#define NUM_SENSORS 5
int trigPins[NUM_SENSORS] = { 22, 34, 26, 28, 30 };
int echoPins[NUM_SENSORS] = { 23, 35, 27, 29, 31 };
#define MAIN_TRIG_PIN 32
#define MAIN_ECHO_PIN 33
#define METAL_SENSOR_PIN 3
#define MOISTURE_SENSOR_PIN A0
#define IR_SENSOR_PIN A1
#define STEPPER_DIR_PIN 7
#define STEPPER_STEP_PIN 8
#define STEPS_PER_REV 200  // full steps (1.8° per step)
#define BUZZER_PIN 10      // Active buzzer → ON at LOW
Servo myServo;
// --- AccelStepper Setup ---AccelStepper stepper(AccelStepper::DRIVER, STEPPER_STEP_PIN, STEPPER_DIR_PIN);
// -------- Helper: Serial + ESP32 messaging --------
void sendMessage(String msg) {
  Serial.println(msg);
  Serial1.println(msg);
}
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);  // TX1=18, RX1=19
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }
  pinMode(MAIN_TRIG_PIN, OUTPUT);
  pinMode(MAIN_ECHO_PIN, INPUT);
  pinMode(METAL_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);  // OFF initially (because active buzzer needs LOW to sound)
  myServo.attach(6);
  myServo.write(0);              // --- Stepper config ---
  stepper.setMaxSpeed(800);      // steps/sec
  stepper.setAcceleration(200);  // steps/sec^2
  sendMessage("System Ready");
}
// -------- Ultrasonic Distance --------
long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000UL);
  if (duration == 0) return -1;  // no echo
  return duration * 0.034 / 2;
}
// -------- Stepper Control (degrees) --------
void rotateStepper(int degrees, bool clockwise) {
  long steps = ((long)STEPS_PER_REV * degrees) / 360;
  if (!clockwise) steps = -steps;
  stepper.move(steps);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}
// -------- Servo Action --------
void operateServo() {
  myServo.write(90);
  delay(1000);
  myServo.write(0);
  delay(1000);
}
void loop() {
  bool buzzerOn = false;
  // --- Check 5 ultrasonic sensors ---
  for (int i = 0; i < NUM_SENSORS; i++) {
    long d = getDistance(trigPins[i], echoPins[i]);
    if (d == -1) d = 9999;  // invalid → treat as far away
    sendMessage("U" + String(i + 1) + ":" + String(d));
    if (d > 0 && d < 4) {  // only valid values under 4cm
      buzzerOn = true;
    }
  }
  // --- Control buzzer (inverted logic) --- if (buzzerOn) {
  digitalWrite(BUZZER_PIN, LOW);  // ON
  sendMessage("ALERT:Buzzer On (Object <4cm)");
}
else {
  digitalWrite(BUZZER_PIN, HIGH);  // OFF
  // Only check main ultrasonic if buzzer not triggered
  long mainDistance = getDistance(MAIN_TRIG_PIN, MAIN_ECHO_PIN);
  if (mainDistance != -1 && mainDistance <= 20) {
    sendMessage("MainUltrasonic:Item Detected");
    // --- Read Sensors ---
    int metalDetected = digitalRead(METAL_SENSOR_PIN);
    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
    int irValue = analogRead(IR_SENSOR_PIN);
    String wasteType = "Non-Transparent Waste";
    int angle = 288;  // default for non-transparent
    // --- Priority: Metal > Wet > IR ---
    if (metalDetected == LOW) {
      wasteType = "Metal";
      angle = 72;
    } else if (moistureValue > 950) {
      wasteType = "Wet Waste";
      angle = 144;
    } else if (irValue < 700) {
      wasteType = "Transparent Waste";
      angle = 216;
    }
    sendMessage("WasteType:" + wasteType);
    // --- Stepper + Servo Sequence ---
    rotateStepper(angle, true);   // move to target position
    operateServo();               // servo action
    rotateStepper(angle, false);  // return to start
  } else {
    sendMessage("MainUltrasonic:No Item");
  }
}
delay(2000);  // update every 2s
}
