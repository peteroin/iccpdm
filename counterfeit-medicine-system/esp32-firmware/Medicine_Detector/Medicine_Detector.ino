// Medicine_Detector.ino
// Main ESP32 Controller - Orchestrates RFID, UV, RGB, database checks, and final decision flow

#include "Databases.h"
#include "RFID_Sensor.h"
#include "UV_Sensor.h"
#include "RGB_Sensor.h"
#include "Final_Decision.h"
#include <WiFi.h>
#include <HTTPClient.h>

// ============================================
// WIFI CONFIGURATION
// ============================================
const char* ssid = "HARRY45";
const char* password = "98765432";
const char* backendUrl = "http://localhost:3001";  // Backend API URL (change to your backend IP)

// ============================================
// GLOBAL STATE
// ============================================
enum ScanStage {
  STAGE_INIT,
  STAGE_RFID,
  STAGE_DATABASE_CHECK,
  STAGE_UV,
  STAGE_RGB,
  STAGE_FINAL_DECISION,
  STAGE_IDLE
};

ScanStage currentStage = STAGE_INIT;
MedicineRecord currentMedicine;
ScanResult currentScanResult;
bool wifiConnected = false;

// ============================================
// SETUP - Runs once at bootup
// ============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========== COUNTERFEIT MEDICINE DETECTOR ==========");
  Serial.println("System Initializing...\n");
  
  // Initialize all sensors and peripherals
  initializeDisplay();  // OLED + Buzzer
  displayMessage("INITIALIZING", "Medicine Detector", "Please wait...");
  delay(500);
  
  initializeRFID();
  initializeUVSensor();
  initializeRGBSensor();
  
  // Initialize WiFi
  Serial.println("Connecting to WiFi...");
  displayMessage("WiFi", "Connecting", "");
  connectToWiFi();
  
  // System ready
  delay(1000);
  displayReady();
  currentStage = STAGE_IDLE;
  
  Serial.println("\n✓ System Ready!\n");
}

// ============================================
// MAIN LOOP - Continuous execution
// ============================================
void loop() {
  switch(currentStage) {
    case STAGE_IDLE:
      idle_Stage();
      break;
    
    case STAGE_RFID:
      rfid_Stage();
      break;
    
    case STAGE_DATABASE_CHECK:
      databaseCheck_Stage();
      break;
    
    case STAGE_UV:
      uv_Stage();
      break;
    
    case STAGE_RGB:
      rgb_Stage();
      break;
    
    case STAGE_FINAL_DECISION:
      finalDecision_Stage();
      break;
    
    default:
      currentStage = STAGE_IDLE;
  }
  
  delay(100);  // Small delay to prevent overwhelming the loop
}

// ============================================
// STAGE: IDLE
// Wait for RFID tag to appear
// ============================================
void idle_Stage() {
  if(checkRFIDPresent()) {
    Serial.println("\n>>> STAGE_RFID: Detected RFID tag");
    displayMessage("TAG DETECTED", "Reading UID", "");
    currentStage = STAGE_RFID;
  }
}

// ============================================
// STAGE: RFID
// Read UID and format it
// ============================================
void rfid_Stage() {
  String uid = readRFID_UID();
  
  if(uid.length() > 0) {
    Serial.println("RFID UID: " + uid);
    displayUID(uid);
    
    currentMedicine.uid = uid;
    currentScanResult.uid = uid;
    currentScanResult.rfidStatus = "PASS";
    
    delay(1000);
    Serial.println(">>> STAGE_DATABASE_CHECK: Fetching medicine record");
    displayProcessing("Checking Database...\n");
    currentStage = STAGE_DATABASE_CHECK;
  } else {
    Serial.println("RFID read failed, returning to IDLE");
    displayReady();
    currentStage = STAGE_IDLE;
  }
}

// ============================================
// STAGE: DATABASE CHECK
// Fetch medicine record and validate status
// ============================================
void databaseCheck_Stage() {
  // Fetch medicine record from online database
  if(!getMedicineRecord(currentMedicine.uid)) {
    Serial.println("ERROR: Medicine not found in database");
    displayDatabaseStatus("UNREGISTERED");
    currentScanResult.finalResult = "UNREGISTERED";
    playBuzzer(BUZZER_REJECT);
    delay(2000);
    resetForNextScan();
    return;
  }
  
  Serial.println("Medicine: " + currentMedicine.medicineName);
  Serial.println("Batch: " + currentMedicine.batchId);
  
  // Check if registered
  if(!currentMedicine.registered) {
    Serial.println("STATUS: UNREGISTERED");
    displayDatabaseStatus("UNREGISTERED");
    currentScanResult.finalResult = "UNREGISTERED";
    playBuzzer(BUZZER_REJECT);
    delay(2000);
    resetForNextScan();
    return;
  }
  
  // Check if expired
  if(isExpired(currentMedicine.expiryDate)) {
    Serial.println("STATUS: EXPIRED");
    displayDatabaseStatus("EXPIRED");
    currentScanResult.finalResult = "EXPIRED";
    playBuzzer(BUZZER_REJECT);
    delay(2000);
    resetForNextScan();
    return;
  }
  
  // Check if recalled
  if(currentMedicine.recalled) {
    Serial.println("STATUS: RECALLED");
    displayDatabaseStatus("RECALLED");
    currentScanResult.finalResult = "RECALLED";
    playBuzzer(BUZZER_REJECT);
    delay(2000);
    resetForNextScan();
    return;
  }
  
  // Check if re-scanned
  if(currentMedicine.scanCount > 0) {
    Serial.println("WARNING: RE-SCANNED (count: " + String(currentMedicine.scanCount) + ")");
    displayDatabaseStatus("RE_SCANNED");
    playBuzzer(BUZZER_WARNING);
    delay(1500);
    // Continue to next stage despite re-scan warning
  }
  
  Serial.println("✓ Medicine valid, proceeding to UV check");
  delay(500);
  
  Serial.println(">>> STAGE_UV: Starting UV measurement");
  displayProcessing("Starting UV Check...");
  currentStage = STAGE_UV;
}

