BOARD = esp8266:esp8266:generic
FQBN = $(BOARD):xtal=80,vt=flash,exception=legacy,ssl=all,ResetMethod=nodemcu,CrystalFreq=26,FlashFreq=40,FlashMode=dout,eesz=1M64,led=2,sdk=nonosdk_190703,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=115200
PORT = /dev/tty.usbmodem143101
CC = arduino-cli

compile: 
	$(CC) compile --fqbn $(FQBN) --libraries=libraries `pwd`

flash: 
	$(CC) upload -p $(PORT) --fqbn $(FQBN) `pwd`

setup:
	arduino-cli config init --additional-urls http://arduino.esp8266.com/stable/package_esp8266com_index.json
	arduino-cli core update-index
	arduino-cli core install esp8266:esp8266
	arduino-cli lib install WiFiManager

clean:
	rm *.elf *.bin
