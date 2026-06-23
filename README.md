# KitchenGadget6000

A round kitchen helper gadget with a rotary encoder, circular display, and RGB LEDs. It rotates, it rotates the other direction, and it clicks!

---

## Hardware

**Device:** [Elecrow CrowPanel 1.28" HMI ESP32 Rotary Display](https://www.elecrow.com/wiki/index.php?title=ESP32_Display_1.28%27%27_Rotary)

| Component | Details |
|---|---|
| MCU | ESP32-S3R8 (dual-core LX7, up to 240 MHz) |
| Memory | 512 KB SRAM, 8 MB PSRAM (OPI), 16 MB Flash |
| Display | 1.28" circular IPS TFT, 240×240, GC9A01 SPI driver |
| Touch | Capacitive, CST816D I2C controller |
| Input | Tactile rotary encoder (EC3501) with push switch |
| LEDs | 5× WS2812B NeoPixel RGB strip + blue power indicator |
| Power | 5V / 1A via FPC port or 1.25 mm connectors |
| Connectivity | Wi-Fi (ESP32-S3 built-in) |

---

## Software Features

Nine screens are available, navigated by rotating the dial and pressing to select. Double-click from any screen returns to the main menu.

| Screen | Description |
|---|---|
| **Timers** | Up to 3 concurrent countdown timers displayed as concentric progress rings. Alarm triggers LED flash and color chase. |
| **Unit Converter** | Convert volume, weight, and temperature between common kitchen units (cups, ml, oz, g, °C, °F, etc.). |
| **Portion Scaler** | Scale any ingredient quantity by preset multipliers (0.25×–4×). |
| **Brewing** | Preset profiles for tea (Green, Black, Herbal) and eggs (Soft, Jammy, Hard-boiled) that launch a timer automatically. |
| **Bread Hydration** | Baker's hydration calculator — dial in flour weight and target hydration % to get exact water weight. |
| **Drink Chiller** | Estimates freezer/fridge chilling time based on container type and starting temperature. |
| **Meat Temperatures** | Quick-reference guide for target internal temperatures (Beef, Poultry, Pork, Fish) in °C and °F. |
| **Dishwasher** | IoT status monitor — polls the Bosch Home Connect API over Wi-Fi to show remaining program time and operation state. |

### Planned / Future
- Oven/grill thermometer via external thermocouple (MAX31855 over I2C)
- Bluetooth kitchen scale display
- Home Assistant / MQTT integration
- Connected recipe guide with per-step timers

---

## Requirements

### Arduino IDE
- Arduino IDE **2.x** (recommended) or 1.8.x
- ESP32 board support package: **esp32 by Espressif** ≥ 3.0.0
  - Add in *File → Preferences → Additional boards manager URLs:*
    `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
  - Then install via *Tools → Board → Boards Manager*

### Libraries (install via Library Manager)
| Library | Purpose |
|---|---|
| **LVGL** | UI framework (graphics, widgets) |
| **LovyanGFX** | Display driver for GC9A01 |
| **Adafruit NeoPixel** | WS2812B LED control |

> The CST816D touch driver is bundled in the project — no separate install needed.

### Credentials
Copy `secrets.h.example` (or create `secrets.h` manually) and fill in your values:

```h
#define WIFI_SSID     "your_network_name"
#define WIFI_PASSWORD "your_wifi_password"

// From https://developer.home-connect.com — only needed for Dishwasher screen
#define HC_ACCESS_TOKEN  "your_home_connect_bearer_token"
#define HC_APPLIANCE_ID  ""   // leave empty to auto-discover
```

`secrets.h` is listed in `.gitignore` and will never be committed.

---

## Uploading to the Device

### Board settings (Arduino IDE → Tools)
| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3MB APP / 9.9MB FATFS) |
| PSRAM | OPI PSRAM |
| Upload Speed | 921600 |

### Steps

1. Clone or download this repository.
2. Create `secrets.h` in the project folder and fill in your credentials (see above).
3. Open `KitchenGadget.ino` in Arduino IDE — all source files in the folder are included automatically.
4. Connect the device via USB.
5. Select the correct port under *Tools → Port*.
6. Click **Upload** (Ctrl+U).

### Arduino CLI (alternative)

```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32s3 KitchenGadget6000/

# Upload (replace /dev/ttyUSB0 with your port)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32s3 KitchenGadget6000/

# Serial monitor
arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
```

