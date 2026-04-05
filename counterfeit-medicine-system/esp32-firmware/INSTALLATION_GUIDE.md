// INSTALLATION_GUIDE.md

# ESP32 Firmware - Installation & Setup Guide

## Prerequisites

- **Arduino IDE** 2.0+ installed
- **USB Cable** - Micro-USB or USB-C (depending on ESP32 board)
- **ESP32 Board Package** installed in Arduino IDE
- **Windows/Mac/Linux** with Arduino IDE

## Step 1: Install Arduino IDE and ESP32 Board

### 1a. Install Arduino IDE 2.0+
Download from: https://www.arduino.cc/en/software

### 1b. Add ESP32 Board Support
1. Open Arduino IDE
2. Go to **File** → **Preferences**
3. In "Additional boards manager URLs", paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click OK
5. Go to **Tools** → **Board Manager**
6. Search "esp32"
7. Install **esp32** by Espressif Systems (latest version)

### 1c. Select ESP32 Board
1. Go to **Tools** → **Board** → **esp32**
2. Select **ESP32 Dev Module**
3. Configure settings:
   - **Upload Speed**: 921600
   - **CPU Frequency**: 80MHz
   - **Flash Frequency**: 80MHz
   - **Flash Mode**: DIO
   - **Flash Size**: 4MB
   - **Partition Scheme**: Default 4MB with spiffs

## Step 2: Install Required Libraries

### Via Arduino Library Manager (Easiest)

1. Go to **Tools** → **Manage Libraries**
2. Search for and install EACH of these:

#### 2a. MFRC522 (RFID)
- Search: "MFRC522"
- Install: **MFRC522** by GithubCommunity
- Version: 1.4.11 or later

#### 2b. Adafruit SSD1306 (OLED Display)
- Search: "SSD1306"
- Install: **Adafruit SSD1306** by Adafruit
- Version: 2.5.9 or later

#### 2c. Adafruit TCS34725 (RGB Sensor)
- Search: "TCS34725"
- Install: **Adafruit TCS34725** by Adafruit
- Version: 1.4.10 or later

#### 2d. ArduinoJson (JSON Parsing)
- Search: "ArduinoJson"
- Install: **ArduinoJson** by Bblanchon
- Version: 7.0.0 or later

#### 2e. Wire Library (I2C - Usually Built-in)
- Should already be included: **Wire** by Arduino

## Step 3: Connect ESP32 to Computer

1. Connect ESP32 via USB cable to your computer
2. Go to **Tools** → **Port**
3. Select the COM port where ESP32 is connected
   - Windows: COM3, COM4, etc.
   - Mac: /dev/cu.usbserial-*
   - Linux: /dev/ttyUSB0, /dev/ttyUSB1, etc.

## Step 4: Download & Configure Firmware

### 4a. Clone or Download Files
Download all files from the `esp32-firmware/` directory:
- `Medicine_Detector.ino` (main file)
- `Databases.h`
- `RFID_Sensor.h`, `RFID_Sensor.cpp`
- `UV_Sensor.h`, `UV_Sensor.cpp`
- `RGB_Sensor.h`, `RGB_Sensor.cpp`
- `Final_Decision.h`, `Final_Decision.cpp`

### 4b. Configure WiFi & Backend

Edit `Medicine_Detector.ino` and update these lines (~13-17):

```cpp
const char* ssid = "YOUR_WIFI_SSID";           // ← Your WiFi network name
const char* password = "YOUR_WIFI_PASSWORD";   // ← Your WiFi password
const char* backendUrl = "http://192.168.1.100:3001";  // ← Backend API IP:Port
const char* backendUrl = "http://192.168.1.100:3001";  // ← Backend API URL
```

**Example**:
```cpp
const char* ssid = "HomeNetwork";
const char* password = "MyPassword123";
const char* backendUrl = "http://192.168.0.50:3001";
const char* backendUrl = "http://192.168.1.106:3001";  // Your backend server IP
```

### 4c. Verify File Structure

Arduino IDE should recognize the file structure:
```
┌─ Medicine_Detector.ino (main, opens in editor)
├─ Databases.h (auto-detected)
├─ RFID_Sensor.h
├─ RFID_Sensor.cpp
├─ UV_Sensor.h
├─ UV_Sensor.cpp
├─ RGB_Sensor.h
├─ RGB_Sensor.cpp
├─ Final_Decision.h
└─ Final_Decision.cpp
```

All files must be in the SAME folder as `Medicine_Detector.ino`.

## Step 5: Compile & Upload

### 5a. Compile
1. Click **Sketch** → **Verify/Compile**
2. Wait for compilation to complete
3. You should see: "Compiling sketch..." and then completion message

