// Final_Decision.cpp
// OLED display and buzzer implementation

#include "Final_Decision.h"
#include <Wire.h>

// ============================================
// GLOBAL VARIABLES
// ============================================

bool displayInitialized = false;
bool buzzerInitialized = false;

// ============================================
// SIMPLE OLED DISPLAY IMPLEMENTATION
// ============================================
// Note: For production, use Adafruit_SSD1306 library
// This is a frameBuffer-based implementation for demonstration

uint8_t displayBuffer[4];  // 4 lines, simplified
String displayLines[4] = {"", "", "", ""};

// ============================================
// INITIALIZATION
// ============================================

void initializeDisplay() {
  Serial.println("≤ Initializing Display and Buzzer...");
  
  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize I2C for OLED (already done by other sensors)
  // Probe for OLED device
  uint8_t oledFound = 0;
  Wire.beginTransmission(OLED_I2C_ADDRESS);
  if(Wire.endTransmission() == 0) {
    oledFound = 1;
  }
  
  if(!oledFound) {
    Serial.println("⚠ OLED not found at address 0x" + String(OLED_I2C_ADDRESS, HEX));
    Serial.println("  (Continuing with serial output only)");
  } else {
    Serial.println("  - OLED display: 128x32 at 0x" + String(OLED_I2C_ADDRESS, HEX));
    displayInitialized = true;
  }
  
  Serial.println("  - Buzzer: GPIO " + String(BUZZER_PIN));
  Serial.println("✓ Display and Buzzer initialized");
  
  buzzerInitialized = true;
}

// ============================================
// DISPLAY FUNCTIONS
// ============================================

void clearDisplay() {
  for(int i = 0; i < 4; i++) {
    displayLines[i] = "";
  }
  
  // In production, send SSD1306 command to clear
  if(displayInitialized) {
    Serial.println("[OLED CLEAR]");
  }
}

void displayMessage(const char* title, const char* status, const char* detail) {
  clearDisplay();
  
  displayLines[0] = String(title);
  displayLines[1] = String(status);
  if(strlen(detail) > 0) {
    displayLines[2] = String(detail);
  }
  
  // Send to OLED if available, or serial
  if(displayInitialized) {
    Serial.println("[OLED] " + displayLines[0]);
    if(strlen(status) > 0) Serial.println("[OLED] " + displayLines[1]);
    if(strlen(detail) > 0) Serial.println("[OLED] " + displayLines[2]);
  } else {
    Serial.println("[DISPLAY] " + displayLines[0]);
    if(strlen(status) > 0) Serial.println("[DISPLAY] " + displayLines[1]);
    if(strlen(detail) > 0) Serial.println("[DISPLAY] " + displayLines[2]);
  }
}

void displayStartup() {
  displayMessage("INITIALIZING", "Medicine Detector", "Please wait...");
  delay(500);
}

void displayReady() {
  clearDisplay();
  displayMessage("READY", "SCAN MEDICINE TAG", "");
}

void displayUID(String uid) {
  String shortUID = uid.substring(0, 8);  // First 8 chars
  displayMessage("RFID TAG DETECTED", "UID: " + shortUID, "Checking database...");
}

void displayDatabaseStatus(const char* status) {
  if(strcmp(status, "REGISTERED") == 0) {
    displayMessage("DATABASE CHECK", "Status: REGISTERED", "✓");
  }
  else if(strcmp(status, "EXPIRED") == 0) {
    displayMessage("DATABASE CHECK", "Status: EXPIRED", "✗ REJECTED");
    playBuzzer(BUZZER_REJECT);
  }
  else if(strcmp(status, "RECALLED") == 0) {
    displayMessage("DATABASE CHECK", "Status: RECALLED", "✗ REJECTED");
    playBuzzer(BUZZER_REJECT);
  }
  else if(strcmp(status, "RE_SCANNED") == 0) {
    displayMessage("DATABASE CHECK", "Re-scanned (3x)", "⚠ WARNING");
    playBuzzer(BUZZER_WARNING);
  }
  else if(strcmp(status, "UNREGISTERED") == 0) {
    displayMessage("DATABASE CHECK", "NOT IN DATABASE", "✗ REJECTED");
    playBuzzer(BUZZER_REJECT);
  }
}

void displayUVStatus(const char* status) {
  if(strcmp(status, "CHECKING...") == 0) {
    displayProcessing("UV Verification");
  }
  else if(strcmp(status, "PASS") == 0) {
    displayMessage("UV VERIFICATION", "Status: PASS", "✓");
    playBuzzer(BUZZER_SUCCESS);
  }
  else if(strcmp(status, "FAIL") == 0) {
    displayMessage("UV VERIFICATION", "Status: FAIL", "✗");
    playBuzzer(BUZZER_REJECT);
  }
}

void displayRGBStatus(const char* status) {
  if(strcmp(status, "CHECKING...") == 0) {
    displayProcessing("RGB Verification");
  }
  else if(strcmp(status, "PASS") == 0) {
    displayMessage("RGB VERIFICATION", "Status: PASS", "✓");
    playBuzzer(BUZZER_SUCCESS);
  }
  else if(strcmp(status, "FAIL") == 0) {
    displayMessage("RGB VERIFICATION", "Status: FAIL", "✗");
    playBuzzer(BUZZER_REJECT);
  }
}

