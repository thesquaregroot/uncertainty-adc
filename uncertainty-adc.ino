// Adapted from https://github.com/oamodular/uncertainty/blob/main/software/stable/uncertainty/uncertainty.ino
// Original license: https://github.com/oamodular/uncertainty/blob/main/LICENSE.md
// License for this code: https://creativecommons.org/licenses/by-sa/4.0/

#include <Arduino.h>
#include <vector>
#include <math.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

// number of output gates
#define NUM_GATES 8
// number of samples to average for signal
#define NUM_AVERAGE_SAMPLES 4
// number of samples to use for rate changing
#define NUM_RATE_SAMPLES 4096
// number of samples between rate calculations
#define UPDATE_FREQUENCY 256

// signal rate of change cutoffs for determining delay times
#define SLOW_CUTOFF 2
#define FAST_CUTOFF 6

// delay limits in microseconds
#define MIN_DELAY 0
#define MAX_DELAY 100000

// ADC input pin
int inputPin = 26;
// hold pins for gates
int gatePins[] = {27,28,29,0,3,4,2,1};

int counter = 0;
int currentDelay = MIN_DELAY;
uint32_t lastUpdate = 0;
std::vector<uint16_t> rateSamples;
std::vector<uint16_t> averagingSamples;

void setup() {
  // 2x overclock for MAX POWER
  set_sys_clock_khz(250000, true);

  Serial.begin(9600);

  // initialize ADC
  adc_init();
  adc_gpio_init(inputPin);
  adc_select_input(0);
  gpio_set_pulls(inputPin, false, false);
  
  // initialize gate out pins
  for(int i=0; i<NUM_GATES; i++) {
    int pin = gatePins[i];
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
  }
}

// adds the value to the provided vector,
// optionally removing a value if the size exceeds the provided limit
void push(const uint16_t& value, std::vector<uint16_t>& vector, const int& sizeLimit) {
  vector.push_back(value);
  if (vector.size() > sizeLimit) {
    vector.erase(vector.begin());
  }
}

int to_8bit(const uint64_t& value) {
  return ((uint16_t)value - (1<<11)) >> 4;
}

void update_sample_rate() {
  if (rateSamples.size() < NUM_RATE_SAMPLES) {
    return;
  }

  // determine how much the signal is changing by summing the differences between samples
  double averageChange = 0;
  for (int i=0; i<NUM_RATE_SAMPLES-1; i++) {
   averageChange += abs(rateSamples[i+1] - (int)rateSamples[i]);
  }
  averageChange = averageChange / (NUM_RATE_SAMPLES - 1);
  averageChange = constrain(averageChange, SLOW_CUTOFF, FAST_CUTOFF);

  // larger changes imply higher frequency signals, so a shorter delay is used

  double linear = (averageChange - SLOW_CUTOFF) / (double)(FAST_CUTOFF - SLOW_CUTOFF);
  //double interpolation = linear;
  double interpolation = log2(linear + 1); // logorithmic interpolation

  currentDelay = MAX_DELAY - interpolation * (MAX_DELAY - MIN_DELAY);
}

void loop() {
  uint16_t input = adc_read();
  push(input, rateSamples, NUM_RATE_SAMPLES);

  counter++;
  if (counter >= UPDATE_FREQUENCY) {
    update_sample_rate();
    counter = 0;
  }

  uint32_t now = micros();
  if (lastUpdate + currentDelay > now) {
    return;
  }

  push(input, averagingSamples, NUM_AVERAGE_SAMPLES);
  uint64_t average = 0;
  for (int i=0; i<averagingSamples.size(); i++) {
    average += averagingSamples[i];
  }
  average /= averagingSamples.size();

  int adjustedInput = to_8bit(average);

  bool signBit = adjustedInput > 0;
  int value = abs(adjustedInput);

  gpio_put(gatePins[0], signBit);
  gpio_put(gatePins[1], (value & 0x40) > 0);
  gpio_put(gatePins[2], (value & 0x20) > 0);
  gpio_put(gatePins[3], (value & 0x10) > 0);
  gpio_put(gatePins[4], (value & 0x08) > 0);
  gpio_put(gatePins[5], (value & 0x04) > 0);
  gpio_put(gatePins[6], (value & 0x02) > 0);
  gpio_put(gatePins[7], (value & 0x01) > 0);

  lastUpdate = now;
}
