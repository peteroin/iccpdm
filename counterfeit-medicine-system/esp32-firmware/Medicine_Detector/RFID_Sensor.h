// RFID_Sensor.h
// RFID sensor function declarations

#ifndef RFID_SENSOR_H
#define RFID_SENSOR_H

#include <Arduino.h>

// ============================================
// RFID HARDWARE CONFIGURATION
// ============================================
// ESP32 Pins for RC522 RFID module (SPI)
#define RFID_SS_PIN 5
#define RFID_SCK_PIN 18
#define RFID_MOSI_PIN 23
#define RFID_MISO_PIN 19
#define RFID_RST_PIN 4

// ============================================
// FUNCTION DECLARATIONS
// ============================================

/**
 * Initialize RFID reader
 */
void initializeRFID();

/**
 * Check if RFID tag is present
 * Returns: true if tag detected, false otherwise
 */
bool checkRFIDPresent();

/**
 * Read RFID UID from tag
 * Returns: UID as string (e.g., "CD8DD505")
 */
String readRFID_UID();

/**
 * Format UID for display
 * Converts byte array to hex string
 */
String formatUID(byte* buffer, byte bufferSize);

/**
 * Get RFID status information
 * Returns: "OK", "WAITING", "ERROR", etc.
 */
String getRFIDStatus();

#endif  // RFID_SENSOR_H
