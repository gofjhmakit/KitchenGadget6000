# CrowPanel 1.28" HMI Rotary Display - Kitchen Helper Documentation

This document contains technical research, pin mapping, audio expansion strategies, and system design specifications for building a Kitchen Helper tool using the **CrowPanel 1.28-inch HMI ESP32 Rotary Display**.

---

## 1. Hardware Overview & Specifications

The CrowPanel 1.28-inch is a circular smart display powered by the ESP32-S3.

*   **Main controller**: ESP32-S3R8 (Xtensa 32-bit LX7 dual-core processor, up to 240MHz).
*   **Memory**: 512KB SRAM, 8MB PSRAM (OPI), 16MB Flash.
*   **Display**: 1.28-inch circular IPS TFT screen (240x240 resolution) driven by a **GC9A01** SPI driver.
*   **Touch screen**: Capacitive touch driven by a **CST816D** I2C controller.
*   **Primary Inputs**: Tactile rotary encoder (EC3501) with integrated push switch.
*   **Visual Indicators**: Power indicator LED, and a strip of 5 addressable WS2812B NeoPixel RGB LEDs.
*   **Power Input**: 5V/1A via FPC burning port or 1.25mm connectors.

---

## 2. Complete Pin Mapping

Below is the definitive pinout for the ESP32-S3 microcontroller on the device, derived from the hardware schematics:

### Rotary Encoder & Switch
| Pin Name | ESP32 GPIO | Description |
| :--- | :--- | :--- |
| `ENCODER_A_PIN` | **GPIO45** | Encoder Phase A (CLK) - Used to detect rotation edge. |
| `ENCODER_B_PIN` | **GPIO42** | Encoder Phase B (DT) - Used to detect direction. |
| `SWITCH_PIN` | **GPIO41** | Rotary press button (Active LOW, requires internal pull-up). |

### Visuals & Backlight
| Pin Name | ESP32 GPIO | Description |
| :--- | :--- | :--- |
| `LED_PIN` | **GPIO48** | Data line for 5x WS2812B NeoPixel RGB LEDs. |
| `SCREEN_BACKLIGHT_PIN` | **GPIO46** | Screen Backlight (controlled via PWM/LEDC). |
| `POWER_LIGHT_PIN` | **GPIO40** | Blue power indicator LED (Active HIGH). |

### GC9A01 SPI Display Interface
| Display Pin | ESP32 GPIO | Description |
| :--- | :--- | :--- |
| `MOSI` | **GPIO11** | SPI Master Out Slave In |
| `SCLK` | **GPIO10** | SPI Clock |
| `CS` | **GPIO9** | Chip Select |
| `DC` | **GPIO3** | Data / Command Selection |
| `RST` | **GPIO14** | Display Reset |

### CST816D I2C Touch Interface
| Touch Pin | ESP32 GPIO | Description |
| :--- | :--- | :--- |
| `TP_SDA` | **GPIO6** | I2C Data line |
| `TP_SCL` | **GPIO7** | I2C Clock line |
| `TP_INT` | **GPIO5** | Touch Interrupt pin |
| `TP_RST` | **GPIO13** | Touch Reset pin |

---

## 3. External Connectors & Audio Integration

The device does **not** have an onboard buzzer or speaker. Audio must be added externally. 

The board exposes three 1.25mm 4-pin connectors:

```
[ J34 (UART0) ]       [ J36 (I2C) ]         [ J37 (USB) ]
1: RXD0 (GPIO44)     1: SCL (GPIO39)       1: VUSB_IN
2: TXD0 (GPIO43)     2: SDA (GPIO38)       2: USB D-
3: OUT_5V            3: OUT_5V             3: USB D+
4: GND               4: GND                4: GND
```

Additionally, there is an **internal 12-pin FPC connector (`U$27`)** used for flashing, which exposes:
*   Pins 1-2: `VCC_5V` | Pins 11-12: `GND`
*   Pins 3-4: **GPIO4** and **GPIO12** (Free GPIOs, not routed to external ports)
*   Pins 5-10: Serial RX/TX, I2C SDA/SCL, USB D-/D+

