require "uart"
require "io/wait"
require "json"

UART.open '/dev/cu.usbmodem1411101' do |serial|
  loop do
    serial.wait_readable
    puts serial.readline
  end
end
