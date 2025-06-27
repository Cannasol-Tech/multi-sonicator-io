# Arduino R4 UNO WiFi as ISP

## Wired Connections

> <UNO_R4_WIFI_PIN> -> <ATmega32A_Pin>

### Numbered Connections

1. **D10** -> 9         `/RESET`
2. **D11** -> 6 (PB5)   `MOSI`
3. **D12** -> 7 (PB6)   `MISO`
4. **D13** -> 8 (PB7)   `SCK`
5. **5V** -> 10/30      `VCC`
6. **GND** -> 11/31     `GND`

### ISP Connection Table

| UNO R4 WiFi | ATmega32A (DIP-40) | Signal |
|-------------|--------------------|--------|
| D10         | 9                  | /RESET |
| D11         | 6 (PB5)            | MOSI   |
| D12         | 7 (PB6)            | MISO   |
| D13         | 8 (PB7)            | SCK    |
| 5V (or 3V3) | 10 / 30            | VCC    |
| GND         | 11 / 31            | GND    |

***Note:***
Add a 10 µF capacitor between RESET and GND on the UNO R4 so it doesn't auto-reset while acting as a programmer (leave the cap off when you need to re-flash the R4 itself).
