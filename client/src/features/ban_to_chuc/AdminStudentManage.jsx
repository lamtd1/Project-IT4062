import React from 'react';
import Button from '../../components/common/Button.jsx';
import { MOCK_DATA } from '../../lib/mockData.js';

const AdminStudentManage = () => (
  <div className="space-y-6">
    <div className="flex flex-col sm:flex-row justify-between gap-4">
      <div>
        <h2 className="text-2xl font-bold text-gray-900">学生管理</h2>
        <p className="text-gray-500 text-sm">学生の進捗状況を追跡する。</p>
      </div>
      <Button>+ 学生を追加</Button>
    </div>

    <div className="bg-white rounded-xl shadow-sm border border-gray-200 overflow-hidden">
      <div className="p-4 border-b border-gray-200 bg-gray-50 flex items-center gap-2">
         <span className="material-symbols-outlined text-gray-400">search</span>
         <input type="text" placeholder="学生名またはメール..." className="bg-transparent outline-none text-sm flex-1"/>
      </div>
      <table className="w-full text-left">
        <thead className="bg-gray-50 text-gray-600 text-sm uppercase font-semibold border-b border-gray-200">
          <tr>
            <th className="px-6 py-4">学生情報</th>
            <th className="px-6 py-4">進捗率</th>
            <th className="px-6 py-4">ステータス</th>
            <th className="px-6 py-4 text-right">操作</th>
          </tr>
        </thead>
        <tbody className="divide-y divide-gray-100">
          {MOCK_DATA.students.map((student) => (
            <tr key={student.id} className="hover:bg-gray-50">
              <td className="px-6 py-4 flex items-center gap-3">
                <div className="w-10 h-10 rounded-full bg-blue-100 text-blue-600 flex items-center justify-center font-bold text-sm">
                        {student.code}
                </div>
                <div><div className="font-medium text-gray-900">{student.name}</div><div className="text-xs text-gray-500">{student.email}</div></div>
              </td>
              <td className="px-6 py-4">
                <div className="flex items-center gap-2">
                  <div className="w-24 h-2 bg-gray-100 rounded-full overflow-hidden"><div style={{ width: `${student.progress}%` }} className="h-full bg-blue-500"></div></div>
                  <span className="text-xs font-medium">{student.progress}%</span>
                </div>
              </td>
              <td className="px-6 py-4">
                <span className={`px-2.5 py-1 rounded-full text-xs font-medium ${student.status === 'online' ? 'bg-green-100 text-green-700' : 'bg-gray-100 text-gray-600'}`}>
                  {student.status === 'online' ? '● オンライン' : '○ オフライン'}
                </span>
              </td>
              <td className="px-6 py-4 text-right"><Button variant="ghost" className="text-sm">詳細</Button></td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  </div>
);

export default AdminStudentManage;