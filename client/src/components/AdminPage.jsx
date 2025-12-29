import React, { useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from "./ui/card";
import { Button } from "./ui/button";

const AdminPage = ({
    username,
    onLogout,
    allUsers,
    onGetAllUsers,
    onDeleteUser,
}) => {
    useEffect(() => {
        // Load users immediately on mount
        onGetAllUsers();

        // Set up auto-refresh every 5 seconds
        const interval = setInterval(() => {
            onGetAllUsers();
        }, 5000);

        // Cleanup interval on unmount
        return () => clearInterval(interval);
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, []); // Empty array = run once on mount only

    const handleDelete = (userId, username) => {
        if (window.confirm(`Bạn có chắc muốn xóa tài khoản "${username}"?`)) {
            onDeleteUser(userId);
        }
    };

    return (
        <div className="min-h-screen bg-gradient-to-br from-indigo-50 via-purple-50 to-pink-50 p-8">
            <div className="max-w-7xl mx-auto">
                {/* Header */}
                <div className="flex justify-between items-center mb-8">
                    <div>
                        <h1 className="text-4xl font-bold text-gray-900 mb-2">
                            Admin Dashboard
                        </h1>
                        <p className="text-gray-600">
                            Chào mừng, <span className="font-semibold text-indigo-600">{username}</span>
                            <span className="ml-3 text-xs text-gray-500">• Tự động cập nhật mỗi 5 giây</span>
                        </p>
                    </div>
                    <Button
                        variant="outline"
                        onClick={onLogout}
                        className="bg-white hover:bg-gray-100"
                    >
                        Đăng xuất
                    </Button>
                </div>

                {/* User Management Card */}
                <Card className="shadow-2xl border-indigo-100">
                    <CardHeader className="bg-gradient-to-r from-indigo-500 to-purple-600 text-white">
                        <CardTitle className="text-2xl flex items-center gap-3">
                            <span>👥</span>
                            <span>Quản Lý Người Dùng</span>
                            <span className="text-sm font-normal opacity-90">({allUsers.length} người dùng)</span>
                        </CardTitle>
                    </CardHeader>
                    <CardContent className="p-0">
                        <div className="overflow-x-auto">
                            <table className="w-full">
                                <thead className="bg-gray-50 border-b-2 border-gray-200">
                                    <tr>
                                        <th className="px-6 py-4 text-left text-xs font-semibold text-gray-700 uppercase tracking-wider">
                                            ID
                                        </th>
                                        <th className="px-6 py-4 text-left text-xs font-semibold text-gray-700 uppercase tracking-wider">
                                            Tên người dùng
                                        </th>
                                        <th className="px-6 py-4 text-left text-xs font-semibold text-gray-700 uppercase tracking-wider">
                                            Tổng thắng
                                        </th>
                                        <th className="px-6 py-4 text-left text-xs font-semibold text-gray-700 uppercase tracking-wider">
                                            Tổng điểm
                                        </th>
                                        <th className="px-6 py-4 text-left text-xs font-semibold text-gray-700 uppercase tracking-wider">
                                            Trạng thái
                                        </th>
                                        <th className="px-6 py-4 text-right text-xs font-semibold text-gray-700 uppercase tracking-wider">
                                            Hành động
                                        </th>
                                    </tr>
                                </thead>
                                <tbody className="bg-white divide-y divide-gray-200">
                                    {allUsers.length === 0 ? (
                                        <tr>
                                            <td colSpan="6" className="px-6 py-12 text-center text-gray-500">
                                                Không có người dùng nào
                                            </td>
                                        </tr>
                                    ) : (
                                        allUsers.map((user) => (
                                            <tr
                                                key={user.id}
                                                className="hover:bg-gray-50 transition-colors"
                                            >
                                                <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-600">
                                                    #{user.id}
                                                </td>
                                                <td className="px-6 py-4 whitespace-nowrap">
                                                    <div className="flex items-center gap-2">
                                                        <span className="font-medium text-gray-900">{user.username}</span>
                                                    </div>
                                                </td>
                                                <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-600">
                                                    {user.total_win}
                                                </td>
                                                <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-600">
                                                    {user.total_score}
                                                </td>
                                                <td className="px-6 py-4 whitespace-nowrap">
                                                    <span
                                                        className={`inline-flex items-center px-3 py-1 rounded-full text-xs font-semibold ${user.is_online
                                                            ? 'bg-green-100 text-green-800'
                                                            : 'bg-gray-100 text-gray-800'
                                                            }`}
                                                    >
                                                        <span className={`w-2 h-2 rounded-full mr-2 ${user.is_online ? 'bg-green-500' : 'bg-gray-400'
                                                            }`}></span>
                                                        {user.is_online ? 'Online' : 'Offline'}
                                                    </span>
                                                </td>
                                                <td className="px-6 py-4 whitespace-nowrap text-right text-sm font-medium">
                                                    <Button
                                                        size="sm"
                                                        variant="destructive"
                                                        onClick={() => handleDelete(user.id, user.username)}
                                                        className="bg-red-50 hover:bg-red-100 text-red-600 border-red-200"
                                                    >
                                                        Xóa
                                                    </Button>
                                                </td>
                                            </tr>
                                        ))
                                    )}
                                </tbody>
                            </table>
                        </div>
                    </CardContent>
                </Card>
            </div>
        </div>
    );
};

export default AdminPage;
