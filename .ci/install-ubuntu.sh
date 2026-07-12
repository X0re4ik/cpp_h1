
# Установка
apt update
apt upgrade
apt install \
    build-essential \
    cmake \
    clang 
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 21
rm -rf ./llvm.sh
sudo apt install -y \
    clang-format-21 \
    clang-tidy-21



# Скачивание
git clone 


# Сборка
rm -rf build
cmake -S . -B build -DUSE_CLANG_FORMAT_CHECK=ON -DUSE_CLANG_TIDY=ON
cmake --build build

# Проверка
chmod -x ./tests/run_tests.sh
./tests/run_tests.sh
