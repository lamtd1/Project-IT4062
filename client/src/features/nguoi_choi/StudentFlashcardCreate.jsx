import React from 'react';
import Card from '../../components/common/Card.jsx';
import Button from '../../components/common/Button.jsx';
import { MOCK_DATA } from '../../lib/mockData.js';

const StudentFlashcardCreate = () => {
  return (
    <div className="space-y-6">
      <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
        <div>
          <h2 className="text-2xl font-bold text-gray-900">フラッシュカード作成</h2>
          <p className="text-gray-500">単語を復習するためのカードセットを作成・管理します。</p>
        </div>
        <Button className="shrink-0">
            <span className="material-symbols-outlined mr-1">add</span> 
            Flashcardを追加
        </Button>
      </div>

      <div className="bg-white p-4 rounded-xl shadow-sm border border-gray-100 flex gap-4">
         <div className="relative flex-1">
            <span className="material-symbols-outlined absolute left-3 top-1/2 -translate-y-1/2 text-gray-400">search</span>
            <input 
                type="text" 
                placeholder="フラッシュカードを検索..." 
                className="w-full pl-10 pr-4 py-2 border border-gray-200 rounded-lg outline-none focus:border-blue-500"
            />
         </div>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        <div className="border-2 border-dashed border-gray-300 rounded-xl p-8 flex flex-col items-center justify-center text-gray-400 hover:bg-gray-50 hover:border-blue-400 hover:text-blue-500 transition-all cursor-pointer h-48">
            <span className="material-symbols-outlined text-4xl mb-2">add</span>
            <span className="font-medium">新規セット作成</span>
        </div>

        {MOCK_DATA.flashcards.map((set) => (
            <Card key={set.id} className="hover:shadow-md transition-all cursor-pointer group flex flex-col h-48 justify-between relative overflow-hidden">
                <div className="absolute top-0 right-0 p-4 opacity-10 group-hover:opacity-20 transition-opacity">
                    <span className="material-symbols-outlined text-6xl text-gray-500">style</span>
                </div>
                <div>
                    <h3 className="text-xl font-bold text-gray-800 mb-1 group-hover:text-blue-600 transition-colors">{set.title}</h3>
                    <p className="text-sm text-gray-500">{set.count} 単語</p>
                </div>
                
                <div className="space-y-2 relative z-10">
                    <div className="flex justify-between text-xs text-gray-600">
                        <span>習得率</span>
                        <span>{set.progress}%</span>
                    </div>
                    <div className="w-full bg-gray-100 h-1.5 rounded-full overflow-hidden">
                        <div className="bg-blue-500 h-full" style={{ width: `${set.progress}%` }}></div>
                    </div>
                </div>
            </Card>
        ))}
      </div>
    </div>
  );
};

export default StudentFlashcardCreate;