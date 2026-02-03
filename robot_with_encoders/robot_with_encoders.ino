// IR OBSTACLE AVOIDANCE WITH HC-020K ENCODER SUPPORT
// Adds encoder-based speed stabilization for better straight-line performance
// HC-020K encoders provide pulse feedback for wheel rotation monitoring
// =====================================================

// ===== L298N PINS (your wiring) =====
const int ENA = 5;
const int IN1 = 7;
const int IN2 = 8;

const int ENB = 6;
const int IN3 = 9;
const int IN4 = 10;

// ===== IR ANALOG SENSOR =====
const int IR_A = A0;

// ===== HC-020K ENCODER PINS =====
// Connect left encoder OUT to pin 2 (interrupt 0)
// Connect right encoder OUT to pin 3 (interrupt 1)
const int ENCODER_LEFT  = 2;
const int ENCODER_RIGHT = 3;

// ===== ENCODER VARIABLES =====
volatile long encoderCountLeft  = 0;
volatile long encoderCountRight = 0;
unsigned long lastEncoderCheck = 0;
const unsigned long ENCODER_CHECK_INTERVAL = 100;  // Check every 100ms

// Speed correction factors
int speedCorrectionLeft  = 0;
int speedCorrectionRight = 0;
const int MAX_CORRECTION = 30;  // Maximum speed adjustment

// ===== THRESHOLD (based on your values) =====
const int IR_THRESH = 100;          // obstacle if >= 100
const unsigned long IR_STABLE_MS = 120;

// ===== SPEEDS (your choice) =====
const int SPEED_FWD  = 120;
const int SPEED_BACK = 115;
const int SPEED_TURN = 110;

// If it doesn't go straight, trim slightly:
int SPEED_FWD_L = SPEED_FWD;
int SPEED_FWD_R = SPEED_FWD;

// If right motor direction is opposite, set true
const bool RIGHT_REVERSED = false;

// ===== TIMINGS (your measures) =====
const unsigned long BACK_MS     = 220;
const unsigned long TURN_MS     = 280;
const unsigned long COOLDOWN_MS = 700;

// ===== STATE =====
enum State { FWD, BACK, TURN, COOLDOWN };
State state = FWD;
unsigned long tState = 0;
bool turnRightNext = true;

// ===== ENCODER INTERRUPT HANDLERS =====
void leftEncoderISR() {
  encoderCountLeft++;
}

void rightEncoderISR() {
  encoderCountRight++;
}

// ---------------- Motor helpers ----------------
void setRightForwardPins() {
  if (!RIGHT_REVERSED) { digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
  else                 { digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
}
void setRightBackwardPins() {
  if (!RIGHT_REVERSED) { digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
  else                 { digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
}

void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  setRightForwardPins();
  
  // Apply speed with encoder-based corrections
  int leftSpeed = constrain(SPEED_FWD_L + speedCorrectionLeft, 0, 255);
  int rightSpeed = constrain(SPEED_FWD_R + speedCorrectionRight, 0, 255);
  
  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

void backward() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  setRightBackwardPins();
  analogWrite(ENA, SPEED_BACK);
  analogWrite(ENB, SPEED_BACK);
}

void turnRight() {
  // pivot: left forward, right backward
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  setRightBackwardPins();
  analogWrite(ENA, SPEED_TURN);
  analogWrite(ENB, SPEED_TURN);
}

void turnLeft() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  setRightForwardPins();
  analogWrite(ENA, SPEED_TURN);
  analogWrite(ENB, SPEED_TURN);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

// ---------------- Encoder Speed Stabilization ----------------
void updateEncoderCorrection() {
  static long lastCountLeft = 0;
  static long lastCountRight = 0;
  
  // Calculate speed difference based on encoder pulses
  long deltaLeft = encoderCountLeft - lastCountLeft;
  long deltaRight = encoderCountRight - lastCountRight;
  
  lastCountLeft = encoderCountLeft;
  lastCountRight = encoderCountRight;
  
  // Calculate the difference
  long diff = deltaLeft - deltaRight;
  
  // Apply proportional correction
  // If left wheel is faster (more pulses), slow it down
  // If right wheel is faster, slow it down
  if (diff > 0) {
    // Left is faster, slow down left, speed up right
    speedCorrectionLeft = constrain(speedCorrectionLeft - 2, -MAX_CORRECTION, MAX_CORRECTION);
    speedCorrectionRight = constrain(speedCorrectionRight + 2, -MAX_CORRECTION, MAX_CORRECTION);
  } else if (diff < 0) {
    // Right is faster, slow down right, speed up left
    speedCorrectionLeft = constrain(speedCorrectionLeft + 2, -MAX_CORRECTION, MAX_CORRECTION);
    speedCorrectionRight = constrain(speedCorrectionRight - 2, -MAX_CORRECTION, MAX_CORRECTION);
  }
  // If diff == 0, speeds are matched, no correction needed
}

void resetEncoderCorrection() {
  speedCorrectionLeft = 0;
  speedCorrectionRight = 0;
}

// ---------------- Analog IR stable confirm ----------------
bool obstacleStable(unsigned long now) {
  static bool tracking = false;
  static unsigned long t0 = 0;

  int v = analogRead(IR_A);
  bool hit = (v >= IR_THRESH);

  if (hit) {
    if (!tracking) { tracking = true; t0 = now; }
    return (now - t0 >= IR_STABLE_MS);
  } else {
    tracking = false;
    return false;
  }
}

void setup() {
  // Setup motor pins
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // Setup encoder pins with pull-up resistors
  pinMode(ENCODER_LEFT, INPUT_PULLUP);
  pinMode(ENCODER_RIGHT, INPUT_PULLUP);
  
  // Attach interrupts for encoders
  // INT0 on pin 2, INT1 on pin 3 (Arduino Uno/Nano)
  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), leftEncoderISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), rightEncoderISR, RISING);

  state = FWD;
  tState = millis();
  lastEncoderCheck = millis();
  
  // Initialize encoder counts
  encoderCountLeft = 0;
  encoderCountRight = 0;
}

void loop() {
  unsigned long now = millis();

  // Periodically update encoder-based speed correction (only during FWD state)
  if (state == FWD && (now - lastEncoderCheck >= ENCODER_CHECK_INTERVAL)) {
    updateEncoderCorrection();
    lastEncoderCheck = now;
  }

  switch (state) {
    case FWD:
      forward();
      if (obstacleStable(now)) {
        state = BACK;
        tState = now;
        resetEncoderCorrection();  // Reset corrections when changing state
      }
      break;

    case BACK:
      backward();
      if (now - tState >= BACK_MS) {
        state = TURN;
        tState = now;
      }
      break;

    case TURN:
      if (turnRightNext) turnRight();
      else               turnLeft();

      if (now - tState >= TURN_MS) {
        turnRightNext = !turnRightNext;
        state = COOLDOWN;
        tState = now;
      }
      break;

    case COOLDOWN:
      // keep moving forward, ignore sensor briefly (prevents jitter)
      forward();
      if (now - tState >= COOLDOWN_MS) {
        state = FWD;
        // Reset encoder counts for fresh comparison
        encoderCountLeft = 0;
        encoderCountRight = 0;
        lastEncoderCheck = now;
      }
      break;
  }
}
