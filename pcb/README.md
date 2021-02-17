# My ESP8266 + Plantower AQ Sensor Project PCB

This is the PCB for the project.  I've checked in the gerber files in
`gerbers.zip`, so you can just upload that to a PCB manufacturer and get PCBs
made.  I like JLCPCB.

## Parts List

Here are the parts I used:

* [ESP-12F](https://www.digikey.com/en/products/detail/adafruit-industries-llc/2491/5761206) for WiFi [China](https://www.aliexpress.com/item/32339917567.html)
* [BME280](https://www.aliexpress.com/item/32862844557.html) for temp / humidity over I2C
* [PMS5003](https://www.aliexpress.com/item/32834164058.html) for Air Quality
* [MCP1700](https://www.digikey.com/en/products/detail/microchip-technology/MCP1700-3302E-TO/652680) 3.3v regulator
* 100nf capacitor (4.7mm disc)
* 10uf capacitor (4mm radial)
* 47nf capacitor (4mm radial)
* 2x 10k resistors (THT, 0207)
* [WM7626CT-ND](https://www.digikey.com/product-detail/en/molex/0532610871/WM7626CT-ND/699113) Molex SMD connector for the AQ sensor [Europe](https://www.reichelt.nl/molex-pin-header-smd-picoblade-1x8-polig-stekker-molex-532610871-p186231.html)
* [USB4085-GF-A](https://www.digikey.com/en/products/detail/gct/USB4085-GF-A/9859733) for 5v power supply and serial communication
* [4 pin header](https://www.digikey.com/product-detail/en/sullins-connector-solutions/PPTC041LFBN-RC/S7002-ND/810144) to plug in the BME280 breakout board [Europe](https://www.reichelt.nl/female-header-2-54mm-straight-1x4-bkl-10120946-p266671.html)
* [MCP2221a](https://www.digikey.com/en/products/detail/microchip-technology/MCP2221A-I-P/6009296) for programming the ESP and serial communication
* [2x JST SH 4 pin Vertical Connector](https://www.adafruit.com/product/4328). These are optional, they're just for future extension to the board
