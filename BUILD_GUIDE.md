# Dasai Mochi Desk Robot - Complete Build Guide
**Date:** 2026-10-03  
**Hardware:** ESP32-WROOM-32 (30-pin DevKit) + 0.96" I2C OLED + TTP223 Touch Sensor

---

## Hardware List

- ESP32 DevKit (ESP32-WROOM-32, 30-pin, CP2102 or CH340 USB)
- 0.96" I2C OLED display, 128x64, SSD1306, address 0x3C (4 pins: GND/VDD/SCK/SDA)
- TTP223 capacitive touch sensor module (digital OUT, 3.3V compatible)
- 3.7V LiPo battery
- TP4056 Type-C charging module with protection
- Slide switch (SPST)
- Stiff wire for frame (1.5-2.5mm brass/copper wire)
- Jumper wires, solder, soldering iron

---

## Pin Connections

| ESP32 GPIO | Component         | Pin/Signal   | Notes                                          |
|:----------:|:------------------|:-------------|:-----------------------------------------------|
| 21         | OLED              | SDA          | I2C data                                       |
| 22         | OLED              | SCL          | I2C clock                                      |
| 3V3        | OLED              | VDD          | +3.3V power                                    |
| GND        | OLED              | GND          | Ground                                         |
| 27         | Touch module      | OUT          | Digital input (active LOW)                     |
| 3V3        | Touch module      | VCC          | +3.3V power                                    |
| GND        | Touch module      | GND          | Ground                                         |
| VIN        | TP4056 OUT+       | —            | Battery power input                            |
| GND        | TP4056 OUT−       | —            | Common ground                                  |

### Power Path
```
LiPo (+) ──[Slide Switch]── TP4056 BAT+
LiPo (−) ─────────────────── TP4056 BAT−

TP4056 OUT+ ── ESP32 VIN
TP4056 OUT− ── ESP32 GND
```

**Safety Notes:**
- LiPo voltage range: 3.0V (dead) to 4.2V (full charge)
- Never discharge below 3.0V or charge above 4.2V
- Keep slide switch OFF during charging
- Never short LiPo terminals

---

## Arduino IDE Setup

### 1. Install ESP32 Board Package
- **File** → **Preferences**
- Add to "Additional Boards Manager URLs":
  ```
  https://dl.espressif.com/dl/package_esp32_index.json
  ```
- **Tools** → **Board Manager** → Search "esp32" → Install latest (e.g., 3.0.x)

### 2. Board Settings
- **Board:** ESP32 Dev Module
- **Upload Speed:** 921600 (or 115200 if upload fails)
- **CPU Frequency:** 240 MHz
- **Flash Size:** 4MB
- **Flash Mode:** QIO
- **Port:** Select your COM port

### 3. Install Libraries
Via **Sketch** → **Include Library** → **Manage Libraries**:
- Adafruit SSD1306 (v2.5.7+)
- Adafruit GFX Library (v1.11.5+)

---

## Test Sketches

### Test 1: I2C Scanner
Run this first to confirm OLED address (should show 0x3C):

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22); // SDA=21, SCL=22
  Serial.println("\nI2C Scanner started");
}

void loop() {
  Serial.println("Scanning I2C addresses...");
  int found = 0;
  for (byte i = 1; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device found at address 0x");
      Serial.println(i, HEX);
      found++;
    }
  }
  Serial.print("Found ");
  Serial.print(found);
  Serial.println(" device(s)\n");
  delay(5000);
}
```

### Test 2: OLED Display Test
Should show shapes and text:

```cpp
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 allocation failed");
    while (1);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Test OK!");
  display.display();
}

void loop() {
  display.clearDisplay();
  display.fillCircle(64, 32, 20, SSD1306_WHITE);
  display.display();
  delay(1000);
}
```

### Test 3: Touch Sensor Test
Open Serial Monitor (115200 baud), should show PRESSED/released:

```cpp
#define TOUCH_PIN 27

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);
  Serial.println("Touch sensor test");
}

