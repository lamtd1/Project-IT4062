import { useState } from 'react';

export default function Login({ onLogin, onRegister }) {
    const [username, setUsername] = useState('');
    const [password, setPassword] = useState('');
    const [isRegistering, setIsRegistering] = useState(false);

    const handleSubmit = (e) => {
        e.preventDefault();
        if (username.trim() && password.trim()) {
            if (isRegistering) {
                onRegister(username, password);
            } else {
                onLogin(username, password);
            }
        }
    };

    return (
        <div className="flex items-center justify-center min-h-screen bg-gradient-to-br from-blue-900 to-purple-900">
            <div className="bg-white/10 backdrop-blur-md p-8 rounded-xl shadow-2xl w-96 border border-white/20">
                <h1 className="text-3xl font-bold text-center mb-8 text-yellow-400 tracking-wider">
                    {isRegistering ? 'REGISTER' : 'LOGIN'}
                </h1>

                <form onSubmit={handleSubmit} className="space-y-6">
                    <div>
                        <label className="block text-sm font-medium text-gray-300 mb-2">Username</label>
                        <input
                            type="text"
                            value={username}
                            onChange={(e) => setUsername(e.target.value)}
                            className="w-full px-4 py-3 bg-gray-800/50 border border-gray-600 rounded-lg focus:ring-2 focus:ring-yellow-400 focus:outline-none text-white placeholder-gray-500 transition-all"
                            placeholder="Enter your name"
                        />
                    </div>

                    <div>
                        <label className="block text-sm font-medium text-gray-300 mb-2">Password</label>
                        <input
                            type="password"
                            value={password}
                            onChange={(e) => setPassword(e.target.value)}
                            className="w-full px-4 py-3 bg-gray-800/50 border border-gray-600 rounded-lg focus:ring-2 focus:ring-yellow-400 focus:outline-none text-white placeholder-gray-500 transition-all"
                            placeholder="Enter password"
                        />
                    </div>

                    <button
                        type="submit"
                        className="w-full py-3 bg-gradient-to-r from-yellow-500 to-yellow-600 text-black font-bold rounded-lg hover:from-yellow-400 hover:to-yellow-500 transform hover:scale-[1.02] transition-all shadow-lg"
                    >
                        {isRegistering ? 'SIGN UP' : 'LOGIN'}
                    </button>
                </form>

                <div className="mt-6 text-center">
                    <button
                        onClick={() => setIsRegistering(!isRegistering)}
                        className="text-sm text-gray-400 hover:text-white underline transition-colors"
                    >
                        {isRegistering ? 'Already have an account? Login' : "Don't have an account? Sign up"}
                    </button>
                </div>
            </div>
        </div>
    );
}
