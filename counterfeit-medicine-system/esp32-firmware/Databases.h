// Databases.h
// Data structures and medicine record definitions

#ifndef DATABASES_H
#define DATABASES_H

#include <Arduino.h>

// ============================================
// DATA STRUCTURES
// ============================================

/**
 * RGB Color Value
 */
struct RGBValue {
  int r;
  int g;
  int b;
};

/**
 * Tolerance for RGB/UV comparisons
 */
struct Tolerance {
  int r;
  int g;
  int b;
};

/**
 * Medicine Record - Matches database schema
 */
struct MedicineRecord {
  String uid;                    // RFID UID
  String medicineName;           // Name of medicine
  String batchId;                // Batch ID
  bool registered;               // Is registered in system
  String expiryDate;             // Expiry date (YYYY-MM-DD)
  bool recalled;                 // Is recalled flag
  int uvRef;                     // UV reference value
  int uvTol;                     // UV tolerance
  RGBValue rgbRef;               // RGB reference values
  Tolerance rgbTol;              // RGB tolerance
  int scanCount;                 // Number of times scanned
  String lastScanDate;           // Last scan timestamp
  String createdAt;              // Record creation date
};

/**
 * Scan Result - Result of one complete scan cycle
 */
struct ScanResult {
  String uid;                    // Medicine UID
  String rfidStatus;             // RFID result
  String uvResult;               // UV result (PASS/FAIL)
  int uvValue;                   // Measured UV value
  String rgbResult;              // RGB result (PASS/FAIL)
  RGBValue rgbValue;             // Measured RGB values
  String finalResult;            // Final decision (GENUINE/SUSPECT/REJECTED/etc)
  String timestamp;              // Scan timestamp
};

// ============================================
// LOCAL BACKUP DATABASE
// ============================================

// Backup data for offline mode
static const MedicineRecord LOCAL_MEDICINES[] = {
  {
    "CD8DD505",
    "Paracetamol 500mg",
    "BATCH-2024-001",
    true,
    "2026-12-31",
    false,
    450,
    50,
    {245, 245, 245},
    {20, 20, 20},
    0,
    "",
    "2026-01-15"
  },
  {
    "4D2CAA04",
    "Aspirin 100mg",
    "BATCH-2024-002",
    true,
    "2026-08-15",
    false,
    420,
    45,
    {255, 255, 255},
    {25, 25, 25},
    0,
    "",
    "2026-01-20"
  }
};

// ============================================
// HELPER FUNCTION DECLARATIONS
// ============================================

/**
 * Get medicine record from local backup
 */
bool getLocalMedicineRecord(String uid) {
  for(int i = 0; i < sizeof(LOCAL_MEDICINES)/sizeof(LOCAL_MEDICINES[0]); i++) {
    if(LOCAL_MEDICINES[i].uid == uid) {
      // Copy to global currentMedicine (in Medicine_Detector.ino)
      return true;
    }
  }
  return false;
}

#endif  // DATABASES_H
