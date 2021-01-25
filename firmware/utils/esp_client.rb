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
    @chip.gp_settings = new_settings if new_settings
  end

  def check_gp_settings gp_settings
    return false if 4.times.all? do |i|
      gp_settings.designation_at(i) == 0 &&
        gp_settings.direction_at(i) == 0 &&
        gp_settings.output_value_at(i) == 1
    end

    4.times do |i|
      gp_settings.set_designation_at(i, 0)  # default to output
      gp_settings.set_direction_at(i, 0)    # default to gpio function
      gp_settings.set_output_value_at(i, 1) # default output to high
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
end
