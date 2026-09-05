# Focused dependency-free checks. Does not launch UE or alter config/maps.
[CmdletBinding()]
param([string]$UnrealEditorPath = 'E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe')

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$launcherPath = Join-Path $PSScriptRoot 'LaunchNetworkPlayDev.ps1'
$testEnginePath = $UnrealEditorPath
. $launcherPath -UnrealEditorPath $testEnginePath
$script:NetworkPlayAssertions = 0
function Assert-True([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }
    $script:NetworkPlayAssertions++
    Write-Host "PASS: $Message"
}
function Assert-Throws([scriptblock]$Action, [string]$Expected, [string]$Message) {
    $caught = ''
    try { & $Action | Out-Null } catch { $caught = $_.Exception.Message }
    Assert-True ($caught.Contains($Expected)) $Message
}

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$configPath = Join-Path $repoRoot 'Config\DefaultEngine.ini'
$configBefore = (Get-FileHash -LiteralPath $configPath).Hash
$plan = Get-NetworkPlayLaunchPlan -EditorPath $testEnginePath
Assert-True ($plan.ProjectPath -eq (Join-Path $repoRoot 'FMCodex.uproject')) 'Project resolves relative to script location'
Push-Location ([IO.Path]::GetTempPath())
try {
    $dryRun = & $launcherPath -UnrealEditorPath $testEnginePath -ValidateOnly
    Assert-True ($dryRun.ProjectPath -eq $plan.ProjectPath) 'ValidateOnly works from unrelated working directory'
    Assert-True (-not (Test-Path -LiteralPath $dryRun.LogDirectory)) 'Validation does not create logs or start UE'
} finally { Pop-Location }
Assert-Throws { Get-NetworkPlayLaunchPlan -EditorPath (Join-Path $repoRoot 'MissingEngine\UnrealEditor.exe') } 'UnrealEditor.exe' 'Missing engine fails clearly'
Assert-Throws { Get-NetworkPlayLaunchPlan -EditorPath (Join-Path (Split-Path -Parent $testEnginePath) 'UnrealEditor-Cmd.exe') } 'UnrealEditor-Cmd.exe' 'Non-editor executable is rejected'
Assert-Throws { Get-NetworkPlayLaunchPlan -ScriptDirectory (Join-Path ([IO.Path]::GetTempPath()) ([guid]::NewGuid().ToString('N') + '\Scripts\NetworkPlay')) } 'Project not found' 'Missing derived project fails clearly'
Assert-True ($plan.HostUrl -eq '/Engine/Maps/Templates/OpenWorld?listen?game=/Script/FMCodex.FMCodexNetworkMatchGameMode') 'Host URL pins the canonical Network GameMode'
Assert-True ($plan.HostArguments[0] -eq ('"{0}"' -f $plan.ProjectPath)) 'Project argument preserves spaces'
Assert-True ($plan.ClientArguments[0] -eq $plan.HostArguments[0]) 'Host and Client use the same project'
Assert-True ($plan.ClientUrl -eq '127.0.0.1:7777') 'Default Client joins port 7777'
$alternate = Get-NetworkPlayLaunchPlan -ListenPort 7788 -Width 1000 -Height 720
Assert-True (($alternate.HostArguments -contains '-port=7788') -and ($alternate.ClientArguments -contains '127.0.0.1:7788')) 'Custom port is identical on Host and Client'
Assert-True (-not (($plan.ClientArguments -join ' ').Contains('?game='))) 'Client does not override GameMode'
Assert-True (($plan.HostArguments -contains '-game') -and ($plan.HostArguments -contains '-windowed') -and -not ($plan.HostArguments -contains '-nullrhi')) 'Launcher uses visible game windows'
Assert-True (($alternate.HostArguments -contains '-ResX=1000') -and ($alternate.ClientArguments -contains '-ResY=720')) 'Window size parameters reach both commands'
Assert-True (($plan.HostArguments -contains '-NoSaveConfig') -and ($plan.ClientArguments -contains '-NoAutoSave')) 'Runtime does not persist test settings'
Assert-True ($plan.HostLog.StartsWith((Join-Path $repoRoot 'Saved\Logs\NetworkPlayDev\'))) 'Logs stay under ignored Saved'
Assert-True ($plan.HostLog -ne $plan.ClientLog) 'Host and Client logs are distinct'
Assert-True ($plan.LogDirectory -ne (Get-NetworkPlayLaunchPlan).LogDirectory) 'Every launch gets fresh logs; stale readiness cannot be reused'

Assert-True (-not (($plan.HostArguments -join ' ').Contains('FMCodexNetworkDeploymentSlice'))) 'Default launch has no deterministic deployment override'
$DeploymentSlice = $true
$deploymentPlan = Get-NetworkPlayLaunchPlan
$DeploymentSlice = $false
Assert-True (($deploymentPlan.HostArguments -contains '-FMCodexNetworkDeploymentSlice') -and ($deploymentPlan.HostArguments -contains '-FMCodexNetworkTestBFirst')) 'Optional deployment fixture reaches only the Host command'
Assert-True (-not (($deploymentPlan.ClientArguments -join ' ').Contains('FMCodexNetwork'))) 'Remote has no fixture or deterministic authority argument'
Assert-True ($deploymentPlan.HostUrl -eq $plan.HostUrl) 'Fixture preserves normal Network GameMode launch path'

$readyLog = "Game class is 'FMCodexNetworkMatchGameMode'`nIpNetDriver listening on port 7777`nAdmitted participant as Side A (same path for host/remote)."
Assert-True (Test-NetworkPlayHostLog $readyLog 7777) 'Correct Network listen/admission markers are accepted'
Assert-True (-not (Test-NetworkPlayHostLog 'IpNetDriver listening on port 7777' 7777)) 'Socket line alone cannot start Client before Host admission'
Assert-True (-not (Test-NetworkPlayHostLog $readyLog 7778)) 'Wrong port log cannot satisfy readiness'
Assert-True (-not (Test-NetworkPlayHostLog ($readyLog.Replace('port 7777', 'port 77770')) 7777)) 'Port matching rejects a numeric prefix'
Assert-True (-not (Test-NetworkPlayHostLog ($readyLog.Replace('FMCodexNetworkMatchGameMode', 'FMCodexLocalMatchHostGameMode')) 7777)) 'Local GameMode cannot satisfy readiness'

$sharedLog = [IO.Path]::GetTempFileName()
$writer = [IO.File]::Open($sharedLog, [IO.FileMode]::Create, [IO.FileAccess]::Write, [IO.FileShare]::ReadWrite)
try {
    $bytes = [Text.Encoding]::UTF8.GetBytes($readyLog)
    $writer.Write($bytes, 0, $bytes.Length)
    $writer.Flush()
    Assert-True ((Read-NetworkPlayHostLog $sharedLog) -eq $readyLog) 'Host log can be read while UE retains its writer handle'
} finally {
    $writer.Dispose()
    [IO.File]::Delete($sharedLog) # Only this test's exact temporary file.
}

$udp = New-Object Net.Sockets.UdpClient
try {
    $udp.Client.Bind((New-Object Net.IPEndPoint([Net.IPAddress]::Loopback, 0)))
    $occupiedPort = $udp.Client.LocalEndPoint.Port
    Assert-Throws { Assert-NetworkPlayPortAvailable $occupiedPort } 'already in use' 'An occupied UDP port is rejected'
    Assert-True $udp.Client.IsBound 'Conflict handling leaves the owning socket alive'
} finally { $udp.Dispose() }
$tcp = New-Object Net.Sockets.TcpListener([Net.IPAddress]::Loopback, 0)
try {
    $tcp.Start()
    Assert-Throws { Assert-NetworkPlayPortAvailable $tcp.LocalEndpoint.Port } 'already in use' 'An occupied TCP port is also rejected'
} finally { $tcp.Stop() }
$fakeHost = [pscustomobject]@{ Id=123456; HasExited=$false }
$fakeHost | Add-Member -MemberType ScriptMethod -Name Refresh -Value {}
$missingLog = Join-Path ([IO.Path]::GetTempPath()) ([guid]::NewGuid().ToString('N') + '.log')
$watch = [Diagnostics.Stopwatch]::StartNew()
Assert-Throws { Wait-NetworkPlayHost $fakeHost $missingLog 7777 1 } 'PID=123456' 'Timeout reports the launched Host PID and does not launch Client'
Assert-True ($watch.Elapsed.TotalSeconds -lt 5) 'Readiness wait is bounded'
$fakeHost.HasExited = $true
Assert-Throws { Wait-NetworkPlayHost $fakeHost $missingLog 7777 10 } 'Host 已提前退出' 'Host exit fails immediately'

$tokens = $null
$parseErrors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile($launcherPath, [ref]$tokens, [ref]$parseErrors)
Assert-True ($parseErrors.Count -eq 0) 'PowerShell syntax parses cleanly'
$commandNames = @($ast.FindAll({ param($node) $node -is [Management.Automation.Language.CommandAst] }, $true) | ForEach-Object { $_.GetCommandName() })
Assert-True (@($commandNames | Where-Object { $_ -in @('Stop-Process','taskkill','Set-ExecutionPolicy','Remove-Item','Move-Item') }).Count -eq 0) 'Launcher has no process kill, global policy change or destructive file command'
$source = [IO.File]::ReadAllText($launcherPath)
Assert-True (-not ($source -match 'save_current_level|save_map|SavePackage|SetDefaultGameMode|WorldSettings|\.umap')) 'Launcher has no Engine map save/mutation commands'
Assert-True (-not ($source.Contains('DefaultEngine.ini'))) 'Launcher never writes LocalPlay config'
$wrapper = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'LaunchNetworkPlayDev.cmd'))
Assert-True ($wrapper.Contains('-ExecutionPolicy Bypass -File "%~dp0LaunchNetworkPlayDev.ps1"')) 'Double-click wrapper uses only a process-local policy override and its own script path'
Assert-True ((Get-FileHash -LiteralPath $configPath).Hash -eq $configBefore) 'DefaultEngine.ini stays byte-identical'
Write-Host ("FMCODEX_NETWORK_LAUNCHER_TESTS=PASS ({0} assertions)" -f $script:NetworkPlayAssertions)
