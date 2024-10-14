// https://wiki.dfrobot.com/TB6600_Stepper_Motor_Driver_SKU__DRI0043
#include <Arduino.h>
#include <Bounce2.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

const int LED = 13; // define LED pin

const int PUL_X = 7; // define Pulse pin
const int DIR_X = 6; // define Direction pin
const int ENA_X = 5; // define Enable Pin

const int PUL_Y = 10; // define Pulse pin
const int DIR_Y = 9; // define Direction pin
const int ENA_Y = 8; // define Enable Pin

const int BUTTON_X_PLUS = 54; // A0
const int BUTTON_X_MINUS = 55; // A1
const int BUTTON_Y_MINUS = 56; // A2
const int BUTTON_Y_PLUS = 57; // A3
const int BUTTON_GRAB= 58; // A4
const int BUTTON_RESET= 59; // A5

const int SWITCH_X_1 = 60; // A6
const int SWITCH_X_2 = 61; // A7
const int SWITCH_Y_1 = 62; // A8
const int SWITCH_Y_2 = 63; // A9

const int SERVO_PIN = 2;

const int RX_PIN = 68; // A14
const int TX_PIN = 69; // A15

SoftwareSerial mySerial (RX_PIN, TX_PIN);
// const int NEOPIXEL_PIN = 65; // A11
// const int NEOPIXEL_NUM = 72;
// int neoPixelBrightness = 20;

// Adafruit_NeoPixel pixels(NEOPIXEL_NUM, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);


Bounce debouncerXPlus = Bounce();
Bounce debouncerXMin = Bounce();
Bounce debouncerYPlus = Bounce();
Bounce debouncerYMinus = Bounce();
Bounce2::Button debouncerGrab = Bounce2::Button();
Bounce debouncerReset = Bounce();
Bounce debouncerSX1 = Bounce();
Bounce debouncerSX2 = Bounce();
Bounce debouncerSY1 = Bounce();
Bounce debouncerSY2 = Bounce();
  
const int MOVE_STEPS = 10;

Servo servo;

bool isGrabbing = false;
const int GRAB_ANGLE = 70;
const int RELEASE_ANGLE = 110;

int softSerialWriteTime = 0;
const int SOFT_SERIAL_WRITE_INTERVAL = 500;

long lastPrintTime = 0;

void setupDebouncers()
{
  debouncerXPlus.attach(BUTTON_X_PLUS, INPUT_PULLUP);
  debouncerXPlus.interval(5);
  debouncerXMin.attach(BUTTON_X_MINUS, INPUT_PULLUP);
  debouncerXMin.interval(5);
  debouncerYPlus.attach(BUTTON_Y_PLUS, INPUT_PULLUP);
  debouncerYPlus.interval(5);
  debouncerYMinus.attach(BUTTON_Y_MINUS, INPUT_PULLUP);
  debouncerYMinus.interval(5);
  debouncerGrab.attach(BUTTON_GRAB, INPUT_PULLUP);
  debouncerGrab.interval(5);
  debouncerGrab.setPressedState(false);
  debouncerReset.attach(BUTTON_RESET, INPUT_PULLUP);
  debouncerReset.interval(5);
  debouncerSX1.attach(SWITCH_X_1, INPUT_PULLUP);
  debouncerSX1.interval(5);
  debouncerSX2.attach(SWITCH_X_2, INPUT_PULLUP);
  debouncerSX2.interval(5);
  debouncerSY1.attach(SWITCH_Y_1, INPUT_PULLUP);
  debouncerSY1.interval(5);
  debouncerSY2.attach(SWITCH_Y_2, INPUT_PULLUP);
  debouncerSY2.interval(5);
}

void updateDebouncers()
{
  debouncerXPlus.update();
  debouncerXMin.update();
  debouncerYPlus.update();
  debouncerYMinus.update();
  debouncerGrab.update();
  debouncerReset.update();
  debouncerSX1.update();
  debouncerSX2.update();
  debouncerSY1.update();
  debouncerSY2.update();
}


