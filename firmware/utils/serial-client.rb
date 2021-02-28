require "uart"
require "io/wait"
require "json"
require "esp_client"

begin

esp = ESPClient.open
esp.gp2 = 1

UART.open '/dev/cu.usbmodem1411101' do |serial|
  loop do
    serial.wait_readable
    puts serial.readline
  end
end

ensure
  puts "turning gp2 off"
  esp.gp2 = 0
end