### Audio Implementation Strategies

Since `J34` is shared with the bootloader/serial console, **the I2C port (`J36`) is the best location to connect external audio hardware.**

#### Strategy 1: Active or Passive Buzzer (Simple & Cost-Effective)
*   **Connection**:
    *   VCC $\rightarrow$ 5V (J36 pin 3)
    *   GND $\rightarrow$ GND (J36 pin 4)
    *   I/O Signal $\rightarrow$ **GPIO38** (J36 pin 2)
*   **Active Buzzer**: Set GPIO38 to `HIGH` to sound a fixed 2-3kHz beep.
*   **Passive Buzzer (Recommended)**: Drive GPIO38 with PWM (using the Arduino `tone()` function or ESP32 `ledc` peripheral) to play distinct frequencies, alarm chirps, or musical alerts.

#### Strategy 2: DFPlayer Mini MP3 Module (High-Quality Voice & Audio)
*   **Connection**:
    *   VCC $\rightarrow$ 5V (J36 pin 3)
    *   GND $\rightarrow$ GND (J36 pin 4)
    *   TX / RX $\rightarrow$ **GPIO38** (J36 pin 2) and **GPIO39** (J36 pin 1)
*   **Firmware Configuration**: The ESP32-S3's internal GPIO matrix allows routing the `Serial1` hardware peripheral to any GPIO pins. Configure `Serial1` to use GPIO38 as TX and GPIO39 as RX.
*   **Control**: Use the `DFRobotDFPlayerMini` library to trigger MP3 files (e.g. voice announcements like *"Timer 1 completed"* or custom sound effects) stored on a micro-SD card.

---

## 4. Kitchen Helper System & UI Design

To optimize the rotary control for quick and intuitive operation in a kitchen setting, the following design guidelines should be used.

### Tactical Rotary Control State Machine
*   **Idle / Main Screen**: Shows current time, active timers, or kitchen dashboards.
*   **Quick Timer Creation (Double-Click)**:
    *   An interrupt on `SWITCH_PIN` (GPIO41) registers a quick double-click.
    *   The UI transitions immediately to the `ui_AddTimerScreen`.
*   **Adjusting Time (Rotation)**:
    *   Turning the knob reads the encoder states on `GPIO45` (A) and `GPIO42` (B).
    *   **Base increment**: 5 seconds per encoder click.
    *   **Adaptive Velocity Scaling**: Software measures time between clicks. If rotated rapidly, the step size increases to 30 seconds or 1 minute. This allows setting long timers quickly.
*   **Confirm Timer (Single-Click)**:
    *   Pressing the encoder confirms the duration, adds the timer to the active list, and returns to the main screen.

### Circular Screen Visualization
The GC9A01 circular screen should leverage concentric elements for multi-timer layouts:
*   **Concentric Countdown Rings**: Up to 3 active timers are displayed as concentric progress rings around the edge of the circular layout (shrinking to zero as time expires).
*   **Focus Center**: The center space displays the numerical time remaining for the timer closest to expiration.
*   **LED Association**: The 5 bottom NeoPixels (`GPIO48`) are mapped to the active timers' colors. When a timer starts running, its associated NeoPixel pulses.
*   **Alarm Notification**: Upon expiration, the screen flashes red, the buzzer/MP3 alarms, and the NeoPixels run a red emergency chase animation. Clicking the knob once silences the active alarm.

---

## 5. Software-Only Kitchen Applications (Zero-Integration)

These features run entirely on the display device itself, leveraging only the screen, rotary input, and ESP32 processor, without requiring any external sensors, smart scale integrations, or network connections.

