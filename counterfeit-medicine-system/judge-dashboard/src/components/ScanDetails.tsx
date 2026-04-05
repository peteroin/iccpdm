// src/components/ScanDetails.tsx
import React from 'react';
import { format } from 'date-fns';

interface Scan {
  id: string;
  uid: string;
  medicineName?: string;
  batchId?: string;
  rfidStatus: string;
  uvResult: string;
  uvValue?: number;
  rgbResult: string;
  rgbValue?: {
    r: number;
    g: number;
    b: number;
  };
  finalResult: string;
  timestamp: string;
}

interface ScanDetailsProps {
  scan: Scan;
}

const getStatusIcon = (status: string): string => {
  switch (status?.toUpperCase()) {
    case 'PASS':
      return '✓';
    case 'FAIL':
      return '✗';
    default:
      return '○';
  }
};

const getStatusColor = (status: string): string => {
  switch (status?.toUpperCase()) {
    case 'PASS':
      return 'text-green-600';
    case 'FAIL':
      return 'text-red-600';
    default:
      return 'text-gray-600';
  }
};

const getResultBackground = (result: string): string => {
  switch (result?.toUpperCase()) {
    case 'GENUINE':
      return 'bg-green-50';
    case 'SUSPECT':
      return 'bg-yellow-50';
    case 'COUNTERFEIT':
    case 'REJECTED':
    case 'UNREGISTERED':
    case 'EXPIRED':
    case 'RECALLED':
      return 'bg-red-50';
    default:
      return 'bg-gray-50';
  }
};

export default function ScanDetails({ scan }: ScanDetailsProps) {
  return (
    <div className="bg-white rounded-lg shadow-md p-6 h-full">
      {/* Final Result */}
      <div className={`${getResultBackground(scan.finalResult)} rounded-lg p-4 mb-6`}>
        <p className="text-sm text-gray-600 mb-1">Final Result</p>
        <p className="text-3xl font-bold text-gray-800">{scan.finalResult}</p>
      </div>

      {/* Medicine Information */}
      <div className="mb-6">
        <h3 className="text-sm font-semibold text-gray-700 mb-3">Medicine Information</h3>
        <div className="space-y-2 text-sm">
          <div>
            <p className="text-gray-500">UID</p>
            <p className="font-mono font-semibold text-gray-800 break-all">{scan.uid}</p>
          </div>
          {scan.medicineName && (
            <div>
              <p className="text-gray-500">Name</p>
              <p className="text-gray-800">{scan.medicineName}</p>
            </div>
          )}
          {scan.batchId && (
            <div>
              <p className="text-gray-500">Batch ID</p>
              <p className="text-gray-800">{scan.batchId}</p>
            </div>
          )}
        </div>
      </div>

      {/* Verification Results */}
      <div className="mb-6">
        <h3 className="text-sm font-semibold text-gray-700 mb-3">Verification Results</h3>
        <div className="space-y-2 text-sm">
          {/* RFID */}
          <div className="flex items-center justify-between p-2 bg-gray-50 rounded">
            <span className="text-gray-700">RFID</span>
            <span className={`font-semibold ${getStatusColor(scan.rfidStatus)}`}>
              {getStatusIcon(scan.rfidStatus)} {scan.rfidStatus}
            </span>
          </div>

          {/* UV */}
          <div className="flex items-center justify-between p-2 bg-gray-50 rounded">
            <span className="text-gray-700">UV Check</span>
            <span className={`font-semibold ${getStatusColor(scan.uvResult)}`}>
              {getStatusIcon(scan.uvResult)} {scan.uvResult}
            </span>
          </div>
          {scan.uvValue !== undefined && (
            <p className="text-xs text-gray-500 ml-2">Value: {scan.uvValue}</p>
          )}

          {/* RGB */}
          <div className="flex items-center justify-between p-2 bg-gray-50 rounded">
            <span className="text-gray-700">RGB Check</span>
            <span className={`font-semibold ${getStatusColor(scan.rgbResult)}`}>
              {getStatusIcon(scan.rgbResult)} {scan.rgbResult}
            </span>
          </div>
          {scan.rgbValue && (
            <div className="text-xs text-gray-500 ml-2 flex items-center gap-2">
              <span>RGB: ({scan.rgbValue.r}, {scan.rgbValue.g}, {scan.rgbValue.b})</span>
              <div
                className="w-6 h-6 rounded border border-gray-300"
                style={{
                  backgroundColor: `rgb(${scan.rgbValue.r}, ${scan.rgbValue.g}, ${scan.rgbValue.b})`,
                }}
              />
            </div>
          )}
        </div>
      </div>

      {/* Timestamp */}
      <div className="pt-6 border-t border-gray-200">
        <p className="text-xs text-gray-500">Scan Time</p>
        <p className="text-sm text-gray-800 font-semibold">
          {format(new Date(scan.timestamp), 'MMM dd, yyyy HH:mm:ss')}
        </p>
      </div>
    </div>
  );
}
