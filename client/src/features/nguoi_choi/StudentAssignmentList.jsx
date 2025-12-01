import React from 'react';
import Card from '../../components/common/Card.jsx';
import Button from '../../components/common/Button.jsx';
import { MOCK_DATA } from '../../lib/mockData.js';

const StudentAssignmentList = () => {
  return (
    <div className="max-w-5xl mx-auto space-y-6">
      <div className="flex justify-between items-center">
        <div>
          <h2 className="text-2xl font-bold text-gray-900">課題一覧</h2>
          <p className="text-gray-500">期限内に課題を完了させてください。</p>
        </div>
        <div className="relative">
            <span className="material-symbols-outlined absolute left-3 top-1/2 -translate-y-1/2 text-gray-400">search</span>
            <input 
                type="text" 
                placeholder="課題を検索..." 
                className="pl-10 pr-10 py-2 border border-gray-300 rounded-lg focus:outline-none focus:border-blue-500"
            />
        </div>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {MOCK_DATA.studentAssignments.map((asm) => (
          <Card key={asm.id} className={`border-l-4 transition-shadow hover:shadow-md ${
            asm.status === 'done' ? 'border-green-500' : 
            asm.status === 'doing' ? 'border-yellow-500' : 'border-blue-500'
          }`}>
            <div className="flex justify-between items-start mb-4">
              <div>
                <h3 className="text-lg font-bold text-gray-800">{asm.title}</h3>
                <div className="flex items-center gap-4 text-sm text-gray-500 mt-1">
                  <span className="flex items-center gap-1">
                    <span className="material-symbols-outlined text-sm">schedule</span> 
                    残り時間: {asm.remainingTime}
                  </span>
                  <span className="flex items-center gap-1">
                    <span className="material-symbols-outlined text-sm">event</span> 
                    期限: {asm.deadline}
                  </span>
                </div>
              </div>
              <span className={`px-2 py-1 rounded text-xs font-bold uppercase ${
                 asm.status === 'done' ? 'bg-green-100 text-green-700' : 
                 asm.status === 'doing' ? 'bg-yellow-100 text-yellow-700' : 'bg-blue-100 text-blue-700'
              }`}>
                {asm.status === 'done' ? '完了' : asm.status === 'doing' ? '進行中' : '未着手'}
              </span>
            </div>

            <div className="mb-4">
              <div className="flex justify-between text-sm mb-1">
                <span className="text-gray-600">進捗率</span>
                <span className="font-bold">{asm.progress}%</span>
              </div>
              <div className="w-full bg-gray-100 h-2 rounded-full overflow-hidden">
                <div 
                  className={`h-full rounded-full ${
                    asm.status === 'done' ? 'bg-green-500' : asm.status === 'doing' ? 'bg-yellow-500' : 'bg-blue-500'
                  }`} 
                  style={{ width: `${asm.progress}%` }}
                ></div>
              </div>
            </div>

            <div className="flex justify-end pt-2 border-t border-gray-50">
                {asm.status === 'done' ? (
                     <Button variant="secondary" className="text-sm py-1.5">
                        <span className="material-symbols-outlined text-sm mr-1">check_circle</span> 
                        レビュー (確認)
                     </Button>
                ) : (
                    <Button className="text-sm py-1.5">
                        <span className="material-symbols-outlined text-sm mr-1">play_arrow</span> 
                        開始する
                    </Button>
                )}
            </div>
          </Card>
        ))}
      </div>
    </div>
  );
};

export default StudentAssignmentList;