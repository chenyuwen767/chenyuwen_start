$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $projectRoot "build"
$program = Join-Path $buildDirectory "notebook-test.exe"
$dataFile = Join-Path $buildDirectory "tasks.txt"

New-Item -ItemType Directory -Force -Path $buildDirectory | Out-Null
Remove-Item -ErrorAction SilentlyContinue $dataFile

g++ -std=c++17 -Wall -Wextra -pedantic (Join-Path $projectRoot "src/main.cpp") -o $program

Push-Location $buildDirectory
try {
    $input = "1`nTwo Sum`nHash Table`nEasy`n4`n1`n0`n"
    $output = $input | & $program
    $outputText = $output | Out-String
}
finally {
    Pop-Location
}

if ($outputText -notmatch "Two Sum" -or
    -not (Test-Path $dataFile)) {
    throw "Test failed: add, complete, or save flow was not as expected."
}

Write-Host "Tests passed: add, complete, and save flow work."
