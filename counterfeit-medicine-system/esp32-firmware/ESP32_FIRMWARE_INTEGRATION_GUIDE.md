// ESP32_FIRMWARE_INTEGRATION_GUIDE.md

# ESP32 Counterfeit Medicine Detection - Firmware Integration Guide

## Overview
Complete modular ESP32 firmware for counterfeit medicine detection using RFID, UV optical verification, RGB color verification, and online database validation.

## Complete File Structure

```
esp32-firmware/
├── Medicine_Detector.ino          (Main orchestrator - 8 stages)
├── Databases.h                    (Data structures + local backup)
├── RFID_Sensor.h/cpp              (RFID UID reading via SPI)
├── UV_Sensor.h/cpp                (UV LED control + photodiode reading)
├── RGB_Sensor.h/cpp               (RGB color sensor TCS34725)
├── Final_Decision.h/cpp           (OLED display + Buzzer control)
├── ESP32_FIRMWARE_INTEGRATION_GUIDE.md  (This file)
└── INSTALLATION_GUIDE.md          (Library setup instructions)
```

## Hardware Pin Configuration

### I2C Bus (GPIO 21/22)
- **SDA (Data)**: GPIO 21
- **SCL (Clock)**: GPIO 22
- Devices:
  - OLED Display (0x3C)
  - TCS34725 RGB Sensor (0x29)
  - ADS1115 ADC for UV (0x48)

### SPI Bus (VSPI)
- **SS/CS**: GPIO 5
- **SCK**: GPIO 18
- **MOSI**: GPIO 23
- **MISO**: GPIO 19
- **RST**: GPIO 4
- Device: RC522 RFID Reader

### GPIO Pins
- **GPIO 12**: UV LED (365nm)
- **GPIO 13**: Buzzer (Active)

## Execution Flow (8-Stage State Machine)

```
┌─────────────────────────────────────────────────────────┐
│ STAGE_INIT → STAGE_IDLE → STAGE_RFID                   │
│                              ↓                          │
│                        STAGE_DATABASE_CHECK             │
│                              ↓                          │
│        ┌────────────────────┬────────────────────┐     │
│        ↓ (If DB fails)      ↓ (If DB passes)    │     │
│    REJECTED          STAGE_UV                   │     │
│                              ↓                          │
│        ┌────────────────────┬────────────────────┐     │
│        ↓ (If UV fails)      ↓ (If UV passes)    │     │
│    SUSPECT           STAGE_RGB                  │     │
│                              ↓                          │
│        ┌────────────────────┬─────────────────────┐    │
│        ↓ (If RGB fails)     ↓ (If RGB passes)     │    │
│    SUSPECT           STAGE_FINAL_DECISION       │    │
│                              ↓                          │
│                      DISPLAY RESULT & BUZZER   │    │
│                              ↓                          │
│                    SEND TO SERVER & BACK TO IDLE      │
└─────────────────────────────────────────────────────────┘
```

## Stage Details

### STAGE_IDLE
- **Function**: `idle_Stage()`
- **Action**: Polls RFID sensor for tag detection
- **Display**: "READY - SCAN MEDICINE TAG"
- **Next**: If tag detected → STAGE_RFID

### STAGE_RFID
- **Function**: `rfid_Stage()`
- **Action**: Reads RFID UID from tag
- **Display**: "TAG DETECTED - UID: {first 8 chars}"
- **Next**: → STAGE_DATABASE_CHECK

### STAGE_DATABASE_CHECK
- **Function**: `databaseCheck_Stage()`
- **Checks**:
  1. Is UID registered? (UNREGISTERED → REJECTED)
  2. Is medicine expired? (EXPIRED → REJECTED)
  3. Is batch recalled? (RECALLED → REJECTED)
  4. Has been scanned before? (RE_SCANNED → WARNING but continues)
- **Display**: Shows database status
- **Buzzer**: "REJECT" for failures, "WARNING" for re-scanned
- **Next**: If all pass → STAGE_UV, else → IDLE

### STAGE_UV
- **Function**: `uv_Stage()`
- **Action**:
  1. Turn ON UV LED
  2. Wait 1 second for stabilization
  3. Read photodiode value from ADS1115
  4. Compare against reference ± tolerance
- **Display**: "UV Check..." → "UV: PASS" or "UV: FAIL"
- **Buzzer**: SUCCESS if pass, REJECT if fail
- **Next**: If pass → STAGE_RGB, else → SUSPECT

### STAGE_RGB
- **Function**: `rgb_Stage()`
- **Action**:
  1. Ensure UV is OFF (for accurate color measurement)
  2. Read RGB values from TCS34725
  3. Normalize to 0-255 scale
  4. Compare all channels against reference ± tolerance
- **Display**: "RGB Check..." → "RGB: PASS" or "RGB: FAIL"
- **Buzzer**: SUCCESS if pass, REJECT if fail
- **Next**: → STAGE_FINAL_DECISION

