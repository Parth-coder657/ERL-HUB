param(
    [switch]$Run
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDirectory = Join-Path $Root 'build'
$Executable = Join-Path $BuildDirectory 'erl_hub.exe'

$Compiler = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $Compiler) {
    throw 'g++ was not found on PATH. Install a C++17 compiler or use CMake with another compiler.'
}

New-Item -ItemType Directory -Force -Path $BuildDirectory | Out-Null
$Sources = @(
    'src/DispatchManifest.cpp',
    'src/DistributionDepot.cpp',
    'src/PersistenceManager.cpp',
    'src/ReliefCargo.cpp',
    'src/ReliefHub.cpp',
    'src/ReliefVehicle.cpp',
    'src/main.cpp'
) | ForEach-Object { Join-Path $Root $_ }

& $Compiler.Source -std=c++17 -Wall -Wextra -Wpedantic -I (Join-Path $Root 'include') @Sources -o $Executable
if ($LASTEXITCODE -ne 0) {
    throw "Compilation failed with exit code $LASTEXITCODE."
}

Write-Output "Build successful: $Executable"
if ($Run) {
    & $Executable
    if ($LASTEXITCODE -ne 0) {
        throw "Program exited with code $LASTEXITCODE."
    }
}
