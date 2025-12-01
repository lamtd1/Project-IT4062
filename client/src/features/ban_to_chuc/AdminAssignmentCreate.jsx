import React from 'react';
import Card from '../../components/common/Card.jsx';
import Button from '../../components/common/Button.jsx';

const AdminAssignmentCreate = () => {
  return (
    <div className="max-w-4xl mx-auto">
      <div className="mb-6">
        <h2 className="text-2xl font-bold text-gray-800">課題作成</h2>
        <p className="text-gray-500 text-sm">全体の課題を作成して登録する</p>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
        <div className="md:col-span-2 space-y-6">
          <Card>
            <h3 className="font-bold text-lg mb-4 pb-2 border-b">課題の詳細</h3>
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-1">タイトル</label>
                <input type="text" className="w-full px-4 py-2 border border-gray-300 rounded-lg outline-none focus:border-blue-500" />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-1">説明</label>
                <textarea rows="4" className="w-full px-4 py-2 border border-gray-300 rounded-lg outline-none focus:border-blue-500"></textarea>
              </div>
              <div className="grid grid-cols-2 gap-4">
                <div>
                  <label className="block text-sm font-medium text-gray-700 mb-1">開始日</label>
                  <input type="date" className="w-full px-4 py-2 border border-gray-300 rounded-lg" />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 mb-1">終了日</label>
                  <input type="date" className="w-full px-4 py-2 border border-gray-300 rounded-lg" />
                </div>
              </div>
            </div>
          </Card>
          <Card>
             <div className="flex justify-between items-center mb-2">
                <h3 className="font-bold text-lg">質問</h3>
                <Button variant="secondary" className="text-sm">+ 質問を追加</Button>
             </div>
             <p className="text-center text-gray-500 py-4 text-sm">「質問を追加」をクリックして開始してください</p>
          </Card>
        </div>

        <div className="space-y-6">
          <Card>
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-1">合計ポイント</label>
                <input type="number" className="w-full px-4 py-2 border border-gray-300 rounded-lg" defaultValue="100" />
              </div>
              <Button className="w-full mt-4">保存</Button>
            </div>
          </Card>
        </div>
      </div>
    </div>
  );
};

export default AdminAssignmentCreate;