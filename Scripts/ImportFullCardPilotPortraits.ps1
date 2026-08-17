param(
    [string]$UnrealEditorCmd = "E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot "FMCodex.uproject"
$importScript = Join-Path $PSScriptRoot "ImportFullCardPilotPortraits.py"
$validateScript = Join-Path $PSScriptRoot "ValidateFullCardPilotPortraits.py"
$common = @(
    $projectFile,
    "-EnablePlugins=PythonScriptPlugin,EditorScriptingUtilities",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-nullrhi",
    "-stdout",
    "-FullStdOutLogOutput"
)

if (-not (Test-Path -LiteralPath $UnrealEditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found: $UnrealEditorCmd"
}

function Invoke-UnrealPython {
    param(
        [string]$ScriptPath,
        [string]$RequiredSentinel,
        [string]$Operation
    )

    $output = & $UnrealEditorCmd @common "-ExecutePythonScript=$ScriptPath" 2>&1
    $exitCode = $LASTEXITCODE
    $output | Write-Output
    if ($exitCode -ne 0) {
        throw "$Operation failed with exit code $exitCode"
    }
    if (-not ($output | Select-String -SimpleMatch $RequiredSentinel -Quiet)) {
        throw "$Operation did not emit required sentinel: $RequiredSentinel"
    }
}

Invoke-UnrealPython -ScriptPath $importScript `
    -RequiredSentinel "FMCODEX_FULL_CARD_PILOT_IMPORT=PASS" `
    -Operation "Full Card pilot portrait import"

# A separate process proves package discovery and load without importer memory.
Invoke-UnrealPython -ScriptPath $validateScript `
    -RequiredSentinel "FMCODEX_FULL_CARD_PILOT_VALIDATION=PASS" `
    -Operation "Fresh-process Full Card pilot portrait validation"
