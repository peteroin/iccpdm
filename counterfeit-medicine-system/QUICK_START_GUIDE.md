// QUICK_START_GUIDE.md

# 🚀 30-MINUTE QUICK START GUIDE

**Complete Counterfeit Medicine Detection System**

---

## Phase 1: Firebase Setup (5 minutes)

### Step 1: Create Firebase Project
1. Go to https://console.firebase.google.com
2. Click "Add project" → Name it "medicine-detector"
3. Enable Realtime Database (Start in test mode)
4. Wait for database to initialize

### Step 2: Import Test Data
1. Download Firebase JSON importer tool (optional) OR use curl
2. Navigate to your database in Firebase Console
3. Click ⋮ → "Import JSON"
4. Upload `firebase-config/INITIAL_DATA.json`

### Step 3: Get Credentials
1. Go to **Settings** (⚙️) → **Project Settings**
2. Go to **Service Accounts** tab
3. Click **Generate New Private Key** → Save as `serviceAccountKey.json`
4. Copy your **Database URL** (format: `https://project.firebasedatabase.app`)

✅ Firebase READY

---

## Phase 2: Backend API Setup (10 minutes)

### Step 1: Install Dependencies
```bash
cd backend-api
npm install
```

### Step 2: Configure Environment
Create `.env` file:
```
FIREBASE_PROJECT_ID=your-project-id
FIREBASE_PRIVATE_KEY=<from serviceAccountKey.json>
FIREBASE_CLIENT_EMAIL=<from serviceAccountKey.json>
FIREBASE_DATABASE_URL=https://your-project.firebasedatabase.app
PORT=3001
```

### Step 3: Start Server
```bash
npm start
```

**Expected Output**:
```
✓ Server running at http://localhost:3001
✓ Firebase connected
```

✅ Backend READY

**Test with**:
```bash
curl http://localhost:3001/api/health
# Should return: {"status":"ok"}
```

---

## Phase 3: ESP32 Firmware Upload (15 minutes)

### Step 1: Install Arduino IDE
- Download: https://www.arduino.cc/en/software
- Install and launch

### Step 2: Add ESP32 Board
1. **File** → **Preferences**
2. Add to "Additional boards manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. **Tools** → **Board Manager** → Search "esp32" → Install

### Step 3: Install Libraries
**Tools** → **Manage Libraries** → Search and install each:
- `MFRC522` (by GithubCommunity)
- `Adafruit SSD1306` (by Adafruit)
- `Adafruit TCS34725` (by Adafruit)
- `ArduinoJson` (by Bblanchon)

### Step 4: Configure and Upload
1. Copy all files from `esp32-firmware/` folder into one Arduino sketch folder
2. Open `Medicine_Detector.ino` in Arduino IDE
3. Edit lines ~13-17:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* backendUrl = "http://192.168.1.X:3001";  // Your backend IP
   ```
4. **Tools** → Select:
   - Board: "ESP32 Dev Module"
   - Port: COMx (your ESP32 port)
   - Upload Speed: 921600
5. **Sketch** → **Upload**

### Step 5: Monitor Serial Output
1. **Tools** → **Serial Monitor**
2. Set baud rate: **115200**
3. Look for:
   ```
   ========== COUNTERFEIT MEDICINE DETECTOR ==========
   ✓ WiFi Connected
   ✓ System Ready!
   ```

✅ ESP32 READY

---

## Phase 4: Dashboard Setup (5 minutes)

### Step 1: Install Dependencies
```bash
cd judge-dashboard
npm install
```

### Step 2: Configure Environment
Edit `.env.local`:
```
NEXT_PUBLIC_API_URL=http://localhost:3001
```

### Step 3: Start Dashboard
```bash
npm run dev
```

**Expected Output**:
```
ready - started server on http://localhost:3000
```

### Step 4: Open in Browser
- Navigate to: **http://localhost:3000**
- Should display: Dashboard with "No scans yet" message

✅ Dashboard READY

---

## 🧪 TEST THE SYSTEM (5 minutes)

### Test 1: API Health Check
```bash
curl http://localhost:3001/api/health
# Expected: {"status":"ok"}
```

### Test 2: Fetch Test Medicine
```bash
curl http://localhost:3001/api/medicines/CD8DD505
# Expected: Full medicine record JSON
```

### Test 3: Submit Test Scan
```bash
curl -X POST http://localhost:3001/api/scans \
  -H "Content-Type: application/json" \
  -d '{
    "uid": "CD8DD505",
    "rfidStatus": "PASS",
    "uvResult": "PASS",
    "uvValue": 450,
    "rgbResult": "PASS",
    "finalResult": "GENUINE"
  }'