void displayFinalResult(const char* rfidStatus, const char* dbStatus,
                       const char* uvStatus, const char* rgbStatus,
                       const char* finalDecision) {
  
  // Show step-by-step results
  Serial.println("\n========== FINAL RESULT ==========");
  Serial.println("RFID Status:     " + String(rfidStatus));
  Serial.println("Database Status: " + String(dbStatus));
  Serial.println("UV Status:       " + String(uvStatus));
  Serial.println("RGB Status:      " + String(rgbStatus));
  Serial.println("----------------------------------");
  Serial.println("FINAL DECISION:  " + String(finalDecision));
  Serial.println("==================================\n");
  
  // Display on OLED (primary info)
  clearDisplay();
  displayLines[0] = "RESULT: " + String(finalDecision);
  
  if(strcmp(finalDecision, "GENUINE") == 0) {
    displayLines[1] = "Medicine Verified";
    displayLines[2] = "✓ ✓ ✓";
    playBuzzer(BUZZER_SUCCESS);
  }
  else if(strcmp(finalDecision, "SUSPECT") == 0) {
    displayLines[1] = "Suspicious Activity";
    displayLines[2] = "⚠ ALERT";
    playBuzzer(BUZZER_WARNING);
  }
  else if(strcmp(finalDecision, "COUNTERFEIT") == 0) {
    displayLines[1] = "COUNTERFEIT";
    displayLines[2] = "✗ ✗ ✗";
    playBuzzer(BUZZER_REJECT);
  }
  else if(strcmp(finalDecision, "REJECTED") == 0) {
    displayLines[1] = "Medicine REJECTED";
    displayLines[2] = "✗ Invalid Status";
    playBuzzer(BUZZER_REJECT);
  }
  
  if(displayInitialized) {
    Serial.println("[OLED] " + displayLines[0]);
    Serial.println("[OLED] " + displayLines[1]);
    Serial.println("[OLED] " + displayLines[2]);
  }
  
  // Display for 3 seconds
  delay(3000);
}

void displayProcessing(const char* message) {
  displayMessage("PROCESSING", message, "Please wait...");
}

void displayError(const char* errorMsg) {
  displayMessage("ERROR", errorMsg, "");
  playBuzzer(BUZZER_ERROR);
}

// ============================================
// BUZZER FUNCTIONS
// ============================================

void playBuzzer(BuzzerPattern pattern) {
  if(!buzzerInitialized) return;
  
  switch(pattern) {
    case BUZZER_SUCCESS:
      // Single short beep
      buzzerPulse(200, 1, 0);
      break;
      
    case BUZZER_WARNING:
      // 3 short beeps
      buzzerPulse(150, 3, 150);
      break;
      
    case BUZZER_REJECT:
      // Long beep
      buzzerPulse(800, 1, 0);
      break;
      
    case BUZZER_ERROR:
      // 2 long beeps
      buzzerPulse(500, 2, 200);
      break;
      
    case BUZZER_SILENT:
    default:
      stopBuzzer();
      break;
  }
}

void buzzerPulse(uint16_t durationMs, uint8_t count, uint16_t pauseMs) {
  if(!buzzerInitialized) return;
  
  for(uint8_t i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(durationMs);
    digitalWrite(BUZZER_PIN, LOW);
    
    if(i < count - 1) {
      delay(pauseMs);
    }
  }
}

void stopBuzzer() {
  if(buzzerInitialized) {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ============================================
// PRODUCTION NOTES
// ============================================
/*
 * Hardware connection:
 * - SSD1306 OLED 128x32: I2C on GPIO21 (SDA) and GPIO22 (SCL)
 *   Default address: 0x3C (configurable via pin)
 * - Active Buzzer: GPIO13 (connected to positive through transistor if high current)
 * 
 * OLED Display Stages (from specification section 15):
 * 1. "Initializing..." - setup
 * 2. "SCAN MEDICINE TAG" - ready state
 * 3. "UID: [first 8 chars]" - tag detected
 * 4. "Valid" / "Expired" / "Unregistered" - database status
 * 5. "UV Pass" / "UV Fail" - UV verification
 * 6. "RGB Pass" / "RGB Fail" - RGB verification
 * 7. "GENUINE" / "COUNTERFEIT" / "SUSPECT" - final result
 * 
 * Buzzer Patterns (specification section 15):
 * - Success: Single short beep (200ms)
 * - Warning: 3x short beeps @ 150ms each (re-scanned alert)
 * - Reject: Long continuous beep (800ms)
 * - Error: 2x long beeps (system error)
 * 
 * For production implementation:
 * - Use Adafruit_SSD1306 library for full display control
 * - Include font definitions for clear text rendering
 * - Add icons/graphics for visual feedback (check marks, X marks)
 * - Consider display backlight control for power saving
 */
