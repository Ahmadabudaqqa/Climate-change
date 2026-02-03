# Wiring Diagram - Robot with Encoders

## Complete Connection Diagram

```
                                    ARDUINO UNO/NANO
                                    +--------------+
                                    |              |
                    IR Sensor ------| A0       D13 |
                                    |              |
    Left Encoder OUT ---------------| D2       D12 |
    Right Encoder OUT --------------| D3       D11 |
                                    |              |
                                    | D4       D10 |------ IN4 (L298N)
           ENA (L298N) -------------| D5        D9 |------ IN3 (L298N)
           ENB (L298N) -------------| D6        D8 |------ IN2 (L298N)
           IN1 (L298N) -------------| D7           |
                                    |              |
    5V (Encoders + Sensors) --------| 5V       GND |------ GND (Common)
                                    |              |
                                    +--------------+


                        L298N MOTOR DRIVER
                        +------------------+
                        |                  |
                    ENA |  [   PWM A   ]   |--- Motor A +
                    IN1 |  [  INPUT A  ]   |
                    IN2 |  [  INPUT A  ]   |--- Motor A -
                        |                  |
                    +5V |  [   LOGIC   ]   | (Connect to Arduino 5V)
                    GND |  [   POWER   ]   | (Connect to Arduino GND)
                        |                  |
                    ENB |  [   PWM B   ]   |--- Motor B +
                    IN3 |  [  INPUT B  ]   |
                    IN4 |  [  INPUT B  ]   |--- Motor B -
                        |                  |
                    +12V|  [ MOTOR PWR ]   | (Connect to battery +)
                    GND |  [ MOTOR PWR ]   | (Connect to battery -)
                        +------------------+


            LEFT ENCODER (HC-020K)          RIGHT ENCODER (HC-020K)
            +---------------+                +---------------+
            |               |                |               |
            |   [LED]   [P] |                |   [LED]   [P] |
            |               |                |               |
            +---------------+                +---------------+
                 | | |                            | | |
                 | | |                            | | |
            VCC--+ | +--GND                  VCC--+ | +--GND
                   |                                |
                   +--OUT (D2)                      +--OUT (D3)


                    IR ANALOG SENSOR
                    +-------------+
                    |             |
                    |   [  IR  ]  |
                    |             |
                    +-------------+
                      | | |
                      | | |
                 VCC--+ | +--GND
                        |
                        +--OUT (A0)


               ENCODER DISK MOUNTING
               
        Motor Shaft        Encoder Position
            |                    __
            |                 __|HC|__
        +---+---+            |  020K  |
        | MOTOR |            |________|
        +---+---+                ||
            |                    ||
         [======]  <-- Encoder Disk (with slots)
         
    The disk should pass through the optical gap
    without touching the encoder sensor.
```

## Pin Summary Table

| Component | Pin Name | Arduino Pin | Notes |
|-----------|----------|-------------|-------|
| L298N Motor Driver | ENA | D5 | PWM for left motor speed |
| L298N Motor Driver | IN1 | D7 | Left motor direction |
| L298N Motor Driver | IN2 | D8 | Left motor direction |
| L298N Motor Driver | ENB | D6 | PWM for right motor speed |
| L298N Motor Driver | IN3 | D9 | Right motor direction |
| L298N Motor Driver | IN4 | D10 | Right motor direction |
| Left Encoder | OUT | D2 | Interrupt 0 |
| Right Encoder | OUT | D3 | Interrupt 1 |
| IR Sensor | OUT | A0 | Analog input |

## Power Connections

### Arduino Power
- **Option 1**: USB power from computer (for testing)
- **Option 2**: External 7-12V via VIN pin
- **Option 3**: 5V regulated power via 5V pin

### Motor Power
- Connect battery (7-12V) to L298N motor power input
- **Important**: Share common ground between Arduino and L298N
- Do NOT power motors from Arduino's 5V output

### Encoder & Sensor Power
- All encoders and sensors use Arduino's 5V pin
- Connect all GND pins together (common ground)

## Recommended Battery Setup

```
     BATTERY (7-12V, e.g., 2S LiPo or 6xAA)
           |     |
           +     -
           |     |
     +-----+     +-----+
     |                 |
     |                 |
  L298N            Arduino
  +12V              VIN
  Motor             (optional)
  Power             
     |                 |
     +--------+--------+
              |
            GND (Common Ground)
```

## Safety Notes

1. **Always connect grounds together** (Arduino GND, L298N GND, Battery -)
2. **Do not reverse polarity** - check + and - before powering
3. **Motor power separate** - Never connect motor power directly to Arduino
4. **Check voltage ratings** - HC-020K works with 3.3-5V
5. **Secure connections** - Use proper connectors or solder joints

## Step-by-Step Wiring Instructions

### Step 1: L298N Motor Driver
1. Connect ENA to Arduino D5
2. Connect IN1 to Arduino D7
3. Connect IN2 to Arduino D8
4. Connect ENB to Arduino D6
5. Connect IN3 to Arduino D9
6. Connect IN4 to Arduino D10
7. Connect L298N GND to Arduino GND
8. Connect L298N +5V to Arduino 5V

### Step 2: Left Encoder (HC-020K)
1. Connect VCC to Arduino 5V
2. Connect GND to Arduino GND
3. Connect OUT to Arduino D2

### Step 3: Right Encoder (HC-020K)
1. Connect VCC to Arduino 5V
2. Connect GND to Arduino GND
3. Connect OUT to Arduino D3

### Step 4: IR Sensor
1. Connect VCC to Arduino 5V
2. Connect GND to Arduino GND
3. Connect OUT to Arduino A0

### Step 5: Motors
1. Connect left motor to Motor A outputs on L298N
2. Connect right motor to Motor B outputs on L298N
3. Test motor direction and swap wires if needed

### Step 6: Power
1. Connect battery + to L298N +12V
2. Connect battery - to L298N GND (shared with Arduino GND)
3. Optionally power Arduino via VIN from same battery

### Step 7: Encoder Disks
1. Attach encoder disks to motor shafts
2. Position HC-020K sensors around disks
3. Ensure disks rotate freely through encoder gaps
4. Secure encoders to chassis

## Verification Checklist

- [ ] All GND connections are common
- [ ] Encoders connected to D2 and D3
- [ ] IR sensor connected to A0
- [ ] L298N motor pins connected correctly
- [ ] Power supply rated for motor current
- [ ] Encoder disks properly aligned
- [ ] No short circuits between + and -
- [ ] Code uploaded successfully

## Testing Individual Components

### Test Encoders Only
```cpp
void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(0, [](){encoderL++;}, RISING);
  attachInterrupt(1, [](){encoderR++;}, RISING);
}
volatile long encoderL=0, encoderR=0;
void loop() {
  Serial.print("L:"); Serial.print(encoderL);
  Serial.print(" R:"); Serial.println(encoderR);
  delay(500);
}
```
Manually rotate wheels and check if counts increase.

### Test Motors Only
```cpp
void setup() {
  pinMode(5, OUTPUT); pinMode(6, OUTPUT);
  pinMode(7, OUTPUT); pinMode(8, OUTPUT);
  pinMode(9, OUTPUT); pinMode(10, OUTPUT);
}
void loop() {
  // Left motor forward
  digitalWrite(7, HIGH); digitalWrite(8, LOW);
  analogWrite(5, 150);
  // Right motor forward  
  digitalWrite(9, HIGH); digitalWrite(10, LOW);
  analogWrite(6, 150);
  delay(2000);
  // Stop
  analogWrite(5, 0); analogWrite(6, 0);
  delay(1000);
}
```
Both wheels should rotate forward together.
