# MongoDB Database Schema - Counterfeit Medicine Detection

## Collections Overview

```
medicine_db/
├── medicines              (Registered medicine records)
├── scanHistory           (Individual scan results)
├── recalledBatches       (Recalled batch tracking)
└── users                 (Optional: user accounts)
```

## 1. medicines Collection

**Purpose**: Store registered medicine and verification parameters

**Indexes**:
```javascript
db.medicines.createIndex({ uid: 1 }, { unique: true });
db.medicines.createIndex({ batchId: 1 });
```

**Schema**:
```json
{
  "_id": ObjectId,
  "uid": String,                    // RFID UID (e.g., "CD8DD505")
  "medicineName": String,           // Product name (e.g., "Paracetamol")
  "batchId": String,                // Batch identifier
  "registered": Boolean,            // Is medicine registered?
  "expiryDate": String,             // Format: "YYYY-MM-DD"
  "recalled": Boolean,              // Is batch recalled?
  "uvRef": Number,                  // UV reference value (0-4095)
  "uvTol": Number,                  // UV tolerance (±)
  "rgbRef": {
    "r": Number,                    // Red reference (0-255)
    "g": Number,                    // Green reference (0-255)
    "b": Number                     // Blue reference (0-255)
  },
  "rgbTol": {
    "r": Number,                    // Red tolerance (±)
    "g": Number,                    // Green tolerance (±)
    "b": Number                     // Blue tolerance (±)
  },
  "scanCount": Number,              // Total scans performed
  "lastScanDate": String,           // Last scan timestamp
  "createdAt": ISO8601Date          // Record created timestamp
}
```

**Example**:
```json
{
  "_id": ObjectId("507f1f77bcf86cd799439011"),
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
  "scanCount": 5,
  "lastScanDate": "2024-12-20T15:30:00Z",
  "createdAt": ISODate("2024-01-01T00:00:00Z")
}
```

## 2. scanHistory Collection

**Purpose**: Log every scan result for analytics and audit

**Indexes**:
```javascript
db.scanHistory.createIndex({ timestamp: -1 });
db.scanHistory.createIndex({ uid: 1 });
db.scanHistory.createIndex({ finalResult: 1 });
```

**Schema**:
```json
{
  "_id": ObjectId,
  "uid": String,                    // Medicine UID
  "rfidStatus": String,             // "PASS" or "FAIL"
  "uvResult": String,               // "PASS", "FAIL", or "ERROR"
  "uvValue": Number,                // Raw ADC reading
  "rgbResult": String,              // "PASS", "FAIL", or "ERROR"
  "rgbValue": {
    "r": Number,                    // Measured red (0-255)
    "g": Number,                    // Measured green (0-255)
    "b": Number                     // Measured blue (0-255)
  },
  "finalResult": String,            // "GENUINE", "SUSPECT", "REJECTED", "UNREGISTERED", "EXPIRED", "RECALLED"
  "timestamp": ISO8601Date          // When scan occurred
}
```

**Example**:
```json
{
  "_id": ObjectId("507f1f77bcf86cd799439012"),
  "uid": "CD8DD505",
  "rfidStatus": "PASS",
  "uvResult": "PASS",
  "uvValue": 450,
  "rgbResult": "PASS",
  "rgbValue": { "r": 245, "g": 245, "b": 245 },
  "finalResult": "GENUINE",
  "timestamp": ISODate("2024-12-20T10:30:00Z")
}
```

## 3. recalledBatches Collection

**Purpose**: Track recalled medicine batches

**Indexes**:
```javascript
db.recalledBatches.createIndex({ batchId: 1 }, { unique: true });
db.recalledBatches.createIndex({ medicines: 1 });
```

**Schema**:
```json
{
  "_id": ObjectId,
  "batchId": String,                // Batch identifier
  "reason": String,                 // Reason for recall
  "recalledDate": String,           // Date recalled (ISO 8601)
  "medicines": [String],            // Array of affected UIDs
  "createdAt": ISO8601Date          // Record created date
}
```

**Example**:
```json
{
  "_id": ObjectId("507f1f77bcf86cd799439013"),
  "batchId": "BATCH-2024-003",
  "reason": "Quality issue detected during inspection",
  "recalledDate": "2024-12-20",
  "medicines": ["CD8DD505", "4D2CAA04"],
  "createdAt": ISODate("2024-12-20T12:00:00Z")
}
```

## 4. users Collection (Optional)

**Purpose**: Track users and admin access (for future authentication)

**Schema**:
```json
{
  "_id": ObjectId,
  "username": String,
  "email": String,
  "passwordHash": String,
  "role": String,                   // "admin", "judge", "scanner"
  "createdAt": ISO8601Date,
  "lastLogin": ISO8601Date
}
```

## Database Design Rationale

### Separation of concerns
- **medicines**: Configuration & reference data (grows linearly)
- **scanHistory**: Transactional data (grows exponentially)
- **recalledBatches**: Reference data (grows slowly)

### Performance optimization
- `uid` indexed for O(1) medicine lookups
- `timestamp` indexed descending for quick recent scans
- `finalResult` indexed for statistics queries

### Data integrity
- Unique constraint on `uid` prevents duplicates
- Unique constraint on `batchId` in recalledBatches
- Timestamps stored as ISO8601 for consistency

## Query Examples

### Fetch medicine by UID
```javascript
db.medicines.findOne({ uid: "CD8DD505" });
```

### Get recent scans (last 20)
```javascript
db.scanHistory.find()
  .sort({ timestamp: -1 })
  .limit(20);
```

### Statistics: Count by result type
```javascript
db.scanHistory.aggregate([
  {
    $group: {
      _id: "$finalResult",
      count: { $sum: 1 }
    }
  }
]);
```

### Find all scans for a medicine
```javascript
db.scanHistory.find({ uid: "CD8DD505" })
  .sort({ timestamp: -1 });
```

### Check if batch is recalled
```javascript
db.recalledBatches.findOne({ batchId: "BATCH-2024-003" });
```

### Update scan count
```javascript
db.medicines.updateOne(
  { uid: "CD8DD505" },
  { 
    $inc: { scanCount: 1 },
    $set: { lastScanDate: new Date() }
  }
);
```

## Migration from Firebase

If migrating from Firebase:

1. Export Firebase data as JSON
2. Transform to MongoDB format (add `_id` fields)
3. Import using MongoDB import tools
4. Update backend connection string
5. Test all API endpoints

---

**Schema version**: 1.0  
**Last updated**: 2024-2025  
**Database**: MongoDB Atlas (Cloud)
