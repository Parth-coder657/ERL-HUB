param(
    [string]$Executable = (Join-Path (Split-Path -Parent $PSScriptRoot) 'build\erl_hub.exe')
)

$ErrorActionPreference = 'Stop'
$Executable = (Resolve-Path -LiteralPath $Executable).Path
$TemporaryDirectory = Join-Path ([System.IO.Path]::GetTempPath()) (
    'erl-hub-smoke-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $TemporaryDirectory | Out-Null
$OriginalLocation = Get-Location

try {
    Set-Location -LiteralPath $TemporaryDirectory
    $FirstRun = @(
        '3',
        'Bridge Camp', 'WATER', '999', '5', '25', 'n', 'n',
        '3',
        'Bridge Camp', 'WATER', '12', '5', '25', 'n', 'n',
        '3',
        'Clinic North', 'MED-KIT', '4', '1', '30', 'y', 'n',
        '2',
        'STEEL', 'Emergency equipment crates', '100', '2', 'n',
        '3',
        'Equipment Depot', 'STEEL', '2', '3', '40', 'n', 'n',
        '4',
        '5',
        '5',
        '5',
        '8',
        '8',
        '9',
        '7',
        '0'
    ) | & $Executable 2>&1
    $FirstOutput = $FirstRun -join "`n"
    if ($LASTEXITCODE -ne 0) { throw "Program failed during first smoke run: $FirstOutput" }
    if ($FirstOutput -notmatch 'exceeds current depot stock') {
        throw 'Insufficient-stock validation was not observed.'
    }
    if ($FirstOutput -notmatch 'MIS-0002.*URGENT MEDICAL') {
        throw 'Urgent medical request was not prioritized ahead of the normal request.'
    }
    if ($FirstOutput -notmatch 'Dispatch successful: MIS-0002') {
        throw 'Expected urgent medical request was not dispatched.'
    }
    if ($FirstOutput -notmatch 'Dispatch successful: MIS-0003 -> GROUND-01') {
        throw 'Vehicle capacity-based dispatch selection was not observed.'
    }
    if ([regex]::Matches($FirstOutput, 'Saved 3 audit record').Count -lt 2) {
        throw 'Audit record was not saved.'
    }
    if ($FirstOutput -notmatch 'Loaded 3 audit record') {
        throw 'Audit record was not loaded.'
    }
    $AuditPath = Join-Path $TemporaryDirectory 'erl_hub_audit.bin'
    if (-not (Test-Path -LiteralPath $AuditPath)) {
        throw 'The binary audit file was not created.'
    }
    $SavedBytes = [System.IO.File]::ReadAllBytes($AuditPath)
    if ($SavedBytes.Length -lt 12) { throw 'Saved audit file is unexpectedly short.' }
    [byte[]]$TruncatedBytes = $SavedBytes[0..7]
    [System.IO.File]::WriteAllBytes($AuditPath, $TruncatedBytes)
    $CorruptRun = @('9', '0') | & $Executable 2>&1
    $CorruptOutput = $CorruptRun -join "`n"
    if ($LASTEXITCODE -ne 0) { throw "Program failed while handling a truncated file: $CorruptOutput" }
    if ($CorruptOutput -notmatch 'Audit file header is corrupt, unsupported, or truncated') {
        throw 'Truncated audit-file validation was not observed.'
    }
    Write-Output 'Smoke test passed: stock validation, priority, capacity-based dispatch, atomic replacement, save/reload, and truncated-file rejection.'
}
finally {
    Set-Location -LiteralPath $OriginalLocation
    if (Test-Path -LiteralPath $TemporaryDirectory) {
        Remove-Item -LiteralPath $TemporaryDirectory -Recurse -Force
    }
}
