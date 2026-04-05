// SYSTEM_ARCHITECTURE.md

# System Architecture - Counterfeit Medicine Detection

## High-Level System Diagram

```
┌──────────────────────────────────────────────────────────────────────┐
│                    COUNTERFEIT MEDICINE DETECTOR                     │
│                         Complete System                              │
└──────────────────────────────────────────────────────────────────────┘

┏━━━━━━━━━━━━━━━━━━━━━━━━┓         ┏━━━━━━━━━━━━━━━━━━━━┓         ┏━━━━━━━━━━━━━━━━━━━┓
┃    HARDWARE LAYER      ┃         ┃   BACKEND LAYER    ┃         ┃   FRONTEND LAYER  ┃
┃   (ESP32 Device)       ┃         ┃  (Node.js/Express) ┃         ┃   (React/Next.js) ┃
┃                        ┃         ┃                    ┃         ┃                   ┃
┃  ┌─ RFID Scanner       ┃         ┃  ┌─ API Server     ┃         ┃  ┌─ Dashboard     ┃
┃  │  (RC522)            ┃◄───────►│  │ (Port 3001)     ┃◄───────►│  │ (Port 3000)    ┃
┃  │  MFRC522 SPI        ┃  HTTP   ┃  │                 ┃  HTTP   ┃  │ Real-time UI   ┃
┃  │                     ┃ (WiFi)  ┃  ├─ MongoDB Conn   ┃         ┃  │                │
┃  ├─ UV Sensor          ┃         ┃  │                 ┃         ┃  ├─ Statistics    ┃
┃  │  (BPW34 + ADS1115)  ┃         ┃  └─ Controllers    ┃         ┃  │ - Total Scans  ┃
┃  │  365nm LED          ┃         ┃                    ┃         ┃  │ - Genuine %    ┃
┃  │  I2C Interface      ┃         ┃  GET  /medicines   ┃         ┃  │ - Suspect %    ┃
┃  │                     ┃         ┃  POST /scans       ┃         ┃  │ - Rejected %   ┃
┃  ├─ RGB Sensor         ┃         ┃  PUT  /count       ┃         ┃  │                │
┃  │  (TCS34725)         ┃         ┃  GET  /stats       ┃         ┃  ├─ Recent Scans ┃
┃  │  I2C Interface      ┃         ┃                    ┃         ┃  │ - UID Display  ┃
┃  │                     ┃         ┃  JSON Processing   ┃         ┃  │ - Status Badges│
┃  ├─ Output (OLED/Buzzer)         ┃  Error Handling    ┃         ┃  │ - Timestamps   ┃
┃  │  (SSD1306 + GPIO13) ┃         ┃                    ┃         ┃  │                │
┃  │  I2C + GPIO         ┃         ┃  Fallback Support  ┃         ┃  └─ Scan Details│
┃  │                     ┃         ┃  (if Redis offline)┃         ┃     - Full Results ┃
┃  └─ Controller         ┃         ┃                    ┃         ┃     - Sensor Values┃
┃     (ESP32 Main)       ┃         ┃                    ┃         ┃     - Color Picker ┃
┃                        ┃         ┃                    ┃         ┃                   ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━┛         ┗━━━━━━━━━━━━━━━━━━━┛         ┗━━━━━━━━━━━━━━━━━━━┛
                    HTTP (WiFi)                    HTTP (REST APIs)
                WiFi: ESP32 ←→ Backend      Browser: Browser ←→ Backend


                         ┏━━━━━━━━━━━━━━━━━━━━━━━━┓
                         ┃   DATABASE LAYER       ┃
                         ┃   (MongoDB Atlas)      ┃
                         ┃                        ┃
                         ┃  ┌─ /medicines         ┃
                         ┃  │  └─ {uid}           ┃
                         ┃  │      ├─ name        ┃
                         ┃  │      ├─ registered  ┃
                         ┃  │      ├─ expiry      ┃
                         ┃  │      ├─ recalled    ┃
                         ┃  │      ├─ uvRef/Tol   ┃
                         ┃  │      └─ rgbRef/Tol  ┃
                         ┃  │                     ┃
                         ┃  ├─ /scanHistory      ┃
                         ┃  │  └─ {scanId}       ┃
                         ┃  │      ├─ uid        ┃
                         ┃  │      ├─ results    ┃
                         ┃  │      └─ timestamp  ┃
                         ┃  │                     ┃
                         ┃  └─ /recalledBatches  ┃
                         ┃     └─ {batchId}      ┃
                         ┃                        ┃
                         ┗━━━━━━━━━━━━━━━━━━━━━━━━┛
```

