// README.md (Root Level)

# Counterfeit Medicine Detection System - Complete Project

🔬 **Multi-Sensor Verification System** combining RFID identification, online database validation, UV optical analysis, and RGB color verification for detecting counterfeit pharmaceuticals.

## Project Overview

This is a complete end-to-end system designed for academic research and competition:
- **Hardware**: ESP32-based detector with RFID, UV sensors, and RGB color analysis
- **Backend**: Node.js/Express API with MongoDB Atlas database (cloud-hosted)
- **Frontend**: React dashboard for judges to view results in real-time
- **Workflow**: RFID → Database Check → UV Verification → RGB Verification → Final Decision

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    COMPLETE DETECTION SYSTEM                │
└─────────────────────────────────────────────────────────────┘

┌──────────────────┐      ┌──────────────────┐      ┌──────────────────┐
│   ESP32 Device   │─────→│  Backend API     │─────→│  Judge Dashboard │
│  (Detector)      │      │  (Node.js)       │      │  (React/Next.js) │
└──────────────────┘      └──────────────────┘      └──────────────────┘
       ▲                           │
       │                           ▼
   RFID Tag                  ┌──────────────────┐
   UV Sensor                 │  MongoDB Atlas   │
   RGB Sensor                │  (Cloud, Free)   │
   OLED Display              └──────────────────┘
   Buzzer Alert
```

### Four Verification Layers

1. **Identity Layer (RFID)**
   - Read medicine UID from RFID tag
   - Format and validate UID

2. **Database Layer (WiFi)**
   - Fetch medicine record from online MongoDB database
   - Check: registered, non-expired, non-recalled
   - Flag re-scanned medicines

3. **Optical Layer 1 (UV)**
   - Enable 365nm UV LED
   - Read photodiode signal via ADS1115
   - Compare measured vs reference value

4. **Optical Layer 2 (RGB)**
   - Disable UV to measure ambient color
   - Read color from TCS34725 RGB sensor
   - Compare measured vs reference for all channels

### Decision Logic

```
RFID Detected?
    ├─ No → Timeout, retry
    └─ Yes
        ├─ Register? No → REJECTED
        ├─ Expired? Yes → REJECTED
        ├─ Recalled? Yes → REJECTED
        ├─ Re-scanned >3x? Yes → WARNING (continue)
        └─ All Pass?
            └─ UV Match?
                ├─ No → SUSPECT
                └─ Yes
                    └─ RGB Match?
                        ├─ No → SUSPECT
                        └─ Yes → GENUINE