// ============================================
// STAGE: UV
// Enable UV, measure, compare with reference
// ============================================
void uv_Stage() {
  turnOnUV();
  delay(1000);  // Wait for UV to stabilize
  
  int uvValue = readUVValue();
  currentScanResult.uvValue = uvValue;
  
  Serial.println("UV Value: " + String(uvValue));
  Serial.println("UV Reference: " + String(currentMedicine.uvRef));
  Serial.println("UV Tolerance: ±" + String(currentMedicine.uvTol));
  
  bool uvPass = compareUVValue(uvValue, currentMedicine.uvRef, currentMedicine.uvTol);
  
  if(uvPass) {
    Serial.println("✓ UV PASS");
    displayUVStatus("PASS");
    currentScanResult.uvResult = "PASS";
    delay(500);
  } else {
    Serial.println("✗ UV FAIL - Suspect");
    displayUVStatus("FAIL");
    currentScanResult.uvResult = "FAIL";
    currentScanResult.finalResult = "SUSPECT";
    playBuzzer(BUZZER_REJECT);
    delay(2000);
    turnOffUV();
    resetForNextScan();
    return;
  }
  
  turnOffUV();
  delay(500);
  waitUVSettling();  // Let ambient light settle
  
  Serial.println(">>> STAGE_RGB: Starting RGB measurement");
  displayProcessing("Starting RGB Check...");
  currentStage = STAGE_RGB;
}

// ============================================
// STAGE: RGB
// Enable RGB, measure, compare with reference
// ============================================
void rgb_Stage() {
  // Ensure UV is OFF during RGB reading
  turnOffUV();
  delay(500);
  
  RGBReading rgbReading = readRGBValue();
  RGBValue rgbValue;
  rgbValue.r = rgbReading.red;
  rgbValue.g = rgbReading.green;
  rgbValue.b = rgbReading.blue;
  currentScanResult.rgbValue = rgbValue;
  
  Serial.print("RGB Value: R=");
  Serial.print(rgbValue.r);
  Serial.print(" G=");
  Serial.print(rgbValue.g);
  Serial.print(" B=");
  Serial.println(rgbValue.b);
  
  Serial.print("RGB Reference: R=");
  Serial.print(currentMedicine.rgbRef.r);
  Serial.print(" G=");
  Serial.print(currentMedicine.rgbRef.g);
  Serial.print(" B=");
  Serial.println(currentMedicine.rgbRef.b);
  
  // Convert RGBValue reference to RGBReading for comparison
  RGBReading refReading;
  refReading.red = currentMedicine.rgbRef.r;
  refReading.green = currentMedicine.rgbRef.g;
  refReading.blue = currentMedicine.rgbRef.b;
  refReading.clear = 0;
  
  bool rgbPass = compareRGBValue(rgbReading, refReading, currentMedicine.rgbTol.r, currentMedicine.rgbTol.g, currentMedicine.rgbTol.b);
  
  if(rgbPass) {
    Serial.println("✓ RGB PASS");
    displayRGBStatus("PASS");
    currentScanResult.rgbResult = "PASS";
    currentScanResult.finalResult = "GENUINE";
    playBuzzer(BUZZER_SUCCESS);
  } else {
    Serial.println("✗ RGB FAIL - Suspect");
    displayRGBStatus("FAIL");
    currentScanResult.rgbResult = "FAIL";
    currentScanResult.finalResult = "SUSPECT";
    playBuzzer(BUZZER_REJECT);
  }
  
  delay(500);
  Serial.println(">>> STAGE_FINAL_DECISION");
  displayProcessing("Computing Final Result...");
  currentStage = STAGE_FINAL_DECISION;
}

