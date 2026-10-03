// ============================================================================
// Dasai Mochi ESP32 Desk Robot (No Speaker Version)
// Gesture-driven expressions with OLED animation
// ============================================================================

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// ============================================================================
// PIN CONFIGURATION
// ============================================================================

#define OLED_SDA_PIN    21
#define OLED_SCL_PIN    22
#define TOUCH_PIN       27

// ============================================================================
// OLED CONFIGURATION
// ============================================================================

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_ADDR       0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ============================================================================
// GESTURE & STATE MACHINE CONSTANTS
// ============================================================================

// Touch debounce and timing thresholds (milliseconds)
#define DEBOUNCE_MS           30
#define SINGLE_TAP_MAX_MS     250    // Single tap duration
#define DOUBLE_TAP_GAP_MS     400    // Gap between taps for double-tap detection
#define RAPID_TAP_INTERVAL_MS 300    // Rapid repeated taps (hitting)
#define HOLD_THRESHOLD_MS     1500   // Transition from touch to "hold"
#define LONG_HOLD_MS          3000   // Long hold -> sleep
#define RUBBING_MIN_DURATION  800    // Slow continuous touch (petting)
#define IDLE_TIMEOUT_MS       20000  // No touch for 20s -> idle animation

// ============================================================================
// MOOD STATES
// ============================================================================

enum MoodState {
  MOOD_DEFAULT,      // Neutral, idle
  MOOD_HAPPY,        // Happy, smiling
  MOOD_PLAYFUL,      // Winking, giggling
  MOOD_ANGRY,        // Angry face
  MOOD_CONFUSED,     // Confused expression
  MOOD_TIRED,        // Droopy, closing eyes
  MOOD_ASLEEP,       // Fully asleep
  MOOD_PETTED,       // Hearts, closing eyes
};

// ============================================================================
// BITMAP ANIMATION FRAMES (Placeholders)
// Each frame is 128x64 bits = 1024 bytes. Use image2cpp to generate.
// Format: 128x64, horizontal, 1 bit per pixel
// ============================================================================

// Placeholder: Happy face with big smile
// Replace with your image2cpp output
const uint8_t happy_frame[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // ... (1024 bytes total - this is just a starter)
};

// Placeholder: Heart eyes
const uint8_t hearts_frame[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // ...
};

// Placeholder: Sleeping (closed eyes, Z's)
const uint8_t sleeping_frame[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // ...
};

// ============================================================================
// ANIMATION & FRAME PLAYER
// ============================================================================

struct AnimationFrame {
  const uint8_t *bitmap;   // Pointer to PROGMEM bitmap
  uint16_t delay_ms;       // Delay before next frame
  uint16_t repeat_count;   // How many times to loop (0 = infinite)
};

class FramePlayer {
public:
  FramePlayer() : current_frame(0), repeat_counter(0), last_frame_time(0), frame_list(nullptr), frame_count(0) {}

  void start(const AnimationFrame *frames, int num_frames) {
    frame_list = frames;
    frame_count = num_frames;
    current_frame = 0;
    repeat_counter = 0;
    last_frame_time = millis();
  }

  bool update(Adafruit_SSD1306 &disp) {
    // Returns true if animation is complete, false if still playing
    if (!frame_list || frame_count == 0) return true;

    unsigned long now = millis();
    if (now - last_frame_time >= frame_list[current_frame].delay_ms) {
      // Draw current frame from PROGMEM
      disp.clearDisplay();
      draw_bitmap_from_progmem(disp, frame_list[current_frame].bitmap);
      disp.display();

      // Advance frame
      current_frame++;
      if (current_frame >= frame_count) {
        current_frame = 0;
        repeat_counter++;
        // Check if animation should stop
        if (frame_list[0].repeat_count > 0 && repeat_counter >= frame_list[0].repeat_count) {
          return true; // Animation complete
        }
      }
      last_frame_time = now;
    }
    return false;
  }

private:
  const AnimationFrame *frame_list;
  int frame_count;
  int current_frame;
  int repeat_counter;
  unsigned long last_frame_time;

  void draw_bitmap_from_progmem(Adafruit_SSD1306 &disp, const uint8_t *bitmap) {
    // Copy 128x64 bitmap from PROGMEM and draw to display buffer
    for (int page = 0; page < 8; page++) {
      for (int col = 0; col < 128; col++) {
        uint8_t byte_val = pgm_read_byte(&bitmap[page * 128 + col]);
        for (int bit = 0; bit < 8; bit++) {
          if (byte_val & (1 << bit)) {
            disp.drawPixel(col, page * 8 + bit, SSD1306_WHITE);
          }
        }
      }
    }
  }
};

