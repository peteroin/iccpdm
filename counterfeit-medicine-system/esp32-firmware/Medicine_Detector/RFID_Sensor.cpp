// RFID_Sensor.cpp
// RFID sensor implementation
// Uses RC522 RFID module via SPI

#include "RFID_Sensor.h"
#include <SPI.h>

// Placeholder RFID library includes
// #include <MFRC522.h>

// Global RFID object (would use MFRC522 in production)
// MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

bool rfidInitialized = false;
String lastUID = "";
unsigned long lastReadTime = 0;

// ============================================
// INITIALIZATION
// ============================================

void initializeRFID() {
  Serial.println("≤ Initializing RC522 RFID...");
  
  // Initialize SPI
  SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
  
  // Initialize RFID (would use: rfid.PCD_Init())
  // For now, simulating with dummy init
  
  Serial.println("  - SPI pins configured");
  Serial.println("  - RFID: SS=" + String(RFID_SS_PIN) + " RST=" + String(RFID_RST_PIN));
  Serial.println("✓ RFID initialized");
  
  rfidInitialized = true;
}

// ============================================
// RFID OPERATIONS
// ============================================

bool checkRFIDPresent() {
  if(!rfidInitialized) return false;
  
  // In production, would call: rfid.PICC_IsNewCardPresent()
  // For now, return false (no card detected in simulation)
  return false;
}

String readRFID_UID() {
  if(!rfidInitialized || !checkRFIDPresent()) {
    return "";
  }
  
  // In production, would implement:
  // if (!rfid.PICC_ReadCardSerial()) return "";
  // byte* uid = rfid.uid.uidByte;
  // byte uidLength = rfid.uid.size;
  // return formatUID(uid, uidLength);
  
  // For testing, return mock UID
  return lastUID;
}

String formatUID(byte* buffer, byte bufferSize) {
  String uid = "";
  
  for(byte i = 0; i < bufferSize; i++) {
    if(buffer[i] < 0x10) {
      uid += "0";
    }
    uid += String(buffer[i], HEX);
    uid.toUpperCase();
  }
  
  lastUID = uid;
  lastReadTime = millis();
  
  return uid;
}

String getRFIDStatus() {
  if(!rfidInitialized) {
    return "NOT_INITIALIZED";
  }
  
  if(checkRFIDPresent()) {
    return "TAG_DETECTED";
  }
  
  return "WAITING";
}

// ============================================
// PRODUCTION NOTES
// ============================================
/*
 * To use with actual RC522 module, install:
 * - MFRC522 library by GithubCommunity
 * 
 * Then uncomment the following:
 * 
 * #include <MFRC522.h>
 * MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
 * 
 * And update these functions:
 * 
 * void initializeRFID() {
 *   SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
 *   rfid.PCD_Init();
 * }
 * 
 * bool checkRFIDPresent() {
 *   return rfid.PICC_IsNewCardPresent();
 * }
 * 
 * String readRFID_UID() {
 *   if (!rfid.PICC_IsNewCardPresent()) return "";
 *   if (!rfid.PICC_ReadCardSerial()) return "";
 *   String uid = formatUID(rfid.uid.uidByte, rfid.uid.size);
 *   rfid.PICC_HaltA();
 *   return uid;
 * }
 */
