// Arduino Reference Manual: https://www.arduino.cc/reference/en/

#include <assert.h>

// #define DEBUG 1

// We'll start by emulating a simple pattern I'm using a uint64_t because I find
// it easier to know exactly how many bits I'm dealing with than having to
// figure out the type sizes for the architecture I'm using. Also, the pattern
// is 60 pulses wide, and that will fit nicely into a 64 bit data type!

// I changed to using an asymmetric pattern to make it easier to see when the
// pattern was written forward or backward
const uint64_t pattern = 0xffffff0f0f0f0f0f;
// bits: 1111 1111 1111 1111 1111 1111 0000 1111 0000 1111 0000 1111 0000 1111
// 0000 1111
const int pattern_size = 60;

// I moved the output pin to pin 11 because pins 2 and 3 are the only externally
// interruptable pins on the Arduino Duemilanove. Pins 2 and 3 are now used for
// intercepting the motor control commmands from the KH910 control board.
int output_pin = 11;
int input_right = 2;
int input_left = 3;

// analogWrite() takes values between 0 and 255
// analogWrite(0) should be 0V
// analogWrite(255) should be V

// To calculate the value that will represent any voltage in the middle, we go
// back to algebra: y = mx+b, where m = (change in y) / (change in x) or (255 -
// 0) / (4.84) and b = 0 in this case. Also I'm using a float here because I
// want the scaling factor to include decimals instead truncating the calculated
// value.
float scale_factor =
    255 / 4.84; // I'm using 4.84V here instead of 5V because I measured 4.84V
                // as the real output voltage. All manufactured parts will have
                // tolerances which means that each board will have variations,
                // such as the real max voltage.

// We want to represent 3 states:

// I'm casting to an int here because analogWrite() expects an int. We will
// lose a bit of accuracy here but that is a limitation of how the DAC hardware
// works.

// steady state = 3.673V
int steady_state = (int)(3.673 * scale_factor);

// light square = 1.8V
int light_square = (int)(1.8 * scale_factor);

// dark square = 0.12V
int dark_square = (int)(0.12 * scale_factor);

// period = 3.3 mS -> ~ 1.7 mS between analog writes
const float periodMs = 3.3; // 303 Hz
const int periodMicro = (int)periodMs * 1000;
const int delayTime = periodMicro / 2;

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

// This enum is used to keep track of the last direction the pattern was written
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
    for (int i = 0; i < pattern_size; i++) {
      // This line is reading the ith bit out of the pattern variable
      // and removing (a.k.a. masking) all of the other bits.
      analogWrite(output_pin, getPatternValue((pattern >> i) & 1));
      delayMicroseconds(delayTime);
      analogWrite(output_pin, steady_state);
      delayMicroseconds(delayTime);
    }

    // reset the left flag
    left = false;
    // update the current state
    last_pass = kLeft;
    return;
  } else if (right) {
#ifdef DEBUG
    Serial.print(count++);
    Serial.write(". right!\n");
#endif
    for (int i = pattern_size; i >= 0; i--) {
      // This line is reading the ith bit out of the pattern variable
      // and removing (a.k.a. masking) all of the other bits.
      analogWrite(output_pin, getPatternValue((pattern >> i) & 1));
      delayMicroseconds(delayTime);
      analogWrite(output_pin, steady_state);
      delayMicroseconds(delayTime);
    }

    // reset the right flag
    right = false;
    // update the current state
    last_pass = kRight;
    return;
  }
}