FramePlayer frame_player;

// ============================================================================
// SIMPLE EYE DRAWING (RoboEyes fallback)
// ============================================================================

class SimpleEyes {
public:
  SimpleEyes() : blink_phase(0), look_dir(0), last_blink(0), last_look(0), sleeping_phase(0) {}

  void draw(Adafruit_SSD1306 &disp, MoodState mood) {
    unsigned long now = millis();

    // Auto-blink every 3-4 seconds
    if (now - last_blink > 3500) {
      blink_phase = 1.0; // Start blink
      last_blink = now;
    }

    // Auto look around every 5-6 seconds (idle)
    if (now - last_look > 5500 && mood == MOOD_DEFAULT) {
      look_dir = (look_dir + 1) % 3; // 0=center, 1=left, 2=right
      last_look = now;
    }

    // Calculate blink animation (duration ~200ms)
    if (now - last_blink < 200) {
      blink_phase = 1.0 - ((float)(now - last_blink) / 200.0);
    } else {
      blink_phase = 0;
    }

    // Zzz animation for sleeping
    sleeping_phase = (now / 500) % 3; // Cycles 0, 1, 2 every 1.5 seconds

    int left_x = 34, right_x = 94, eye_y = 28;
    int eye_w = 20, eye_h = 24; // Bigger, rounder Mochi eyes

    // Draw left eye
    draw_eye(disp, left_x, eye_y, eye_w, eye_h, mood, blink_phase, look_dir);

    // Draw right eye
    draw_eye(disp, right_x, eye_y, eye_w, eye_h, mood, blink_phase, look_dir);

    // Draw mouth based on mood
    draw_mouth(disp, 64, 48, mood);

    // Draw extras (like Zzzs for sleeping)
    if (mood == MOOD_ASLEEP) {
      disp.setTextSize(1);
      disp.setTextColor(SSD1306_WHITE);
      if (sleeping_phase >= 0) { disp.setCursor(90, 10); disp.print("z"); }
      if (sleeping_phase >= 1) { disp.setCursor(100, 5); disp.print("z"); }
      if (sleeping_phase >= 2) { disp.setCursor(110, 0); disp.print("Z"); }
    }
  }

private:
  float blink_phase;
  int look_dir;
  unsigned long last_blink, last_look;
  int sleeping_phase;

  void draw_eye(Adafruit_SSD1306 &disp, int cx, int cy, int w, int h,
                MoodState mood, float blink, int look) {

    // Check if eye is blinking
    if (blink > 0.1 && mood != MOOD_ASLEEP && mood != MOOD_PETTED) {
      int h_blink = h * (1.0 - blink);
      if (h_blink < 2) h_blink = 2; // Flat line when fully blinked
      disp.fillRoundRect(cx - w/2, cy - h_blink/2, w, h_blink, h_blink/2, SSD1306_WHITE);
      return;
    }

    // Draw eye based on mood
    switch (mood) {
      case MOOD_TIRED:
      case MOOD_ASLEEP:
        // Closed horizontal line (sleepy)
        disp.fillRect(cx - w/2, cy + h/4, w, 4, SSD1306_WHITE);
        break;

      case MOOD_PETTED:
        // Happy closed eyes (thick upward arc) ⌒ ⌒
        for (int i = 0; i < 3; i++) { // Draw 3 overlapping arcs for thickness
          draw_arc(disp, cx, cy + 6 + i, w/2, 180, 360);
        }
        break;

      case MOOD_ANGRY:
        // Angry slanted eyes '\  /'
        if (cx < 64) {
          // Left eye slanted down towards center
          disp.fillRoundRect(cx - w/2, cy - h/4, w, 6, 2, SSD1306_WHITE);
          // Add angular top line
          disp.drawLine(cx - w/2, cy - h/2 - 2, cx + w/2 + 4, cy, SSD1306_WHITE);
          disp.drawLine(cx - w/2, cy - h/2 - 3, cx + w/2 + 4, cy - 1, SSD1306_WHITE);
        } else {
          // Right eye
          disp.fillRoundRect(cx - w/2, cy - h/4, w, 6, 2, SSD1306_WHITE);
          // Add angular top line
          disp.drawLine(cx - w/2 - 4, cy, cx + w/2, cy - h/2 - 2, SSD1306_WHITE);
          disp.drawLine(cx - w/2 - 4, cy - 1, cx + w/2, cy - h/2 - 3, SSD1306_WHITE);
        }
        break;

      case MOOD_PLAYFUL:
        // Wink: Left eye closed line, Right eye open
        if (cx < 64) {
          disp.fillRect(cx - w/2 + 2, cy + 2, w - 4, 4, SSD1306_WHITE); // Winking
        } else {
           // Open eye
          disp.fillRoundRect(cx - w/2, cy - h/2, w, h, 6, SSD1306_WHITE);
        }
        break;

      case MOOD_HAPPY:
      case MOOD_CONFUSED:
      default:
        // Standard wide-open Mochi eye
        disp.fillRoundRect(cx - w/2, cy - h/2, w, h, 6, SSD1306_WHITE);
        break;
    }
  }

