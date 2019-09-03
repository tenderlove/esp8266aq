# A program to listen for AQ info

require "socket"
require "ipaddr"
require "json"

MULTICAST_ADDR = "224.0.0.1"
BIND_ADDR = "0.0.0.0"
PORT = 9000

if_addr = Socket.getifaddrs.find { |s| s.addr.ipv4? && !s.addr.ipv4_loopback? }
p if_addr.addr.ip_address

socket = UDPSocket.new
membership = IPAddr.new(MULTICAST_ADDR).hton + IPAddr.new(BIND_ADDR).hton

socket.setsockopt(:IPPROTO_IP, :IP_ADD_MEMBERSHIP, membership)
socket.setsockopt(:IPPROTO_IP, :IP_MULTICAST_TTL, 1)
socket.setsockopt(:SOL_SOCKET, :SO_REUSEPORT, 1)

socket.bind(BIND_ADDR, PORT)

class Sample < Struct.new(:time,
                          :pm1_0_standard, :pm2_5_standard, :pm10_standard,
                          :pm1_0_env,      :pm2_5_env,
                          :concentration_unit,

                          # These fields are "number of particles beyond N um
                          # per 0.1L of air". These numbers are multiplied by
                          # 10, so 03um == "number of particles beyond 0.3um
                          # in 0.1L of air"
                          :particle_03um,   :particle_05um,   :particle_10um,
                          :particle_25um,   :particle_50um,   :particle_100um)
end

loop do
  m, _ = socket.recvfrom(2000)
  record = JSON.load(m)[1]

  data = record["record"].unpack("m0").first
  unpack = data.unpack('CCnn14')
  crc = 0x42 + 0x4d + 28 + data.bytes.drop(4).first(26).inject(:+)
  unless crc != unpack.last
    p Sample.new(Time.now.utc, *unpack.drop(3).first(12))
  end
end

