[CmdletBinding()]
param(
    [string]$BuildDir = "build",
    [string]$DistDir = "dist",
    [string]$QtBin = ""
)

$ErrorActionPreference = "Stop"

if ($QtBin) {
    $env:PATH = "$QtBin;$env:PATH"
}

$cache = Join-Path $BuildDir "CMakeCache.txt"
if (-not (Test-Path $cache)) {
    throw "找不到 $cache，请先用 CMake 配置并构建项目。"
}

if (Test-Path $DistDir) {
    Remove-Item -LiteralPath $DistDir -Recurse -Force
}
New-Item -ItemType Directory -Path $DistDir | Out-Null

cmake --install $BuildDir --prefix $DistDir --config Release
if ($LASTEXITCODE -ne 0) {
    throw "cmake --install 失败。"
}

$exe = Join-Path $DistDir "task_notebook_gui.exe"
if (-not (Test-Path $exe)) {
    throw "安装目录中缺少 task_notebook_gui.exe。"
}

windeployqt --release --no-translations --compiler-runtime $exe
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt 失败，请确认 Qt 的 bin 目录在 PATH 中。"
}

$zip = "task_notebook_gui-windows.zip"
Compress-Archive -Path (Join-Path $DistDir "*") -DestinationPath $zip -Force
Write-Host "已生成 $zip"
Write-Host "目标机器无需安装 Qt 即可运行。"