// ============================================
// STAGE: FINAL DECISION
// Display results, update database, send to server
// ============================================
void finalDecision_Stage() {
  // Display final result
  displayFinalResult(currentScanResult.rfidStatus.c_str(), 
                     (currentScanResult.finalResult == "GENUINE" ? "REGISTERED" : currentScanResult.finalResult).c_str(),
                     currentScanResult.uvResult.c_str(),
                     currentScanResult.rgbResult.c_str(),
                     currentScanResult.finalResult.c_str());
  
  // Send scan result to backend server
  if(wifiConnected) {
    sendScanToServer(currentScanResult);
  } else {
    Serial.println("WARNING: WiFi not connected, scan not sent to server");
  }
  
  // Update medicine scan count in database
  if(wifiConnected) {
    updateScanCountOnServer(currentMedicine.uid);
  }
  
  delay(3000);
  resetForNextScan();
}

// ============================================
// HELPER FUNCTIONS
// ============================================

/**
 * Connect to WiFi
 */
void connectToWiFi() {
  WiFi.begin(ssid, password);
  int attempts = 0;
  
  while(WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\n✓ WiFi Connected: " + WiFi.localIP().toString());
    displayMessage("WiFi", "Connected", "Ready for Scanning");
  } else {
    wifiConnected = false;
    Serial.println("\n✗ WiFi Failed");
    displayMessage("WiFi", "Failed", "Using Local Mode");
  }
  
  delay(1000);
}

/**
 * Get medicine record from online database via backend
 */
bool getMedicineRecord(String uid) {
  if(!wifiConnected) {
    Serial.println("WARNING: WiFi not connected, using backup data");
    return getLocalMedicineRecord(uid);  // Fallback to local
  }
  
  HTTPClient http;
  String url = String(backendUrl) + "/api/medicines/" + uid;
  
  Serial.println("Fetching: " + url);
  http.begin(url);
  
  int httpCode = http.GET();
  
  if(httpCode == 200) {
    String payload = http.getString();
    Serial.println("Response: " + payload);
    
    // Parse JSON and populate currentMedicine
    // (Use ArduinoJson library for this in production)
    parseJSONMedicineRecord(payload);
    
    http.end();
    return true;
  } else {
    Serial.println("HTTP Error: " + String(httpCode));
    http.end();
    return false;
  }
}

/**
 * Send scan result to backend server
 */
void sendScanToServer(ScanResult result) {
  if(!wifiConnected) return;
  
  HTTPClient http;
  String url = String(backendUrl) + "/api/scans";
  
  String jsonData = buildScanJSON(result);
  
  Serial.println("Sending scan to: " + url);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(jsonData);
  
  if(httpCode == 201) {
    Serial.println("✓ Scan sent to server successfully");
  } else {
    Serial.println("✗ Failed to send scan: " + String(httpCode));
  }
  
  http.end();
}

/**
 * Update scan count on server
 */
void updateScanCountOnServer(String uid) {
  if(!wifiConnected) return;
  
  HTTPClient http;
  String url = String(backendUrl) + "/api/medicines/" + uid + "/count";
  
  Serial.println("Updating scan count for: " + uid);
  http.begin(url);
  
  int httpCode = http.PUT("{}");
  
  if(httpCode == 200) {
    Serial.println("✓ Scan count updated");
  } else {
    Serial.println("✗ Failed to update scan count");
  }
  
  http.end();
}

/**
 * Reset system for next scan
 */
void resetForNextScan() {
  turnOffUV();
  currentStage = STAGE_IDLE;
  delay(2000);
  displayReady();
  Serial.println("\n>>> Ready for next scan\n");
}

/**
 * Check if date is expired
 */
bool isExpired(String expiryDateStr) {
  // Simple date comparison (format: YYYY-MM-DD)
  // In production, implement proper date parsing
  if(expiryDateStr.length() == 0) return false;
  return false;  // Placeholder
}

/**
 * Parse JSON response from server
 * Note: Use ArduinoJson library in production
 */
void parseJSONMedicineRecord(String json) {
  // Placeholder - implement JSON parsing with ArduinoJson library
  // For now, set dummy values
  currentMedicine.medicineName = "Medicine";
  currentMedicine.batchId = "BATCH-001";
  currentMedicine.registered = true;
  currentMedicine.expiryDate = "2026-12-31";
  currentMedicine.recalled = false;
  currentMedicine.uvRef = 450;
  currentMedicine.uvTol = 50;
  currentMedicine.rgbRef = {245, 245, 245};
  currentMedicine.rgbTol = {20, 20, 20};
  currentMedicine.scanCount = 0;
}

/**
 * Build JSON string for scan result
 */
String buildScanJSON(ScanResult result) {
  String json = "{";
  json += "\"uid\":\"" + result.uid + "\",";
  json += "\"rfidStatus\":\"" + result.rfidStatus + "\",";
  json += "\"uvResult\":\"" + result.uvResult + "\",";
  json += "\"uvValue\":" + String(result.uvValue) + ",";
  json += "\"rgbResult\":\"" + result.rgbResult + "\",";
  json += "\"finalResult\":\"" + result.finalResult + "\"";
  json += "}";
  return json;
}