# Expected: {"success":true,"scanId":"..."}
```

### Test 4: View in Dashboard
1. Go to http://localhost:3000
2. Should see scan appear in "Recent Scans"
3. Click to view details
4. Statistics should update

✅ FULL SYSTEM WORKING

---

## 🔍 TROUBLESHOOTING

### "Backend not found" error
- Verify backend running: `npm start` in backend-api folder
- Check port 3001 is accessible: `curl http://localhost:3001/api/health`
- Check firewall rules

### "ESP32 Serial Monitor blank"
- Verify Com port selected (look in Device Manager)
- Verify baud rate is 115200
- Try different USB port or cable

### "OLED display blank but scanner works"
- Check I2C address: 0x3C (verify with I2C scanner)
- Check SDA/SCL wiring (GPIO 21/22)
- Check 4.7kΩ pull-up resistors on I2C lines

### "WiFi connection fails"
- Verify SSID and password are correct (case sensitive)
- Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check ESP32 can see network: list all available networks in code

### "Dashboard shows 'Failed to load'"
- Check `.env.local` has correct API URL
- Verify backend is running
- Check browser console for CORS errors
- Verify no trailing slash in URL

---

## 📱 HARDWARE QUICK REFERENCE

### SPI (RFID) Pins
| MFRC522 | ESP32 |
|---------|-------|
| SDA (CS) | GPIO 5 |
| SCK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 4 |
| GND | GND |
| 3.3V | 3.3V |

### I2C Pins (All Sensors)
| Sensor | I2C Addr | Connection |
|--------|----------|------------|
| OLED | 0x3C | SDA→GPIO21, SCL→GPIO22 |
| RGB | 0x29 | SDA→GPIO21, SCL→GPIO22 |
| ADC (UV) | 0x48 | SDA→GPIO21, SCL→GPIO22 |

### Other GPIO
| Device | Pin |
|--------|-----|
| UV LED | GPIO 12 |
| Buzzer | GPIO 13 |

---

## 📊 TEST DATA (Pre-loaded)

Place RFID tag near scanner with these UIDs:

**CD8DD505** → Paracetamol (Genuine)
- Expected result after scanning: "GENUINE" ✓

**4D2CAA04** → Aspirin (Genuine)
- Expected result after scanning: "GENUINE" ✓

---

## ✅ COMPLETE SYSTEM CHECKLIST

- [ ] Firebase database created and data imported
- [ ] Backend API running on http://localhost:3001
- [ ] ESP32 connected to WiFi (Serial Monitor shows confirmation)
- [ ] ESP32 displays "SCAN MEDICINE TAG" on OLED
- [ ] Dashboard loads on http://localhost:3000
- [ ] Test API endpoints work (curl commands)
- [ ] Test scan appears in dashboard

**All checked? → System is FULLY OPERATIONAL!**

---

## 📚 DETAILED DOCUMENTATION

Need more info? Check these files:

| Document | For |
|----------|-----|
| README.md | System overview & architecture |
| firebase-config/setup-firebase.md | Detailed Firebase setup |
| backend-api/README.md | API documentation |
| esp32-firmware/INSTALLATION_GUIDE.md | Firmware library setup |
| judge-dashboard/README.md | Dashboard features |
| SYSTEM_ARCHITECTURE.md | Complete system design |
| COMPLETION_SUMMARY.md | What was built |

---

## 🎯 NEXT STEPS

1. **Calibrate with Real Medicine**
   - Follow: esp32-firmware/ESP32_FIRMWARE_INTEGRATION_GUIDE.md
   - Take 10 UV readings, 10 RGB readings
   - Calculate mean ± std dev × 2
   - Update database with calibrated values

2. **Add More Medicines**
   - POST to /api/medicines with calibrated values

3. **Run Production**
   - Deploy backend to cloud (Heroku, AWS, etc.)
   - Deploy dashboard to Vercel or similar
   - Configure ESP32 to use cloud backend URL

4. **Advanced Features**
   - Add authentication (JWT tokens)
   - Add data export (CSV reports)
   - Add mobile app (React Native)
   - Add computer vision (image comparison)

---

**Questions? Check the detailed README.md or specific component guides.**

**System complete and ready to use! 🎉**
