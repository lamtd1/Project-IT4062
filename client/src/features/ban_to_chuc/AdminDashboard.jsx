import React from 'react';
import Card from '../../components/common/Card.jsx';
import Button from '../../components/common/Button.jsx';
import { MOCK_DATA } from '../../lib/mockData.js';

const AdminDashboard = () => (
  <div className="space-y-6">
    <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
      <Card className="border-l-4 border-blue-500 flex items-center justify-between">
        <div><p className="text-gray-500 mb-1 text-sm font-bold">スライド</p><h3 className="text-3xl font-bold text-gray-800">{MOCK_DATA.stats.slides}</h3></div>
        <div className="p-2 bg-blue-50 text-blue-600 rounded-lg"><span className="material-symbols-outlined">co_present</span></div>
      </Card>
      <Card className="border-l-4 border-orange-500 flex items-center justify-between">
        <div><p className="text-gray-500 mb-1 text-sm font-bold">課題</p><h3 className="text-3xl font-bold text-gray-800">{MOCK_DATA.stats.assignments}</h3></div>
        <div className="p-2 bg-orange-50 text-orange-600 rounded-lg"><span className="material-symbols-outlined">assignment</span></div>
      </Card>
      <Card className="border-l-4 border-green-500 flex items-center justify-between">
        <div><p className="text-gray-500 mb-1 text-sm font-bold">学生</p><h3 className="text-3xl font-bold text-gray-800">{MOCK_DATA.stats.students}</h3></div>
        <div className="p-2 bg-green-50 text-green-600 rounded-lg"><span className="material-symbols-outlined">group</span></div>
      </Card>
    </div>

    <div className="grid lg:grid-cols-2 gap-6">
      <Card>
        <h3 className="font-bold text-lg mb-4 text-gray-800">概要</h3>
        <ul className="space-y-3">
            <li className="text-sm text-gray-600">- スライド: {MOCK_DATA.stats.slides}件</li>
            <li className="text-sm text-gray-600">- 課題: {MOCK_DATA.stats.assignments}件</li>
            <li className="text-sm text-gray-600">- メンバー: {MOCK_DATA.stats.students}人</li>
        </ul>
        <p className="text-sm text-gray-500 mt-4">システムの全体状況をひと目で把握できる。</p>
      </Card>
    </div>
  </div>
);

export default AdminDashboard;