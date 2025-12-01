import React, { useState } from 'react';
import { MOCK_DATA } from '../../lib/mockData.js';

const LoginPage = ({ onLogin }) => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');

  const handleSubmit = (e) => {
    e.preventDefault();
    // Logic kiểm tra đăng nhập giả lập
    if (email.includes('admin')) {
      onLogin(MOCK_DATA.currentUser.admin);
    } else if (email.includes('student')) {
      onLogin(MOCK_DATA.currentUser.student);
    } else {
      setError('メールアドレスまたはパスワードが正しくありません');
    }
  };

  return (
    <div className="min-h-screen w-full flex items-center justify-center bg-white p-6 lg:gap-32 md:gap-10">
      {/* Left Side: Logo (Hidden on mobile, visible on medium screens and up) */}
      <div className="hidden md:block">
        <h1 className="text-5xl font-black tracking-widest text-black">KAKEHASHI</h1>
      </div>

      {/* Mobile Logo */}
      <div className="md:hidden absolute top-10 left-0 w-full text-center">
         <h1 className="text-4xl font-black tracking-widest text-black">KAKEHASHI</h1>
      </div>

      {/* Right Side: Login Form Card */}
      <div className="w-full max-w-[400px] bg-[#9AC5F4] rounded-xl shadow-xl p-8 pt-12 pb-6 relative">
        <div className="text-center mb-10">
          <h2 className="text-5xl font-bold text-black tracking-wide" style={{ fontFamily: '"Noto Sans JP", sans-serif' }}>ログイン</h2>
        </div>

        <form onSubmit={handleSubmit} className="space-y-6">
          <div>
            <label className="block text-sm font-bold text-white mb-2 pl-1">Eメール</label>
            <input
              type="text"
              className="w-full px-4 py-3 rounded-xl border-none outline-none focus:ring-2 focus:ring-blue-500 bg-white text-gray-800 shadow-inner"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
            />
          </div>
          <div>
            <label className="block text-sm font-bold text-white mb-2 pl-1">パスワード</label>
            <input
              type="password"
              className="w-full px-4 py-3 rounded-xl border-none outline-none focus:ring-2 focus:ring-blue-500 bg-white text-gray-800 shadow-inner"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
            />
          </div>

          {error && (
            <div className="text-red-600 text-sm font-bold bg-white/80 p-2 rounded text-center">
              {error}
            </div>
          )}

          <div className="flex justify-center mt-8 mb-4">
             <button
                type="submit"
                className="bg-[#1E50A2] text-white font-bold text-xl py-3 px-16 rounded-lg hover:bg-blue-900 transition-all shadow-md active:scale-95"
             >
               ログイン
             </button>
          </div>

          <div className="text-xs text-white space-y-2 mt-6 px-2">
            <div className="flex justify-between items-center">
               <span>パスワードをお忘れですか？</span>
               <a href="#" className="underline text-[#1E50A2] font-bold hover:text-blue-900">再設定</a>
            </div>
            <div className="flex justify-between items-center">
               <span>アカウントをお持ちないですか？</span>
               <a href="#" className="underline text-[#1E50A2] font-bold hover:text-blue-900">新規登録</a>
            </div>
          </div>
        </form>
      </div>
    </div>
  );
};

export default LoginPage;