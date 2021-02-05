require "uart"
require "io/wait"
require "json"

AQ = Struct.new(:pm1_0_standard, :pm2_5_standard, :pm10_standard,
                :pm1_0_env,      :pm2_5_env,
                :concentration_unit,

                # These fields are "number of particles beyond N um
                # per 0.1L of air". These numbers are multiplied by
                # 10, so 03um == "number of particles beyond 0.3um
                # in 0.1L of air"
                :particle_03um,   :particle_05um,   :particle_10um,
                :particle_25um,   :particle_50um,   :particle_100um)

TempRH = Struct.new(:temp, :rh)

UART.open '/dev/cu.usbmodem1411101' do |serial|
  loop do
    serial.wait_readable
    str = serial.readline
    begin
      record = JSON.load(str.strip)
      data = record["aq"].unpack("m0").first
      unpack = data.unpack('CCnn14')
      crc = 0x42 + 0x4d + 28 + data.bytes.drop(4).first(26).inject(:+)
      unless crc != unpack.last
        aq = AQ.new(*unpack.drop(3).first(12))
        temprh = TempRH.new(record["temperature"] / 100.0, record["humidity"] / 100.0)

        puts "Temperature #{temprh.temp}C"
        puts "Humidity #{temprh.rh}%"
        puts "PM2.5 #{aq.pm2_5_standard}%"
        puts
      end
    rescue JSON::ParseError, StandardError
      p str
    end
  end
end