  void draw_mouth(Adafruit_SSD1306 &disp, int cx, int cy, MoodState mood) {
    switch (mood) {
      case MOOD_HAPPY:
        // Big open smile 'D' shape
        disp.fillRoundRect(cx - 10, cy, 20, 12, 4, SSD1306_WHITE);
        disp.fillRect(cx - 10, cy, 20, 4, SSD1306_BLACK); // cut off top to make an open mouth
        disp.fillRect(cx - 10, cy - 2, 20, 3, SSD1306_WHITE); // draw straight top lip
        break;

      case MOOD_PLAYFUL:
      case MOOD_PETTED:
        // Small cat mouth '=w='
        disp.fillCircle(cx - 4, cy, 3, SSD1306_WHITE);
        disp.fillCircle(cx + 4, cy, 3, SSD1306_WHITE);
        disp.fillCircle(cx - 4, cy - 1, 3, SSD1306_BLACK); // hollow out
        disp.fillCircle(cx + 4, cy - 1, 3, SSD1306_BLACK); // hollow out
        break;

      case MOOD_ANGRY:
        // Small downward frown
        disp.fillRoundRect(cx - 6, cy + 4, 12, 4, 2, SSD1306_WHITE);
        break;

      case MOOD_CONFUSED:
        // Diagonal small mouth
        disp.drawLine(cx - 4, cy - 2, cx + 4, cy + 2, SSD1306_WHITE);
        disp.drawLine(cx - 4, cy - 1, cx + 4, cy + 3, SSD1306_WHITE);
        disp.drawLine(cx - 4, cy, cx + 4, cy + 4, SSD1306_WHITE);
        break;

      case MOOD_TIRED:
      case MOOD_ASLEEP:
      case MOOD_DEFAULT:
      default:
        // Tiny cute straight mouth '-'
        disp.fillRoundRect(cx - 4, cy + 2, 8, 3, 1, SSD1306_WHITE);
        break;
    }

    // Add blushing cheeks if Happy or Petted
    if (mood == MOOD_HAPPY || mood == MOOD_PETTED) {
      // Dithered cheek pattern (like a little hash mark)
      disp.drawLine(cx - 24, cy - 4, cx - 20, cy - 8, SSD1306_WHITE);
      disp.drawLine(cx - 21, cy - 4, cx - 17, cy - 8, SSD1306_WHITE);

      disp.drawLine(cx + 20, cy - 8, cx + 24, cy - 4, SSD1306_WHITE);
      disp.drawLine(cx + 17, cy - 8, cx + 21, cy - 4, SSD1306_WHITE);
    }
  }

  void draw_heart(Adafruit_SSD1306 &disp, int cx, int cy, int size) {
    // Simple heart shape
    disp.fillCircle(cx - size/2, cy, size/2, SSD1306_WHITE);
    disp.fillCircle(cx + size/2, cy, size/2, SSD1306_WHITE);
    disp.fillTriangle(cx - size, cy, cx + size, cy, cx, cy + size, SSD1306_WHITE);
  }

  // Helper to draw a smooth arc (for happy eyes)
  void draw_arc(Adafruit_SSD1306 &disp, int cx, int cy, int r, float start_angle, float end_angle) {
    for (float a = start_angle; a <= end_angle; a += 5) {
      float rad = a * PI / 180.0;
      int x = cx + r * cos(rad);
      int y = cy + r * sin(rad);
      disp.drawPixel(x, y, SSD1306_WHITE);
    }
  }
};

SimpleEyes eyes;

// ============================================================================
// GESTURE DETECTOR STATE MACHINE
// ============================================================================

