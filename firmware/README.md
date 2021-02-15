# My ESP8266 + Plantower AQ Sensor Project Firmware

This is the firmware for the project.  I wrote an Arduino sketch.  I'd rather
have written it in C, but writing an Arduino sketch seemed like the fastest way
to get the project done, and I'd rather be finished than happy (this is a joke,
I am perfectly happy even though it isn't C).

## Prerequisites

This firmware uses [PlatformIO](https://platformio.org).  Make sure you install
PlatformIO first!

## MQTT

This firmware uses MQTT to publish data to an MQTT server.  That means the
module must be configured with connection information so it knows where to
connect.  Setting up an MQTT server is outside the scope of this document, but
I've heard that [Home Assistant](https://www.home-assistant.io) makes it easy.

When compiling the firmware, you'll need the following information about the
MQTT connection:

1. The node name. A unique name given to this module. The name can be anything as long as it's unique.  For example, I'm using `office-aq` for the sensor in my office.
2. The MQTT server hostname. (Make sure it's not an mDNS name)
3. The MQTT prefix. This is a prefix string that indicates where the sensor is. For my office I'm using `home/office/esp8266aq`.

## Compiling and Uploading

The MQTT configuration can be specified by environment variables.

I compile and flash my firmware like this:

```
$ MQTT_SERVER=tender.home NODE_NAME=office-aq MQTT_PREFIX=home/office/esp8266aq make flash
```

If this is your first time flashing the chip, make sure to read the next step!!

## IMPORTANT: First Time Uploading

This firmware makes use of a special filesystem that is flashed independently
of the normal runtime firmware.  You need to upload the file system
independently of the normal firmware.  This step writes all of the file in the
`data` directory to the ESP.

To upload the file system, do:

```
$ make uploadfs
```

You only need to upload the filesystem whenever things in the `data` directory change.

1. `make flash` to compile stuff and upload it to the ESP

## First Time Connecting

The first time the chip boots, by default the ESP8266
will create an ad-hoc network.  Connect to that network and configure the chip
with your wifi credentials.  After it connects to your network it will start
broadcasting sensor information to your MQTT server.

## Development

My typical development process looks like this:

1. Edit code
2. Run `make flash && ruby utils/serial-client.rb`
