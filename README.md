# ESP32 Smart Hub Bluetooth Home Automation System

A compact **ESP32-based smart home automation system** that controls a servo driven door lock, a light circuit, and status alerts (LED + buzzer)  all over **Bluetooth Classic (SPP)**  with real-time feedback on an **OLED display**.

![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B%20(Arduino)-informational)
![Bluetooth](https://img.shields.io/badge/comm-Bluetooth%20Classic%20(SPP)-brightgreen)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

## Features

-  **PIN-protected smart door lock**  servo motor toggles between locked (0°) and open (90°) positions after a correct PIN over Bluetooth.
-  **Light control**  toggle via Bluetooth command *or* a physical push button (debounced), independent of BT connection state.
-  **Live OLED status screen** (SSD1306, 128×64) showing:
  - Bluetooth connect/disconnect toast messages
  - PIN entry prompt
  - Command feedback (`LIGHT ON`, `WRONG PIN!`, `WRONG COMMAND`, etc.)
  - Default home screen with door + light status
- 🟢 **Green LED** flashes briefly to confirm a successful door action.
- 🔴 **Red LED + buzzer alarm** triggers on a wrong PIN or unrecognized command, auto-clearing after a timeout.
-  Fully **non-blocking timing** using `millis()`  no `delay()` calls in the main logic path.

---

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32-WROOM-32 Dev Board | Main microcontroller + Bluetooth Classic |
| SSD1306 OLED Display (128×64, I2C) | Status/feedback display |
| SG90 (or similar) Servo Motor | Drives the door lock mechanism |
| Push Button | Manual light toggle |
| Green LED | Door action confirmation |
| Red LED | Wrong PIN / error alarm indicator |
| Buzzer | Audible alarm on wrong PIN/command |
| Resistors (200 Ω, 2.00 kΩ) | LED current limiting / pull-down |
| Battery Pack | Standalone power supply |

### Pin Mapping

| Function | GPIO |
|---|---|
| Push Button | `GPIO 4` |
| Light Relay/Output | `GPIO 2` |
| Green LED | `GPIO 5` |
| Red LED | `GPIO 18` |
| Buzzer | `GPIO 26` |
| Servo Motor | `GPIO 15` |
| OLED (I2C) | SDA/SCL via `Wire` (address `0x3C`) |

### Circuit Diagram

The full wiring layout (designed in **Cirkit Designer**) shows the ESP32, breadboard resistor/button network, OLED, servo, LEDs, and battery power path:

🔗 [View Circuit Diagram](https://postimg.cc/tnh3H96m)

---

## Bluetooth Commands

Connect to the device via Bluetooth Classic  it advertises as:

```
ESP32_Smart_Hub
```

| Command | Action |
|---|---|
| `L` or `l` | Toggle light on/off |
| `1` | Enter PIN-input mode (device waits for the next character) |
| `9` *(after `1`)* | Correct PIN → toggles door lock (servo 0° ↔ 90°) |
| *anything else* | Triggers "WRONG COMMAND" alarm (red LED + buzzer) |

## How It Works

The firmware runs a single non-blocking `loop()` that:
1. Polls Bluetooth connection state and flashes a temporary "connected/disconnected" toast on the OLED.
2. Reads the push button with simple debounce logic to toggle the light independently of Bluetooth.
3. Parses incoming Bluetooth bytes as a lightweight command protocol, with a `waitingForPin` state machine for secure door actuation.
4. Manages LED/buzzer/alarm timing windows via `millis()` comparisons instead of blocking `delay()` calls.
5. Renders context-aware screens to the OLED  prioritizing BT status → PIN prompt → action feedback → default home screen.

---

## Roadmap

- [ ] Multi-digit PIN with masked input feedback
- [ ] Encrypted/paired Bluetooth authentication
- [ ] Wi-Fi + mobile app control alongside Bluetooth
- [ ] Persistent state storage (EEPROM/NVS) across reboots
- [ ] Multiple relay-controlled appliances

---

## Authors

Built by:
- **[obsfusc8](https://github.com/obsfusc8)**
- **[Mohammad35786](https://github.com/Mohammad35786)**

---

## License

This project is open-sourced under the [MIT License](LICENSE).
