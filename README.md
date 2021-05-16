# My ESP8266 + Plantower AQ sensor project

This project is an ESP8266 combined with an AQ sensor and temp / humidity
sensor.  I have these set up around my house so I can monitor air quality and
temp.  The sensor publishes data to an MQTT server, then I have a listener that
connects to the MQTT server and writes all of the AQ data to an Influx
database.  I'm using Grafana to graph the data in the InfluxDB.

## Overview

Here is a photo of the completed project:

![Photo of Project](pics/project-small.jpeg)

The sensor gets power over USB.  When you power the sensor up, it will create
an ad-hoc wifi network.  You can connect to that network then configure the
sensor with the network you want it to connect to.

## Hardware

The PCB is found in the [pcb](pcb) directory.  It's just a Kicad project.  The
[README](pcb/README.md) in that folder has more information about the hardware
side of this project (including parts, etc).

## Firmware

The firmware is found in the [firmware](firmware) directory.  The
[README](firmware/README.md) file in that folder has more information about the
firmware side of this project.

## Grafana

Here is a sample graph of the data:

![Grafana screenshot](pics/grafana.png)
