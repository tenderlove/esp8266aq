Setup:

```
$ arduino-cli config init --additional-urls http://arduino.esp8266.com/stable/package_esp8266com_index.json
$ arduino-cli core update-index
$ arduino-cli core install esp8266:esp8266
```

Fix Python Path: (on macOS)

```
$ cd ~/Library/Arduino15/packages/esp8266/tools/python3/3.7.2-post1
$ rm python3 
$ ln -s /usr/bin/python3
```
