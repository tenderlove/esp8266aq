# This program tells the MCP2221 to put the ESP in to programming mode.

require_relative "esp_client"

chip = ESPClient.open

unless chip
  warn "Couldn't find MCP2221. Is it plugged in?"
  exit 1
end

chip.program!