### 5b. Upload
1. Click **Sketch** → **Upload**
2. Wait for upload to complete (~30 seconds)
3. You should see "Uploading..." progress, then "Installation complete"

### 5c. Monitor Output
1. Go to **Tools** → **Serial Monitor**
2. Set baud rate to **115200**
3. You should see startup messages:
   ```
   ========== COUNTERFEIT MEDICINE DETECTOR ==========
   System Initializing...
   ≤ Initializing Display and Buzzer...
   ≤ Initializing RFID Sensor System...
   ≤ Initializing UV Sensor System...
   ≤ Initializing RGB Sensor (TCS34725)...
   Connecting to WiFi...
   .....
   ✓ WiFi Connected: 192.168.1.X
   ✓ System Ready!
   ```

## Step 6: Test Hardware

### 6a. Check OLED Display
- Should display "READY" and "SCAN MEDICINE TAG"
- If blank, check I2C wiring and address (0x3C)

### 6b. Check Buzzer
- Manually place RFID tag on scanner
- Should see UID detected in Serial Monitor
- Listen for buzzer feedback

### 6c. Check Sensors
- Monitor serial output for sensor readings:
  ```
  UV Value: 450
  RGB Value: R=245 G=245 B=245
  ```

## Troubleshooting Installation

### Issue: "Board not recognized"
**Solution**: Reinstall CH340 USB driver
- Download from: https://www.wch.cn/downloads/CH341SER_ZIP.html
- Extract and run installer
- Restart Arduino IDE

### Issue: "Compile error: expected ';' before 'RGBReading'"
**Solution**: Verify all .h and .cpp files are in the same folder
- Check file locations in Arduino sketchbook

### Issue: "Error: espressif boards are not installed"
**Solution**: Reinstall ESP32 board package
1. **Tools** → **Board Manager**
2. Uninstall **esp32** package
3. Search "esp32" again and reinstall

### Issue: "Upload failed: no hardware serial port selected"
**Solution**: Select correct COM port
1. **Tools** → **Port**
2. Try each COM port until upload succeeds
3. Or check Device Manager on Windows to find correct port

### Issue: Library not found errors at compile
**Solution**: Reinstall specific library
1. **Tools** → **Manage Libraries**
2. Search for the missing library
3. Click **Uninstall**, then **Install** again

## Network Configuration

### Finding Backend IP Address

**For Node.js Backend Running Locally**:
1. Open Command Prompt / Terminal where backend is running
2. Look for line: `✓ Server running at http://192.168.X.X:3001`
3. Use that IP in `backendUrl`

**If Backend is on Different Computer**:
1. Run `ipconfig` (Windows) or `ifconfig` (Mac/Linux)
2. Find IPv4 address of backend computer
3. Example: `http://192.168.1.50:3001`

### Backend API Server Configuration

The ESP32 connects to your Node.js backend API server running on port 3001.

**To find your backend server IP**:
- Windows: Open Command Prompt, type `ipconfig`, look for IPv4 Address
- Linux/Mac: Open Terminal, type `hostname -I` or `ifconfig`
- Format for Arduino: `const char* backendUrl = "http://192.168.x.x:3001";`

**Verify backend is running**:
```bash
cd backend-api
npm start
# Should output: ✓ MongoDB connected to medicine_db
```

## Hardware Testing Checklist

- [ ] ESP32 powers on (LED indicator)
- [ ] USB connection to computer works
- [ ] Arduino IDE detects board and COM port
- [ ] Sketch compiles without errors
- [ ] Sketch uploads successfully
- [ ] Serial Monitor shows startup messages
- [ ] WiFi connects (see "✓ WiFi Connected" in terminal)
- [ ] OLED display shows text
- [ ] Placing RFID tag triggers detection
- [ ] Buzzer beeps when medicine result determined

## Next Steps

1. **Calibrate Sensors**: See ESP32_FIRMWARE_INTEGRATION_GUIDE.md
2. **Test with Sample UIDs**: Use CD8DD505 and 4D2CAA04
3. **Check Backend**: Verify Node.js server is running
4. **Monitor Scans**: Open Judge Dashboard to see results

## Support Resources

- **MFRC522 Library**: https://github.com/miguelbalboa/rfid
- **Adafruit Libraries**: https://github.com/adafruit
- **ArduinoJson**: https://arduinojson.org/
- **ESP32 Official**: https://docs.espressif.com/projects/esp-idf/

## Safety Warnings

⚠️ **IMPORTANT**:
- Do NOT expose UV LED directly to eyes (365nm is harmful)
- UV LED should shine only on tablet, not on user
- Check all connections before powering on
- Use appropriate PSU for current requirements (~500mA)
- Do not modify circuit while powered
