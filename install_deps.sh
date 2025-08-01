#!/bin/bash
set -e

echo "[+] Detecting package manager..."

if command -v apt >/dev/null; then
    PM="apt"
elif command -v dnf >/dev/null; then
    PM="dnf"
elif command -v pacman >/dev/null; then
    PM="pacman"
elif command -v zypper >/dev/null; then
    PM="zypper"
else
    echo "[-] Unsupported package manager. Please install GMP development files manually."
    exit 1
fi

echo "[+] Installing dependencies using $PM..."

case "$PM" in
    apt)
        sudo apt update
        sudo apt install -y build-essential libgmp-dev
        ;;
    dnf)
        sudo dnf install -y gcc gmp-devel make
        ;;
    pacman)
        sudo pacman -Sy --noconfirm base-devel gmp
        ;;
    zypper)
        sudo zypper install -y gcc make gmp-devel
        ;;
esac

echo "[+] Dependencies installed successfully."
