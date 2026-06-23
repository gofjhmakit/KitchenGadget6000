/**
 * TEA STEEPING TIMES DATABASE
 * 
 * Professional tea steeping times (seconds) with water temperatures.
 * Based on specialty tea supplier recommendations and SCAA standards.
 * 
 * Format: {name, steep_seconds, temp_celsius}
 */

#ifndef TEA_STEEPING_TIMES_H
#define TEA_STEEPING_TIMES_H

struct TeaPreset {
  const char* name;
  uint16_t steep_seconds;
  uint16_t water_temp_c;  // Water temperature in Celsius
};

// Comprehensive tea database
const TeaPreset tea_presets[] = {
  // BLACK TEAS (93-100°C)
  {"Black Tea (General)", 300, 100},
  {"Assam (Malty)", 270, 100},
  {"Ceylon (Brisk)", 240, 100},
  {"Darjeeling (Delicate)", 180, 90},
  {"Earl Grey", 300, 100},
  {"English Breakfast", 300, 100},
  {"Pu-erh (Aged)", 420, 100},
  
  // GREEN TEAS (70-75°C)
  {"Green Tea (General)", 180, 75},
  {"Japanese Sencha", 180, 70},
  {"Matcha (Whisk)", 60, 70},
  {"Dragon Well (Longjing)", 180, 70},
  {"Jasmine Green", 300, 75},
  
  // WHITE TEAS (60-75°C)
  {"White Tea (General)", 300, 70},
  {"Silver Needle", 300, 70},
  {"White Peony", 300, 70},
  {"White Oolong", 300, 70},
  
  // OOLONG TEAS (85-100°C)
  {"Oolong (General)", 300, 95},
  {"Tie Guan Yin", 180, 90},
  {"Da Hong Pao", 180, 90},
  {"Taiwan High Mountain", 300, 100},
  
  // HERBAL INFUSIONS (93-100°C)
  {"Herbal (General)", 420, 100},
  {"Chamomile", 420, 100},
  {"Peppermint", 300, 100},
  {"Rooibos (Red Bush)", 600, 100},
  {"Hibiscus", 360, 100},
  {"Ginger", 540, 100},
  {"Turmeric (Golden Milk)", 600, 100},
  
  // YERBA MATE & SIMILAR
  {"Yerba Mate", 300, 70},
  {"Yerba Mate (Hot)", 420, 100},
  
  // EGG COOKER PRESETS (for reference)
  {"Soft-Boiled Egg", 360, 100},
  {"Medium Egg", 450, 100},
  {"Hard-Boiled Egg", 600, 100}
};

#define TEA_PRESET_COUNT (sizeof(tea_presets) / sizeof(TeaPreset))

/**
 * Helper: Get tea steep time in seconds
 */
inline uint16_t get_tea_steep_time(int tea_idx) {
  if (tea_idx < 0 || tea_idx >= TEA_PRESET_COUNT) {
    return 300;  // Default to 5 minutes
  }
  return tea_presets[tea_idx].steep_seconds;
}

/**
 * Helper: Get tea display name
 */
inline const char* get_tea_name(int tea_idx) {
  if (tea_idx < 0 || tea_idx >= TEA_PRESET_COUNT) {
    return "Unknown";
  }
  return tea_presets[tea_idx].name;
}

#endif // TEA_STEEPING_TIMES_H
