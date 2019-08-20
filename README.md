# Fridge Screamer

![](schematic.png?raw=true)

This is the firmware for a fridge door sensor that beeps when the door is open
for too long. 

It was written for an ATtiny25V but it _should_ work on Attiny45/85 with no
changes (apart from makefile configuration).

## How it works
Approximately once a second the MCU will wake up and check the state of PB2. If
it's HIGH it means there's a magnet (the door) near the reed switch, so the
door is considered closed and the MCU will go back to power-down mode. If PB2
is LOW it means the door is open and a counter will increase before going back
to power-down mode.

The process repeats until the counter reaches the warning threshold, in which
point a square wave is emitted through PB0 to trigger a buzzer for 5ms and go
back to power-down mode. Closing the door again resets the counter, which stops
the buzzer from being triggered.
