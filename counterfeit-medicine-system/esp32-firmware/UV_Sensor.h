// UV_Sensor.h
// UV LED and BPW34 photodiode with signal conditioning

#ifndef UV_SENSOR_H
#define UV_SENSOR_H

#include <Arduino.h>

// ============================================
// UV HARDWARE CONFIGURATION
// ============================================
#define UV_LED_PIN 12               // GPIO pin controlling UV LED (365nm)
#define ADS1115_SDA_PIN 21          // I2C SDA
#define ADS1115_SCL_PIN 22          // I2C SCL
#define ADS1115_ADDRESS 0x48        // I2C address of ADS1115

// ============================================
// FUNCTION DECLARATIONS
// ============================================

/**
 * Initialize UV sensor system
 * - Sets up GPIO for UV LED
 * - Initializes I2C communication for ADS1115
 * - Configures ADC for photodiode reading
 */
void initializeUVSensor();

/**
 * Turn UV LED ON
 */
void turnOnUV();

/**
 * Turn UV LED OFF
 */
void turnOffUV();

/**
 * Read raw UV value from photodiode via ADS1115 ADC
 * Returns: ADC value (0-32767 for 16-bit conversion)
 */
int readUVValue();

/**
 * Convert raw ADC value to voltage (mV)
 * Returns: millivolts
 */
float convertADCtoVoltage(int adcValue);

/**
 * Compare measured UV with reference value and tolerance
 * Returns: true if within tolerance, false otherwise
 */
bool compareUVValue(int measuredValue, int referenceValue, int tolerance);

/**
 * Get current UV LED status
 * Returns: "ON", "OFF", "ERROR"
 */
String getUVStatus();

#endif  // UV_SENSOR_H
