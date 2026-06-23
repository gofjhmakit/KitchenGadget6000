#define LGFX_USE_V1
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include "CST816D.h"
#include "KitchenUI.h"

// -----------------------------------------------------------------------------
// Pin Configuration
// -----------------------------------------------------------------------------
#define POWER_LIGHT_PIN 40       // Power indicator LED (Blue)
#define LED_PIN 48               // Neopixel LED strip (5 pixels)
#define LED_NUM 5
#define ENCODER_A_PIN 45         // CLK
#define ENCODER_B_PIN 42         // DT
#define SWITCH_PIN 41            // Encoder press button

#define TP_INT 5                 // Touch interrupt
#define TP_RST 13                // Touch reset
#define SCREEN_BACKLIGHT_PIN 46  // Backlight PWM control

// I2C pins
#define TP_I2C_SDA_PIN 6
#define TP_I2C_SCL_PIN 7
#define I2C_SDA_PIN 38
#define I2C_SCL_PIN 39

// -----------------------------------------------------------------------------
// Global Drivers
// -----------------------------------------------------------------------------
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_GC9A01 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST; 
      cfg.spi_mode = 0;      
      cfg.freq_write = 80000000;    
      cfg.freq_read = 20000000;         
      cfg.spi_3wire = true;      
      cfg.use_lock = true;       
      cfg.dma_channel = SPI_DMA_CH_AUTO;  
      cfg.pin_sclk = 10; 
      cfg.pin_mosi = 11;  
      cfg.pin_miso = -1; 
      cfg.pin_dc = 3;  
      _bus_instance.config(cfg);          
      _panel_instance.setBus(&_bus_instance); 
    }
    {                            
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 9;                 
      cfg.pin_rst = 14;       
      cfg.pin_busy = -1;         
      cfg.memory_width = 240; 
      cfg.memory_height = 240;  
      cfg.panel_width = 240;
      cfg.panel_height = 240; 
      cfg.offset_x = 0;   
      cfg.offset_y = 0;   
      cfg.offset_rotation = 0;  
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;  
      cfg.readable = false;  
      cfg.invert = true;    
      cfg.rgb_order = false;   
      cfg.dlen_16bit = false;   
      cfg.bus_shared = false;   
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);  
  }
};

LGFX gfx;
CST816D touch(TP_I2C_SDA_PIN, TP_I2C_SCL_PIN, TP_RST, TP_INT);
Adafruit_NeoPixel led = Adafruit_NeoPixel(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);

// Screen backlight PWM settings
const int pwmFreq = 5000;
const int pwmChannel = 0;
const int pwmResolution = 8;

// Display Buffers for LVGL in PSRAM
static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf = NULL;
static lv_color_t *buf1 = NULL;

// RTOS Task Handles
TaskHandle_t encTaskHandle = NULL;
TaskHandle_t appTaskHandle = NULL;

// WiFi & API credentials — edit secrets.h, not here
#include "secrets.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* hc_access_token = HC_ACCESS_TOKEN;
const char* hc_appliance_id = HC_APPLIANCE_ID; // Optional: Set your dishwasher haId. Leave empty to auto-discover.
const unsigned long hc_poll_interval_ms = 20000;

static String hc_cached_haid = "";
static bool hc_missing_config_warned = false;

// -----------------------------------------------------------------------------
// Home Connect Helpers
// -----------------------------------------------------------------------------
String hc_unquote(const String& in) {
  if (in.length() >= 2 && in[0] == '"' && in[in.length() - 1] == '"') {
    return in.substring(1, in.length() - 1);
  }
  return in;
}

bool hc_get_json_field_after_key(const String& json, const String& key, String& value_out) {
  int key_pos = json.indexOf("\"key\":\"" + key + "\"");
  if (key_pos < 0) {
    return false;
  }

  int value_pos = json.indexOf("\"value\":", key_pos);
  if (value_pos < 0) {
    return false;
  }
  value_pos += 8;

  while (value_pos < (int)json.length() && (json[value_pos] == ' ' || json[value_pos] == '\n' || json[value_pos] == '\r' || json[value_pos] == '\t')) {
    value_pos++;
  }

  if (value_pos >= (int)json.length()) {
    return false;
  }

  if (json[value_pos] == '"') {
    int end = value_pos + 1;
    while (end < (int)json.length()) {
      if (json[end] == '"' && json[end - 1] != '\\') {
        break;
      }
      end++;
    }
    if (end >= (int)json.length()) {
      return false;
    }
    value_out = json.substring(value_pos + 1, end);
    return true;
  }

  int end = value_pos;
  while (end < (int)json.length()) {
    char c = json[end];
    if (c == ',' || c == '}' || c == ']') {
      break;
    }
    end++;
  }
  value_out = json.substring(value_pos, end);
  value_out.trim();
  return value_out.length() > 0;
}