class GestureDetector {
public:
  enum GestureType {
    GESTURE_NONE,
    GESTURE_SINGLE_TAP,
    GESTURE_DOUBLE_TAP,
    GESTURE_RAPID_TAPS,
    GESTURE_RUBBING,
    GESTURE_HOLD,
    GESTURE_LONG_HOLD,
  };

  GestureDetector()
    : state(STATE_IDLE),
      touch_start(0),
      last_release(0),
      tap_count(0),
      last_tap_end(0),
      debounce_until(0) {}

  GestureType update(bool touch_pressed) {
    unsigned long now = millis();

    // Debouncing
    if (now < debounce_until) return GESTURE_NONE;

    GestureType result = GESTURE_NONE;

    switch (state) {
      case STATE_IDLE:
        if (touch_pressed) {
          touch_start = now;
          debounce_until = now + DEBOUNCE_MS;
          state = STATE_TOUCHING;
        }
        break;

      case STATE_TOUCHING: {
        unsigned long touch_duration = now - touch_start;

        if (!touch_pressed) {
          // Released
          debounce_until = now + DEBOUNCE_MS;

          // Determine if this is a tap or end of rubbing
          if (touch_duration < SINGLE_TAP_MAX_MS) {
            // Tap detected
            tap_count++;
            last_tap_end = now;

            // Wait for second tap (double-tap window)
            state = STATE_TAP_RELEASED;
          } else if (touch_duration >= RUBBING_MIN_DURATION) {
            // Long continuous touch -> rubbing (petting)
            result = GESTURE_RUBBING;
            tap_count = 0;
            state = STATE_IDLE;
          } else {
            // Just released (short hold)
            state = STATE_IDLE;
          }
        } else if (touch_duration > LONG_HOLD_MS) {
          // Very long hold -> sleep
          result = GESTURE_LONG_HOLD;
          state = STATE_IDLE;
        } else if (touch_duration > HOLD_THRESHOLD_MS) {
          // Medium hold -> just a hold (not a tap)
          state = STATE_HOLDING;
        }
        break;
      }

      case STATE_TAP_RELEASED: {
        unsigned long time_since_release = now - last_tap_end;

        if (touch_pressed) {
          // Touch down again
          touch_start = now;
          debounce_until = now + DEBOUNCE_MS;
          state = STATE_TOUCHING;
        } else if (time_since_release > DOUBLE_TAP_GAP_MS) {
          // Double-tap window closed -> emit gesture
          if (tap_count == 1) {
            result = GESTURE_SINGLE_TAP;
          } else if (tap_count == 2) {
            result = GESTURE_DOUBLE_TAP;
          } else if (tap_count >= 3) {
            result = GESTURE_RAPID_TAPS;
          }
          tap_count = 0;
          state = STATE_IDLE;
        }
        break;
      }

      case STATE_HOLDING: {
        unsigned long touch_duration = now - touch_start;

        if (!touch_pressed) {
          // Released after hold
          result = GESTURE_HOLD;
          tap_count = 0;
          state = STATE_IDLE;
          debounce_until = now + DEBOUNCE_MS;
        } else if (touch_duration > LONG_HOLD_MS) {
          // Long hold -> sleep
          result = GESTURE_LONG_HOLD;
          state = STATE_IDLE;
        }
        break;
      }
    }

    return result;
  }

private:
  enum State {
    STATE_IDLE,
    STATE_TOUCHING,
    STATE_TAP_RELEASED,
    STATE_HOLDING,
  };

  State state;
  unsigned long touch_start;
  unsigned long last_release;
  int tap_count;
  unsigned long last_tap_end;
  unsigned long debounce_until;
};

GestureDetector gesture_detector;

// ============================================================================
// MAIN STATE MACHINE
// ============================================================================

MoodState current_mood = MOOD_DEFAULT;
unsigned long mood_changed_at = 0;
unsigned long last_touch = 0;

