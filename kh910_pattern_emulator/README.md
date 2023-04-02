# Pattern Emulator
This project emulates the pattern subsystem for the Brother KH910 knitting machine. More context for what this project emulates can be found in this [video](https://youtu.be/gdRmH8IVRM4). 

# Accompanying videos
<!-- TODO: Add link to video -->

# Files
I included a copy of the code at each part of the project so that you can see how the code evolved over time!

## 1_dac
This version of the code was written for the Arduino Due. It uses the on board DAC to generate a signal at various voltages. This configuration does not give the full range of voltage needed.

## 2_pwm
This version of the code moves to an Arduino Duemilanove that runs on 5V logic giving us a larger available voltage range. However the board doesn't have a DAC so the code changes to a PWM signal. 

Changes: 
* Output pin changed to pin 5
* Voltage scaling changed to 0-5V

## 3_pwm_at_speed
This version of the code decreases the delays between pulses to increase the speed of the pattern to the required 303Hz.

Changes:
* Decreased delay between pulses from 100ms to 1650us
* Increase base PWM frequency
* Move output pin to use new base PWM frequency

## 4_motor_input
This version of the code adds the KH910 motor control signals as inputs to trigger the pattern signal generation. This allows the KH910 to control when to get a new pattern row. The major catch here is that the original pattern system works by moving a motor from right to left then back to the right. This means that the pattern needs alternate being generated in forward and reverse.

Changes:
* Change to an asymmetric pattern
* Add motor control signals as interrupt inputs on pins 2 and 3
* Move pattern output to pin 5
* Add logic to alternate between forward and reverse pattern generation

## 5_smiley_face
This version of the code adds a smiley face pattern to the pattern generation. To create this pattern, the code needed to be modified to allow for a variable number of rows in the pattern. 

Changes:
* Add smiley face pattern
* Add logic to index through the pattern rows