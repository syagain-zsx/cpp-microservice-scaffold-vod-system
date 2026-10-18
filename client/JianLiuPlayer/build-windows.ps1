# 笺流客户端（JianLiuPlayer）Windows 构建脚本
#
# 用法（PowerShell）:
#   .\build-windows.ps1 -QtPath "D:/Qt/6.7.3/mingw_64" [-BuildDir build] [-Jobs 8]
#
# 前提:
#   1. 已安装 Qt 6.7.x MinGW 64-bit（QtPath 指向 mingw_64 目录）；
#   2. libmpv-2.dll 已放到 mpv/dll/（见 README）；
#   3. MinGW 的 bin（Qt/Tools/mingw1310_64/bin）在 PATH 中，或通过 -MinGwPath 指定。

param(
    [Parameter(Mandatory = $true)][string]$QtPath,
    [string]$BuildDir = "build",
    [string]$MinGwPath = "",
    [int]$Jobs = 8
)

$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $here

if ($MinGwPath -ne "") {
    $env:PATH = "$MinGwPath;$env:PATH"
}

if (-not (Test-Path "$QtPath\bin\qmake.exe")) {
    Write-Error "QtPath 不正确（找不到 $QtPath\bin\qmake.exe）"
}

$mpvDll = Join-Path $here "mpv/dll/libmpv-2.dll"
if (-not (Test-Path $mpvDll)) {
    Write-Warning "缺少 libmpv-2.dll：$mpvDll（构建会失败，请先下载放到 mpv/dll/）"
}

Write-Host "[build] Qt: $QtPath"
Write-Host "[build] BuildDir: $BuildDir"

cmake -S . -B $BuildDir -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QtPath"
if ($LASTEXITCODE -ne 0) { throw "cmake 配置失败" }

cmake --build $BuildDir -j $Jobs
if ($LASTEXITCODE -ne 0) { throw "编译失败" }

Write-Host ""
Write-Host "[build] 完成：$BuildDir\JianLiuPlayer.exe"
Write-Host "[build] 运行前："
Write-Host "  1) 用 windeployqt 收集 Qt DLL： windeployqt $BuildDir\JianLiuPlayer.exe"
Write-Host "  2) 把 libmpv-2.dll 复制到 exe 同目录"
Write-Host "  3) 在「当前工作目录」放置 config.ini（serverIp/serverPort）"
