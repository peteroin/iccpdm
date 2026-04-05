# MongoDB Setup Guide - Counterfeit Medicine Detection System

## Overview
Replace Firebase Realtime Database with MongoDB Atlas for cloud-hosted document database storage.

## Step 1: Create MongoDB Atlas Account

1. Go to https://www.mongodb.com/cloud/atlas
2. Click **Sign Up** or **Sign In**
3. Create a new account (free tier available)
4. Verify email address

## Step 2: Create Cluster

1. In MongoDB Atlas, click **Create a Deployment**
2. Choose **Free (M0)** tier (includes 512 MB storage)
3. Cloud Provider: **AWS** (or your preference)
4. Region: Choose nearest to your location
5. Cluster Name: `medicine-detector` (or your choice)
6. Click **Create Deployment**

Wait 3-5 minutes for cluster initialization.

## Step 3: Create Database User

1. In left sidebar, click **Database Access**
2. Click **Add New Database User**
3. Authentication Method: **Password**
4. Username: `medicine-admin`
5. Password: Generate strong password (save it!)
6. Database User Privileges: **Atlas admin** (for development)
7. Click **Add User**

## Step 4: Set Network Access

1. In left sidebar, click **Network Access**
2. Click **Add IP Address**
3. Select **Allow Access from Anywhere** (for development)
   - For production, restrict to your backend IP
4. Click **Confirm**

## Step 5: Get Connection String

1. Go to **Databases** → Your cluster
2. Click **Connect**
3. Choose **Drivers**
4. Select **Node.js** and version **4.x or later**
5. Copy the connection string:
   ```
   mongodb+srv://medicine-admin:<password>@medicine-detector.xxxxx.mongodb.net/?retryWrites=true&w=majority
   ```
6. Replace `<password>` with your database user password

## Step 6: Create Collections

1. Go to **Databases** → Your cluster → **Collections**
2. Click **Create Database**
3. Database Name: `medicine_db`
4. Collection Name: `medicines`
5. Click **Create**

Repeat to create collections:
- `scanHistory`
- `recalledBatches`
- `users` (optional for authentication)

## Step 7: Add Initial Data

### Option A: Import JSON (Recommended)

1. Download `INITIAL_DATA.json` from this folder
2. In MongoDB Atlas, go to Collections
3. Collection: `medicines`
4. Click **Import Data** → **JSON**
5. Upload `INITIAL_DATA.json`

### Option B: Manual Insert

1. In Collections → `medicines`
2. Click **Insert Document**
3. Paste each document from `INITIAL_DATA.json`

## Environment Configuration

In `backend-api/.env`, add:

```env
MONGODB_URI=mongodb+srv://medicine-admin:YOUR_PASSWORD@medicine-detector.xxxxx.mongodb.net/?retryWrites=true&w=majority
MONGODB_DATABASE=medicine_db
PORT=3001
```

## Verification

Test connection:

```bash
cd backend-api
npm start
```

Expected output:
```
✓ MongoDB connected to medicine_db
✓ Server running at http://localhost:3001
```

Test with curl:
```bash
curl http://localhost:3001/api/medicines/CD8DD505
```

Should return medicine record.

## Database Schema

### medicines Collection
```json
{
  "_id": "ObjectId",
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
  "lastScanDate": "2024-01-01",
  "createdAt": "2024-01-01"
}
```

### scanHistory Collection
```json
{
  "_id": "ObjectId",
  "uid": "CD8DD505",
  "rfidStatus": "PASS",
  "uvResult": "PASS",
  "uvValue": 450,
  "rgbResult": "PASS",
  "finalResult": "GENUINE",
  "timestamp": "2024-12-20T10:30:00Z"
}
```

### recalledBatches Collection
```json
{
  "_id": "ObjectId",
  "batchId": "BATCH-2024-003",
  "reason": "Quality issue",
  "recalledDate": "2024-12-20",
  "medicines": ["CD8DD505", "4D2CAA04"]
}
```

## Production Considerations

### Increase Cluster Tier
- For production, upgrade from M0 to **M2** or higher
- Add automatic backup
- Enable encryption at rest

### Network Security
- Restrict IP access to backend server only
- Use VPC peering for AWS deployments
- Enable TLS 1.2+ (default in Atlas)

### Monitoring
- Enable MongoDB Atlas alerts
- Set up performance advisor
- Regular backup testing

### Indexing
- Create index on `uid` field for fast lookups
- Create index on `timestamp` for scan history

```javascript
db.medicines.createIndex({ uid: 1 });
db.scanHistory.createIndex({ timestamp: -1 });
```

## Troubleshooting

### Connection Error: "getaddrinfo ENOTFOUND"
- Verify IP address is added to Network Access
- Check connection string is correct
- Verify password has no special characters (or URL-encode them)

### Authentication Failed
- Verify database user exists in Database Access
- Confirm password is correct
- Check user has appropriate permissions

### Collection Not Found
- Verify collection names match (case-sensitive)
- Ensure data was imported to correct database
- Check database name in connection string

### Timeout Errors
- Add `serverSelectionTimeoutMS=5000` to connection string
- Verify network connectivity to MongoDB servers
- Check firewall allows outbound HTTPS (port 27017)

## Backup & Recovery

### Automatic Backups (M2+ tier)
- MongoDB Atlas automatically backs up hourly
- Retention: 7 days for free tier, 30+ days for paid

### Manual Backup
```bash
# Export all data
mongodump --uri="YOUR_CONNECTION_STRING"

# Import backup
mongorestore --uri="YOUR_CONNECTION_STRING" ./dump
```

## Next Steps

1. ✅ Create MongoDB Atlas account and cluster
2. ✅ Set up database user and network access
3. ✅ Get connection string
4. ✅ Import initial data
5. ✅ Update backend `.env` file
6. ✅ Test API endpoints
7. ✅ Run ESP32 and Dashboard (API stays the same!)

---

**MongoDB Atlas is now your cloud database! 🚀**
