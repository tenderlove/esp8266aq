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

  def write_temp location, value
    data = {
      :values => { :value => value, :unit => "C" },
      :tags   => { :location => location }
    }

    @influx.write_point("temperatures", data)
  end

  def write_humidity location, value
    data = {
      :values => { :value => value, :unit => "%" },
      :tags   => { :location => location }
    }

    @influx.write_point("humidity_samples", data)
  end

  def write_dewpoint location, value
    data = {
      :values => { :value => value, :unit => "C" },
      :tags   => { :location => location }
    }

    @influx.write_point("dewpoint_samples", data)
  end

  def write_pm25 location, sample
    values = sample.to_h.select { |k, _| k =~ /^(pm|particle|concentration)/ }
    tags   = { location: location }
    data   = { :values => values, :tags   => tags }

    @influx.write_point("particle_samples", data)
  end

  def self.connect
    InfluxWrapper.new InfluxDB::Client.new("house", host: HOST)
  end
end

influx = InfluxWrapper.connect

MQTT::Client.connect HOST do |client|
  client.get "home/+/esp8266aq/+" do |topic, message|
    if topic =~ /^(.*\/esp8266aq)\/(.*)$/
      location = $1
      measurement = $2
      case measurement
      when "humidity"
        influx.write_humidity location, message.to_f
      when "temperature"
        influx.write_temp location, message.to_f
      when "dewpoint"
        influx.write_dewpoint location, message.to_f
      when "pm2_5_standard"
        influx.write_pm25(location, { "pm2_5_standard" => message.to_i })
      end
    end
  end
end
