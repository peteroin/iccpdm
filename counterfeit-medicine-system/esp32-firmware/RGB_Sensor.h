// RGB_Sensor.h
// TCS34725 RGB color sensor for medicine color verification

#ifndef RGB_SENSOR_H
#define RGB_SENSOR_H

#include <Arduino.h>

// ============================================
// RGB HARDWARE CONFIGURATION
// ============================================
#define RGB_SENSOR_I2C_ADDRESS 0x29  // TCS34725 I2C address
#define RGB_SENSOR_SDA_PIN 21        // I2C SDA (shared with UV)
#define RGB_SENSOR_SCL_PIN 22        // I2C SCL (shared with UV)

// ============================================
// TCS34725 REGISTER DEFINITIONS
// ============================================
#define TCS34725_ENABLE_REG 0x00
#define TCS34725_TIMING_REG 0x01
#define TCS34725_CONTROL_REG 0x0F
#define TCS34725_DATA_START 0x14

// ============================================
// RGB VALUE STRUCTURE
// ============================================
struct RGBReading {
  uint16_t red;
  uint16_t green;
  uint16_t blue;
  uint16_t clear;
};

// ============================================
// FUNCTION DECLARATIONS
// ============================================

/**
 * Initialize RGB sensor (TCS34725)
 * - Configures I2C communication
 * - Sets integration time and gain
 * - Enables color measurement
 */
void initializeRGBSensor();

/**
 * Read RGB values from TCS34725
 * Returns: RGBReading struct with r, g, b, clear values
 */
RGBReading readRGBValue();

/**
 * Convert raw ADC values to normalized RGB (0-255 scale)
 * Input: raw RGBReading from sensor
 * Returns: RGBReading with normalized 0-255 values
 */
RGBReading normalizeRGBValue(RGBReading raw);

/**
 * Compare measured RGB with reference and tolerance
 * Returns: true if all channels within tolerance, false otherwise
 */
bool compareRGBValue(RGBReading measured, RGBReading reference, 
                     uint16_t rTol, uint16_t gTol, uint16_t bTol);

/**
 * Get current RGB sensor status
 * Returns: "ACTIVE", "IDLE", "ERROR", "NOT_INITIALIZED"
 */
String getRGBStatus();

/**
 * When UV is ON, color sensing can be affected
 * This function should be called AFTER UV is OFF
 */
void waitUVSettling();

#endif  // RGB_SENSOR_H
