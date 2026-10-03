# Dasai Mochi Desk Robot

A gesture-driven desk robot built with ESP32 and OLED display, inspired by the Dasai Mochi.

## Hardware

- ESP32-WROOM-32 DevKit (30-pin)
- 0.96" I2C OLED display (128x64, SSD1306)
- TTP223 capacitive touch sensor
- 3.7V LiPo battery + TP4056 charging module
- Wire frame for structure

## Features

- 🎭 **Expressive animations** - Happy, angry, playful, tired, sleeping faces
- 👆 **Gesture detection** - Single tap, double tap, rapid taps, rubbing, holding
- 💤 **Idle behavior** - Auto-blink and look around when untouched
- 🔋 **Battery powered** - LiPo with USB-C charging
- 🎨 **Mochi-style** - Cute rounded eyes, smooth animations

## Quick Start

1. Install Arduino IDE and ESP32 board support
2. Install libraries: Adafruit SSD1306, Adafruit GFX
3. Open `dasai_mochi_robot.ino`
4. Select board: ESP32 Dev Module
5. Upload to your ESP32

See `BUILD_GUIDE.md` for complete wiring diagrams, test sketches, and troubleshooting.

## Gestures

- **Single tap** → Happy smile
- **Double tap** → Playful wink
- **Rapid taps** → Angry face
- **Rubbing** → Hearts and cat mouth
- **Hold (1.5s)** → Tired
- **Long hold (3s)** → Asleep with Zzz
- **Idle (20s)** → Auto animations

## Pin Connections

| ESP32 | Component |
|-------|-----------|
| GPIO21 | OLED SDA |
| GPIO22 | OLED SCL |
| GPIO27 | Touch OUT |
| 3V3 | Power |
| GND | Ground |

## Project Structure

```
dasai_mochi_robot/
├── dasai_mochi_robot.ino    # Main robot code
├── BUILD_GUIDE.md            # Complete build documentation
├── README.md                 # This file
└── BetterMochi/              # Reference implementation
```

## Credits

Based on the Dasai Mochi concept. Reference implementation from [BetterMochi](https://github.com/NeoDoggy/BetterMochi) by NeoDoggy.

Adapted for standard ESP32 hardware with simplified touch-based gestures.

---

**Built:** 2026-10-03  
**License:** MIT
