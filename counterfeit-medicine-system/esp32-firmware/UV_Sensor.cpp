// UV_Sensor.cpp
// UV LED control and BPW34 photodiode reading via ADS1115

#include "UV_Sensor.h"
#include <Wire.h>

// ============================================
// GLOBAL VARIABLES
// ============================================

bool uvInitialized = false;
bool uvLedOn = false;
int lastUVValue = 0;

// ============================================
// ADS1115 I2C REGISTER DEFINITIONS
// ============================================

#define ADS1115_CONFIG_REG 0x01
#define ADS1115_CONVERSION_REG 0x00

// ============================================
// INITIALIZATION
// ============================================

void initializeUVSensor() {
  Serial.println("≤ Initializing UV Sensor System...");
  
  // Initialize UV LED GPIO
  pinMode(UV_LED_PIN, OUTPUT);
  digitalWrite(UV_LED_PIN, LOW);
  
  // Initialize I2C for ADS1115
  Wire.begin(ADS1115_SDA_PIN, ADS1115_SCL_PIN);
  Wire.setClock(100000);  // 100kHz I2C clock
  
  // Configure ADS1115
  configureADS1115();
  
  Serial.println("  - UV LED pin: GPIO " + String(UV_LED_PIN));
  Serial.println("  - ADS1115 I2C address: 0x" + String(ADS1115_ADDRESS, HEX));
  Serial.println("✓ UV Sensor initialized");
  
  uvInitialized = true;
}

// ============================================
// ADS1115 CONFIGURATION
// ============================================

void configureADS1115() {
  // Configuration for ADS1115:
  // - Single-ended input on channel 0
  // - ±4.096V range
  // - 128 samples per second
  // - Continuous conversion mode
  
  uint16_t config = 0x8183;  // Example configuration
  
  Wire.beginTransmission(ADS1115_ADDRESS);
  Wire.write(ADS1115_CONFIG_REG);
  Wire.write((config >> 8) & 0xFF);
  Wire.write(config & 0xFF);
  Wire.endTransmission();
  
  delay(100);
}

// ============================================
// UV LED CONTROL
// ============================================

void turnOnUV() {
  if(!uvInitialized) return;
  
  digitalWrite(UV_LED_PIN, HIGH);
  uvLedOn = true;
  
  Serial.println("UV LED: ON");
  delay(100);  // Wait for LED to stabilize
}

void turnOffUV() {
  if(!uvInitialized) return;
  
  digitalWrite(UV_LED_PIN, LOW);
  uvLedOn = false;
  
  Serial.println("UV LED: OFF");
}

// ============================================
// PHOTODIODE READING
// ============================================

int readUVValue() {
  if(!uvInitialized || !uvLedOn) {
    return 0;
  }
  
  // Read conversion result from ADS1115
  Wire.beginTransmission(ADS1115_ADDRESS);
  Wire.write(ADS1115_CONVERSION_REG);
  Wire.endTransmission(false);  // Repeated start
  
  Wire.requestFrom(ADS1115_ADDRESS, 2);
  
  if(Wire.available() >= 2) {
    uint16_t rawValue = (Wire.read() << 8) | Wire.read();
    lastUVValue = rawValue >> 4;  // 12-bit conversion
    return lastUVValue;
  }
  
  return 0;
}

// ============================================
// VALUE CONVERSION & COMPARISON
// ============================================

float convertADCtoVoltage(int adcValue) {
  // For ±4.096V range with 16-bit resolution
  // Each step = 2 * 4.096V / 32767 = 0.25mV
  float voltage = adcValue * 0.25;
  return voltage;
}

bool compareUVValue(int measuredValue, int referenceValue, int tolerance) {
  int diff = abs(measuredValue - referenceValue);
  
  Serial.println("  Measured: " + String(measuredValue));
  Serial.println("  Reference: " + String(referenceValue));
  Serial.println("  Tolerance: ±" + String(tolerance));
  Serial.println("  Difference: " + String(diff));
  
  if(diff <= tolerance) {
    Serial.println("  Result: PASS ✓");
    return true;
  } else {
    Serial.println("  Result: FAIL ✗");
    return false;
  }
}

// ============================================
// STATUS
// ============================================

String getUVStatus() {
  if(!uvInitialized) {
    return "NOT_INITIALIZED";
  }
  
  if(uvLedOn) {
    return "ON";
  }
  
  return "OFF";
}

// ============================================
// PRODUCTION NOTES
// ============================================
/*
 * Hardware connection:
 * - UV LED (365nm) controlled via GPIO12 (through transistor/relay)
 * - BPW34 photodiode output → MCP6002 op-amp (gain ~10-100x)
 * - Op-amp output → ADS1115 input (channel 0)
 * - ADS1115: I2C on GPIO21 (SDA) and GPIO22 (SCL)
 * 
 * Calibration:
 * - With known genuine tablet, record UV readings 10 times
 * - Calculate mean as uvRef and std dev * 2 as uvTol
 * - Store these values in database
 */