bool hc_http_get(const String& path, String& body, int* status_code_out = nullptr) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  const String url = String("https://api.home-connect.com/api") + path;
  if (!http.begin(client, url)) {
    return false;
  }

  http.addHeader("Authorization", String("Bearer ") + hc_access_token);
  http.addHeader("Accept", "application/vnd.bsh.sdk.v1+json");

  int status_code = http.GET();
  if (status_code_out) {
    *status_code_out = status_code;
  }
  if (status_code > 0) {
    body = http.getString();
  } else {
    body = "";
  }
  http.end();
  return status_code >= 200 && status_code < 300;
}

bool hc_resolve_dishwasher_id(String& out_haid) {
  if (strlen(hc_appliance_id) > 0) {
    out_haid = String(hc_appliance_id);
    return true;
  }

  if (hc_cached_haid.length() > 0) {
    out_haid = hc_cached_haid;
    return true;
  }

  String body;
  if (!hc_http_get("/homeappliances", body)) {
    return false;
  }

  const String marker = "\"haId\":\"";
  String first_dishwasher = "";
  int cursor = 0;
  while (true) {
    int ha_pos = body.indexOf(marker, cursor);
    if (ha_pos < 0) {
      break;
    }

    int id_start = ha_pos + marker.length();
    int id_end = body.indexOf('"', id_start);
    if (id_end < 0) {
      break;
    }

    int obj_start = body.lastIndexOf('{', ha_pos);
    int obj_end = body.indexOf('}', id_end);
    if (obj_start < 0 || obj_end < 0) {
      cursor = id_end + 1;
      continue;
    }

    String obj = body.substring(obj_start, obj_end + 1);
    bool is_dishwasher = (obj.indexOf("\"type\":\"Dishwasher\"") >= 0);
    bool is_connected = (obj.indexOf("\"connected\":true") >= 0);

    if (is_dishwasher) {
      String haid = body.substring(id_start, id_end);
      if (first_dishwasher.length() == 0) {
        first_dishwasher = haid;
      }
      if (is_connected) {
        hc_cached_haid = haid;
        out_haid = haid;
        return true;
      }
    }

    cursor = id_end + 1;
  }

  if (first_dishwasher.length() > 0) {
    hc_cached_haid = first_dishwasher;
    out_haid = first_dishwasher;
    return true;
  }

  return false;
}

String hc_map_operation_state(const String& operation_state) {
  if (operation_state.indexOf("Run") >= 0) return "Running";
  if (operation_state.indexOf("Pause") >= 0) return "Paused";
  if (operation_state.indexOf("DelayedStart") >= 0) return "Delayed";
  if (operation_state.indexOf("ActionRequired") >= 0) return "Attention";
  if (operation_state.indexOf("Finished") >= 0) return "Finished";
  if (operation_state.indexOf("Error") >= 0) return "Error";
  if (operation_state.indexOf("Ready") >= 0) return "Ready";
  if (operation_state.indexOf("Inactive") >= 0) return "Inactive";
  return "Syncing";
}

void hc_poll_dishwasher_status() {
  if (strlen(hc_access_token) < 20 || String(hc_access_token).startsWith("YOUR_")) {
    if (!hc_missing_config_warned) {
      Serial.println("Home Connect disabled: set hc_access_token to enable dishwasher sync.");
      hc_missing_config_warned = true;
    }
    return;
  }

  String haid;
  if (!hc_resolve_dishwasher_id(haid)) {
    return;
  }

  String program_name = "Dishwasher";
  String operation_state = "";
  int remaining_seconds = -1;
  int total_minutes = 1;

  // Active program (may return 404 when nothing is running)
  {
    String body;
    int status_code = 0;
    String path = "/homeappliances/" + haid + "/programs/active";
    bool ok = hc_http_get(path, body, &status_code);
    if (ok) {
      int name_pos = body.indexOf("\"name\":\"");
      if (name_pos >= 0) {
        int start = name_pos + 8;
        int end = body.indexOf('"', start);
        if (end > start) {
          program_name = body.substring(start, end);
        }
      }
    } else if (status_code == 404) {
      program_name = "No active program";
    }
  }

  // Appliance status keys for operation state and remaining time
  {
    String body;
    String path = "/homeappliances/" + haid + "/status";
    if (hc_http_get(path, body)) {
      String op_value;
      if (hc_get_json_field_after_key(body, "BSH.Common.Status.OperationState", op_value)) {
        operation_state = op_value;
      }

      String remaining_value;
      if (!hc_get_json_field_after_key(body, "BSH.Common.Option.RemainingProgramTime", remaining_value)) {
        hc_get_json_field_after_key(body, "Dishcare.Dishwasher.Option.RemainingProgramTime", remaining_value);
      }
      if (remaining_value.length() > 0) {
        remaining_seconds = remaining_value.toInt();
      }

      String progress_value;
      if (hc_get_json_field_after_key(body, "BSH.Common.Option.ProgramProgress", progress_value)) {
        float progress = progress_value.toFloat();
        if (remaining_seconds > 0 && progress > 0.5f && progress < 99.5f) {
          float total_est_mins = (remaining_seconds / 60.0f) / (1.0f - (progress / 100.0f));
          total_minutes = (int)total_est_mins;
        }
      }
    }
  }

  int remaining_minutes = 0;
  if (remaining_seconds > 0) {
    remaining_minutes = (remaining_seconds + 59) / 60;
  }

  if (remaining_minutes > 0 && total_minutes < remaining_minutes) {
    total_minutes = remaining_minutes;
  } else if (remaining_minutes == 0) {
    total_minutes = 1;
  }

  String status_label = hc_map_operation_state(operation_state);
  if (remaining_minutes == 0 && status_label == "Running") {
    status_label = "Finishing";
  }

  kitchen_ui_set_dishwasher_data(status_label.c_str(), program_name.c_str(), remaining_minutes, total_minutes);
}