## Data Flow Architecture

### 1. Scan Initiation Flow

```
User scans RFID tag
        ↓
┌─ ESP32 RFID Detects ─┐
│ readRFID_UID()       │  ~500ms
│ Format as hex string │
└──────────┬───────────┘
           ↓
    Convert UID
    (e.g., CD8DD505)
           ↓
    Store in memory
    (MedicineRecord)
           ↓
    Advance to
    DATABASE_CHECK stage
```

### 2. Database Verification Flow

```
├─ Check WiFi Connected?
│  ├─ No → Use LOCAL_MEDICINES[]
│  └─ Yes → HTTP GET /api/medicines/{uid}
│
├─ Frontend Response:
│  └─ 200: Return medicine record
│     404: Return null (not found)
│
├─ Validate Medicine Status:
│  ├─ registered? → No: REJECTED
│  ├─ expired? → Yes: REJECTED
│  ├─ recalled? → Yes: REJECTED
│  ├─ scanCount > 3? → Yes: WARNING (continue)
│  └─ All pass? → Proceed to UV stage
│
└─ Store values in
   currentMedicine structure
```

### 3. UV Verification Flow

```
┌─ turnOnUV() ─────────┐
│ GPIO12 → HIGH        │  100ms setup
└──────────┬───────────┘
           ↓
    delay(1000ms)
    (LED stabilization)
           ↓
┌─ readUVValue() ──┐
│ I2C read from   │  ~50ms
│ ADS1115 (0x48)  │
│ Read conversion │
│ register (0x00) │
└────────┬─────────┘
         ↓
    Get raw ADC value
    (0-32767)
         ↓
┌──────────────────────────┐
│ compareUVValue()         │
│ ├─ measured vs reference│
│ ├─ Check tolerance      │
│ └─ PASS or FAIL         │
└────────┬─────────────────┘
         ↓
    PASS → Continue to RGB
    FAIL → Set SUSPECT, STOP
         ↓
┌──────────────────┐
│ turnOffUV()      │
│ GPIO12 → LOW     │
└──────────────────┘
```

### 4. RGB Verification Flow

```
┌─ waitUVSettling() ──┐
│ delay(500ms)        │  Allow ambient light
│ Let UV decay        │  to stabilize
└──────────┬──────────┘
           ↓
┌─ readRGBValue() ──────┐
│ I2C read from        │
│ TCS34725 (0x29)      │  ~100ms
│ Read R,G,B,Clear     │
│ channels             │
└────────┬──────────────┘
         ↓
    Get raw RGB array
    (16-bit per channel)
         ↓
┌─ normalizeRGBValue() ─┐
│ Scale to 0-255       │
│ using clear channel  │
│ as reference         │
└────────┬──────────────┘
         ↓
┌──────────────────────────┐
│ compareRGBValue()        │
│ ├─ R: measured vs ref   │
│ ├─ G: measured vs ref   │
│ ├─ B: measured vs ref   │
│ └─ All within tolerance?│
└────────┬─────────────────┘
         ↓
    PASS → GENUINE result
    FAIL → SUSPECT result
         ↓
    Advance to
    FINAL_DECISION stage
```

### 5. Decision & Output Flow

