RUBY = ruby
PLATFORMIO = platformio

NODE_NAME ?= esp8266aq
MQTT_SERVER ?= ""
MQTT_PORT ?= 1883
MQTT_PREFIX ?= esp8266aq

CFLAGS="-DNODE_NAME='\"$(NODE_NAME)\"' -DMQTT_SERVER='\"$(MQTT_SERVER)\"' -DMQTT_PORT=$(MQTT_PORT) -DMQTT_PREFIX='\"$(MQTT_PREFIX)\"'"


compile:
	BUILD_FLAGS=$(CFLAGS) platformio run

flash:
	$(RUBY) utils/prog.rb
	BUILD_FLAGS=$(CFLAGS) platformio run -t upload

uploadfs:
	$(RUBY) utils/prog.rb
	platformio run -t uploadfs
