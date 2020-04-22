# My ESP8266 + Plantower AQ Sensor Project Firmware

This is the firmware for the project.  I wrote an Arduino sketch.  I'd rather
have written it in C, but writing an Arduino sketch seemed like the fastest way
to get the project done, and I'd rather be finished than happy (this is a joke,
I am perfectly happy even though it isn't C).

## Prerequisites

Make sure you have Arduino IDE installed.  I like to use the command line tools,
so I made a `Makefile` that will setup most of the stuff.

You'll also need a programmer for the ESP8266.  [This
one](https://www.amazon.com/gp/product/B07KF119YB/) works well for me.

If you're on macOS, the Python path seems broken, so you might have to do this:

```
$ cd ~/Library/Arduino15/packages/esp8266/tools/python3/3.7.2-post1
$ rm python3 
$ ln -s /usr/bin/python3
```

## Command Line

If you're using the command line, just do:

1. `make setup` to install libraries
2. `make compile` to compile stuff
3. `make flash` to flash the ESP8266

If you've already assembled the PCB, this should be it.  By default the ESP8266
will create an ad-hoc network.  Connect to that network and configure the chip
with your wifi credentials.  After it connects to your network it will start
broadcasting sensor information.

You may need to adjust `PORT` inside the `Makefile` in case your programmer is
in a different location.

See `client.rb` in the main director for a Ruby client that listens for sensor data.
