# 设置执行策略（防止权限问题）
if ((Get-ExecutionPolicy) -ne 'RemoteSigned') {
    Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process -Force
}

# 1. 路径定义
$CLION_BIN = "C:\Program Files\JetBrains\CLion 2025.3.1.1\bin"
$CMAKE_EXE = "$CLION_BIN\cmake\win\x64\bin\cmake.exe"
$NINJA_EXE = "$CLION_BIN\ninja\win\x64\ninja.exe"
# 修正 vcpkg 路径
$VCPKG_TOOLCHAIN = "C:\Users\churu\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake"

$SCRIPT_PATH = Split-Path -Parent $MyInvocation.MyCommand.Definition
$PROJECT_ROOT = Split-Path -Parent $SCRIPT_PATH

$BUILD_DIR = "$PROJECT_ROOT\cmake-build-release"
$DIST_DIR = "$PROJECT_ROOT\dist"

Write-Host ">>> Project Root: $PROJECT_ROOT" -ForegroundColor Magenta
Write-Host ">>> Cleaning old directories..." -ForegroundColor Cyan
if (Test-Path $DIST_DIR) { Remove-Item -Recurse -Force $DIST_DIR }
if (Test-Path $BUILD_DIR) { Remove-Item -Recurse -Force $BUILD_DIR }

# 2. 配置 CMake (仅执行一次，带完整参数)
Write-Host ">>> Configuring CMake with Ninja & vcpkg (Force x64)..." -ForegroundColor Cyan

& $CMAKE_EXE -S $PROJECT_ROOT -B $BUILD_DIR `
    -G "Ninja" `
    -DCMAKE_MAKE_PROGRAM="$NINJA_EXE" `
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" `
    -DVCPKG_TARGET_TRIPLET="x64-windows" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_C_COMPILER="cl.exe" `
    -DCMAKE_CXX_COMPILER="cl.exe"

# 3. 编译项目
Write-Host ">>> Building Project (Release)..." -ForegroundColor Cyan
& $CMAKE_EXE --build $BUILD_DIR --config Release -j 30

# 4. 安装并自动搜集 DLL
Write-Host ">>> Installing and Collecting DLLs..." -ForegroundColor Cyan
& $CMAKE_EXE --install $BUILD_DIR --prefix $DIST_DIR --config Release

Write-Host "`n>>> Done! Portable app located at: $DIST_DIR" -ForegroundColor Green
Write-Host ">>> Check if sqlite3.dll and SDL3.dll are in the dist folder." -ForegroundColor Yellow