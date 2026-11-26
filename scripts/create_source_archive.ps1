param(
    [string]$OutputPath = (Join-Path (Split-Path -Parent $PSScriptRoot) 'ERL-HUB-source.zip')
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ResolvedOutput = [System.IO.Path]::GetFullPath($OutputPath)
$MaximumBytes = 10MB
$ExcludedDirectories = @('.git', '.vscode', '.vs', '.idea', 'build', 'dist', '__pycache__')
$ExcludedFiles = @('create_source_archive.ps1', 'erl_hub_audit.bin', 'erl_hub_audit.bin.tmp',
    'erl_hub_audit.bin.bak', 'Thumbs.db', 'Desktop.ini')
$ExcludedExtensions = @('.exe', '.dll', '.obj', '.o', '.pdb', '.ilk', '.log', '.zip')

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

$OutputDirectory = Split-Path -Parent $ResolvedOutput
if (-not (Test-Path -LiteralPath $OutputDirectory)) {
    New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
}
if (Test-Path -LiteralPath $ResolvedOutput) {
    Remove-Item -LiteralPath $ResolvedOutput -Force
}

$Archive = $null
try {
    $Archive = [System.IO.Compression.ZipFile]::Open(
        $ResolvedOutput, [System.IO.Compression.ZipArchiveMode]::Create)
    $Files = Get-ChildItem -LiteralPath $ProjectRoot -File -Recurse -Force |
        Where-Object {
            $Relative = $_.FullName.Substring($ProjectRoot.Length).TrimStart('\', '/')
            $Parts = $Relative -split '[\\/]'
            ($Parts | Where-Object { $ExcludedDirectories -contains $_ }).Count -eq 0 -and
                $ExcludedFiles -notcontains $_.Name -and
                $ExcludedExtensions -notcontains $_.Extension.ToLowerInvariant() -and
                [System.IO.Path]::GetFullPath($_.FullName) -ne $ResolvedOutput
        }
    foreach ($File in $Files) {
        $RelativePath = $File.FullName.Substring($ProjectRoot.Length).TrimStart('\', '/')
        $EntryName = $RelativePath -replace '\\', '/'
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
            $Archive, $File.FullName, $EntryName,
            [System.IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
}
finally {
    if ($null -ne $Archive) {
        $Archive.Dispose()
    }
}

$Length = (Get-Item -LiteralPath $ResolvedOutput).Length
Write-Output "Source archive: $ResolvedOutput"
Write-Output ("Archive size: {0:N2} MB" -f ($Length / 1MB))
if ($Length -gt $MaximumBytes) {
    Remove-Item -LiteralPath $ResolvedOutput -Force
    throw 'Archive exceeded the 10 MB limit and was removed. Reduce included source/documentation size.'
}
