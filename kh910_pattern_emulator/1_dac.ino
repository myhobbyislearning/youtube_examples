// Arduino Reference Manual: https://www.arduino.cc/reference/en/

#include <assert.h>

#define DEBUG 1

// I'll start by emulating a simple pattern I'm using a uint64_t because I find
// it easier to know exactly how many bits I'm dealing with than having to
// figure out the type sizes for the architecture I'm using. Also, the pattern
// is 60 pulses wide, and that will fit nicely into a 64 bit data type!
const uint64_t pattern = 0x0f0f0f0f0f0f0f0f;
// bits: 0000 1111 0000 1111 0000 1111 0000 1111 0000 1111 0000 1111 0000 1111
// 0000 1111
const int pattern_size = 60;

// I am using DAC0 for my output. A DAC (Digital to Analog Converter) allows
// us set an arbitrary output voltage instead of requiring the pin to be fully
// 0 or fully 1.
int output_pin = DAC0;

// I am going to use analogWrite() instead of digitalWrite() because we want to
// be able to output arbitrary voltages. Digital logic only allows pins to be 0
// or 1, high or low.

// operating voltage is 3.3V (https://store.arduino.cc/products/arduino-due)

// analogWrite() takes values between 0 and 255
// analogWrite(0) should be 0V
// analogWrite(255) should be 3.3V

// To calculate the value that will represent any voltage in the middle, we go
// back to algebra: y = mx+b, where m = (change in y) / (change in x) or (255 -
// 0) / (3.3) and b = 0 in this case. Also I'm using a float here because I want
// the scaling factor to include decimals instead truncating the calculated
// value.
float scale_factor = 255 / (3.3);

// We want to represent 3 states:
// steady state = 3.673V --> unfortunately our max output voltage is 3.3V, so
// we'll see if we can get away with it
int steady_state = 255;

// I'm casting to an int here because analogWrite() expects an int. We will
// lose a bit of accuracy here but that is a limitation of how the DAC hardware
// works.

// light square = 1.8V
int light_square = (int)(1.8 * scale_factor);

// dark square = 0.12V
int dark_square = (int)(0.12 * scale_factor);

int getPatternValue(unsigned int bit) {
  if (1 == bit) {
    return dark_square;
  } else if (0 == bit) {
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

  // 1. Pause for 1 second before beginning our pattern loop so that we can see
  // the start of the pattern
  // 2. Set pin to next pattern value
  // 3. Delay to create the low part of the pulse
  // 4. Set pin to steady state
  // 5. Delay to create the high part of the pulse
  // 6. Repeat!

  delay(1000);

  for (int i = 0; i < pattern_size; i++) {
    // This line is reading the ith bit out of the pattern variable and removing
    // (a.k.a. masking) all of the other bits.
    analogWrite(output_pin, getPatternValue((pattern >> i) & 1));
    delay(100);
    analogWrite(output_pin, steady_state);
    delay(100);
  }
}