void loop() {
  int reading = digitalRead(TOUCH_PIN);
  Serial.println(reading == LOW ? "PRESSED" : "released");
  delay(100);
}
```

---

## Gesture Behaviors

The robot detects these gestures from touch timing:

| Gesture | Duration/Pattern | Reaction |
|---------|------------------|----------|
| **Single Tap** | Quick tap < 250ms | Happy face with big smile + blushing cheeks |
| **Double Tap** | Two taps within 400ms | Playful wink |
| **Rapid Taps** | 3+ taps quickly (hitting) | Angry face with slanted eyes + frown |
| **Rubbing** | Continuous touch 800ms+ | Petted face with hearts + cat mouth |
| **Hold** | Hold 1.5-3 seconds | Tired droopy eyes |
| **Long Hold** | Hold 3+ seconds | Asleep with "Zzz" animation |
| **Idle** | No touch for 20 seconds | Auto-blink, look around |

---

## Main Code

The complete robot code is in:
```
C:\Users\new\dasai_mochi_robot\dasai_mochi_robot.ino
```

Key features:
- Non-blocking animation (uses `millis()`, never `delay()`)
- Gesture state machine with debouncing
- Improved Mochi-style faces with rounded eyes, smooth mouths
- Auto-idle behavior with blinking and looking around
- Serial debug output for gesture detection

---

## Mechanical Assembly

### Wireframe
1. Use 1.5-2.5mm brass or steel wire
2. Create a cube/rectangular frame ~40mm per side
3. Mount OLED on front face, centered
4. Mount TTP223 touch sensor on top surface (the "head")
5. Secure ESP32, TP4056, battery on back/inside with foam tape or zip-ties

### Touch Sensor Placement
- Position sensor on **top** of robot (like petting a head)
- Keep sensor PCB isolated from metal frame using plastic spacer or foam tape
- Metal frame touching the sensor will cause noise/false triggers

### Cable Management
- Twist SDA/SCL wires together
- Route touch sensor wire away from I2C lines
- Keep wires short
- Common ground point: battery GND → TP4056 GND → ESP32 GND → OLED GND → Touch GND

---

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| **OLED blank** | I2C connection or address wrong | Run I2C scanner, check GPIO 21/22, verify 0x3C address |
| **Touch fires randomly** | Noise, frame interference | Add 100nF capacitor across touch VCC/GND; isolate sensor from metal frame |
| **Upload fails** | Serial port in use | Close Serial Monitor; hold BOOT button during upload |
| **Random resets** | Brownout from low voltage | Add 100µF capacitor across ESP32 3V3/GND; check LiPo voltage |
| **Gestures wrong** | Timing threshold mismatch | Open Serial Monitor to see gesture logs; adjust `SINGLE_TAP_MAX_MS` etc. in code |

---

## How to Add Custom Expressions

1. Add a new mood state in the enum:
   ```cpp
   enum MoodState {
     MOOD_DEFAULT,
     MOOD_HAPPY,
     // ... existing ones
     MOOD_MY_CUSTOM,  // Add this
   };
   ```

2. Add a case in `SimpleEyes::draw_eye()` or `draw_mouth()`:
   ```cpp
   case MOOD_MY_CUSTOM:
     // Draw custom eye/mouth shape here
     disp.fillCircle(cx, cy, w/2, SSD1306_WHITE);
     break;
   ```

3. Trigger it from a gesture:
   ```cpp
   case GestureDetector::GESTURE_SINGLE_TAP:
     current_mood = MOOD_MY_CUSTOM;
     mood_changed_at = now;
     last_touch = now;
     break;
   ```

---

## Reference Images

The robot should look similar to Dasai Mochi:
- Wire cube frame with OLED face
- Touch sensor on top
- Simple, expressive pixelated faces
- Auto-animated idle behavior

---

## Credits & Resources

- Based on Dasai Mochi desk robot concept
- Reference repository: https://github.com/NeoDoggy/BetterMochi (ESP32-S3/M5Dial version)
- Adapted for standard ESP32 + I2C OLED + single touch sensor
- Built with Adafruit libraries

---

**Build date:** 2026-10-03  
**Conversation saved for reference**