int steps = 0;

void move(int dir, int ena, int pul, int steps, bool isForward)
{
  for (int i = 0; i < steps; i++)
  {
    digitalWrite(dir, isForward ? LOW : HIGH);
    digitalWrite(ena, HIGH);
    digitalWrite(pul, HIGH);
    delayMicroseconds(50);
    digitalWrite(pul, LOW);
    delayMicroseconds(50);
  }
}

void moveX(int steps, bool isForward)
{
  move(DIR_X, ENA_X, PUL_X, steps, isForward);
}

void moveY(int steps, bool isForward)
{
  move(DIR_Y, ENA_Y, PUL_Y, steps, isForward);
}

void freeX()
{
  digitalWrite(ENA_X, LOW);
  digitalWrite(PUL_X, LOW);
}

void freeY()
{
  digitalWrite(ENA_Y, LOW);
  digitalWrite(PUL_Y, LOW);
}

void moveToOrigin()
{
  servo.write(RELEASE_ANGLE);

  // X
  // bool isSwitchX1Press = debouncerSX1.read() == HIGH;
  bool isSwitchX2Press = debouncerSX2.read() == LOW; // Caution: The switch is inverted
  while (!isSwitchX2Press)
  {
    updateDebouncers();
    // isSwitchX1Press = debouncerSX1.read() == HIGH;
    isSwitchX2Press = debouncerSX2.read() == LOW; // Caution: The switch is inverted
    // Serial.print("isSwitchX1Press: "); Serial.print(isSwitchX1Press); Serial.print(" ");
    // Serial.print("isSwitchX2Press: "); Serial.println(isSwitchX2Press);
    
    Serial.println("Moving X");
    moveX(MOVE_STEPS, true);
    digitalWrite(LED, HIGH);
  }
  freeX();
  digitalWrite(LED, LOW);
  
  // Y
  // bool isSwitchY1Press = debouncerSY1.read() == HIGH;
  bool isSwitchY2Press = debouncerSY2.read() == HIGH;
  while (!isSwitchY2Press)
  {
    updateDebouncers();
    // isSwitchY1Press = debouncerSY1.read() == HIGH;
    isSwitchY2Press = debouncerSY2.read() == HIGH;
    // Serial.print("isSwitchY1Press: ");  Serial.print(isSwitchY1Press); Serial.print(" ");
    // Serial.print("isSwitchY2Press: "); Serial.println(isSwitchY2Press);
    
    Serial.println("Moving Y");
    moveY(MOVE_STEPS, false);
    digitalWrite(LED, HIGH);
  }
  freeY();
  digitalWrite(LED, LOW);
}

void setup()
{
  Serial.begin(115200);
  
  // pixels.begin(); // NeoPixel出力ピンの初期化
  // pixels.setBrightness(neoPixelBrightness);
  
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  mySerial.begin(115200);


  pinMode(LED, OUTPUT);

  // Motor X
  pinMode(PUL_X, OUTPUT);
  pinMode(DIR_X, OUTPUT);
  pinMode(ENA_X, OUTPUT);
  digitalWrite(ENA_X, LOW);
  digitalWrite(PUL_X, LOW);

  // Motor Y
  pinMode(PUL_Y, OUTPUT);
  pinMode(DIR_Y, OUTPUT);
  pinMode(ENA_Y, OUTPUT);
  digitalWrite(ENA_Y, LOW);
  digitalWrite(PUL_Y, LOW);

  setupDebouncers();

  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(90);

  delay(2000);

  moveToOrigin();
}

