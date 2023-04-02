// Arduino Reference Manual: https://www.arduino.cc/reference/en/

#include <assert.h>

// #define DEBUG 1

// We'll start by emulating a simple pattern I'm using a uint64_t because I find
// it easier to know exactly how many bits I'm dealing with than having to
// figure out the type sizes for the architecture I'm using. Also, the pattern
// is 60 pulses wide, and that will fit nicely into a 64 bit data type!
const uint64_t pattern = 0x0f0f0f0f0f0f0f0f;
// bits: 0000 1111 0000 1111 0000 1111 0000 1111 0000 1111 0000 1111 0000 1111
// 0000 1111
const int pattern_size = 60;

// To get the voltage range we need, I moved to a different board that operates
// on 5V logic instead of 3.3V. Unfortunately this board doesn't have a DAC, so
// we are going to use a combination of Pulse Width Modulation (PWM) and an RC
// circuit on pin 5.
int output_pin = 5;

// analogWrite() takes values between 0 and 255
// analogWrite(0) should be 0V
// analogWrite(255) should be 5V

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
// lose a bit of accuracy here but that is a limitation of how the PWM hardware
// works.

// steady state = 3.673V
int steady_state = (int)(3.673 * scale_factor);

// light square = 1.8V
int light_square = (int)(1.8 * scale_factor);

// dark square = 0.12V
int dark_square = (int)(0.12 * scale_factor);

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

  // Set pin to the steady state value
  analogWrite(output_pin, steady_state);
}

void loop() {

  // 1. Pause for 1 second before beginning our pattern loop so that
  // we can see the start of the pattern
  // 2. Set pin to next pattern value
  // 3. Delay to create the low part of the pulse
  // 4. Set pin to steady state
  // 5. Delay to create the high part of the pulse
  // 6. Repeat!

  delay(1000);

  // period = 3.3 mS -> ~ 1.7 mS between analog writes

  for (int i = 0; i < pattern_size; i++) {
    // This line is reading the ith bit out of the pattern variable
    // and removing (a.k.a. masking) all of the other bits.
    analogWrite(output_pin, getPatternValue((pattern >> i) & 1));
    delay(100);
    analogWrite(output_pin, steady_state);
    delay(100);
  }
}