```

## Quick Start

### Prerequisites
- Node.js 18+
- Arduino IDE 2.0+ (for ESP32 firmware)
- ESP32 microcontroller board
- MongoDB Atlas account (free tier, no credit card required)

### 1️⃣ MongoDB Atlas Setup (5 minutes)

```bash
cd mongodb-config
# Follow MONGODB_SETUP.md instructions
# Creates cloud database for medicines + scan history
# Get your connection string from MongoDB Atlas
```

### 2️⃣ Backend API Setup (10 minutes)

```bash
cd backend-api
npm install
# Edit .env with MONGODB_URI from MongoDB Atlas
npm start
# Server runs at http://localhost:3001
```

### 3️⃣ ESP32 Firmware Upload (15 minutes)

```bash
cd esp32-firmware
# Open Medicine_Detector.ino in Arduino IDE
# Install required libraries (see INSTALLATION_GUIDE.md)
# Configure WiFi + Backend URL in code
# Upload to ESP32
```

### 4️⃣ Judge Dashboard (10 minutes)

```bash
cd judge-dashboard
npm install
# Edit .env.local: NEXT_PUBLIC_API_URL=http://localhost:3001
npm run dev
# Open http://localhost:3000
```

## Directory Structure

```
counterfeit-medicine-system/
│
├── mongodb-config/
│   ├── DATABASE_SCHEMA.md          # Complete database structure
│   ├── INITIAL_DATA.json           # Test medicines data
│   ├── MONGODB_SETUP.md            # MongoDB Atlas setup guide
│   └── README.md
│
├── backend-api/
│   ├── server.js                   # Express API server
│   ├── mongodb-service.js          # MongoDB integration
│   ├── package.json                # Node dependencies
│   ├── README.md                   # API documentation
│   └── .env.example
│
├── esp32-firmware/
│   ├── Medicine_Detector.ino       # Main controller (470+ lines)
│   ├── Databases.h                 # Data structures + local backup
│   ├── RFID_Sensor.h/cpp           # RFID module (SPI)
│   ├── UV_Sensor.h/cpp             # UV verification (I2C+GPIO)
│   ├── RGB_Sensor.h/cpp            # RGB verification (I2C)
│   ├── Final_Decision.h/cpp        # OLED + Buzzer output
│   ├── INSTALLATION_GUIDE.md       # Library setup
│   ├── ESP32_FIRMWARE_INTEGRATION_GUIDE.md
│   └── README.md
│
├── judge-dashboard/
│   ├── pages/                      # Next.js pages
│   │   ├── _app.tsx                # App wrapper
│   │   └── index.tsx               # Dashboard home
│   ├── components/                 # React components
│   │   ├── StatsOverview.tsx
│   │   ├── RecentScans.tsx
│   │   └── ScanDetails.tsx
│   ├── styles/                     # Tailwind CSS
│   ├── package.json
│   ├── next.config.js
│   ├── tailwind.config.js
│   ├── tsconfig.json
│   ├── README.md
│   └── .env.local
│
├── README.md                       # This file
└── SYSTEM_ARCHITECTURE.md         # Detailed system design
```

## Component Details

### 🔌 ESP32 Hardware Module
**Location**: `esp32-firmware/`

**Components**:
- ESP32 Dev Module (WiFi, Bluetooth-capable)
- RC522 RFID reader + RFID tags
- 365nm UV LED + BPW34 photodiode
- MCP6002 op-amp (signal conditioning)
- ADS1115 ADC (analog→digital conversion)
- TCS34725 RGB color sensor
- SSD1306 0.96" OLED display
- Active buzzer

**Communication**:
- SPI: RFID reader
- I2C: OLED, RGB sensor, ADC
- WiFi: Backend API communication

**Features**:
- 7-stage state machine
- Fallback to local database if WiFi unavailable
- Real-time OLED feedback at each stage
- Buzzer alerts (success/warning/reject patterns)
- HTTP integration with backend

### 🌐 Backend API Service
**Location**: `backend-api/`

**Stack**: Node.js + Express + MongoDB + Mongoose

**Endpoints**:
- `GET /api/medicines/:uid` - Fetch medicine record
- `POST /api/medicines` - Register new medicine
- `PUT /api/medicines/:uid/calibrate` - Update calibration values
- `POST /api/scans` - Submit scan result
- `GET /api/scans/:uid` - Get scan history for medicine
- `GET /api/stats/summary` - Statistics overview
- `GET /api/stats/recent-scans` - Latest scans
- `GET /api/health` - Health check

**Features**:
- Middleware: CORS, body-parser, error handling
- MongoDB Atlas cloud database integration
- HTTP status codes (201 create, 404 not found, 500 error)
- No authentication (for tournament use; add in production)

### 🗄️ MongoDB Database
**Location**: `mongodb-config/`

**Collections**:
- `medicines` - Registered medicine records
- `scanHistory` - Individual scan results
- `recalledBatches` - Recalled batch tracking

**Schema Example**:
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
  "rgbRef": { "r": 245, "g": 245, "b": 245 },
  "rgbTol": { "r": 20, "g": 20, "b": 20 },
  "scanCount": 0,
  "lastScanDate": "2024-01-01"
}
```

### 📊 Judge Dashboard
**Location**: `judge-dashboard/`

**Stack**: Next.js + React + TypeScript + Tailwind CSS + Recharts

**Pages**:
- Home: Real-time scan dashboard

**Components**:
- `StatsOverview`: Statistics cards + distribution chart
- `RecentScans`: Scrollable list of latest 20 scans
- `ScanDetails`: Full verification breakdown

**Features**:
- Auto-refresh every 5 seconds
- Responsive design (desktop/mobile)
- Color-coded results (green=genuine, yellow=suspect, red=rejected)
- Click scan to view full details
- Displays raw sensor values (UV, RGB)

## Hardware Pin Configuration

### SPI Bus (VSPI - RFID)
| Signal | GPIO | Purpose |
|--------|------|---------|
| SS/CS | 5 | Chip select |
| SCK | 18 | Clock |
| MOSI | 23 | Data out (host) |
| MISO | 19 | Data in (host) |
| RST | 4 | Reset |

### I2C Bus (Shared)
| Device | Address | GPIO |
|--------|---------|------|
| OLED | 0x3C | SDA=21, SCL=22 |
| TCS34725 RGB | 0x29 | SDA=21, SCL=22 |
| ADS1115 ADC | 0x48 | SDA=21, SCL=22 |

### GPIO Pins
| Function | GPIO |
|----------|------|
| UV LED | 12 |
| Buzzer | 13 |

## Test Data

Two pre-configured test medicines (for offline testing):

```
UID 1: CD8DD505 (Paracetamol)
├─ Batch: BATCH-2024-001
├─ UV Ref: 450 ± 50
└─ RGB Ref: (245,245,245) ± (20,20,20)

UID 2: 4D2CAA04 (Aspirin)
├─ Batch: BATCH-2024-002
├─ UV Ref: 420 ± 45
└─ RGB Ref: (255,255,255) ± (25,25,25)
```

## Testing Workflow

