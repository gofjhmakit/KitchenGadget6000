/**
 * USDA-COMPLIANT MEAT TEMPERATURES
 * 
 * Safe minimum internal temperatures (Celsius)
 * Based on USDA Food Safety and Inspection Service (FSIS) guidelines
 * 
 * Format: [Rare/Light Pink | Medium-Rare | Medium | Well-Done]
 */

#ifndef MEAT_TEMPERATURES_H
#define MEAT_TEMPERATURES_H

// Meat types with USDA-safe temperatures
const char* meat_types[] = {
  "Beef",          // 0
  "Veal",          // 1
  "Lamb",          // 2
  "Pork",          // 3
  "Ground Beef",   // 4
  "Poultry",       // 5
  "Ground Poultry",// 6
  "Fish",          // 7
  "Duck",          // 8
  "Ground Lamb"    // 9
};
#define MEAT_TYPE_COUNT 10

// Doneness levels
const char* meat_doneness[] = {
  "Rare/Pink",
  "Medium-Rare",
  "Medium",
  "Well-Done"
};
#define DONENESS_COUNT 4

/**
 * Temperature targets in CELSIUS [Rare, Med-Rare, Medium, Well-Done]
 * 
 * USDA Guidelines:
 * - Beef steaks/roasts: 63°C minimum (rest to 65°C)
 * - Veal: 63°C
 * - Lamb: 63°C
 * - Pork: 63°C (no pink safe)
 * - Poultry: 74°C minimum
 * - Ground meat: 74°C
 * - Fish: 63°C
 * - Duck: 71°C medium, 74°C well
 * 
 * NOTE: Rare/Med-Rare for beef/lamb are temps BEFORE resting.
 * After resting (carryover cooking), add ~2-3°C.
 */
const int meat_temps[MEAT_TYPE_COUNT][DONENESS_COUNT] = {
  // Beef (steaks/roasts)
  {45, 52, 63, 71},
  
  // Veal
  {52, 58, 63, 71},
  
  // Lamb (steaks/roasts)
  {52, 58, 63, 71},
  
  // Pork (no safe "rare" - minimum 63°C)
  {63, 63, 68, 71},
  
  // Ground Beef (minimum 74°C)
  {74, 74, 74, 74},
  
  // Poultry (minimum 74°C)
  {74, 74, 74, 74},
  
  // Ground Poultry (minimum 74°C)
  {74, 74, 74, 74},
  
  // Fish (minimum 63°C)
  {52, 58, 63, 71},
  
  // Duck (medium-rare 65°C, medium 71°C, well 74°C)
  {63, 68, 71, 74},
  
  // Ground Lamb (minimum 74°C)
  {74, 74, 74, 74}
};

/**
 * Helper: Get temperature for meat type and doneness
 * Returns temperature in CELSIUS
 */
inline int get_meat_temp(int meat_idx, int doneness_idx) {
  if (meat_idx < 0 || meat_idx >= MEAT_TYPE_COUNT) return 63;
  if (doneness_idx < 0 || doneness_idx >= DONENESS_COUNT) return 63;
  return meat_temps[meat_idx][doneness_idx];
}

#endif // MEAT_TEMPERATURES_H