1.  **Tactile Measurement & Unit Converter**:
    *   *Concept*: A quick-conversion utility for volume, weight, and temperature.
    *   *Interface*:
        1. Rotate to select dimension (e.g., Volume, Weight, Temperature) and press to confirm.
        2. Rotate to select "Convert From" unit (e.g., Cups) and press.
        3. Rotate to dial in the amount (shown in large numbers) and press.
        4. Rotate to select "Convert To" unit (e.g., Milliliters) and view the result in real-time.
    *   *Supported Units*: Cups, Milliliters, Fluid Ounces, Tablespoons, Teaspoons, Grams, Ounces, Celsius, Fahrenheit.

2.  **Recipe Portion Scaler**:
    *   *Concept*: Quickly scales ingredient amounts up or down for different serving sizes.
    *   *Interface*:
        1. Rotate the knob to select a scaling multiplier (preset factors: `0.25x`, `0.5x`, `0.75x`, `1.5x`, `2x`, `3x`, `4x`).
        2. Click to lock the multiplier.
        3. Rotate the knob to dial in any ingredient quantity (e.g., `250` grams or `1.5` cups).
        4. The scaled quantity (e.g., `375` grams or `2.25` cups for `1.5x`) is calculated instantly and displayed on the circular screen.

3.  **Egg & Tea Brewing Assistant (Preset Profiles)**:
    *   *Concept*: A visual guide and automatic timer for common time-sensitive kitchen prep.
    *   *Interface*:
        *   Rotate to select a profile:
            *   *Egg Profiles*: Soft-boiled (6m), Jammy/Medium (7m 30s), Hard-boiled (10m).
            *   *Tea Profiles*: Green Tea (3m), Black Tea (5m), Herbal Tea (7m).
        *   Clicking the knob starts the pre-defined countdown timer immediately.

4.  **Baker's Sourdough Hydration Calculator**:
    *   *Concept*: A calculator for bread makers to balance flour and water ratios.
    *   *Interface*:
        1. Rotate to dial in the weight of the flour (e.g., `500g`) and click.
        2. Rotate to select the target hydration percentage (e.g., `72%`) and click.
        3. The screen displays the exact water weight needed (`360g`) and starter weight based on standard baker's percentages.

5.  **Beverage Chilling Estimator & Timer**:
    *   *Concept*: Calculates how long a warm drink needs to sit in the freezer or refrigerator to reach optimal drinking temperature.
    *   *Interface*:
        1. Select the beverage container (Can, Glass Bottle, Plastic Bottle).
        2. Select the cooling location (Freezer vs. Refrigerator).
        3. Select the starting temperature (Room Temp vs. Warm/Pantry).
        4. The algorithm sets a custom timer (e.g., `45 minutes` for a glass bottle in the freezer) and starts the countdown to prevent exploding bottles.

6.  **Meat Doneness Reference Guide**:
    *   *Concept*: A quick-reference cheat sheet for target cooking temperatures.
    *   *Interface*:
        *   Rotate the dial to cycle through icons of Beef, Poultry, Pork, or Fish.
        *   Click to view a list of doneness levels (Rare, Medium-Rare, Medium, Well-Done) and their corresponding target internal temperatures in both °C and °F (e.g., Beef Medium-Rare $\rightarrow$ 57°C / 135°F) for quick cross-referencing with a manual food thermometer.

---

## 6. Hardware-Integrated & IoT Kitchen Use Cases

These advanced applications expand the device's capability by connecting to external electronics or smart home servers:

1.  **Oven/Grill Thermometer**: Connects to an external thermocouple sensor (e.g., MAX31855) through the `J36` I2C port. The screen shows the current internal temperature, and the rotary dial sets a threshold alarm.
2.  **Bluetooth Scale Display**: Connects via BLE to smart kitchen scales, displaying live weights in grams/ounces and using the button to tare.
3.  **Smart Kitchen Controller**: Uses WiFi to communicate via MQTT/HTTP to Home Assistant, allowing the rotary dial to dim kitchen lights, change speaker volumes, or adjust range hood fan speeds.
4.  **Connected Recipe Guide**: Fetches step-by-step recipes over WiFi. Turning the dial scrolls steps; clicking the dial starts pre-programmed cooking timers for that specific step.

