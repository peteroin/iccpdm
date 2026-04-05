# Backend API Setup

## Installation

```bash
cd backend-api
npm install
```

## Configuration

### 1. MongoDB Atlas Connection

1. Create account at https://www.mongodb.com/cloud/atlas (free tier available)
2. Create cluster (free M0 tier)
3. Create database user and get connection string
4. See `../mongodb-config/MONGODB_SETUP.md` for step-by-step instructions

### 2. Create `.env` file

```env
MONGODB_URI=mongodb+srv://medicine-admin:PASSWORD@medicine-detector.mongodb.net/?retryWrites=true&w=majority
MONGODB_DATABASE=medicine_db
PORT=3001
NODE_ENV=development
```

**Get MONGODB_URI from MongoDB Atlas**:
- Go to Cluster → Connect → Drivers
- Copy connection string and replace PASSWORD with your user password

## Running the Server

```bash
# Development (with auto-reload)
npm run dev

# Production
npm start
```

Server will start at `http://localhost:3001`

Expected output:
```
✓ MongoDB connected to medicine_db
✓ Backend API running on http://localhost:3001
```

## API Endpoints

### Medicines
- `GET /api/medicines/:uid` - Get medicine record
- `POST /api/medicines` - Create new medicine
- `PUT /api/medicines/:uid/calibrate` - Update calibration values

### Scans
- `POST /api/scans` - Record scan result
- `GET /api/scans/:uid` - Get scan history

### Statistics
- `GET /api/stats/summary` - System summary
- `GET /api/stats/recent-scans` - Recent scan results

### Health
- `GET /api/health` - Check API & MongoDB status

## Testing with cURL

```bash
# Health check (verify MongoDB connection)
curl http://localhost:3001/api/health

# Get medicine
curl http://localhost:3001/api/medicines/CD8DD505

# Get recent scans
curl http://localhost:3001/api/stats/recent-scans

# Record scan
curl -X POST http://localhost:3001/api/scans \
  -H "Content-Type: application/json" \
  -d '{
    "uid": "CD8DD505",
    "rfidStatus": "PASS",
    "uvResult": "PASS",
    "rgbResult": "PASS",
    "finalResult": "GENUINE"
  }'
```

## ESP32 Integration

ESP32 should make requests to backend API:

```cpp
const char* backendUrl = "http://192.168.1.100:3001";  // Change to your backend IP

// Example: GET medicine record
String url = String(backendUrl) + "/api/medicines/" + uid;
HTTPClient http;
http.begin(url);
int httpCode = http.GET();
if (httpCode == 200) {
  String payload = http.getString();
  // Parse JSON response
}
http.end();
```

## File Structure

- `server.js` - Express API server (all endpoints)
- `mongodb-service.js` - MongoDB integration layer using Mongoose
- `package.json` - Dependencies (express, mongoose, cors, dotenv)
- `.env.example` - Environment variable template

## Dependencies

- `express` - REST API framework
- `mongoose` - MongoDB ODM for Node.js
- `cors` - Cross-Origin Resource Sharing middleware
- `body-parser` - JSON body parsing
- `dotenv` - Environment variable management

## Database Schema

**Collections in MongoDB**:
- `medicines` - Registered medicine records
- `scanHistory` - Individual scan results
- `recalledBatches` - Recalled batch tracking

See `../mongodb-config/DATABASE_SCHEMA.md` for complete schema.

## Troubleshooting

| Problem | Solution |
|---------|----------|
| MongoDB connection failed | Check MONGODB_URI and IP whitelisting in Atlas |
| "Authentication failed" | Verify database user credentials and password |
| Collection not found | Ensure collections exist in MongoDB Atlas |
| API returns empty results | Verify initial data was imported to MongoDB |
| Port 3001 already in use | Change PORT in .env or kill process: `lsof -ti:3001 \| xargs kill` |
| CORS errors | Update CORS config in server.js (default allows all) |

## Performance

- **Connection pooling**: 10 simultaneous connections (configurable)
- **Response time**: <100ms average
- **Free tier storage**: 512MB (sufficient for 500K+ scans)

## Production Deployment

For live deployment:

1. **Upgrade MongoDB tier**: M0 (free) → M2+ for production
2. **IP Whitelisting**: Restrict to backend server IPs only (don't use "Allow from Anywhere")
3. **Enable SSL/TLS**: Already enabled by default in MongoDB Atlas
4. **Backups**: Enable automatic daily backups (M2+ tier)
5. **Monitoring**: Set up alerts in MongoDB Atlas console

## Documentation

- `../mongodb-config/MONGODB_SETUP.md` - Step-by-step MongoDB setup
- `../mongodb-config/DATABASE_SCHEMA.md` - Complete database schema
- `../MONGODB_MIGRATION.md` - Migration from Firebase to MongoDB
- `../README.md` - System-wide setup guide