### STAGE_FINAL_DECISION
- **Function**: `finalDecision_Stage()`
- **Action**:
  1. Determine final result: GENUINE / SUSPECT / COUNTERFEIT
  2. Display full decision tree on OLED
  3. Play appropriate buzzer pattern
  4. Send scan record to backend server
  5. Update scan count in database
- **Display**: Full verification results (4-line breakdown)
- **Buzzer**: SUCCESS/WARNING/REJECT based on result
- **Next**: → STAGE_IDLE

## Module Integration Details

### Medicine_Detector.ino (Main Controller)
**Responsibilities**:
- Central state machine orchestration
- WiFi connectivity management
- HTTP communication with backend
- Fallback to local database when WiFi unavailable
- Overall decision logic

**Key Functions**:
- `setup()`: Initialize all sensors and WiFi
- `loop()`: Dispatch to current stage handler
- `idle_Stage()` through `finalDecision_Stage()`: 6 stage handlers
- `connectToWiFi()`: WiFi connection with retry
- `getMedicineRecord()`: HTTP GET from backend
- `sendScanToServer()`: HTTP POST scan result
- `updateScanCountOnServer()`: HTTP PUT to increment count
- `resetForNextScan()`: Reset state for next scan

### Databases.h (Data Structures)
**Provides**:
- `MedicineRecord` struct (13 fields matching database schema)
- `ScanResult` struct (8 fields for one complete scan)
- `RGBValue` struct (r, g, b components)
- `Tolerance` struct (r, g, b tolerance ranges)
- `LOCAL_MEDICINES[]` array with 2 pre-loaded test records
- `getLocalMedicineRecord()` function for offline fallback

**Local Backup Medicines**:
1. **CD8DD505** - Paracetamol (BATCH-2024-001)
   - UV Reference: 450 ± 50
   - RGB Reference: (245, 245, 245) ± (20, 20, 20)
   
2. **4D2CAA04** - Aspirin (BATCH-2024-002)
   - UV Reference: 420 ± 45
   - RGB Reference: (255, 255, 255) ± (25, 25, 25)

### RFID_Sensor.h/cpp (RFID Module)
**Responsibilities**:
- Initialize RC522 via SPI
- Detect RFID tag presence
- Read UID and format as hex string
- Report RFID status

**Key Functions**:
- `initializeRFID()`: Configure SPI and RC522
- `checkRFIDPresent()`: Poll for tag (requires MFRC522 library)
- `readRFID_UID()`: Get UID as formatted string (e.g., "CD8DD505")
- `getRAFIDStatus()`: Return current status string

**Hardware Config**:
- SPI: SS=GPIO5, SCK=GPIO18, MOSI=GPIO23, MISO=GPIO19, RST=GPIO4
- Requires: MFRC522 library

### UV_Sensor.h/cpp (UV Module)
**Responsibilities**:
- Control UV LED (365nm)
- Read photodiode signal via ADS1115 ADC
- Compare UV values with tolerance

**Key Functions**:
- `initializeUVSensor()`: Configure GPIO and I2C
- `turnOnUV()`: Enable UV LED
- `turnOffUV()`: Disable UV LED
- `readUVValue()`: Get ADC reading from photodiode
- `convertADCtoVoltage()`: Convert raw ADC to millivolts
- `compareUVValue()`: Check if within tolerance

**Hardware Config**:
- UV LED: GPIO 12
- I2C: SDA=GPIO21, SCL=GPIO22
- ADS1115 address: 0x48
- Requires: Wire library (built-in)

### RGB_Sensor.h/cpp (RGB Module)
**Responsibilities**:
- Initialize TCS34725 RGB sensor
- Read RGB and clear channel values
- Normalize to 0-255 scale
- Compare with reference and tolerance

**Key Functions**:
- `initializeRGBSensor()`: Configure I2C and sensor
- `readRGBValue()`: Get raw RGB + clear readings
- `normalizeRGBValue()`: Scale to 0-255 using clear channel As reference
- `compareRGBValue()`: Check all channels within tolerance
- `waitUVSettling()`: Delay after UV OFF for ambient light stabilization

**Hardware Config**:
- I2C: SDA=GPIO21, SCL=GPIO22, Address=0x29
- Requires: Wire library (built-in)
- Note: Reads ambient light unless UV reflects from tablet

### Final_Decision.h/cpp (Output Module)
**Responsibilities**:
- Display messages on SSD1306 OLED
- Control buzzer with various patterns
- Show step-by-step results

**Key Functions**:
- `initializeDisplay()`: Setup OLED and Buzzer
- `displayMessage()`: Show 1-3 lines of text
- `displayReady()`: "SCAN MEDICINE TAG"
- `displayUID()`: Show detected UID
- `displayDatabaseStatus()`: Show DB check result
- `displayUVStatus()`: Show UV result
- `displayRGBStatus()`: Show RGB result
- `displayFinalResult()`: Full results breakdown
- `playBuzzer()`: Play pattern (SUCCESS/WARNING/REJECT/ERROR)
- `buzzerPulse()`: Low-level buzzer control

