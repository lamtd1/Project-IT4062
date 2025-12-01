import React from 'react';
import Card from '../../components/common/Card.jsx';
import Button from '../../components/common/Button.jsx';

const AdminSlideUpload = () => (
  <div className="max-w-4xl mx-auto">
     <div className="mb-6">
        <h2 className="text-2xl font-bold text-gray-800">スライドをアップロード</h2>
        <p className="text-gray-500 text-sm">検索と用語のインデックス作成のために自動テキスト抽出機能を備えたスライドをアップロードします。</p>
      </div>
    <Card>
      <div className="border-2 border-dashed border-gray-300 rounded-xl p-10 text-center hover:bg-gray-50 transition-colors cursor-pointer">
        <div className="w-16 h-16 bg-blue-100 text-blue-600 rounded-full flex items-center justify-center mx-auto mb-4">
          <span className="material-symbols-outlined text-3xl">upload_file</span>
        </div>
        <h3 className="text-lg font-medium text-gray-900">ファイルのアップロード</h3>
        <p className="text-sm text-gray-500 mt-1">対応形式: PDF (最大100MB)</p>
        <Button variant="primary" className="mt-4 mx-auto">ファイルを選択</Button>
      </div>
      <div className="mt-6 space-y-4">
        <div>
          <label className="text-sm font-medium text-gray-700 block mb-1.5">タイトル</label>
          <input className="w-full px-4 py-2 border border-gray-300 rounded-lg focus:border-blue-500 outline-none" />
        </div>
        <div>
          <label className="text-sm font-medium text-gray-700 block mb-1.5">タグ</label>
          <input className="w-full px-4 py-2 border border-gray-300 rounded-lg focus:border-blue-500 outline-none" />
        </div>
        <div className="flex justify-end pt-2">
          <Button>アップロード</Button>
        </div>
      </div>
    </Card>
  </div>
);

export default AdminSlideUpload;