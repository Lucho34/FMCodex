param(
    [string]$UnrealEditorCmd = "E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    [string]$PythonExe = "python",
    [string[]]$PlayerKeys = @()
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot "FMCodex.uproject"
$derivativeScript = Join-Path $PSScriptRoot "GenerateSharedPortraitRuntimeDerivatives.py"
$importScript = Join-Path $PSScriptRoot "ImportPrototypeTeamUIAssets.py"
$validateScript = Join-Path $PSScriptRoot "ValidatePrototypeTeamUIAssets.py"
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

$pythonCommand = Get-Command $PythonExe -ErrorAction Stop
$resolvedPythonExe = $pythonCommand.Source

function Invoke-DerivativeGeneration {
    param([string]$RequiredSentinel)

    $output = & $resolvedPythonExe -B $derivativeScript 2>&1
    $exitCode = $LASTEXITCODE
    $output | Write-Output
    if ($exitCode -ne 0) {
        throw "Shared Portrait runtime derivative generation failed with exit code $exitCode"
    }
    if (-not ($output | Select-String -SimpleMatch $RequiredSentinel -Quiet)) {
        throw "Derivative generation did not emit required sentinel: $RequiredSentinel"
    }
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

$batchEnvironmentName = "FMCODEX_SHARED_PORTRAIT_PLAYER_KEYS"
$previousBatchSelection = [Environment]::GetEnvironmentVariable($batchEnvironmentName)
$selectedCount = $PlayerKeys.Count
try {
    if ($selectedCount -gt 0) {
        [Environment]::SetEnvironmentVariable(
            $batchEnvironmentName, ($PlayerKeys -join ";"))
    } else {
        [Environment]::SetEnvironmentVariable($batchEnvironmentName, $null)
    }
    $expectedSuffix = if ($selectedCount -gt 0) {
        " selected=$selectedCount"
    } else {
        ""
    }

    Invoke-DerivativeGeneration -RequiredSentinel `
        "FMCODEX_SHARED_PORTRAIT_DERIVATIVE_GENERATION=PASS$expectedSuffix"

    Invoke-UnrealPython -ScriptPath $importScript `
        -RequiredSentinel "FMCODEX_PROTOTYPE_TEAM_IMPORT=PASS$expectedSuffix" `
        -Operation "Prototype-team portrait import"

    # A separate process proves package discovery and load without importer memory.
    Invoke-UnrealPython -ScriptPath $validateScript `
        -RequiredSentinel "FMCODEX_PROTOTYPE_TEAM_VALIDATION=PASS$expectedSuffix" `
        -Operation "Fresh-process prototype-team portrait validation"
} finally {
    [Environment]::SetEnvironmentVariable(
        $batchEnvironmentName, $previousBatchSelection)
}
