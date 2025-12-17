import React from 'react';
import { Link } from 'react-router-dom';
import { Card, CardContent, CardHeader, CardTitle, CardDescription, CardFooter } from "./ui/card";
import { Input } from "./ui/input";
import { Button } from "./ui/button";

const RegisterPage = ({ username, setUsername, password, setPassword, onSubmit, status }) => (
    <Card className="w-full max-w-sm mx-auto shadow-2xl bg-white border-zinc-200">
        <CardHeader className="space-y-1">
            <CardTitle className="text-2xl font-bold text-center">Tạo Tài Khoản</CardTitle>
            <CardDescription className="text-center">Tham gia đấu trường trí tuệ</CardDescription>
        </CardHeader>
        <CardContent className="space-y-4">
            {status.msg && (
                <div className={`p-3 rounded-lg text-sm text-center font-medium ${status.type === 'success' ? 'bg-emerald-100 text-emerald-700' : 'bg-red-100 text-red-700'}`}>
                    {status.msg}
                </div>
            )}
            <div className="space-y-2">
                <label className="text-sm font-medium leading-none peer-disabled:cursor-not-allowed peer-disabled:opacity-70">Tên tài khoản</label>
                <Input
                    type="text"
                    placeholder="Chọn username"
                    value={username}
                    onChange={(e) => setUsername(e.target.value)}
                />
            </div>
            <div className="space-y-2">
                <label className="text-sm font-medium leading-none peer-disabled:cursor-not-allowed peer-disabled:opacity-70">Mật khẩu</label>
                <Input
                    type="password"
                    placeholder="Tạo password"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                />
            </div>
            <Button
                className="w-full font-bold"
                onClick={() => onSubmit(0x02)}
            >
                Đăng ký
            </Button>
        </CardContent>
        <CardFooter className="justify-center">
            <p className="text-sm text-zinc-500">
                Đã có tài khoản?{" "}
                <Link className="text-zinc-900 font-semibold hover:underline" to="/">
                    Đăng nhập
                </Link>
            </p>
        </CardFooter>
    </Card>
);

export default RegisterPage;
