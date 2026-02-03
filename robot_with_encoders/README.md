# Robot with HC-020K Encoders - Setup Guide

## Overview
This Arduino sketch adds HC-020K encoder support to your obstacle-avoidance robot for improved straight-line stability and movement performance.

## Features
- **Encoder-Based Speed Stabilization**: Uses HC-020K photoelectric encoders to monitor wheel rotation
- **Automatic Speed Correction**: Dynamically adjusts motor speeds to keep the robot moving straight
- **IR Obstacle Avoidance**: Maintains original obstacle detection and avoidance behavior
- **No Delay Approach**: Uses non-blocking timing for smooth operation

## Hardware Requirements

### Components
1. Arduino Uno/Nano (or compatible)
2. L298N Motor Driver
3. 2x DC Motors with wheels
4. 2x HC-020K Photoelectric Encoders
5. IR Analog Sensor (connected to A0)
6. Power supply for motors

## HC-020K Encoder Specifications
- **Type**: Photoelectric speed sensor
- **Output**: Digital pulse (HIGH/LOW)
- **Operating Voltage**: 3.3V - 5V
- **Detection Method**: Optical (infrared LED + phototransistor)
- **Use**: Counts pulses as the wheel rotates past the sensor

## Wiring Diagram

### L298N Motor Driver (same as before)
```
Arduino -> L298N
Pin 5 (ENA) -> Enable A
Pin 7 (IN1) -> Input 1
Pin 8 (IN2) -> Input 2
Pin 6 (ENB) -> Enable B
Pin 9 (IN3) -> Input 3
Pin 10 (IN4) -> Input 4
```

### HC-020K Encoders (NEW)
```
Left Encoder:
- VCC -> 5V
- GND -> GND
- OUT -> Arduino Pin 2 (INT0)

Right Encoder:
- VCC -> 5V
- GND -> GND
- OUT -> Arduino Pin 3 (INT1)
```

### IR Sensor (same as before)
```
IR Sensor:
- VCC -> 5V
- GND -> GND
- OUT -> Arduino Pin A0
```

## Encoder Mounting
1. Mount each HC-020K encoder near the motor shaft or wheel
2. Attach an encoder disk (slotted wheel) to the motor shaft
3. Position the encoder so the disk passes through the optical gap
4. Ensure the disk rotates freely without touching the encoder
5. Typical encoder disk: 20 slots = 20 pulses per revolution

## How It Works

### Encoder Pulse Counting
- Each time a slot passes through the encoder, it generates a pulse
- Interrupt handlers count these pulses in real-time
- Higher pulse count = faster wheel rotation

### Speed Stabilization Algorithm
1. Every 100ms, compare pulse counts from left and right encoders
2. Calculate the difference between wheels
3. Apply proportional correction:
   - If left wheel is faster: decrease left speed, increase right speed
   - If right wheel is faster: increase left speed, decrease right speed
4. Maximum correction: ±30 PWM units

### Benefits
- **Straight Movement**: Corrects for motor differences and friction variations
- **Consistent Speed**: Maintains desired speed despite battery voltage changes
- **Better Accuracy**: Improved positioning during obstacle avoidance maneuvers

## Configuration

### Adjustable Parameters

```cpp
// Encoder check frequency (milliseconds)
const unsigned long ENCODER_CHECK_INTERVAL = 100;

// Maximum speed adjustment (PWM units)
const int MAX_CORRECTION = 30;

// Base speeds
const int SPEED_FWD  = 120;
const int SPEED_BACK = 115;
const int SPEED_TURN = 110;
```

### Tuning Tips

1. **If robot drifts left/right:**
   - The encoder correction will automatically compensate
   - If still drifting, adjust `SPEED_FWD_L` or `SPEED_FWD_R` slightly

2. **If corrections are too aggressive:**
   - Reduce `MAX_CORRECTION` (try 20 or 15)
   - Increase `ENCODER_CHECK_INTERVAL` to 150ms

3. **If corrections are too slow:**
   - Increase `MAX_CORRECTION` (try 40 or 50)
   - Decrease `ENCODER_CHECK_INTERVAL` to 50ms

4. **Encoder sensitivity:**
   - Use encoder disks with more slots for finer control (20 slots recommended)
   - Ensure encoders are properly aligned and secured

## States and Behavior

The robot operates in four states:

1. **FWD**: Move forward with encoder-based speed stabilization
2. **BACK**: Back up after detecting obstacle (no correction during backup)
3. **TURN**: Turn left or right to avoid obstacle
4. **COOLDOWN**: Move forward briefly while ignoring sensor (prevents false triggers)

## Troubleshooting

### Encoders Not Working
- Check encoder wiring (VCC, GND, OUT)
- Verify encoder disk alignment
- Test encoder with Serial monitor: print `encoderCountLeft` and `encoderCountRight`
- Ensure encoder pins (2, 3) are not used by other components

### Robot Still Not Moving Straight
- Verify both encoders are generating pulses (add Serial.println for debugging)
- Check that motors are securely mounted
- Ensure wheels have equal diameter and good traction
- Try adjusting `MAX_CORRECTION` value

### Robot Moves Erratically
- Reduce `MAX_CORRECTION` to prevent overcompensation
- Increase `ENCODER_CHECK_INTERVAL` for smoother corrections
- Check for loose encoder mounting

### No Obstacle Detection
- Verify IR sensor wiring
- Adjust `IR_THRESH` based on your sensor readings
- Original issue not related to encoders

## Testing Procedure

1. **Upload the sketch** to your Arduino
2. **Power on** the robot
3. **Place robot** on a flat, open surface
4. **Observe** straight-line movement - should be more stable than before
5. **Test obstacle avoidance** by placing an object in front
6. **Monitor corrections** by adding Serial.println() to see encoder counts

## Optional: Debug Mode

To see encoder values in real-time, add to `setup()`:
```cpp
Serial.begin(9600);
```

And in `loop()` after encoder check:
```cpp
if (state == FWD && (now - lastEncoderCheck >= ENCODER_CHECK_INTERVAL)) {
    updateEncoderCorrection();
    lastEncoderCheck = now;
    
    // Debug output
    Serial.print("L: "); Serial.print(encoderCountLeft);
    Serial.print(" R: "); Serial.print(encoderCountRight);
    Serial.print(" CorrL: "); Serial.print(speedCorrectionLeft);
    Serial.print(" CorrR: "); Serial.println(speedCorrectionRight);
}
```

## License
Open source - feel free to modify and adapt for your needs.

## Credits
Based on original obstacle avoidance code with encoder stabilization enhancement.
