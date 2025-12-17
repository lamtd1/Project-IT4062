import React from 'react';
import { Link } from 'react-router-dom';

const LoginPage = ({ username, setUsername, password, setPassword, onSubmit, status }) => (
    <div className="bg-white p-8 rounded-2xl shadow-lg w-full max-w-sm">
        <h1 className="text-2xl font-bold text-center text-gray-900 mb-2">Đăng Nhập</h1>
        <p className="text-sm text-gray-500 text-center mb-6">Chào mừng bạn quay trở lại</p>

        {status.msg && (
            <div className={`mb-4 p-3 rounded-lg text-sm text-center ${status.type === 'success' ? 'bg-green-100 text-green-700 border border-green-300' : 'bg-red-100 text-red-700 border border-red-300'}`}>
                {status.msg}
            </div>
        )}

        <div className="mb-4">
            <label className="block text-sm font-medium text-gray-700 mb-1">Tên tài khoản</label>
            <input
                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-400 outline-none transition"
                type="text"
                placeholder="Nhập username"
                value={username}
                onChange={(e) => setUsername(e.target.value)}
            />
        </div>

        <div className="mb-6">
            <label className="block text-sm font-medium text-gray-700 mb-1">Mật khẩu</label>
            <input
                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-400 outline-none transition"
                type="password"
                placeholder="Nhập password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
            />
        </div>

        <button
            className="w-full bg-blue-600 hover:bg-blue-700 text-white py-2.5 rounded-lg font-semibold transition shadow-md"
            onClick={() => onSubmit(0x01)}
        >
            Đăng nhập
        </button>

        <p className="text-center text-sm text-gray-600 mt-4">
            Chưa có tài khoản?{" "}
            <Link className="text-blue-600 font-medium hover:underline" to="/register">
                Đăng ký ngay
            </Link>
        </p>
    </div>
);

export default LoginPage;