void loop() {
  updateDebouncers();

  bool isSwitchX1Press = debouncerSX1.read() == HIGH;
  bool isSwitchX2Press = debouncerSX2.read() == LOW; // Caution: The switch is inverted
  bool isSwitchY1Press = debouncerSY1.read() == HIGH;
  bool isSwitchY2Press = debouncerSY2.read() == HIGH;
  
  bool isXPlusPress = debouncerXPlus.read() == LOW;
  bool isXMinusPress = debouncerXMin.read() == LOW;
  bool isYPlusPress = debouncerYPlus.read() == LOW;
  bool isYMinusPress = debouncerYMinus.read() == LOW;
  bool isGrabPress = debouncerGrab.pressed();//read() == LOW;
  bool isResetPress = debouncerReset.read() == LOW;
  
  long time = millis();
  if (time - lastPrintTime > 500) {
    Serial.println("====================================");
    Serial.print("isSwitchX1Press: "); Serial.println(isSwitchX1Press);
    Serial.print("isSwitchX2Press: "); Serial.println(isSwitchX2Press);
    Serial.print("isSwitchY1Press: ");  Serial.println(isSwitchY1Press);
    Serial.print("isSwitchY2Press: "); Serial.println(isSwitchY2Press);
    
    Serial.print("isXPlusPress: "); Serial.println(isXPlusPress);
    Serial.print("isXMinusPress: ");  Serial.println(isXMinusPress);
    Serial.print("isYPlusPress: "); Serial.println(isYPlusPress);
    Serial.print("isYMinusPress: "); Serial.println(isYMinusPress);
    
    Serial.print("isGrabPress: "); Serial.println(isGrabPress);
    Serial.print("isResetPress: "); Serial.println(isResetPress);
    lastPrintTime = time;
  }

  // Reset 
  if (isResetPress) {
    moveToOrigin();
  }

  // Hand
  bool isGrabbingPrev = isGrabbing;
  if (isGrabPress) {
    isGrabbing = !isGrabbing;
  }
  if (isGrabbingPrev != isGrabbing) {
    if (isGrabbing) {
      servo.write(GRAB_ANGLE);
    } else {
      servo.write(RELEASE_ANGLE);
    }
  }

  if (millis() - softSerialWriteTime >= SOFT_SERIAL_WRITE_INTERVAL) {
    softSerialWriteTime = millis();
    if (isGrabbing) {
      mySerial.write(uint8_t(1));
    } else {
      mySerial.write(uint8_t(0));
    }
  }


  // Move the motors
  if (isXPlusPress) {
    // Move the motor
    digitalWrite(LED, HIGH);
    if (!isSwitchX2Press)
      moveX((int)(MOVE_STEPS * 0.7), true);
  } else {
    // Stop the motor
    digitalWrite(LED, LOW);
    freeX();
  }

  if (isXMinusPress) {
    // Move the motor
    digitalWrite(LED, HIGH);
    if (!isSwitchX1Press)
      moveX((int)(MOVE_STEPS * 0.7), false);
  } else {
    // Stop the motor
    digitalWrite(LED, LOW);
    freeX();
  }

  if (isYPlusPress) {
    // Move the motor
    digitalWrite(LED, HIGH);
    if (!isSwitchY2Press)
      moveY((int)(MOVE_STEPS * 2), false);
  } else {
    // Stop the motor
    digitalWrite(LED, LOW);
    freeY();
  }

  if (isYMinusPress) {
    // Move the motor
    digitalWrite(LED, HIGH);
    if (!isSwitchY1Press)
      moveY((int)(MOVE_STEPS * 2), true);
  } else {
    // Stop the motor
    digitalWrite(LED, LOW);
    freeY();
  }

  // delayMicroseconds(500);

  // for(int i=0; i<NEOPIXEL_NUM; i++) {
  //   float fv1 = cos(millis() / 1000.0 * 2 * PI);
  //   int v1 = (fv1 + 1) * 127.5;
  //   float fv2 = cos(millis() / 1000.0 * 2 * PI + PI / 3);
  //   int v2 = (fv2 + 1) * 127.5;
  //   float fv3 = cos(millis() / 1000.0 * 2 * PI + PI * 2 / 3);
  //   int v3 = (fv3 + 1) * 127.5;
  //   pixels.setPixelColor(i, pixels.Color(v1, v2, v3));
  // }
  // pixels.show();

}