begin
  require "uchip/mcp2221"
rescue LoadError
  warn "UChip gem insn't installed, installing now"
  Gem.install "uchip"
  retry
end

class ESPClient
  FLASH_PIN = 0
  RST_PIN   = 1
  ESP_PIN14 = 2
  ESP_PIN12 = 3

  def self.open
    # Find the first connected chip
    chip = UChip::MCP2221.first
    return unless chip
    client = new chip
    client.ensure_pin_settings
    client
  end

  attr_reader :chip

  def initialize chip
    @chip = chip
  end

  # Make sure the power-up settings for the MCP are correct
  def ensure_pin_settings
    new_settings = check_gp_settings @chip.gp_settings
    if new_settings
      @chip.gp_settings = new_settings
      4.times do |i|
        chip.set_gpio_value i, (PIN_VALUE_BITMASK >> i) & 0x1
      end
    end
  end

  ##
  # Make sure +pin+ is an output pin and the default value is +val+
  def check_pin gp_settings, pin, val
    gp_settings.designation_at(pin) == 0 &&
      gp_settings.direction_at(pin) == 0 &&
      gp_settings.output_value_at(pin) == val
  end

  def set_pin gp_settings, pin, val
    gp_settings.set_designation_at(pin, 0)  # default to output
    gp_settings.set_direction_at(pin, 0)    # default to gpio function
    gp_settings.set_output_value_at(pin, val) # default output to val
  end

  # default pin value bitmask.  We want pin 0 and 1 to be HIGH, but 2 and 3
  # to be LOW
  PIN_VALUE_BITMASK = 0b0011

  def check_gp_settings gp_settings
    pin_bitmask = PIN_VALUE_BITMASK

    return false if 4.times.all? do |i|
      val = (pin_bitmask >> i) & 0x1
      check_pin gp_settings, i, val
    end

    4.times do |i|
      val = (pin_bitmask >> i) & 0x1
      set_pin gp_settings, i, val
    end

    gp_settings
  end

  def program!
    chip.set_gpio_value RST_PIN, 0
    chip.set_gpio_value FLASH_PIN, 0
    chip.set_gpio_value RST_PIN, 1

    sleep 0.5

    chip.set_gpio_value FLASH_PIN, 1
  end

  def esp_pin14
    chip.gpio_value ESP_PIN14
  end

  def esp_pin14= v
    chip.set_gpio_value ESP_PIN14, v
  end

  alias gp2 esp_pin14
  alias gp2= esp_pin14=

  def esp_pin12
    chip.gpio_value ESP_PIN12
  end

  def esp_pin12= v
    chip.set_gpio_value ESP_PIN12, v
  end

  alias gp3 esp_pin12
  alias gp3= esp_pin12=
end
