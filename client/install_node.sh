#!/bin/bash

# Ensure curl is installed
sudo apt-get update
sudo apt-get install -y curl

# Install Node.js 20.x (LTS)
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs

# Verify installation
node -v
npm -v

echo "Node.js and npm installed successfully!"
echo "Now run 'npm install' to install dependencies."
