import React from 'react';

export const CardSkeleton = () => (
  <div className="card p-6 animate-pulse">
    <div className="h-6 bg-gray-700 rounded w-1/3 mb-4"></div>
    <div className="space-y-3">
      <div className="h-4 bg-gray-700 rounded w-full"></div>
      <div className="h-4 bg-gray-700 rounded w-5/6"></div>
      <div className="h-4 bg-gray-700 rounded w-4/6"></div>
    </div>
  </div>
);

export const VideoSkeleton = () => (
  <div className="card p-6 animate-pulse">
    <div className="h-6 bg-gray-700 rounded w-1/4 mb-4"></div>
    <div className="bg-gray-700 rounded-lg aspect-video"></div>
  </div>
);

export const SystemHealthSkeleton = () => (
  <div className="card p-6 animate-pulse">
    <div className="h-6 bg-gray-700 rounded w-1/3 mb-4"></div>
    <div className="space-y-4">
      {[1, 2, 3].map((i) => (
        <div key={i} className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-8 h-8 bg-gray-700 rounded-full"></div>
            <div className="h-4 bg-gray-700 rounded w-20"></div>
          </div>
          <div className="h-3 bg-gray-700 rounded w-16"></div>
        </div>
      ))}
    </div>
  </div>
);

export const DashboardLoader = () => (
  <div className="min-h-screen bg-gradient-to-br from-nightfall-primary via-nightfall-secondary to-nightfall-primary p-6">
    <div className="card p-6 mb-6 animate-pulse">
      <div className="h-8 bg-gray-700 rounded w-1/3"></div>
    </div>
    <div className="grid grid-cols-1 xl:grid-cols-12 gap-6">
      <div className="xl:col-span-7">
        <VideoSkeleton />
      </div>
      <div className="xl:col-span-3">
        <SystemHealthSkeleton />
      </div>
      <div className="xl:col-span-2">
        <CardSkeleton />
      </div>
    </div>
  </div>
);
