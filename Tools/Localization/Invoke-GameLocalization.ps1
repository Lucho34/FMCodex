param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Gather', 'Export', 'Import', 'Compile')]
    [string] $Step,
    [Parameter(Mandatory = $true)]
    [string] $EngineRoot
)
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$editorCmd = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$project = Join-Path $projectRoot 'FMCodex.uproject'
$config = Join-Path $projectRoot "Config/Localization/Game_$Step.ini"
$logDirectory = Join-Path $projectRoot 'Saved/LocalizationFoundation'
New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
$log = Join-Path $logDirectory "$Step.log"
if (!(Test-Path -LiteralPath $editorCmd) -or !(Test-Path -LiteralPath $config)) {
    throw 'UnrealEditor-Cmd or the localization config does not exist.'
}
# Standard UE commandlets only; no SCC flags or custom translation lookup.
& $editorCmd $project '-run=GatherText' "-config=$config" '-unattended' '-nop4' '-nosplash' "-abslog=$log"
if ($LASTEXITCODE -ne 0) { throw "Localization $Step failed ($LASTEXITCODE). See $log" }
Write-Host "Localization $Step completed. Review warnings and coverage in $log"
