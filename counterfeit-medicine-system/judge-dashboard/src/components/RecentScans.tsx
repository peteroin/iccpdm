// src/components/RecentScans.tsx
import React from 'react';
import { formatDistanceToNow } from 'date-fns';

interface Scan {
  id: string;
  uid: string;
  medicineName?: string;
  batchId?: string;
  rfidStatus: string;
  uvResult: string;
  rgbResult: string;
  finalResult: string;
  timestamp: string;
}

interface RecentScansProps {
  scans: Scan[];
  onSelectScan: (scan: Scan) => void;
  selectedScan: Scan | null;
}

const getResultColor = (result: string): string => {
  switch (result?.toUpperCase()) {
    case 'GENUINE':
      return 'bg-green-50 border-l-4 border-l-green-500';
    case 'SUSPECT':
      return 'bg-yellow-50 border-l-4 border-l-yellow-500';
    case 'COUNTERFEIT':
    case 'REJECTED':
    case 'UNREGISTERED':
    case 'EXPIRED':
    case 'RECALLED':
      return 'bg-red-50 border-l-4 border-l-red-500';
    default:
      return 'bg-gray-50 border-l-4 border-l-gray-300';
  }
};

const getResultBadge = (result: string): { bg: string; text: string; icon: string } => {
  switch (result?.toUpperCase()) {
    case 'GENUINE':
      return { bg: 'bg-green-100', text: 'text-green-800', icon: '✓' };
    case 'SUSPECT':
      return { bg: 'bg-yellow-100', text: 'text-yellow-800', icon: '⚠️' };
    case 'COUNTERFEIT':
    case 'REJECTED':
    case 'UNREGISTERED':
    case 'EXPIRED':
    case 'RECALLED':
      return { bg: 'bg-red-100', text: 'text-red-800', icon: '✗' };
    default:
      return { bg: 'bg-gray-100', text: 'text-gray-800', icon: '?' };
  }
};

export default function RecentScans({
  scans,
  onSelectScan,
  selectedScan,
}: RecentScansProps) {
  return (
    <div className="bg-white rounded-lg shadow-md">
      <div className="border-b border-gray-200 px-6 py-4">
        <h2 className="text-xl font-semibold text-gray-800">Recent Scans</h2>
        <p className="text-sm text-gray-500 mt-1">{scans.length} scans</p>
      </div>

      <div className="divide-y divide-gray-200 max-h-96 overflow-y-auto">
        {scans.length > 0 ? (
          scans.map((scan) => {
            const badge = getResultBadge(scan.finalResult);
            const isSelected = selectedScan?.id === scan.id;

            return (
              <div
                key={scan.id}
                onClick={() => onSelectScan(scan)}
                className={`p-4 cursor-pointer hover:bg-gray-50 transition ${getResultColor(
                  scan.finalResult
                )} ${isSelected ? 'bg-blue-100 border-l-4 border-l-blue-500' : ''}`}
              >
                <div className="flex items-start justify-between">
                  <div className="flex-1">
                    <div className="flex items-center gap-2">
                      <p className="font-mono text-sm font-semibold text-gray-800">{scan.uid}</p>
                      <span className={`px-2 py-1 rounded text-xs font-semibold ${badge.bg} ${badge.text}`}>
                        {badge.icon} {scan.finalResult}
                      </span>
                    </div>
                    {scan.medicineName && (
                      <p className="text-sm text-gray-600 mt-1">{scan.medicineName}</p>
                    )}
                    {scan.batchId && (
                      <p className="text-xs text-gray-500 mt-1">Batch: {scan.batchId}</p>
                    )}
                  </div>
                  <div className="text-right">
                    <p className="text-xs text-gray-500">
                      {formatDistanceToNow(new Date(scan.timestamp), { addSuffix: true })}
                    </p>
                  </div>
                </div>
              </div>
            );
          })
        ) : (
          <div className="p-8 text-center text-gray-500">
            <p className="text-lg">No scans yet</p>
            <p className="text-sm mt-2">Scanned medicines will appear here</p>
          </div>
        )}
      </div>
    </div>
  );
}
