/**
 * MongoDB Service - Database abstraction layer for MongoDB Atlas
 * Replaces firebase-service.js with equivalent MongoDB operations
 * 
 * Provides identical function signatures for seamless backend integration
 */

const mongoose = require('mongoose');

// MongoDB Connection URI from environment
const MONGODB_URI = process.env.MONGODB_URI || 'mongodb+srv://medicine-admin:password@medicine-detector.mongodb.net/?retryWrites=true&w=majority';
const DB_NAME = process.env.MONGODB_DATABASE || 'medicine_db';

// Define Schemas
const medicineSchema = new mongoose.Schema({
  uid: { type: String, required: true, unique: true, index: true },
  medicineName: String,
  batchId: String,
  registered: Boolean,
  expiryDate: String,
  recalled: Boolean,
  uvRef: Number,
  uvTol: Number,
  rgbRef: {
    r: Number,
    g: Number,
    b: Number
  },
  rgbTol: {
    r: Number,
    g: Number,
    b: Number
  },
  scanCount: { type: Number, default: 0 },
  lastScanDate: String,
  createdAt: { type: Date, default: Date.now }
});

const scanHistorySchema = new mongoose.Schema({
  uid: { type: String, required: true, index: true },
  rfidStatus: String,
  uvResult: String,
  uvValue: Number,
  rgbResult: String,
  rgbValue: {
    r: Number,
    g: Number,
    b: Number
  },
  finalResult: String,
  timestamp: { type: Date, default: Date.now, index: -1 }
});

const recalledBatchSchema = new mongoose.Schema({
  batchId: { type: String, required: true, unique: true, index: true },
  reason: String,
  recalledDate: String,
  medicines: [String],
  createdAt: { type: Date, default: Date.now }
});

// Create Models
let Medicine;
let ScanHistory;
let RecalledBatch;
let isConnected = false;

/**
 * Initialize MongoDB connection
 */
async function initializeMongoDB() {
  try {
    if (isConnected) {
      console.log('✓ MongoDB already connected');
      return;
    }

    await mongoose.connect(MONGODB_URI, {
      dbName: DB_NAME,
      maxPoolSize: 10,
      serverSelectionTimeoutMS: 5000,
    });

    // Create models
    Medicine = mongoose.model('Medicine', medicineSchema, 'medicines');
    ScanHistory = mongoose.model('ScanHistory', scanHistorySchema, 'scanHistory');
    RecalledBatch = mongoose.model('RecalledBatch', recalledBatchSchema, 'recalledBatches');

    isConnected = true;
    console.log(`✓ MongoDB connected to ${DB_NAME}`);
  } catch (error) {
    console.error('✗ MongoDB connection failed:', error.message);
    throw error;
  }
}

/**
 * Get medicine record by UID
 * @param {string} uid - Medicine RFID UID
 * @returns {Promise<Object|null>} Medicine record or null
 */
async function getMedicineRecord(uid) {
  try {
    if (!Medicine) throw new Error('MongoDB not initialized');
    
    const medicine = await Medicine.findOne({ uid });
    return medicine ? medicine.toObject() : null;
  } catch (error) {
    console.error('Error fetching medicine:', error.message);
    throw error;
  }
}

/**
 * Add scan record to history
 * @param {Object} scanData - Scan result object
 * @returns {Promise<Object>} Created scan record with _id
 */
async function addScanRecord(scanData) {
  try {
    if (!ScanHistory) throw new Error('MongoDB not initialized');
    
    const scanRecord = new ScanHistory({
      uid: scanData.uid,
      rfidStatus: scanData.rfidStatus,
      uvResult: scanData.uvResult,
      uvValue: scanData.uvValue,
      rgbResult: scanData.rgbResult,
      rgbValue: scanData.rgbValue,
      finalResult: scanData.finalResult,
      timestamp: new Date()
    });

    const saved = await scanRecord.save();
    return {
      success: true,
      scanId: saved._id.toString(),
      timestamp: saved.timestamp
    };
  } catch (error) {
    console.error('Error adding scan record:', error.message);
    throw error;
  }
}

/**
 * Update scan count for a medicine
 * @param {string} uid - Medicine RFID UID
 * @returns {Promise<Object>} Updated medicine record
 */
