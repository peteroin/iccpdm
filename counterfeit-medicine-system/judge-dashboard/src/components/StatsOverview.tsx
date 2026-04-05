// src/components/StatsOverview.tsx
import React from 'react';
import { BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';

interface Stats {
  totalScans: number;
  genuineCount: number;
  suspectCount: number;
  rejectedCount: number;
}

interface StatsOverviewProps {
  stats: Stats;
}

export default function StatsOverview({ stats }: StatsOverviewProps) {
  const genuinePercent = stats.totalScans > 0 ? ((stats.genuineCount / stats.totalScans) * 100).toFixed(1) : 0;
  const suspectPercent = stats.totalScans > 0 ? ((stats.suspectCount / stats.totalScans) * 100).toFixed(1) : 0;
  const rejectedPercent = stats.totalScans > 0 ? ((stats.rejectedCount / stats.totalScans) * 100).toFixed(1) : 0;

  const chartData = [
    { name: 'Genuine', value: stats.genuineCount },
    { name: 'Suspect', value: stats.suspectCount },
    { name: 'Rejected', value: stats.rejectedCount },
  ];

  return (
    <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
      {/* Total Scans */}
      <div className="bg-white rounded-lg shadow-md p-6">
        <div className="flex items-center justify-between">
          <div>
            <p className="text-gray-500 text-sm">Total Scans</p>
            <p className="text-3xl font-bold text-gray-800">{stats.totalScans}</p>
          </div>
          <div className="text-4xl">📊</div>
        </div>
      </div>

      {/* Genuine */}
      <div className="bg-white rounded-lg shadow-md p-6">
        <div className="flex items-center justify-between">
          <div>
            <p className="text-gray-500 text-sm">Genuine</p>
            <p className="text-3xl font-bold text-green-600">{stats.genuineCount}</p>
            <p className="text-xs text-gray-400 mt-1">{genuinePercent}%</p>
          </div>
          <div className="text-4xl">✓</div>
        </div>
      </div>

      {/* Suspect */}
      <div className="bg-white rounded-lg shadow-md p-6">
        <div className="flex items-center justify-between">
          <div>
            <p className="text-gray-500 text-sm">Suspect</p>
            <p className="text-3xl font-bold text-yellow-600">{stats.suspectCount}</p>
            <p className="text-xs text-gray-400 mt-1">{suspectPercent}%</p>
          </div>
          <div className="text-4xl">⚠️</div>
        </div>
      </div>

      {/* Rejected */}
      <div className="bg-white rounded-lg shadow-md p-6">
        <div className="flex items-center justify-between">
          <div>
            <p className="text-gray-500 text-sm">Rejected</p>
            <p className="text-3xl font-bold text-red-600">{stats.rejectedCount}</p>
            <p className="text-xs text-gray-400 mt-1">{rejectedPercent}%</p>
          </div>
          <div className="text-4xl">✗</div>
        </div>
      </div>

      {/* Chart */}
      <div className="md:col-span-4 bg-white rounded-lg shadow-md p-6">
        <h3 className="text-lg font-semibold text-gray-800 mb-4">Scan Results Distribution</h3>
        <ResponsiveContainer width="100%" height={300}>
          <BarChart data={chartData}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis dataKey="name" />
            <YAxis />
            <Tooltip />
            <Legend />
            <Bar dataKey="value" fill="#3b82f6" />
          </BarChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
}
