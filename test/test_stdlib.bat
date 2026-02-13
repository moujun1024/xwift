@echo off
REM XWift 标准库模块编译和测试脚本 (Windows)

echo ==========================================
echo XWift 标准库模块编译和测试
echo ==========================================
echo.

REM 检查 xwift 编译器是否存在
if not exist "build\tools\xwift\xwift.exe" (
    if not exist "build\Release\xwift.exe" (
        if not exist "build\Debug\xwift.exe" (
            echo [错误] xwift 编译器未找到
            echo.
            echo 请先编译 xwift 编译器：
            echo   mkdir build
            echo   cd build
            echo   cmake .. -G "Visual Studio 17 2022" -A x64
            echo   cmake --build . --config Release
            echo.
            exit /b 1
        )
    )
)

REM 确定 xwift 可执行文件路径
set XWIFT_EXE=
if exist "build\tools\xwift\xwift.exe" (
    set XWIFT_EXE=build\tools\xwift\xwift.exe
) else if exist "build\Release\xwift.exe" (
    set XWIFT_EXE=build\Release\xwift.exe
) else if exist "build\Debug\xwift.exe" (
    set XWIFT_EXE=build\Debug\xwift.exe
) else (
    echo [错误] 无法找到 xwift 可执行文件
    exit /b 1
)

echo [成功] 找到 xwift 编译器: %XWIFT_EXE%
echo.

REM 检查 XWift 标准库模块文件
echo 检查 XWift 标准库模块文件...
if exist "lib\stdlib\Logging\formatters.xw" (
    echo   [OK] lib\stdlib\Logging\formatters.xw
) else (
    echo   [错误] lib\stdlib\Logging\formatters.xw (未找到)
    exit /b 1
)

if exist "lib\stdlib\Filesystem\path_utils.xw" (
    echo   [OK] lib\stdlib\Filesystem\path_utils.xw
) else (
    echo   [错误] lib\stdlib\Filesystem\path_utils.xw (未找到)
    exit /b 1
)

if exist "lib\stdlib\Config\config.xw" (
    echo   [OK] lib\stdlib\Config\config.xw
) else (
    echo   [错误] lib\stdlib\Config\config.xw (未找到)
    exit /b 1
)

if exist "lib\stdlib\Utils\utils.xw" (
    echo   [OK] lib\stdlib\Utils\utils.xw
) else (
    echo   [错误] lib\stdlib\Utils\utils.xw (未找到)
    exit /b 1
)

echo.

REM 检查测试文件
set TEST_FILE=test\test_stdlib_modules.xw
if exist "%TEST_FILE%" (
    echo [成功] 找到测试文件: %TEST_FILE%
) else (
    echo [错误] 测试文件未找到: %TEST_FILE%
    exit /b 1
)

echo.

REM 运行测试
echo 运行测试文件...
echo ==========================================
%XWIFT_EXE% "%TEST_FILE%"
set EXIT_CODE=%ERRORLEVEL%
echo ==========================================
echo.

REM 检查测试结果
if %EXIT_CODE% equ 0 (
    echo [成功] 测试成功！
    echo.
    echo 所有 XWift 标准库模块功能正常：
    echo   [OK] 日志格式化 ^(SimpleFormatter, DetailedFormatter, JsonFormatter^)
    echo   [OK] 文件路径处理 ^(Path 类^)
    echo   [OK] 配置管理 ^(Config, ConfigValue, ConfigLoader^)
    echo   [OK] 工具函数 ^(StringUtils, DateUtils, ValidationUtils, ArrayUtils^)
    echo.
    exit /b 0
) else (
    echo [错误] 测试失败，退出码: %EXIT_CODE%
    echo.
    echo 请检查：
    echo   1. 模块语法是否正确
    echo   2. 模块之间的依赖关系
    echo   3. xwift 编译器的实现
    echo.
    exit /b 1
)
