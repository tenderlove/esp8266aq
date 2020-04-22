# My ESP8266 + Plantower AQ Sensor Project PCB

This is the PCB for the project.  I've checked in the gerber files in
`gerber.zip`, so you can just upload that to a PCB manufacturer and get PCBs
made.  I like JLCPCB.

## Parts List

Here are the parts I used:

* [ESP8266-01](https://www.digikey.com/product-detail/en/sparkfun-electronics/WRL-13678/1568-1235-ND/5725944) for WiFi
* [Si7021](https://www.aliexpress.com/item/4000598507886.html) for temp / humidity over I2C
* [PMS5003](https://www.aliexpress.com/item/32834164058.html) for Air Quality
* [LD1117V33](https://www.digikey.com/product-detail/en/stmicroelectronics/LD1117V33/497-1491-5-ND/586012) 3.3v regulator
* 100nf capacitor
* 10uf capacitor (4mm radial)
* [WM7626CT-ND](https://www.digikey.com/product-detail/en/molex/0532610871/WM7626CT-ND/699113) Molex SMD connector for the AQ sensor
* [USB 2.0 Mini connector](https://www.digikey.com/product-detail/en/edac-inc/690-005-299-043/151-1206-1-ND/4312192) for 5v power supply
* [6 pin header](https://www.digikey.com/product-detail/en/sullins-connector-solutions/PPTC032LFBN-RC/S7071-ND/810210) to plug in the ESP8266
* [4 pin header](https://www.digikey.com/product-detail/en/sullins-connector-solutions/PPTC041LFBN-RC/S7002-ND/810144) to plug in the Si7021 breakout board

## Programmer

I bought a separate programmer for the ESP8266.
[This](https://www.amazon.com/gp/product/B07KF119YB/) is the one I bought and
it seems to work well for me.
