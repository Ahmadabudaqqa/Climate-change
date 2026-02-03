// ADVANCED VERSION WITH SERIAL DEBUGGING
// This version includes Serial output for monitoring encoder performance
// Uncomment/comment features as needed
// =====================================================

// ===== L298N PINS =====
const int ENA = 5;
const int IN1 = 7;
const int IN2 = 8;
const int ENB = 6;
const int IN3 = 9;
const int IN4 = 10;

// ===== IR ANALOG SENSOR =====
const int IR_A = A0;

// ===== HC-020K ENCODER PINS =====
const int ENCODER_LEFT  = 2;
const int ENCODER_RIGHT = 3;

// ===== CONFIGURATION =====
// Enable/disable features
#define ENABLE_SERIAL_DEBUG false    // Set to true for debugging
#define ENABLE_ENCODER_STABILIZATION true  // Set to false to disable encoders

// ===== ENCODER VARIABLES =====
volatile long encoderCountLeft  = 0;
volatile long encoderCountRight = 0;
unsigned long lastEncoderCheck = 0;
const unsigned long ENCODER_CHECK_INTERVAL = 100;

// Speed correction factors
int speedCorrectionLeft  = 0;
int speedCorrectionRight = 0;
const int MAX_CORRECTION = 30;

// PID-like control gains (advanced tuning)
const float KP = 2.0;  // Proportional gain
const float KI = 0.1;  // Integral gain (accumulates error over time)
const float KD = 0.5;  // Derivative gain (responds to rate of change)

long lastError = 0;
long errorIntegral = 0;

// ===== THRESHOLD =====
const int IR_THRESH = 100;
const unsigned long IR_STABLE_MS = 120;

// ===== SPEEDS =====
const int SPEED_FWD  = 120;
const int SPEED_BACK = 115;
const int SPEED_TURN = 110;

int SPEED_FWD_L = SPEED_FWD;
int SPEED_FWD_R = SPEED_FWD;

const bool RIGHT_REVERSED = false;

// ===== TIMINGS =====
const unsigned long BACK_MS     = 220;
const unsigned long TURN_MS     = 280;
const unsigned long COOLDOWN_MS = 700;

// ===== STATE =====
enum State { FWD, BACK, TURN, COOLDOWN };
State state = FWD;
unsigned long tState = 0;
bool turnRightNext = true;

// ===== ENCODER ISR =====
void leftEncoderISR() {
  encoderCountLeft++;
}

void rightEncoderISR() {
  encoderCountRight++;
}

// ===== MOTOR HELPERS =====
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
  
  #if ENABLE_ENCODER_STABILIZATION
    int leftSpeed = constrain(SPEED_FWD_L + speedCorrectionLeft, 0, 255);
    int rightSpeed = constrain(SPEED_FWD_R + speedCorrectionRight, 0, 255);
  #else
    int leftSpeed = SPEED_FWD_L;
    int rightSpeed = SPEED_FWD_R;
  #endif
  
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

// ===== ENCODER CORRECTION (Simple Proportional) =====
void updateEncoderCorrectionSimple() {
  static long lastCountLeft = 0;
  static long lastCountRight = 0;
  
  long deltaLeft = encoderCountLeft - lastCountLeft;
  long deltaRight = encoderCountRight - lastCountRight;
  
  lastCountLeft = encoderCountLeft;
  lastCountRight = encoderCountRight;
  
  long diff = deltaLeft - deltaRight;
  
  if (diff > 0) {
    speedCorrectionLeft = constrain(speedCorrectionLeft - 2, -MAX_CORRECTION, MAX_CORRECTION);
    speedCorrectionRight = constrain(speedCorrectionRight + 2, -MAX_CORRECTION, MAX_CORRECTION);
  } else if (diff < 0) {
    speedCorrectionLeft = constrain(speedCorrectionLeft + 2, -MAX_CORRECTION, MAX_CORRECTION);
    speedCorrectionRight = constrain(speedCorrectionRight - 2, -MAX_CORRECTION, MAX_CORRECTION);
  }
  
  #if ENABLE_SERIAL_DEBUG
    Serial.print("EncL:");  Serial.print(deltaLeft);
    Serial.print(" EncR:"); Serial.print(deltaRight);
    Serial.print(" Diff:"); Serial.print(diff);
    Serial.print(" CorrL:"); Serial.print(speedCorrectionLeft);
    Serial.print(" CorrR:"); Serial.println(speedCorrectionRight);
  #endif
}