// -----------------------------------------------------------------------------
// Backlight Controller
// -----------------------------------------------------------------------------
void initBacklight() {
  // ESP32 S3: Use newer ledcAttach API
  ledcAttach(SCREEN_BACKLIGHT_PIN, pwmFreq, pwmResolution);
  ledcWrite(SCREEN_BACKLIGHT_PIN, (60 * 255) / 100); // 60% Brightness default
}

// -----------------------------------------------------------------------------
// LVGL Flush & Touch callbacks
// -----------------------------------------------------------------------------
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  if (gfx.getStartCount() > 0) {
    gfx.endWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  uint8_t gesture;
  bool touched = touch.getTouch(&touchX, &touchY, &gesture);

  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

// -----------------------------------------------------------------------------
// Low-Latency Encoder & Button Scanning Task (Core 0)
// -----------------------------------------------------------------------------
void encTask(void *pvParameters) {
  int lastStateCLK = digitalRead(ENCODER_A_PIN);
  int clickCount = 0;
  unsigned long lastPressTime = 0;
  const unsigned long doubleClickTime = 250; // ms
  
  bool prevBtnState = false;
  unsigned long btnPressTime = 0;
  bool longPressTriggered = false;

  while (1) {
    // 1. Scan Encoder Rotation
    int currentStateCLK = digitalRead(ENCODER_A_PIN);
    if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
      // Rising edge detected
      int stateDT = digitalRead(ENCODER_B_PIN);
      int8_t dir = (stateDT != currentStateCLK) ? -1 : 1; // CCW vs CW
      kitchen_ui_handle_rotation(dir);
    }
    lastStateCLK = currentStateCLK;

    // 2. Scan Switch Button (Active LOW)
    bool btnState = (digitalRead(SWITCH_PIN) == LOW);
    if (btnState != prevBtnState) {
      if (btnState) {
        // Pressed
        btnPressTime = millis();
        longPressTriggered = false;
      } else {
        // Released
        if (!longPressTriggered) {
          clickCount++;
          lastPressTime = millis();
        }
      }
      prevBtnState = btnState;
    }

    // Handle Long Press (held > 800ms)
    if (btnState && !longPressTriggered && (millis() - btnPressTime > 800)) {
      kitchen_ui_handle_long_press();
      longPressTriggered = true;
    }

    // Process Clicks
    if (clickCount == 1 && (millis() - lastPressTime > doubleClickTime)) {
      kitchen_ui_handle_click();
      clickCount = 0;
    } else if (clickCount >= 2) {
      kitchen_ui_handle_double_click();
      clickCount = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(2)); // Polling frequency 500Hz
  }
}

