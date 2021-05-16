require "esp_client"

begin
  esp = ESPClient.open
  esp.gp3 = 1

  #i2c  = esp.i2c_on 0x51
  #i2c.cancel
  #i2c.write 0x2.chr
  #p i2c.read 7

  i2c  = esp.i2c_on 0x15
  i2c.cancel
  i2c.write [0x04, 0x13, 0x8B, 0x00, 0x01].pack("C*")
  sleep 1
  p i2c.read 4
ensure
  esp.gp2 = 0
  esp.gp3 = 0
end