// ===== ENCODER CORRECTION (PID) =====
void updateEncoderCorrectionPID() {
  static long lastCountLeft = 0;
  static long lastCountRight = 0;
  
  long deltaLeft = encoderCountLeft - lastCountLeft;
  long deltaRight = encoderCountRight - lastCountRight;
  
  lastCountLeft = encoderCountLeft;
  lastCountRight = encoderCountRight;
  
  // Error: difference between left and right wheel speeds
  long error = deltaLeft - deltaRight;
  
  // Integral: accumulate error over time
  errorIntegral += error;
  errorIntegral = constrain(errorIntegral, -100, 100);  // Prevent integral windup
  
  // Derivative: rate of change of error
  long errorDerivative = error - lastError;
  lastError = error;
  
  // PID correction
  float correction = (KP * error) + (KI * errorIntegral) + (KD * errorDerivative);
  
  // Apply correction (positive correction = left is faster, slow it down)
  speedCorrectionLeft = constrain(-correction, -MAX_CORRECTION, MAX_CORRECTION);
  speedCorrectionRight = constrain(correction, -MAX_CORRECTION, MAX_CORRECTION);
  
  #if ENABLE_SERIAL_DEBUG
    Serial.print("Err:");  Serial.print(error);
    Serial.print(" Int:"); Serial.print(errorIntegral);
    Serial.print(" Der:"); Serial.print(errorDerivative);
    Serial.print(" Corr:"); Serial.println(correction);
  #endif
}

void resetEncoderCorrection() {
  speedCorrectionLeft = 0;
  speedCorrectionRight = 0;
  errorIntegral = 0;
  lastError = 0;
}

// ===== IR OBSTACLE DETECTION =====
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

// ===== SETUP =====
void setup() {
  #if ENABLE_SERIAL_DEBUG
    Serial.begin(9600);
    Serial.println("Robot with Encoders - Debug Mode");
  #endif
  
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  #if ENABLE_ENCODER_STABILIZATION
    pinMode(ENCODER_LEFT, INPUT_PULLUP);
    pinMode(ENCODER_RIGHT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), leftEncoderISR, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), rightEncoderISR, RISING);
  #endif

  state = FWD;
  tState = millis();
  lastEncoderCheck = millis();
  encoderCountLeft = 0;
  encoderCountRight = 0;
}

// ===== MAIN LOOP =====
void loop() {
  unsigned long now = millis();

  #if ENABLE_ENCODER_STABILIZATION
    if (state == FWD && (now - lastEncoderCheck >= ENCODER_CHECK_INTERVAL)) {
      // Choose which correction method to use:
      updateEncoderCorrectionSimple();  // Simple proportional control
      // updateEncoderCorrectionPID();   // More advanced PID control (uncomment to use)
      lastEncoderCheck = now;
    }
  #endif

  switch (state) {
    case FWD:
      forward();
      if (obstacleStable(now)) {
        #if ENABLE_SERIAL_DEBUG
          Serial.println("Obstacle detected!");
        #endif
        state = BACK;
        tState = now;
        resetEncoderCorrection();
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
      forward();
      if (now - tState >= COOLDOWN_MS) {
        state = FWD;
        encoderCountLeft = 0;
        encoderCountRight = 0;
        lastEncoderCheck = now;
      }
      break;
  }
}
