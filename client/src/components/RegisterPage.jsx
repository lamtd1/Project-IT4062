import React from 'react';
import { Link } from 'react-router-dom';

const RegisterPage = ({ username, setUsername, password, setPassword, onSubmit, status }) => (
    <div className="w-full max-w-sm mx-auto shadow-2xl bg-white border-2 border-gray-200 rounded-xl">
        <div className="space-y-1 p-6">
            <h2 className="text-2xl font-bold text-center text-black">Tạo Tài Khoản</h2>
            <p className="text-center text-gray-500">Tham gia đấu trường trí tuệ</p>
        </div>
        <div className="space-y-4 px-6 pb-6">
            {status.msg && (
                <div className={`p-3 rounded-lg text-sm text-center font-medium ${status.type === 'success' ? 'bg-emerald-100 text-emerald-700' : 'bg-red-100 text-red-700'
                    }`}>
                    {status.msg}
                </div>
            )}
            <div className="space-y-2">
                <label className="text-sm font-medium">Tên tài khoản</label>
                <input
                    type="text"
                    placeholder="Chọn username"
                    value={username}
                    onChange={(e) => setUsername(e.target.value)}
                    className="w-full px-4 py-2 border-2 border-gray-300 rounded-lg focus:outline-none focus:border-black transition"
                />
            </div>
            <div className="space-y-2">
                <label className="text-sm font-medium">Mật khẩu</label>
                <input
                    type="password"
                    placeholder="Tạo password"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    className="w-full px-4 py-2 border-2 border-gray-300 rounded-lg focus:outline-none focus:border-black transition"
                />
            </div>
            <button
                onClick={() => onSubmit(0x02)}
                className="w-full py-3 bg-black text-white font-bold rounded-lg hover:bg-gray-800 transition"
            >
                Đăng ký
            </button>
        </div>
        <div className="flex justify-center p-6 pt-0">
            <p className="text-sm text-gray-500">
                Đã có tài khoản?{" "}
                <Link className="text-black font-semibold hover:underline" to="/">
                    Đăng nhập
                </Link>
            </p>
        </div>
    </div>
);

export default RegisterPage;