```
┌──────────────────────────────┐
│ Determine Final Result        │
│ ├─ RFID: PASS (always, if got UID)
│ ├─ DB: PASS/UNREGISTERED/EXPIRED/RECALLED
│ ├─ UV: PASS/FAIL
│ ├─ RGB: PASS/FAIL
│ └─ DECISION: GENUINE/SUSPECT/COUNTERFEIT
└────────────┬─────────────────┘
             ↓
     ┌─ updateDisplay() ─────┐
     │ Show 4-line result    │
     │ (RFID→DB→UV→RGB)      │
     │ on OLED 128x32        │
     └─────────┬─────────────┘
               ↓
     ┌─ playBuzzer() ──────────┐
     │ Pattern based on result │
     │ SUCCESS: 1x 200ms beep  │
     │ REJECT: 1x 800ms beep   │
     │ WARNING: 3x 150ms beeps │
     └─────────┬───────────────┘
               ↓
     If WiFi connected:
     ├─ sendScanToServer()
     │  └─ HTTP POST /api/scans
     └─ updateScanCountOnServer()
        └─ HTTP PUT /api/medicines/{uid}/count
             ↓
     Wait 3 seconds
             ↓
     Reset to IDLE
```

### 6. Server-Side Processing

```
ESP32 → HTTP POST to /api/scans
┌───────────────────────────────────┐
│ Backend Receives ScanResult JSON  │
│ {uid, rfidStatus, uvResult,       │
│  rgbResult, finalResult, ...}     │
├───────────────────────────────────┤
│ Validate JSON structure           │
│ ├─ Check required fields          │
│ ├─ Verify UID format              │
│ └─ Check result values            │
├───────────────────────────────────┤
│ Parse with ArduinoJson / JSON lib │
├───────────────────────────────────┤
│ Write to MongoDB                  │
│ POST /scanHistory/{pushId}        │
├───────────────────────────────────┤
│ Response: 201 Created             │
│ Return: {success: true, scanId}   │
└───────────────────────────────────┘
           ↓
    Update /medicines/{uid}/scanCount
    INCREMENT by 1
    UPDATE lastScanDate
           ↓
    Dashboard polls every 5s
           ├─ GET /api/stats/summary
           ├─ GET /api/stats/recent-scans
           └─ Display latest results
```

## Modular Hardware Design

### ESP32 Main Controller (Medicine_Detector.ino)
**Responsibility**: Central orchestration, state machine, WiFi communication

**Key Components**:
- 7-stage state machine (enum: STAGE_*)
- WiFi connector (WiFi library)
- HTTP client (HTTPClient library)
- State variables (currentStage, currentMedicine, currentScanResult)
- Display initializer

**Call Sequence** (in loop):
```
loop()
  ├─ switch(currentStage)
  ├─ IDLE → Check for RFID tag
  ├─ RFID → Read UID
  ├─ DATABASE_CHECK → Fetch from server
  ├─ UV → Turn ON, read, compare
  ├─ RGB → Read normalized value, compare
  └─ FINAL_DECISION → Display + buzz + send to server
```

### RFID_Sensor.h/cpp (RC522 Module)
**Responsibility**: RFID tag detection and UID reading via SPI

**Key Functions**:
- `initializeRFID()` - SPI bus setup
- `checkRFIDPresent()` - Poll for tag
- `readRFID_UID()` - Format UID as hex string
- Hardware config: SPI (GPIO 5,18,23,19,4)

**Library**: MFRC522 (from Arduino Library Manager)

### UV_Sensor.h/cpp (Photodiode Module)
**Responsibility**: UV LED control and photodiode signal reading via I2C ADC

**Key Functions**:
- `initializeUVSensor()` - Configure ADS1115
- `turnOnUV()` / `turnOffUV()` - GPIO 12 control
- `readUVValue()` - I2C read from ADS1115 (0x48)
- `compareUVValue()` - Verify within tolerance

**Hardware Config**:
- LED GPIO: 12
- I2C: SDA=21, SCL=22
- ADS1115 address: 0x48

