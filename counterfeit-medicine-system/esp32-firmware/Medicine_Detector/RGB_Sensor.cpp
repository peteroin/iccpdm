// RGB_Sensor.cpp
// TCS34725 color sensor implementation

#include "RGB_Sensor.h"
#include <Wire.h>

// ============================================
// GLOBAL VARIABLES
// ============================================

bool rgbInitialized = false;
RGBReading lastRGBReading = {0, 0, 0, 0};

// ============================================
// INITIALIZATION
// ============================================

void initializeRGBSensor() {
  Serial.println("≤ Initializing RGB Sensor (TCS34725)...");
  
  // I2C already initialized by UV_Sensor, just probe device
  uint8_t sensorFound = 0;
  
  Wire.beginTransmission(RGB_SENSOR_I2C_ADDRESS);
  if(Wire.endTransmission() == 0) {
    sensorFound = 1;
  }
  
  if(!sensorFound) {
    Serial.println("✗ TCS34725 not found at address 0x" + String(RGB_SENSOR_I2C_ADDRESS, HEX));
    return;
  }
  
  // Enable the sensor
  Wire.beginTransmission(RGB_SENSOR_I2C_ADDRESS);
  Wire.write(TCS34725_ENABLE_REG);
  Wire.write(0x01);  // Enable power
  Wire.endTransmission();
  
  delay(10);
  
  // Set integration time and gain
  // Integration time: 2.4ms (register value 0xFF)
  Wire.beginTransmission(RGB_SENSOR_I2C_ADDRESS);
  Wire.write(TCS34725_TIMING_REG);
  Wire.write(0xFF);
  Wire.endTransmission();
  
  // Set gain to 4x (register 0x02)
  Wire.beginTransmission(RGB_SENSOR_I2C_ADDRESS);
  Wire.write(TCS34725_CONTROL_REG);
  Wire.write(0x02);  // 4x gain
  Wire.endTransmission();
  
  delay(100);
  
  Serial.println("  - TCS34725 I2C address: 0x" + String(RGB_SENSOR_I2C_ADDRESS, HEX));
  Serial.println("  - Integration time: 2.4ms");
  Serial.println("  - Gain: 4x");
  Serial.println("✓ RGB Sensor initialized");
  
  rgbInitialized = true;
}

// ============================================
// RGB READING
// ============================================

RGBReading readRGBValue() {
  if(!rgbInitialized) {
    return {0, 0, 0, 0};
  }
  
  RGBReading reading = {0, 0, 0, 0};
  
  // Request data from color sensor
  Wire.beginTransmission(RGB_SENSOR_I2C_ADDRESS);
  Wire.write(TCS34725_DATA_START);  // Start reading from color data registers
  Wire.endTransmission(false);  // Repeated start
  
  Wire.requestFrom(RGB_SENSOR_I2C_ADDRESS, 8);
  
  // Read 8 bytes: clear(2), red(2), green(2), blue(2)
  if(Wire.available() >= 8) {
    reading.clear = (Wire.read() | (Wire.read() << 8));
    reading.red   = (Wire.read() | (Wire.read() << 8));
    reading.green = (Wire.read() | (Wire.read() << 8));
    reading.blue  = (Wire.read() | (Wire.read() << 8));
  }
  
  lastRGBReading = reading;
  return reading;
}

// ============================================
// VALUE NORMALIZATION & COMPARISON
// ============================================

RGBReading normalizeRGBValue(RGBReading raw) {
  // Normalize to 0-255 scale using clear channel as reference
  RGBReading normalized;
  
  if(raw.clear == 0) {
    return {0, 0, 0, 0};  // Avoid division by zero
  }
  
  // Scale to 0-255 based on clear channel
  float scaleFactor = 255.0 / raw.clear;
  
  normalized.red   = (uint16_t)(raw.red * scaleFactor);
  normalized.green = (uint16_t)(raw.green * scaleFactor);
  normalized.blue  = (uint16_t)(raw.blue * scaleFactor);
  normalized.clear = raw.clear;
  
  // Clamp to 255
  if(normalized.red > 255) normalized.red = 255;
  if(normalized.green > 255) normalized.green = 255;
  if(normalized.blue > 255) normalized.blue = 255;
  
  return normalized;
}

bool compareRGBValue(RGBReading measured, RGBReading reference,
                     uint16_t rTol, uint16_t gTol, uint16_t bTol) {
  
  // Normalize measured values
  RGBReading normMeasured = normalizeRGBValue(measured);
  
  Serial.println("  RGB Comparison:");
  
  // Check Red channel
  int rDiff = abs((int)normMeasured.red - (int)reference.red);
  Serial.println("    R: " + String(normMeasured.red) + " (ref: " + String(reference.red) + 
                 ", tol: ±" + String(rTol) + ", diff: " + String(rDiff) + ")");
  
  // Check Green channel
  int gDiff = abs((int)normMeasured.green - (int)reference.green);
  Serial.println("    G: " + String(normMeasured.green) + " (ref: " + String(reference.green) + 
                 ", tol: ±" + String(gTol) + ", diff: " + String(gDiff) + ")");
  
  // Check Blue channel
  int bDiff = abs((int)normMeasured.blue - (int)reference.blue);
  Serial.println("    B: " + String(normMeasured.blue) + " (ref: " + String(reference.blue) + 
                 ", tol: ±" + String(bTol) + ", diff: " + String(bDiff) + ")");
  
  // All channels must be within tolerance
  bool rPass = (rDiff <= rTol);
  bool gPass = (gDiff <= gTol);
  bool bPass = (bDiff <= bTol);
  
  if(rPass && gPass && bPass) {
    Serial.println("  Result: PASS ✓");
    return true;
  } else {
    Serial.println("  Result: FAIL ✗");
    if(!rPass) Serial.println("    - Red channel out of tolerance");
    if(!gPass) Serial.println("    - Green channel out of tolerance");
    if(!bPass) Serial.println("    - Blue channel out of tolerance");
    return false;
  }
}

// ============================================
// UTILITY FUNCTIONS
// ============================================

void waitUVSettling() {
  // After UV LED is turned OFF, wait for ambient light to stabilize
  // and for sensor to settle
  delay(500);
}

String getRGBStatus() {
  if(!rgbInitialized) {
    return "NOT_INITIALIZED";
  }
  
  return "ACTIVE";
}

// ============================================
// PRODUCTION NOTES
// ============================================
/*
 * Hardware connection:
 * - TCS34725 RGB sensor on I2C bus
 * - Shared I2C with ADS1115 (different addresses: 0x29 vs 0x48)
 * - Red, Green, Blue, Clear optical detectors
 * - Integration time determines measurement duration
 * - Gain adjusts sensitivity (4x recommended for colors)
 * 
 * IMPORTANT - From specification section 7:
 * "Keep the RGB sensor facing the tablet at a fixed distance
 *  and ensure UV is OFF during RGB reading to avoid interference"
 * 
 * Measurement workflow:
 * 1. Turn OFF UV LED (if on)
 * 2. Wait 500ms for settling
 * 3. Read RGB values from sensor
 * 4. Normalize to 0-255 scale using clear channel
 * 5. Compare against reference values
 * 
 * Calibration (from spec section 8):
 * - With genuine tablet: take 10 readings
 * - Calculate mean RGB values → reference
 * - Calculate standard deviation × 2 → tolerance
 * - Store in database
 */
