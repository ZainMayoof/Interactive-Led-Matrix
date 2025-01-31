# Interactive-Led-Matrix
An interactive 6x6 LED grid that responds to capacitive touch input. Built using an Arduino MEGA, MPR121 capacitive touch sensors, and WS2812 addressable LEDs, this project combines light and touch interaction.

## 📌 Features
✅ 6x6 grid of individually addressable LEDs
✅ Capacitive touch interaction using MPR121 sensors
✅ Expandable to include sound effects for interactive feedback

## 🛠 Hardware Requirements
| Component                   | Quantity | Notes                                      |
|-----------------------------|----------|--------------------------------------------|
| Arduino MEGA                | 1        | Required for memory constraints            |
| WS2812 LED Strip            | 2 (10m)  | Addressable LEDs                           |
| MPR121 Touch Sensor         | 3        | Handles 12 touch electrodes per board      |
| TCA9548A Multiplexer        | 1        | Expands I2C channels for multiple MPR121   |
| Wood + Acrylic Sheets       | 1 each   | Structure for housing LEDs and sensors     |
| Copper Mesh                 | 36       | For capacitive touch surface               |
| Power Supply (5V)           | 1        | Must support LEDs and sensors              |
| Jumper Wires                | Many     | For connections                            |

## 💾 Required Libraries
- Adafruit MPR121
- FastLED


## DEMO VIDEO
https://imgflip.com/gif/9impmh
