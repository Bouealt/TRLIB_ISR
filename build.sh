#!/bin/bash

# 脚本用于构建TRLIB项目

# 确保脚本出错时停止执行
set -e

# 创建构建目录
mkdir -p build
cd build

# 运行CMake配置
echo "====== 配置 TRLIB 项目 ======"
cmake ..

# 编译 (兼容旧版本CMake的方式)
echo "====== 编译 TRLIB 项目 ======"
CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
MAJOR_VERSION=$(echo $CMAKE_VERSION | cut -d. -f1)
MINOR_VERSION=$(echo $CMAKE_VERSION | cut -d. -f2)

# 获取CPU核心数用于并行编译
CORES=$(nproc 2>/dev/null || echo 2)

# CMake 3.12+ 支持直接使用 -j 参数
if [ $MAJOR_VERSION -gt 3 ] || ([ $MAJOR_VERSION -eq 3 ] && [ $MINOR_VERSION -ge 12 ]); then
    cmake --build . --parallel $CORES
else
    # 对于老版本CMake，需要将并行参数传递给原生构建工具
    cmake --build . -- -j$CORES
fi

echo "====== 构建完成 ======"
echo "可执行文件位于: $(pwd)/bin/ISR"

# 返回到原目录
cd ..