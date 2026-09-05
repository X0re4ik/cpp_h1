#!/usr/bin/env bash;

# Установка
sudo apt -y update
sudo apt -y upgrade
sudo apt install -y \
    build-essential \
    cmake \
    clang 
sudo wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 21
rm -rf ./llvm.sh
sudo apt install -y \
    clang-format-21 \
    clang-tidy-21


sudo apt install software-properties-common -y
sudo add-apt-repository ppa:deadsnakes/ppa -y
sudo apt update
sudo apt install python3.10 python3.10-full python3.10-dev -y
python3.10 --version


# Скачивание
sudo git clone -b master https://github.com/X0re4ik/cpp_h1.git
cd cpp_h1


# Сборка
sudo rm -rf build
sudo cmake -S . -B build -DUSE_CLANG_FORMAT_CHECK=ON -DUSE_CLANG_TIDY=ON
sudo cmake --build build --target install