void update_mood_from_gesture(GestureDetector::GestureType gesture) {
  unsigned long now = millis();

  switch (gesture) {
    case GestureDetector::GESTURE_SINGLE_TAP:
      Serial.println("Gesture: Single Tap -> Happy");
      current_mood = MOOD_HAPPY;
      mood_changed_at = now;
      last_touch = now;
      break;

    case GestureDetector::GESTURE_DOUBLE_TAP:
      Serial.println("Gesture: Double Tap -> Playful");
      current_mood = MOOD_PLAYFUL;
      mood_changed_at = now;
      last_touch = now;
      break;

    case GestureDetector::GESTURE_RAPID_TAPS:
      Serial.println("Gesture: Rapid Taps -> Angry");
      current_mood = MOOD_ANGRY;
      mood_changed_at = now;
      last_touch = now;
      break;

    case GestureDetector::GESTURE_RUBBING:
      Serial.println("Gesture: Rubbing -> Petted");
      current_mood = MOOD_PETTED;
      mood_changed_at = now;
      last_touch = now;
      break;

    case GestureDetector::GESTURE_HOLD:
      Serial.println("Gesture: Hold -> Tired");
      current_mood = MOOD_TIRED;
      mood_changed_at = now;
      last_touch = now;
      break;

    case GestureDetector::GESTURE_LONG_HOLD:
      Serial.println("Gesture: Long Hold -> Asleep");
      current_mood = MOOD_ASLEEP;
      mood_changed_at = now;
      last_touch = now;
      break;

    default:
      break;
  }
}

void update_idle_behavior() {
  unsigned long now = millis();

  // Auto-transition after mood expires (except asleep)
  if (current_mood != MOOD_DEFAULT && current_mood != MOOD_ASLEEP) {
    if (now - mood_changed_at > 5000) {
      current_mood = MOOD_DEFAULT;
      Serial.println("Mood expired -> Default");
    }
  }

  // Idle animation if untouched for 20s
  if (now - last_touch > IDLE_TIMEOUT_MS && current_mood == MOOD_DEFAULT) {
    // Eyes will auto-blink and look around in SimpleEyes::draw()
  }
}

void draw_frame() {
  display.clearDisplay();

  // Use SimpleEyes or bitmap frames depending on mood
  switch (current_mood) {
    case MOOD_PETTED:
      // Use SimpleEyes with hearts
      eyes.draw(display, current_mood);
      break;

    case MOOD_ASLEEP:
      // TODO: Use sleeping bitmap if you generate one
      eyes.draw(display, current_mood);
      break;

    default:
      eyes.draw(display, current_mood);
      break;
  }

  display.display();
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== Dasai Mochi Robot (No Speaker) ===");

  // Initialize I2C and OLED
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: SSD1306 allocation failed");
    while (1);
  }
  display.clearDisplay();
  display.display();
  Serial.println("OLED initialized");

  // Initialize touch pin
  pinMode(TOUCH_PIN, INPUT);
  Serial.println("Touch sensor initialized on GPIO27");

  Serial.println("Robot ready!");

  last_touch = millis();
  mood_changed_at = millis();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

unsigned long last_touch_sample = 0;
const unsigned long TOUCH_SAMPLE_INTERVAL = 20; // ms

void loop() {
  unsigned long now = millis();

  // Sample touch input every 20ms (non-blocking)
  if (now - last_touch_sample >= TOUCH_SAMPLE_INTERVAL) {
    last_touch_sample = now;

    bool touch_active = (digitalRead(TOUCH_PIN) == LOW); // Pulled high, active low
    GestureDetector::GestureType gesture = gesture_detector.update(touch_active);

    if (gesture != GestureDetector::GESTURE_NONE) {
      update_mood_from_gesture(gesture);
    }
  }

  // Update idle behavior and mood transitions
  update_idle_behavior();

  // Draw animation frame
  draw_frame();

  // Brief delay to yield CPU
  delay(1);
}

// ============================================================================
// HOW TO ADD A NEW EXPRESSION
// ============================================================================
/*
 * 1. Define a new MoodState enum value:
 *    MOOD_CUSTOM_EXPRESSION
 *
 * 2. Add a new bitmap to PROGMEM (optional for bitmap-based expression):
 *    const uint8_t custom_frame[] PROGMEM = { ... }; // From image2cpp
 *
 * 3. Add a case in draw_frame() or SimpleEyes::draw():
 *    case MOOD_CUSTOM_EXPRESSION:
 *      eyes.draw(display, current_mood);
 *      // Or draw custom bitmap
 *      break;
 *
 * 4. Trigger it from update_mood_from_gesture() or another event:
 *    case GestureDetector::GESTURE_NEW_EVENT:
 *      current_mood = MOOD_CUSTOM_EXPRESSION;
 *      break;
 *
 * Tips:
 * - Keep bitmaps in PROGMEM to save RAM.
 * - Use millis() and timers instead of delay() for responsiveness.
 * - Test gesture thresholds with Serial.println() to tune.
 * - Use image2cpp (http://javl.github.io/image2cpp/) to convert images:
 *   - Image size: 128x64
 *   - Code output format: Arduino code, array
 *   - Draw mode: Horizontal - 1 bit per pixel
 */
