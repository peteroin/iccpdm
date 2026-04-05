// README.md

# Judge Dashboard - Counterfeit Medicine Detection System

Real-time web interface for viewing and analyzing medicine verification scan results.

## Overview

The Judge Dashboard is a Next.js/React web application that displays:
- **Real-time scan results** from ESP32 detectors
- **Statistics and analytics** on detection accuracy
- **Detailed scan information** including RFID, UV, and RGB verification results
- **Scan history** for tracking trends and validation

## Features

### 📊 Dashboard Home Page
- **Statistics Overview**: Total scans, genuine count, suspect count, rejected count
- **Distribution Chart**: Visual representation of scan results
- **Recent Scans List**: Latest 20 scans with quick status view
- **Detailed Results Panel**: Full breakdown of selected scan

### 🔍 Scan Details
For each scan, displays:
- **Medicine Information**: UID, Name, Batch ID
- **RFID Status**: Tag detected/valid
- **UV Verification**: Pass/Fail + raw value
- **RGB Verification**: Pass/Fail + color value
- **Final Result**: GENUINE / SUSPECT / REJECTED
- **Timestamp**: Exact scan date/time

### 📈 Statistics
- Total scans processed
- Percentage breakdown: Genuine / Suspect / Rejected
- Bar chart visualization
- Real-time updates (auto-refresh every 5 seconds)

## Tech Stack

- **Framework**: Next.js 14
- **UI Library**: React 18
- **Styling**: Tailwind CSS
- **Charts**: Recharts
- **HTTP Client**: Axios
- **Language**: TypeScript
- **Date Handling**: date-fns

## Installation & Setup

### Requirements
- Node.js 18+ 
- npm or yarn
- Backend API running (see backend-api/README.md)

### 1. Install Dependencies

```bash
cd judge-dashboard
npm install
```

### 2. Configure Environment

Edit `.env.local` and set the backend API URL:

```env
NEXT_PUBLIC_API_URL=http://localhost:3001
```

**For remote backend:**
```env
NEXT_PUBLIC_API_URL=http://192.168.1.100:3001
```

### 3. Run Development Server

```bash
npm run dev
```

Dashboard will be available at: **http://localhost:3000**

### 4. Build for Production

```bash
npm run build
npm start
```

## Project Structure

```
judge-dashboard/
├── pages/
│   ├── _app.tsx                 # App wrapper and global setup
│   ├── index.tsx                # Home page (main dashboard)
│   └── api/                     # API routes (if needed)
├── components/
│   ├── StatsOverview.tsx        # Statistics cards and chart
│   ├── RecentScans.tsx          # Scan list with filtering
│   └── ScanDetails.tsx          # Detailed scan information
├── styles/
│   └── globals.css              # Global styles
├── public/                      # Static assets
├── package.json                 # Dependencies
├── next.config.js               # Next.js configuration
├── tailwind.config.js           # Tailwind CSS config
├── tsconfig.json                # TypeScript config
└── .env.local                   # Environment variables (git-ignored)
```

## API Integration

The dashboard connects to the backend API at the URL specified in `.env.local`.

### Required Endpoints

**1. Get Statistics**
```
GET /api/stats/summary
Response: { totalScans, genuineCount, suspectCount, rejectedCount }
```

**2. Get Recent Scans**
```
GET /api/stats/recent-scans?limit=20
Response: Array of scan objects
```

**3. Get Single Scan (Optional)**
```
GET /api/scans/:id
Response: Single scan with all details
```

### Response Format

```json
{
  "id": "scan-001",
  "uid": "CD8DD505",
  "medicineName": "Paracetamol",
  "batchId": "BATCH-2024-001",
  "rfidStatus": "PASS",
  "uvResult": "PASS",
  "uvValue": 450,
  "rgbResult": "PASS",
  "rgbValue": { "r": 245, "g": 245, "b": 245 },
  "finalResult": "GENUINE",
  "timestamp": "2024-12-20T10:30:00Z"
}
```

## Features & Usage

### Viewing Statistics

