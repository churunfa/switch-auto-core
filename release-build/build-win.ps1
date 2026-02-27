# x64 Native Tools Command Prompt
# cd C:\Users\churu\Desktop\code\botw-auto\switch-auto-core\release-build
# powershell .\build-win.ps1
# 设置执行策略（防止权限问题）
if ((Get-ExecutionPolicy) -ne 'RemoteSigned') {
    Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process -Force
}

# 1. 路径定义
$CLION_BIN = "C:\Program Files\JetBrains\CLion 2025.3.1.1\bin"
$CMAKE_EXE = "$CLION_BIN\cmake\win\x64\bin\cmake.exe"
$NINJA_EXE = "$CLION_BIN\ninja\win\x64\ninja.exe"
$VCPKG_ROOT = "C:\Users\churu\.vcpkg-clion\vcpkg"
$VCPKG_TOOLCHAIN = "$VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"

$SCRIPT_PATH = Split-Path -Parent $MyInvocation.MyCommand.Definition
$PROJECT_ROOT = Split-Path -Parent $SCRIPT_PATH

$BUILD_DIR = "$PROJECT_ROOT\cmake-build-release"
$DIST_DIR = "$PROJECT_ROOT\dist"

Write-Host ">>> Project Root: $PROJECT_ROOT" -ForegroundColor Magenta
Write-Host ">>> Cleaning old directories..." -ForegroundColor Cyan
if (Test-Path $DIST_DIR) { Remove-Item -Recurse -Force $DIST_DIR }
if (Test-Path $BUILD_DIR) { Remove-Item -Recurse -Force $BUILD_DIR }

# 2. 配置 CMake
Write-Host ">>> Configuring CMake with Ninja & vcpkg (Force x64)..." -ForegroundColor Cyan
$env:VSCMD_ARG_HOST_ARCH="x64"
$env:VSCMD_ARG_TGT_ARCH="x64"

& $CMAKE_EXE -S $PROJECT_ROOT -B $BUILD_DIR `
    -G "Ninja" `
    -DCMAKE_MAKE_PROGRAM="$NINJA_EXE" `
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" `
    -DVCPKG_TARGET_TRIPLET="x64-windows" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL" `
    -DCMAKE_C_COMPILER="cl.exe" `
    -DCMAKE_CXX_COMPILER="cl.exe"

# 3. 编译项目
Write-Host ">>> Building Project (Release)..." -ForegroundColor Cyan
& $CMAKE_EXE --build $BUILD_DIR --config Release -j 30

# 4. 准备发布目录
Write-Host ">>> Killing existing processes to avoid file lock..." -ForegroundColor Cyan
Stop-Process -Name "switch_auto_core" -ErrorAction SilentlyContinue

Write-Host ">>> Installing to dist directory..." -ForegroundColor Cyan
& $CMAKE_EXE --install $BUILD_DIR --prefix $DIST_DIR --config Release

# 5. 补丁：手动拷贝 EXE
$EXE_NAME = "switch_auto_core.exe"
$FOUND_EXE = Get-ChildItem -Path $BUILD_DIR -Filter $EXE_NAME -Recurse | Select-Object -First 1
if ($FOUND_EXE) {
    if (!(Test-Path $DIST_DIR)) { New-Item -ItemType Directory -Path $DIST_DIR }
    Copy-Item $FOUND_EXE.FullName -Destination $DIST_DIR -Force
    Write-Host ">>> Verified EXE in dist folder." -ForegroundColor Green
}

# 6. 核心：自动收集 vcpkg 依赖的 DLL
Write-Host ">>> Collecting DLL dependencies..." -ForegroundColor Cyan
$VCPKG_BIN = "$VCPKG_ROOT\installed\x64-windows\bin"
$DLL_LIST = @("sqlite3.dll", "SDL3.dll", "grpc++.dll", "libprotobuf.dll", "libcrypto-3-x64.dll", "libssl-3-x64.dll")

foreach ($DLL in $DLL_LIST) {
    if (Test-Path "$VCPKG_BIN\$DLL") {
        Copy-Item "$VCPKG_BIN\$DLL" -Destination $DIST_DIR -Force
    }
}

# ---------------------------------------------------------
# 7. 清理多余文件夹 (新加内容)
# ---------------------------------------------------------
Write-Host ">>> Cleaning up intermediate and unnecessary folders..." -ForegroundColor Cyan

# A. 删除 dist 里的开发头文件、静态库和配置 (打包不需要这些)
$DIR_TO_REMOVE_IN_DIST = @("include", "lib", "share")
foreach ($dir in $DIR_TO_REMOVE_IN_DIST) {
    $target = Join-Path $DIST_DIR $dir
    if (Test-Path $target) {
        Remove-Item -Recurse -Force $target
        Write-Host "    [-] Removed $target" -ForegroundColor Gray
    }
}

# B. 删除庞大的编译产物目录
if (Test-Path $BUILD_DIR) {
    Remove-Item -Recurse -Force $BUILD_DIR
    Write-Host "    [-] Removed $BUILD_DIR" -ForegroundColor Gray
}

# C. 额外：确保控制台使用 UTF-8 防止乱码
chcp 65001

Write-Host "`n>>> Done! Clean portable app located at: $DIST_DIR" -ForegroundColor Green
Write-Host ">>> Everything is ready. You can now zip the 'dist' folder." -ForegroundColor Yellow