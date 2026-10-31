[CmdletBinding()]
param(
    [string]$BuildDir = "build",
    [string]$DistDir = "dist",
    [string]$QtBin = ""
)

$ErrorActionPreference = "Stop"

if (-not $QtBin) {
    $windeployCommand = Get-Command windeployqt.exe -ErrorAction SilentlyContinue
    if ($windeployCommand) {
        $QtBin = Split-Path $windeployCommand.Source
    }
}

if ($QtBin) {
    $env:PATH = "$QtBin;$env:PATH"
}

function Copy-MingwRuntimeDependencies {
    param(
        [string]$Directory,
        [string]$QtBinDirectory
    )

    $objdump = Join-Path $QtBinDirectory "objdump.exe"
    if (-not (Test-Path $objdump)) {
        Write-Host "未找到 objdump，跳过 MinGW 运行库依赖复制（MSVC 构建通常不需要）。"
        return
    }

    $queue = [System.Collections.Generic.Queue[string]]::new()
    $files = @{}
    Get-ChildItem -Path $Directory -Recurse -File -Include *.exe, *.dll | ForEach-Object {
        $files[$_.Name] = $_.FullName
        $queue.Enqueue($_.FullName)
    }

    $seen = @{}
    $copied = 0
    while ($queue.Count -gt 0) {
        $file = $queue.Dequeue()
        if ($seen.ContainsKey($file)) {
            continue
        }
        $seen[$file] = $true

        $dependencies = & $objdump -p $file 2>$null |
            Select-String 'DLL Name:\s*(.+)$' |
            ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() }

        foreach ($dependency in $dependencies) {
            if ($files.ContainsKey($dependency)) {
                $queue.Enqueue($files[$dependency])
                continue
            }

            $candidate = Join-Path $QtBinDirectory $dependency
            if (Test-Path $candidate) {
                $target = Join-Path $Directory $dependency
                Copy-Item -LiteralPath $candidate -Destination $target
                $files[$dependency] = $target
                $queue.Enqueue($target)
                ++$copied
            }
        }
    }

    Write-Host "已补充 $copied 个 MinGW/第三方依赖 DLL。"
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

if ($QtBin) {
    Copy-MingwRuntimeDependencies -Directory $DistDir -QtBinDirectory $QtBin
}

$zip = "task_notebook_gui-windows.zip"
Compress-Archive -Path (Join-Path $DistDir "*") -DestinationPath $zip -Force
Write-Host "已生成 $zip"
Write-Host "目标机器无需安装 Qt 即可运行。"