### RGB_Sensor.h/cpp (Color Sensor Module)
**Responsibility**: RGB color measurement and comparison via I2C

**Key Functions**:
- `initializeRGBSensor()` - Configure TCS34725
- `readRGBValue()` - Get R,G,B,Clear values
- `normalizeRGBValue()` - Scale to 0-255
- `compareRGBValue()` - Per-channel tolerance check

**Hardware Config**:
- I2C: SDA=21, SCL=22
- TCS34725 address: 0x29

### Final_Decision.h/cpp (Output Module)
**Responsibility**: OLED display messages and buzzer patterns

**Key Functions**:
- `initializeDisplay()` - Setup OLED & buzzer
- `displayMessage()` - Show text on 128x32 display
- `playBuzzer()` - Play pattern (SUCCESS/WARNING/REJECT/ERROR)
- `displayFinalResult()` - Show 4-line verification tree

**Hardware Config**:
- OLED I2C: 0x3C
- Buzzer GPIO: 13

### Databases.h (Data Structures)
**Provides**:
- `MedicineRecord` struct (13 fields)
- `ScanResult` struct (8 fields)
- `RGBValue` struct (r, g, b)
- `LOCAL_MEDICINES[]` array (backup data)
- `getLocalMedicineRecord()` function

## Communication Protocols

### 1. SPI (RFID to ESP32)
```
ESP32 ←→ RC522 (SPI Bus)
CS (GPIO 5) → Chip Select
SCK (GPIO 18) → Clock
MOSI (GPIO 23) → Master Out, Slave In
MISO (GPIO 19) → Master In, Slave Out
RST (GPIO 4) → Reset

Frame format: 8-bit commands + data
Transaction: Set CS low → Send byte → Read byte → Set CS high
```

### 2. I2C (Multiple Sensors to ESP32)
```
Pull-ups: 4.7kΩ on both SDA and SCL
SDA (GPIO 21) ← Data line (open-drain)
SCL (GPIO 22) ← Clock line (open-drain)

Device Addresses:
├─ OLED (SSD1306) → 0x3C
├─ RGB Sensor (TCS34725) → 0x29
└─ ADC (ADS1115) → 0x48

Transaction flow:
Start → Address + R/W bit → ACK → Data bytes → Stop
```

### 3. HTTP (ESP32 to Backend)
```
WiFi: 802.11 b/g/n (2.4GHz)

Methods: GET, POST, PUT
Host: Backend IP (configurable)
Port: 3001
Content-Type: application/json

Example GET:
─────────────────
GET /api/medicines/CD8DD505 HTTP/1.1
Host: 192.168.1.100:3001
Connection: close
─────────────────

Example POST:
─────────────────────────────────────────
POST /api/scans HTTP/1.1
Host: 192.168.1.100:3001
Content-Type: application/json
Content-Length: 150

{
  "uid": "CD8DD505",
  "rfidStatus": "PASS",
  "uvResult": "PASS",
  "uvValue": 450,
  "rgbResult": "PASS",
  "finalResult": "GENUINE"
}
─────────────────────────────────────────
```

### 4. MongoDB Atlas Database (Backend to Cloud)
```
Connection: Mongoose ODM + MongoDB Driver (Node.js)
Database: MongoDB Atlas Cloud (murphy.mongodb.net)

Backend API (for ESP32 & Dashboard):
GET  /api/medicines/{uid}
POST /api/scans
GET  /api/stats/summary

MongoDB Collections:
- medicines: Registered medicine records
- scanHistory: Individual scan results
- recalledBatches: Recalled batch tracking

Authentication: MongoDB connection string (MONGODB_URI)
```

## Performance Characteristics