// -----------------------------------------------------------------------------
// Core Background Application Task (Core 1)
// -----------------------------------------------------------------------------
void appTask(void *pvParameters) {
  unsigned long last_time = millis();
  unsigned long last_wifi_check = 0;
  unsigned long last_hc_poll = 0;
  bool ntp_synced = false;

  while (1) {
    unsigned long current_time = millis();
    float delta_sec = (current_time - last_time) / 1000.0f;
    last_time = current_time;

    // 1. Update general UI calculations and timers
    kitchen_ui_update(delta_sec);

    // 2. WiFi Background Non-blocking Handler
    if (current_time - last_wifi_check > 5000) {
      last_wifi_check = current_time;
      bool is_connected = (WiFi.status() == WL_CONNECTED);
      if (is_connected) {
        kitchen_ui_set_wifi_status(true, WiFi.localIP().toString().c_str());
        
        // Sync time once over NTP
        if (!ntp_synced) {
          configTime(0, 0, "pool.ntp.org", "time.nist.gov");
          // Set to timezone offset (E.g. UTC+3 for Finland: 3 * 3600 = 10800)
          setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
          tzset();
          
          struct tm timeinfo;
          if (getLocalTime(&timeinfo)) {
            ntp_synced = true;
          }
        }
      } else {
        kitchen_ui_set_wifi_status(false, NULL);
      }
    }

    // 2b. Poll Bosch Home Connect dishwasher status
    if (WiFi.status() == WL_CONNECTED && (current_time - last_hc_poll > hc_poll_interval_ms)) {
      last_hc_poll = current_time;
      hc_poll_dishwasher_status();
    }

    // 3. Ambient LED animations
    uint8_t r, g, b;
    int state = kitchen_ui_get_led_state(&r, &g, &b);
    led.clear();

    if (state == 2) {
      // ALARM state: rapid strobe red flash
      bool blink = ((millis() / 150) % 2 == 0);
      if (blink) {
        for (int i = 0; i < LED_NUM; i++) {
          led.setPixelColor(i, led.Color(255, 0, 0));
        }
      }
    } else if (state == 1) {
      // TIMER running state: pulsing sine glow in the timer's color
      float pulse = (sin(millis() / 250.0f) + 1.0f) / 2.0f; // 0.0 to 1.0
      uint8_t pulse_r = (uint8_t)(r * (0.2f + 0.8f * pulse));
      uint8_t pulse_g = (uint8_t)(g * (0.2f + 0.8f * pulse));
      uint8_t pulse_b = (uint8_t)(b * (0.2f + 0.8f * pulse));
      for (int i = 0; i < LED_NUM; i++) {
        led.setPixelColor(i, led.Color(pulse_r, pulse_g, pulse_b));
      }
    } else {
      // IDLE state: very slow breathing of warm ambient glow
      float pulse = (sin(millis() / 1500.0f) + 1.0f) / 2.0f; // 0.0 to 1.0
      uint8_t glow_r = (uint8_t)(r * (0.4f + 0.3f * pulse));
      uint8_t glow_g = (uint8_t)(g * (0.4f + 0.3f * pulse));
      uint8_t glow_b = (uint8_t)(b * (0.4f + 0.3f * pulse));
      for (int i = 0; i < LED_NUM; i++) {
        led.setPixelColor(i, led.Color(glow_r, glow_g, glow_b));
      }
    }
    led.show();

    vTaskDelay(pdMS_TO_TICKS(20)); // Task runs at 50Hz
  }
}

// -----------------------------------------------------------------------------
// Setup & Loop
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Power configurations for safety
  pinMode(POWER_LIGHT_PIN, OUTPUT);
  digitalWrite(POWER_LIGHT_PIN, LOW); // Activate blue power light indicator
  
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  // Initialize hardware pins
  pinMode(ENCODER_A_PIN, INPUT);
  pinMode(ENCODER_B_PIN, INPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  // Initialize external I2C for potential expansions (shared with external devices)
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Initialize capacitive touch driver (uses Wire1 internally)
  touch.begin();

  // Initialize LovyanGFX display
  gfx.init();
  gfx.initDMA();
  gfx.startWrite();
  gfx.fillScreen(TFT_BLACK);

  // Initialize LVGL 8
  lv_init();

  // Allocate display buffers in PSRAM
  size_t buffer_size = sizeof(lv_color_t) * screenWidth * screenHeight;
  buf = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  buf1 = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  
  if (!buf || !buf1) {
    Serial.println("Error: Failed to allocate PSRAM memory buffers for LVGL!");
    while(1) delay(100);
  }
  
  lv_disp_draw_buf_init(&draw_buf, buf, buf1, screenWidth * screenHeight);

  // Register display driver to LVGL
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Register touch panel driver to LVGL
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // Initialize display backlight control
  initBacklight();

  // Initialize ambient LED strip
  led.begin();
  led.setBrightness(40); // Moderate max brightness
  led.clear();
  led.show();

  // Build the Kitchen UI structure
  kitchen_ui_init();

  // Non-blocking WiFi initialization
  Serial.println("Attempting to connect to WiFi in background...");
  WiFi.begin(ssid, password);

  // Pinned tasks initialization
  // Pinned to Core 0 (where networking/comms and quick ISR scanning should run)
  xTaskCreatePinnedToCore(encTask, "Encoder polling", 3072, NULL, 3, &encTaskHandle, 0);
  
  // Pinned to Core 1 (where the main sketch code runs, handles heavy UI drawings)
  xTaskCreatePinnedToCore(appTask, "App updating", 4096, NULL, 1, &appTaskHandle, 1);

  Serial.println("Kitchen Gadget firmware loaded successfully.");
}

void loop() {
  // Let the GUI do its work
  lv_timer_handler();
  delay(5);
}
