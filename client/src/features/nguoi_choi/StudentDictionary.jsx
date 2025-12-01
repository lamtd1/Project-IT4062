import React, { useState } from 'react';
import Card from '../../components/common/Card.jsx';
import Button from '../../components/common/Button.jsx';
import { MOCK_DATA } from '../../lib/mockData.js';

const StudentDictionary = () => {
  const [term, setTerm] = useState('');
  const [result, setResult] = useState(null);

  const handleSearch = (e) => {
    e.preventDefault();
    if(term) setResult({ word: term, pronunciation: "/term/", type: "名詞", meanings: ["ここに意味が表示されます"], examples: [{ ja: "例文", vi: "Ví dụ" }] });
  };

  return (
    <div className="max-w-3xl mx-auto space-y-6">
      <div className="text-center py-6">
        <h2 className="text-2xl font-bold text-gray-900">日本語 ベトナム語辞書</h2>
      </div>
      <Card className="p-2 shadow-lg shadow-blue-100 border-blue-100">
        <form onSubmit={handleSearch} className="flex gap-2">
          <div className="relative flex-1 flex items-center px-4 border border-gray-300 rounded-lg bg-white">
            <span className="material-symbols-outlined text-gray-400">search</span>
            <input type="text" className="w-full pl-2 py-3 outline-none text-gray-700" placeholder="日本語またはベトナム語で入力してください" value={term} onChange={e=>setTerm(e.target.value)} />
          </div>
        </form>
      </Card>
      {result ? (
        <Card>
           <h3 className="text-3xl font-bold text-blue-600 mb-2">{result.word}</h3>
           <p>語彙を調べたところ</p>
        </Card>
      ) : (
        <div className="mt-8">
          <div className="flex flex-wrap gap-2">{MOCK_DATA.dictionaryHistory.map(item => <button key={item.id} onClick={()=>setTerm(item.word)} className="px-3 py-1.5 bg-white border border-gray-200 rounded-full text-sm text-gray-600 hover:border-blue-500 transition-all cursor-pointer">{item.word}</button>)}</div>
        </div>
      )}
    </div>
  );
};

export default StudentDictionary;