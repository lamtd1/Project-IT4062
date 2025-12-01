import React from 'react';
import Card from '../../components/common/Card.jsx';
import Button from '../../components/common/Button.jsx';
import { MOCK_DATA } from '../../lib/mockData.js';

const StudentDashboard = () => (
  <div className="space-y-8">
    <div className="relative overflow-hidden bg-gradient-to-r from-blue-600 to-blue-400 rounded-2xl p-8 text-white shadow-xl">
      <div className="relative z-10">
        <h2 className="text-3xl font-bold mb-2">おかえり！</h2>
        <p className="text-blue-100 max-w-xl">続けましょう</p>
      </div>
    </div>

    <div>
      <h3 className="text-xl font-bold text-gray-900 mb-4 flex items-center gap-2">
        最近のアクティビティ
      </h3>
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6">
        {MOCK_DATA.slides.map((slide) => (
           <Card key={slide.id} className="hover:shadow-md transition-all cursor-pointer group border-l-4 border-blue-500">
             <div className="flex justify-between items-start mb-3">
               <span className="text-xs font-bold text-gray-500">これまでに見たレッスン</span>
             </div>
             <h4 className="font-bold text-gray-900 mb-2">{slide.title}</h4>
             <p className="text-xs text-gray-500 mb-4">{slide.date}</p>
           </Card>
        ))}
      </div>
    </div>
  </div>
);

export default StudentDashboard;