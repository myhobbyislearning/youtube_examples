// Arduino Reference Manual: https://www.arduino.cc/reference/en/

#include <assert.h>

// #define DEBUG 1

// We'll start by emulating a simple pattern I'm using a uint64_t because I find
// it easier to know exactly how many bits I'm dealing with than having to
// figure out the type sizes for the architecture I'm using. Also, the pattern
// is 60 pulses wide, and that will fit nicely into a 64 bit data type!

// I added a more interesting pattern!
static const uint64_t pattern_len = 56;
static const uint64_t pattern[pattern_len] = {
    0x0,
    0x0,
    0x0,
    0x00000fff000000,
    0x00007fffe00000,
    0x0003e000780000,
    0x000780001e0000,
    0x001e0000078000,
    0x0038000001c000,
    0x0060000000e000,
    0x00c00000003000,
    0x01800000001800,
    0x03000000000c00,
    0x06001801000e00,
    0x06003c03800600,
    0x0c007c07c00300,
    0x18007e07c00300,
    0x18007e07c00180,
    0x10007e07e00180,
    0x30007e07e000c0,
    0x30007e07e000c0,
    0x20007e07e000c0,
    0x60007e07c00060,
    0x60007c07c00060,
    0x60003c03c00060,
    0x60001803800060,
    0x60000000000060,
    0x60000000000060,
    0x60600000006060,
    0x63f0000001f860,
    0x63f0000001fc60,
    0x60e00000006060,
    0x60600000006060,
    0x6060000000e060,
    0x2070000000c040,
    0x3030000001c0c0,
    0x303800000180c0,
    0x101c0000038180,
    0x180c0000070180,
    0x180700000e0300,
    0x0c0380001c0300,
    0x0601c000780600,
    0x0600f801e00e00,
    0x03003fffc00c00,
    0x01800ffe001800,
    0x00c00000003000,
    0x00700000006000,
    0x0038000001c000,
    0x001c0000038000,
    0x000700000e0000,
    0x0003e0007c0000,
    0x00007fffe00000,
    0x00000fff000000,
    0x0,
    0x0,
    0x0,
};

const int pattern_width = 60;
int pattern_index =
    pattern_len -
    1; // the pattern should be generated from the bottom up, so start with the
       // pattern_index at the last value in the pattern array

int output_pin = 11; // pattern generation pin
int input_right = 2; // motor input pin
int input_left = 3;  // motor input pin

// analogWrite() takes values between 0 and 255
// analogWrite(0) should be 0V
// analogWrite(255) should be V

// To calculate the value that will represent any voltage in the middle, we go
// back to algebra: y = mx+b, where m = (change in y) / (change in x)
// or (255 - 0) / (4.84) and b = 0 in this case. Also I'm using a float here
// because I want the scaling factor to include decimals instead truncating the
// calculated value.
static const float scale_factor =
    255 / 4.84; // I'm using 4.84V here instead of 5V because I measured 4.84V
                // as the real output voltage. All manufactured parts will have
                // tolerances which means that each board will have variations,
                // such as the real max voltage.

// We want to represent 3 states:

// I'm casting to an int here because analogWrite() expects an int. We will
// lose a bit of accuracy here but that is a limitation of how the DAC hardware
// works.

// steady state = 3.673V
static const int steady_state = (int)(3.673 * scale_factor);

// light square = 1.8V
static const int light_square = (int)(1.8 * scale_factor);

// dark square = 0.12V
static const int dark_square = (int)(0.12 * scale_factor);

// period = 3.3 mS -> ~ 1.7 mS between analog writes
static const float periodMs = 3.3; // 303 Hz
static const int periodMicro = (int)periodMs * 1000;
static const int delayTime = periodMicro / 2;

// These global variables are used to keep track of which direction the pattern
// should be read out
bool right = false;
bool left = false;

// This function is called when the right motor control pin is triggered
void pattern_left() { left = true; }

// This function is called when the left motor control pin is triggered
void pattern_right() { right = true; }

int getPatternValue(unsigned int bit) {
  if (1 == bit) {
#ifdef DEBUG
    Serial.print("dark: ");
    Serial.println(dark_square);
#endif
    return dark_square;
  } else if (0 == bit) {
#ifdef DEBUG
    Serial.print("light: ");
    Serial.println(light_square);
#endif
    return light_square;
  }

#ifdef DEBUG
  Serial.write("bit: ");
  Serial.print(bit, HEX);
  Serial.write("\n");
#endif

  // This will crash the program if we reach this line. I like adding asserts
  // while I'm developing to make sure I'm not accidently doing silly things.
  assert(false);
}

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  Serial.write("testing!!\n");
#endif

  // change the clock divider for this timer to increase the PWM frequency
  TCCR2B = _BV(CS20);

  pinMode(input_left, INPUT);
  pinMode(input_right, INPUT);

  attachInterrupt(digitalPinToInterrupt(input_left), pattern_left, RISING);
  attachInterrupt(digitalPinToInterrupt(input_right), pattern_right, RISING);

  // Set pin to the steady state value
  analogWrite(output_pin, steady_state);
}

typedef enum {
  kUnknown,
  kLeft,
  kRight,
} State;

// This variable is used to keep track of the last direction the pattern was and
// to prevent out of order pattern writes. The circuitry I am using to read in
// the motor control signals is not very robust and sometimes gets erroneous
// triggers, so I'm using this state as a brute force way to mitigate this by
// discarding any out of order triggers.
State last_pass = kUnknown;

#ifdef DEBUG
uint32_t count = 0;
#endif

void loop() {

  // Discard any out of order triggers
  if (last_pass == kRight) {
    right = false;
  }
  if (last_pass == kLeft) {
    left = false;
  }

  // 1. Set pin to next pattern value
  // 2. Delay to create the low part of the pulse
  // 3. Set pin to steady state
  // 4. Delay to create the high part of the pulse

  // To support the concept of the motor moving either direction, the loop is
  // separated into a left and right section where the right section writes the
  // pattern out backwards.
  if (left) {
#ifdef DEBUG
    Serial.print(count++);
    Serial.write(". left!\n");
#endif
    for (int i = 0; i < pattern_width; i++) {
      // This line is reading the ith bit out of the pattern variable
      // and removing (a.k.a. masking) all of the other bits.
      analogWrite(output_pin,
                  getPatternValue((pattern[pattern_index] >> i) & 1));
      delayMicroseconds(delayTime);
      analogWrite(output_pin, steady_state);
      delayMicroseconds(delayTime);
    }

    // reset the left flag
    left = false;
    // update the current state
    last_pass = kLeft;

    // move to the next pattern index, looping back to the beginning if needed
    pattern_index--;
    if (pattern_index <= 0) {
      pattern_index = pattern_len - 1;
    }

    return;
  } else if (right) {
#ifdef DEBUG
    Serial.print(count++);
    Serial.write(". right!\n");
#endif
    for (int i = pattern_width; i >= 0; i--) {
      // This line is reading the ith bit out of the pattern variable
      // and removing (a.k.a. masking) all of the other bits.
      analogWrite(output_pin,
                  getPatternValue((pattern[pattern_index] >> i) & 1));
      delayMicroseconds(delayTime);
      analogWrite(output_pin, steady_state);
      delayMicroseconds(delayTime);
    }

    // reset the right flag
    right = false;
    // update the current state
    last_pass = kRight;

    // move to the next pattern index, looping back to the beginning if needed
    pattern_index--;
    if (pattern_index <= 0) {
      pattern_index = pattern_len - 1;
    }
    return;
  }
}
