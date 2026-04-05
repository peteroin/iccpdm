// Final_Decision.h
// OLED display management and buzzer pattern control

#ifndef FINAL_DECISION_H
#define FINAL_DECISION_H

#include <Arduino.h>

// ============================================
// HARDWARE CONFIGURATION
// ============================================
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 32

#define BUZZER_PIN 13              // GPIO pin for active buzzer

// ============================================
// BUZZER PATTERN DEFINITIONS
// ============================================
enum BuzzerPattern {
  BUZZER_SUCCESS,    // Short beep (200ms)
  BUZZER_WARNING,    // 3x short beeps (warning pattern)
  BUZZER_REJECT,     // Long beep (800ms)
  BUZZER_ERROR,      // 2x long beeps
  BUZZER_SILENT      // No sound
};

// ============================================
// FUNCTION DECLARATIONS
// ============================================

/**
 * Initialize OLED display and buzzer
 */
void initializeDisplay();

/**
 * Display a message on OLED (max 2-3 lines)
 * Parameters:
 *   - title: Main message (top line, bold)
 *   - status: Secondary message (middle line)
 *   - detail: Optional third line
 */
void displayMessage(const char* title, const char* status = "", const char* detail = "");

/**
 * Display startup message
 */
void displayStartup();

/**
 * Display "SCAN MEDICINE TAG" when ready
 */
void displayReady();

/**
 * Display RFID UID that was just scanned
 */
void displayUID(String uid);

/**
 * Display database status check results
 * Parameter: status (one of: "REGISTERED", "EXPIRED", "RECALLED", "UNREGISTERED", "RE_SCANNED")
 */
void displayDatabaseStatus(const char* status);

/**
 * Display UV measurement results
 * Parameter: status ("CHECKING...", "PASS", "FAIL")
 */
void displayUVStatus(const char* status);

/**
 * Display RGB measurement results
 * Parameter: status ("CHECKING...", "PASS", "FAIL")
 */
void displayRGBStatus(const char* status);

/**
 * Display final result with full details
 * Shows all 4 verification steps + final decision
 */
void displayFinalResult(const char* rfidStatus, const char* dbStatus, 
                       const char* uvStatus, const char* rgbStatus, 
                       const char* finalDecision);

/**
 * Clear OLED display
 */
void clearDisplay();

/**
 * Play buzzer pattern
 * Parameter: pattern (BUZZER_SUCCESS, BUZZER_WARNING, BUZZER_REJECT, BUZZER_ERROR)
 */
void playBuzzer(BuzzerPattern pattern);

/**
 * Quick buzzer pulse (used for multiple patterns)
 * Parameters:
 *   - durationMs: Duration of single pulse
 *   - count: How many pulses
 *   - pauseMs: Pause between pulses
 */
void buzzerPulse(uint16_t durationMs, uint8_t count = 1, uint16_t pauseMs = 100);

/**
 * Stop buzzer immediately
 */
void stopBuzzer();

/**
 * Display error message and buzzer alert
 */
void displayError(const char* errorMsg);

/**
 * Show a loading/progress indicator
 */
void displayProcessing(const char* message = "Processing...");

#endif  // FINAL_DECISION_H
