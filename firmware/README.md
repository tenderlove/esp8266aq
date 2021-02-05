# My ESP8266 + Plantower AQ Sensor Project Firmware

This is the firmware for the project.  I wrote an Arduino sketch.  I'd rather
have written it in C, but writing an Arduino sketch seemed like the fastest way
to get the project done, and I'd rather be finished than happy (this is a joke,
I am perfectly happy even though it isn't C).

## Prerequisites

This firmware uses [PlatformIO](https://platformio.org).  Make sure you install
PlatformIO first!

## Command Line

If you're using the command line, just do:

1. `make flash` to compile stuff and upload it to the ESP

If you've already assembled the PCB, this should be it.  By default the ESP8266
will create an ad-hoc network.  Connect to that network and configure the chip
with your wifi credentials.  After it connects to your network it will start
broadcasting sensor information.

You may need to adjust `PORT` inside the `Makefile` in case your programmer is
in a different location.

See [`client.rb`](../client.rb) in the main director for a Ruby client that listens for sensor data.

## Development

If you want to hack on the firmware, just do `make` to compile stuff.  My
typical development process looks like this:

1. Edit code
2. Run `make flash && ruby utils/serial-client.rb`