async function updateScanCount(uid) {
  try {
    if (!Medicine) throw new Error('MongoDB not initialized');
    
    const updated = await Medicine.findOneAndUpdate(
      { uid },
      {
        $inc: { scanCount: 1 },
        $set: { lastScanDate: new Date().toISOString() }
      },
      { new: true }
    );

    return updated ? updated.toObject() : null;
  } catch (error) {
    console.error('Error updating scan count:', error.message);
    throw error;
  }
}

/**
 * Get all registered medicines
 * @returns {Promise<Array>} Array of all medicines
 */
async function getAllMedicines() {
  try {
    if (!Medicine) throw new Error('MongoDB not initialized');
    
    const medicines = await Medicine.find({});
    return medicines.map(m => m.toObject());
  } catch (error) {
    console.error('Error fetching all medicines:', error.message);
    throw error;
  }
}

/**
 * Check if batch is recalled
 * @param {string} batchId - Batch identifier
 * @returns {Promise<Object|null>} Recalled batch record or null
 */
async function isRecalled(batchId) {
  try {
    if (!RecalledBatch) throw new Error('MongoDB not initialized');
    
    const batch = await RecalledBatch.findOne({ batchId });
    return batch ? batch.toObject() : null;
  } catch (error) {
    console.error('Error checking recalled batch:', error.message);
    throw error;
  }
}

/**
 * Get recent scans
 * @param {number} limit - Number of recent scans to retrieve
 * @returns {Promise<Array>} Array of recent scan records
 */
async function getRecentScans(limit = 20) {
  try {
    if (!ScanHistory) throw new Error('MongoDB not initialized');
    
    const scans = await ScanHistory.find({})
      .sort({ timestamp: -1 })
      .limit(limit);
    
    return scans.map(s => s.toObject());
  } catch (error) {
    console.error('Error fetching recent scans:', error.message);
    throw error;
  }
}

/**
 * Get statistics summary
 * @returns {Promise<Object>} Statistics object
 */
async function getStatisticsSummary() {
  try {
    if (!ScanHistory) throw new Error('MongoDB not initialized');
    
    const stats = await ScanHistory.aggregate([
      {
        $group: {
          _id: '$finalResult',
          count: { $sum: 1 }
        }
      }
    ]);

    const result = {
      totalScans: 0,
      genuineCount: 0,
      suspectCount: 0,
      rejectedCount: 0
    };

    for (const stat of stats) {
      const statusMap = {
        'GENUINE': 'genuineCount',
        'SUSPECT': 'suspectCount',
        'REJECTED': 'rejectedCount'
      };

      if (statusMap[stat._id]) {
        result[statusMap[stat._id]] = stat.count;
      }
      result.totalScans += stat.count;
    }

    return result;
  } catch (error) {
    console.error('Error fetching statistics:', error.message);
    throw error;
  }
}

/**
 * Health check - verify MongoDB connection
 * @returns {Promise<Object>} Connection status
 */
async function healthCheck() {
  try {
    if (!isConnected || !mongoose.connection.db) {
      return {
        status: 'disconnected',
        database: DB_NAME,
        message: 'MongoDB not connected'
      };
    }

    // Ping MongoDB
    await mongoose.connection.db.admin().ping();
    
    return {
      status: 'connected',
      database: DB_NAME,
      message: 'MongoDB Atlas connection verified'
    };
  } catch (error) {
    console.error('Health check failed:', error.message);
    return {
      status: 'error',
      database: DB_NAME,
      message: error.message
    };
  }
}

/**
 * Disconnect from MongoDB
 */
async function disconnect() {
  try {
    if (isConnected) {
      await mongoose.disconnect();
      isConnected = false;
      console.log('✓ MongoDB disconnected');
    }
  } catch (error) {
    console.error('Error disconnecting MongoDB:', error.message);
    throw error;
  }
}

// Export service functions
module.exports = {
  initializeMongoDB,
  getMedicineRecord,
  addScanRecord,
  updateScanCount,
  getAllMedicines,
  isRecalled,
  getRecentScans,
  getStatisticsSummary,
  healthCheck,
  disconnect
};