The top section shows:
- **Total Scans**: Cumulative count of all scans
- **Genuine**: Count and percentage of verified medicines
- **Suspect**: Count and percentage of suspicious medicines
- **Rejected**: Count and percentage of invalid medicines
- **Chart**: Bar chart showing distribution

### Browsing Recent Scans

The left panel shows recent scans. Each scan displays:
- **UID**: Medicine RFID tag ID
- **Status Badge**: Visual indicator (✓ ✗ ⚠️)
- **Medicine Name**: Product name (if available)
- **Batch ID**: Batch number (if available)
- **Time**: How long ago the scan occurred

Click any scan to view full details.

### Viewing Scan Details

The right panel shows comprehensive information:
- **Final Result**: Large status indicator
- **Medicine Info**: UID, name, batch
- **Verification Steps**: RFID, UV, RGB results
- **Raw Values**: Sensor readings and color codes
- **Timestamp**: Precise date/time

## Real-time Updates

The dashboard auto-refreshes data every 5 seconds:
- Statistics update automatically
- New scans appear immediately
- No manual refresh needed

## Deployment

### Local Network
1. Build: `npm run build`
2. Start: `npm start`
3. Access from any device on network: `http://<computer-ip>:3000`

### Docker (Optional)

Create `Dockerfile`:
```dockerfile
FROM node:18-alpine
WORKDIR /app
COPY . .
RUN npm install && npm run build
EXPOSE 3000
CMD ["npm", "start"]
```

Build and run:
```bash
docker build -t medicine-dashboard .
docker run -p 3000:3000 -e NEXT_PUBLIC_API_URL=http://backend:3001 medicine-dashboard
```

### Cloud Deployment (Vercel)

```bash
npm install -g vercel
vercel
```

Set environment variable in Vercel dashboard:
- `NEXT_PUBLIC_API_URL` → Your backend URL

## Troubleshooting

### Issue: "Failed to load statistics"
**Solution**: Check backend API URL in `.env.local` and verify backend is running

### Issue: Dashboard shows "No scans yet"
**Solution**: 
1. Verify backend is connected to MongoDB
2. Run scans on ESP32 device
3. Check backend receives scan data

### Issue: Old data not refreshing
**Solution**: 
1. Hard refresh browser (Ctrl+F5 or Cmd+Shift+R)
2. Check browser console for errors
3. Verify API endpoint paths

### Issue: CORS errors in console
**Solution**: 
1. Check backend has CORS enabled
2. Verify `NEXT_PUBLIC_API_URL` matches backend URL
3. No trailing slash should be in URL

## Development

### Add New Component
```bash
# Create new component
touch components/YourComponent.tsx

# Import in pages/index.tsx
import YourComponent from '../components/YourComponent';
```

### Modify Styling
- Edit `styles/globals.css` for global styles
- Use Tailwind classes directly in components
- Theme colors in `tailwind.config.js`

### Add New Page
```typescript
// pages/history.tsx
export default function HistoryPage() {
  return <div>Scan History Page</div>;
}
```

## API Error Codes

| Code | Meaning |
|------|---------|
| 200 | Success |
| 404 | Not found |
| 500 | Server error |
| ECONNREFUSED | Backend not running |
| ERR_NAME_NOT_RESOLVED | Invalid API URL |

## Performance Tips

- Dashboard loads quickly (Next.js optimizations)
- Auto-refresh every 5s (configurable)
- Responsive design works on mobile
- Charts render efficiently with Recharts

## Support Files

- **Backend Setup**: See `backend-api/README.md`
- **ESP32 Setup**: See `esp32-firmware/ESP32_FIRMWARE_INTEGRATION_GUIDE.md`
- **MongoDB Setup**: See `mongodb-config/MONGODB_SETUP.md`

## License

Academic prototype for medicine counterfeit detection research.

## Next Steps

1. ✅ Run `npm install` to install dependencies
2. ✅ Configure `NEXT_PUBLIC_API_URL` in `.env.local`
3. ✅ Start backend API (see backend-api folder)
4. ✅ Run `npm run dev` to start dashboard
5. ✅ Open http://localhost:3000 in browser
6. ✅ Scan medicines with ESP32 device to see results