### Latency Profile (Per Scan)
```
STAGE_RFID             500ms  (MFRC522 SPI communication)
STAGE_DATABASE_CHECK   800ms  (WiFi HTTP GET request)
├─ WiFi latency      500ms
├─ Backend processing 200ms
└─ MongoDB query     100ms
STAGE_UV            2000ms  (LED stabilization + ADC read)
├─ Turn ON           100ms
├─ Stabilization    1000ms
├─ I2C read         100ms
├─ Comparison        50ms
└─ Turn OFF           50ms
STAGE_RGB           1500ms  (UV settling + TCS34725 read)
├─ UV settling       500ms
├─ I2C read          100ms
├─ Normalization     50ms
├─ Comparison        50ms
└─ Processing       800ms
STAGE_FINAL_DECISION 500ms  (Display + HTTP POST)
├─ Display           100ms
├─ Buzzer            500ms (concurrent)
├─ HTTP POST         800ms
└─ Count update      200ms
─────────────────────────────
TOTAL                ~5.5-6 seconds per scan
```

### Memory Usage
```
Program Memory:
├─ Main code          ~50KB
├─ RFID driver        ~20KB
├─ WiFi/HTTP stack   ~100KB
├─ I2C driver         ~20KB
├─ MongoDB/JSON       ~30KB
├─ OLED driver        ~15KB
└─ Boot/System        ~50KB
─────────────────────────────
Total                 ~285KB ✓ Fits in 4MB Flash

RAM Usage:
├─ Global variables   ~10KB
├─ Stack (functions)  ~20KB
├─ Buffers (HTTP)     ~4KB
├─ I2C buffer        ~1KB
└─ Free              ~40KB
─────────────────────────────
Total                 ~50-75KB (PSRAM available: 4MB)
```

## Failure Modes & Recovery

### Hardware Failures
| Failure | Detection | Recovery |
|---------|-----------|----------|
| RFID reader dead | No response on poll | Skip stage, retry timeout |
| UV LED burnt out | ADC always 0 | Display "UV: ERROR", stop |
| RGB sensor unresponsive | I2C NACK | Display "RGB: ERROR", stop |
| OLED broken | I2C NACK | Continue scanning (no feedback) |
| Buzzer silent | No current draw | Continue (silent failure) |

### Software Failures
| Failure | Detection | Recovery |
|---------|-----------|----------|
| WiFi timeout | HTTP GET timeout | Fall back to LOCAL_MEDICINES[] |
| JSON parse error | Exception thrown | Use default values |
| Out of heap | Memory allocation fails | Soft reset loop stage |
| Infinite loop | Watchdog timer | Auto-reboot after 10s timeout |

### Network Failures
| Failure | Detection | Recovery |
|---------|-----------|----------|
| MongoDB offline | Cannot connect | Use offline fallback |
| DNS unavailable | Cannot resolve IP | Use pre-configured IP |
| Database rule denied | 403 Unauthorized | Log error, continue |

## System Scalability

### Single Device
- 1 ESP32 + 1 set of sensors
- Processes 1 scan every 5-6 seconds
- ~600 scans per hour (theoretical max)

### Multiple Devices
- Each ESP32 needs unique IP/WiFi connection
- Backend handles multiple concurrent HTTP requests
- MongoDB Atlas supports 10+ concurrent connection pools (configurable)
- Dashboard displays all scans in real-time

### Database Growth
- Each scan record: ~500 bytes JSON
- 600 scans/hour → ~300KB/hour
- 24 hours → ~7.2MB
- Monthly → ~216MB (well within MongoDB Atlas 512MB free tier)

## Future Expansion Points

1. **Multi-detector**: Multiple ESP32s → central backend
2. **Analytics**: Batch analysis, trend detection
3. **Mobile app**: iOS/Android companion scanner
4. **Barcode**: QR code addition for batch tracking
5. **Computer vision**: Tablet image recognition
6. **Machine learning**: Anomaly detection on sensor values
7. **Blockchain**: Immutable scan record ledger

---

**Generated**: 2024-2025 Academic  
**Project**: Counterfeit Medicine Detection System  
**Architecture Version**: 1.0
