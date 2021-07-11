# Connects to MQTT, subscribes to the topics published by the esp8266aq sensor,
# then feeds the data in to InfluxDB (which is the data store for Grafana).

require "uri"
require "mqtt"
require "influxdb"

HOST = "tender.home"

class InfluxWrapper
  def initialize influx
    @influx = influx
  end

  def write_co2 location, value, sensor_type
    data = msg({ :value => value, :unit => "PPM" }, location, sensor_type)
    @influx.write_point("co2_samples", data)
  end

  def write_temp location, value, sensor_type
    data = msg({ :value => value, :unit => "C" }, location, sensor_type)
    @influx.write_point("temperatures", data)
  end

  def write_humidity location, value, sensor_type
    data = msg({ :value => value, :unit => "%" }, location, sensor_type)
    @influx.write_point("humidity_samples", data)
  end

  def write_dewpoint location, value, sensor_type
    data = msg({ :value => value, :unit => "C" }, location, sensor_type)
    @influx.write_point("dewpoint_samples", data)
  end

  def write_pm25 location, value, sensor_type
    data   = msg({ "pm2_5_standard" => value }, location, sensor_type)
    @influx.write_point("particle_samples", data)
  end

  private

  def msg values, location, sensor_type
    { values: values, tags: { location: location, sensor_type: sensor_type } }
  end

  def self.connect
    InfluxWrapper.new InfluxDB::Client.new("house", host: HOST)
  end
end

influx = InfluxWrapper.connect

MQTT::Client.connect HOST do |client|
  client.subscribe "home/+/esp/+"
  client.subscribe "home/+/esp8266aq/+"
  client.subscribe "home/+/+/esp8266aq/+"

  loop do
    topic, message = client.get

    if topic =~ /^(.*)\/(esp(?:8266aq)?)\/(.*)$/
      location = $1
      sensor_type = $2 == "esp" ? "fridge" : "room"
      measurement = $3
      case measurement
      when "humidity"
        influx.write_humidity location, message.to_f, sensor_type
      when "co2"
        influx.write_co2 location, message.to_i, sensor_type
      when "temperature"
        influx.write_temp location, message.to_f, sensor_type
      when "dewpoint"
        influx.write_dewpoint location, message.to_f, sensor_type
      when "pm2_5_standard"
        influx.write_pm25 location, message.to_i, sensor_type
      end
    end
  end
end
