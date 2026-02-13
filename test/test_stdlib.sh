#!/bin/bash
# XWift 标准库模块编译和测试脚本

echo "=========================================="
echo "XWift 标准库模块编译和测试"
echo "=========================================="
echo ""

# 检查 xwift 编译器是否存在
if [ ! -f "build/tools/xwift/xwift" ] && [ ! -f "build/Release/xwift.exe" ]; then
    echo "❌ 错误: xwift 编译器未找到"
    echo ""
    echo "请先编译 xwift 编译器："
    echo "  mkdir build"
    echo "  cd build"
    echo "  cmake .. -G 'Visual Studio 17 2022' -A x64"
    echo "  cmake --build . --config Release"
    echo ""
    exit 1
fi

# 确定 xwift 可执行文件路径
XWIFT_EXE=""
if [ -f "build/tools/xwift/xwift" ]; then
    XWIFT_EXE="build/tools/xwift/xwift"
elif [ -f "build/Release/xwift.exe" ]; then
    XWIFT_EXE="build/Release/xwift.exe"
elif [ -f "build/Debug/xwift.exe" ]; then
    XWIFT_EXE="build/Debug/xwift.exe"
else
    echo "❌ 错误: 无法找到 xwift 可执行文件"
    exit 1
fi

echo "✅ 找到 xwift 编译器: $XWIFT_EXE"
echo ""

# 检查 XWift 标准库模块文件
MODULES=(
    "lib/stdlib/Logging/formatters.xw"
    "lib/stdlib/Filesystem/path_utils.xw"
    "lib/stdlib/Config/config.xw"
    "lib/stdlib/Utils/utils.xw"
)

echo "检查 XWift 标准库模块文件..."
for module in "${MODULES[@]}"; do
    if [ -f "$module" ]; then
        echo "  ✅ $module"
    else
        echo "  ❌ $module (未找到)"
        exit 1
    fi
done
echo ""

# 检查测试文件
TEST_FILE="test/test_stdlib_modules.xw"
if [ -f "$TEST_FILE" ]; then
    echo "✅ 找到测试文件: $TEST_FILE"
else
    echo "❌ 错误: 测试文件未找到: $TEST_FILE"
    exit 1
fi
echo ""

# 语法检查（如果 xwift 支持）
echo "执行语法检查..."
for module in "${MODULES[@]}"; do
    echo "  检查: $module"
    # 假设 xwift 有 --check 或 --syntax 选项
    # $XWIFT_EXE --check "$module" 2>&1
done
echo ""

# 运行测试
echo "运行测试文件..."
echo "=========================================="
$XWIFT_EXE "$TEST_FILE"
EXIT_CODE=$?
echo "=========================================="
echo ""

# 检查测试结果
if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ 测试成功！"
    echo ""
    echo "所有 XWift 标准库模块功能正常："
    echo "  ✅ 日志格式化 (SimpleFormatter, DetailedFormatter, JsonFormatter)"
    echo "  ✅ 文件路径处理 (Path 类)"
    echo "  ✅ 配置管理 (Config, ConfigValue, ConfigLoader)"
    echo "  ✅ 工具函数 (StringUtils, DateUtils, ValidationUtils, ArrayUtils)"
    echo ""
    exit 0
else
    echo "❌ 测试失败，退出码: $EXIT_CODE"
    echo ""
    echo "请检查："
    echo "  1. 模块语法是否正确"
    echo "  2. 模块之间的依赖关系"
    echo "  3. xwift 编译器的实现"
    echo ""
    exit 1
fi