1. **Hardware Self-Test** (5 min)
   ```bash
   cd esp32-firmware
   # Open Serial Monitor at 115200 baud
   # Verify all sensors initialize
   # Place RFID tag: should display UID
   ```

2. **Offline Test** (5 min)
   - Device works without WiFi using local medicines
   - Test with test UIDs (CD8DD505, 4D2CAA04)
   - Observe OLED feedback and buzzer patterns

3. **Online Test** (10 min)
   ```bash
   # Start backend: npm start (in backend-api)
   # Start dashboard: npm run dev (in judge-dashboard)
   # Scan medicine: should appear in dashboard immediately
   ```

4. **Calibration** (20 min per medicine)
   - Take 10 UV readings with known genuine tablet
   - Take 10 RGB readings with known genuine tablet
   - Calculate mean ± std dev × 2
   - Update database with calibrated values

## System Requirements Summary

### Hardware
- ESP32 Dev Board (~$15)
- RFID RC522 + Tags (~$10)
- UV LED 365nm + Photodiode (~$20)
- RGB Sensor TCS34725 (~$8)
- OLED Display SSD1306 (~$5)
- Op-amp, ADC, Buzzer (~$10)
- **Total BOM**: ~$70-100

### Software
- Arduino IDE 2.0+ (free)
- Node.js 18+ (free)
- MongoDB Atlas account (free tier, 512MB storage)
- Visual Studio Code (free)

### Connectivity
- WiFi network (2.4GHz, WPA2)
- USB cable for programming
- Computer to run backend/dashboard

## Troubleshooting Guide

### ESP32 Connection Issues
1. Check USB cable (data cable, not power-only)
2. Install CH340 USB driver
3. Select correct COM port in Arduino IDE
4. Try different USB port on computer

### WiFi Connection Failed
1. Verify SSID and password in code (no quotes needed in constants)
2. Ensure 2.4GHz WiFi (5GHz not supported on ESP32)
3. Check router allows ESP32 connection
4. Try static IP instead of DHCP

### RFID Not Detected
1. Verify RC522 powered (3.3V, not 5V)
2. Check SPI wiring matches pin configuration
3. Install MFRC522 library
4. Restart Arduino IDE

### OLED Blank
1. Verify I2C address (default 0x3C, check with scanner)
2. Check SDA/SCL pull-up resistors (~4.7k)
3. Look for voltage regulator on OLED module
4. Test with i2cScanner sketch

### Backend API Not Responding
1. Verify Node.js running: `npm start` output
2. Check firewall allows port 3001
3. Verify MongoDB connection credentials in `.env`
4. Test with curl: `curl http://localhost:3001/api/health`

### Dashboard Shows "Failed to load statistics"
1. Check backend URL in `.env.local`
2. Verify backend is running
3. Check CORS errors in browser console
4. Verify API endpoints implemented

## Security Notes

⚠️ **Production Considerations**:
- Add user authentication (not implemented for tournament)
- Use HTTPS/TLS for all communications
- Hash/encrypt sensitive data in MongoDB
- Implement API rate limiting
- Use environment variables for credentials
- Add input validation for all endpoints

## Performance Metrics

- **RFID Read**: ~500ms
- **Database Check**: ~800ms (WiFi dependent)
- **UV Measurement**: ~2s (LED stabilization + reading)
- **RGB Measurement**: ~1s
- **Total Scan**: ~5-6 seconds
- **Dashboard Update**: ~5s (polling interval)

## Future Enhancements

- Web camera integration for tablet photo verification
- Machine learning for anomaly detection
- Mobile app for scanning (in-store use)
- Database analytics and reporting
- Multi-device synchronization
- Blockchain for immutable record keeping
- Integration with government drug databases

## References

- **RFID/NFC**: MFRC522 datasheet, ISO14443-A standard
- **UV Photodiode**: BPW34 spectral response
- **RGB Sensor**: TCS34725 color sensing
- **Micro-controller**: ESP32 technical reference manual
- **Optical Design**: Reflectance measurement techniques

## Team & Credits

**Project**: Counterfeit Medicine Detection System
**Type**: Academic research / competition prototype
**Year**: 2024-2025

## License

For academic and competition use. Not for commercial distribution.

---

**Quick Links**:
- 🚀 [ESP32 Setup](esp32-firmware/INSTALLATION_GUIDE.md)
- 🌐 [Backend Setup](backend-api/README.md)
- 📊 [Dashboard Setup](judge-dashboard/README.md)
- 🗄️ [MongoDB Setup](mongodb-config/MONGODB_SETUP.md)
- 📐 [System Architecture](SYSTEM_ARCHITECTURE.md)

**Need Help?**
1. Check troubleshooting guides in each component folder
2. Review error messages in serial output / browser console
3. Verify all connections match pin configuration
4. Ensure all libraries are installed (see INSTALLATION_GUIDE.md)
