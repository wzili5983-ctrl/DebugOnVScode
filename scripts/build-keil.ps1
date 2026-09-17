param([string]$KeilPath = 'D:\Keil 5\UV4\UV4.exe')

$ErrorActionPreference = 'Stop'
try {
    $projectRoot = Split-Path -Parent $PSScriptRoot
    $project = Join-Path $projectRoot 'USER\LED.uvprojx'
    $log = Join-Path $projectRoot 'OBJ\vscode-build.log'
    if (-not (Test-Path -LiteralPath $KeilPath)) {
        throw "Keil UV4.exe not found: $KeilPath. Update scripts/build-keil.ps1."
    }
    New-Item -ItemType Directory -Force -Path (Join-Path $projectRoot 'OBJ') | Out-Null
    if (Test-Path -LiteralPath $log) { Remove-Item -LiteralPath $log }
    # Rebuild so the AXF, HEX and source always match. uVision returns 1 for warnings.
    $arguments = '-r "{0}" -t "LED" -o "{1}"' -f $project, $log
    $process = Start-Process -FilePath $KeilPath -ArgumentList $arguments -WorkingDirectory (Join-Path $projectRoot 'USER') -WindowStyle Hidden -Wait -PassThru
    if (Test-Path -LiteralPath $log) { Get-Content -LiteralPath $log }
    if ($process.ExitCode -notin @(0, 1)) { throw "Keil build failed (exit $($process.ExitCode))." }
    foreach ($output in @('LED.axf', 'LED.hex')) {
        if (-not (Test-Path -LiteralPath (Join-Path $projectRoot "OBJ\$output"))) {
            throw "Missing build output: $output"
        }
    }
    exit 0
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
}