**Hardware Config**:
- OLED: I2C address 0x3C, 128x32 pixels
- Buzzer: GPIO 13 (active high)
- I2C: SDA=GPIO21, SCL=GPIO22

**Buzzer Patterns**:
- **BUZZER_SUCCESS**: Single 200ms beep
- **BUZZER_WARNING**: 3x 150ms beeps with 150ms pause
- **BUZZER_REJECT**: Single 800ms beep
- **BUZZER_ERROR**: 2x 500ms beeps with 200ms pause

## Required Arduino Libraries

Add these via Arduino IDE → Sketch → Include Library → Manage Libraries:

1. **MFRC522** by GithubCommunity
   - For RC522 RFID reader control
   - SPI protocol interface

2. **Adafruit-SSD1306** by Adafruit
   - For 128x32 OLED display
   - I2C communication

3. **Adafruit_TCS34725** by Adafruit
   - For RGB color sensor
   - I2C communication

4. **ArduinoJson** by Bblanchon
   - For JSON parsing (HTTP API responses)
   - Light library version (using static allocation)

## WiFi Configuration

Edit these constants in `Medicine_Detector.ino`:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* backendUrl = "http://YOUR_BACKEND_IP:3001";
const char* backendUrl = "http://192.168.1.100:3001";
```

## Backend API Integration

### Required Endpoints

1. **GET /api/medicines/:uid**
   - Fetch medicine record
   - Response: MedicineRecord JSON (13 fields)

2. **POST /api/scans**
   - Submit scan result
   - Payload: ScanResult JSON (8 fields)

3. **PUT /api/medicines/:uid/count**
   - Increment scan counter
   - Response: Updated count

### Expected Responses

**Success (200/201)**:
```json
{
  "uid": "CD8DD505",
  "medicineName": "Paracetamol",
  "batchId": "BATCH-2024-001",
  "registered": true,
  "expiryDate": "2026-12-31",
  "recalled": false,
  "uvRef": 450,
  "uvTol": 50,
  "rgbRef": {"r": 245, "g": 245, "b": 245},
  "rgbTol": {"r": 20, "g": 20, "b": 20},
  "scanCount": 3,
  "lastScanDate": "2024-12-20",
  "createdAt": "2024-01-01"
}
```

**Error (404)**:
```json
{"error": "Medicine not found"}
```

## Calibration Procedure

### UV Calibration (Per Tablet Model)
1. Obtain known genuine tablet
2. Place in scanner enclosure
3. Take 10 UV readings
4. Calculate average (uvRef) and std dev × 2 (uvTol)
5. Update database: `medicines/{uid}/uvRef` and `medicines/{uid}/uvTol`

### RGB Calibration (Per Tablet Model)
1. Ensure UV is OFF
2. Place known genuine tablet at fixed distance
3. Take 10 RGB readings with UV OFF
4. Normalize each to 0-255 scale (using clear channel)
5. Calculate average (rgbRef) and per-channel tolerance
6. Update database: `medicines/{uid}/rgbRef` and `medicines/{uid}/rgbTol`

## Testing Modes

### Local Mode (WiFi Unavailable)
- Falls back to LOCAL_MEDICINES[] array
- Works offline with pre-programmed test UIDs
- Test UIDs: CD8DD505, 4D2CAA04

### Debug Mode
- Serial output at 115200 baud
- Shows all decisions and values
- Can be captured with Serial Monitor

## Production Notes

1. **Binary Sizes**: Each module compiles to firmware ~500KB
2. **EEPROM Usage**: Optional for storing calibration data locally
3. **Power**: ~300mA average during scanning, ~50mA idle
4. **Temperature**: Keep enclosure 15-30°C for accurate UV/RGB
5. **Humidity**: Avoid condensation on optical sensors
6. **Signal Integrity**: Shield UV LED wires from digital noise

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| RFID not detected | MFRC522 not initialized | Check SPI wiring, verify CS pin |
| OLED blank | I2C communication failure | Check SDA/SCL pull-ups, verify address 0x3C |
| UV always "PASS" | Photodiode saturated/dark | Adjust amplifier gain in MCP6002, check LED brightness |
| RGB inconsistent | Light interference | Ensure UV OFF before RGB reading, check enclosure sealing |
| WiFi timeout | Network unreachable | Verify SSID/password, check backend server status |
| HTTP 404 on GET | UID not in database | Add medicine to online database, verify exact UID format |

## Next Steps

1. Install all required libraries (see above)
2. Configure WiFi credentials in Medicine_Detector.ino
3. Point backendUrl to running Node.js/MongoDB backend
4. Upload to ESP32 via Arduino IDE
5. Open Serial Monitor (115200 baud) to see debug output
6. Perform calibration with known genuine tablets
7. Test with provided test UIDs (CD8DD505, 4D2CAA04)
