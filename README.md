# uncertainty-adc

An analog-to-digital converter for the eurorack module [Uncertainty](https://github.com/oamodular/uncertainty)
by [Olivia Artz Modular](https://github.com/oamodular).

## About this firmware

The input +5V/-5V signal is encoded as an 8-bit sign-magnitude value.
The top output corresponds to the sign of the input with a high gate for a
positive signal and zero for a negative signal. The output below that will
emit a high gate when the signal's magnitude is greater than 2.5V (i.e. above
2.5V or below -2.5V). Generally, each of output corresponds to the nth bit of
the encoded value, with the lowest output corresponding to the least significant
bit (LSB).

The highest 2-3 outputs respond well to a wide range of frequencies, from LFO range
into low-to-mid audio range. The lower outputs can be somewhat stable within this
range as the frequencies decrease, some jitter may be introduced due to noise in
at the input. As the frequencies increase, they may begin to glitch and skip
beats due to aliasing. At higher audio frequencies and very slow LFO rates,
the same effects may occur even with the highest outputs. As a result, though
a fair amount of effort went into keeping things stable, the outputs are best
thought of as coming with some...uncertainty.

## Installing

If you want to install this firmware as-is, you can download the .uf2 file and
follow the instructions [here](https://github.com/oamodular/uncertainty?tab=readme-ov-file#how-to-reinstall-the-default-firmware).

If you would like to modify the code and run it, you can do so using the Arduino
IDE, following the instructions
[here](https://wiki.seeedstudio.com/XIAO-RP2040-with-Arduino/).

## Use-cases

### Rhythm Generation

Each of the outputs could be used to trigger a separate voice, resulting in
syncopated rhythms with increasing frequency as you move down the outputs.

### Comparator

At the most basic, the top-most output (sign bit) can be used to trigger events
when a signal goes from negative to positive.  This could be useful for create a
square output for an LFO or VCO that doesn't provide one, which could in turn be
useful for syncing multiple oscillators.

When combined with an external offset, this could function as a more general comparator.

The second output could be used to activate a change when a signal crosses the +/-2.5V
threshold.  This threshold could effectively be tuned by passing the signal
through an external attenuator/amplifier first.

### LFO to noisy VCO

Patching a fast LFO into the input, the lowest output (LSB) will output a chaotic
audio-rate square wave.  Various the speed of the LFO can give a variety of
effects.

