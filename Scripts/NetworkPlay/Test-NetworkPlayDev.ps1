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


Assert-True (-not (($plan.HostArguments -join ' ').Contains('FMCodexNetworkRouteMilestone'))) 'Default launch has no route milestone'
foreach ($family in @('Cross','PassControl','ThroughBall')) {
    $InitialRouteMilestone = $family
    $milestonePlan = Get-NetworkPlayLaunchPlan
    Assert-True ($milestonePlan.HostArguments -contains ('-FMCodexNetworkRouteMilestone=' + $family)) 'Milestone family reaches Host only'
    Assert-True (-not (($milestonePlan.ClientArguments -join ' ').Contains('FMCodexNetwork'))) 'Milestone cannot become a Remote authority flag'
    Assert-True (($milestonePlan.HostArguments -contains '-FMCodexNetworkTestBFirst') -eq ($family -ne 'Cross')) 'Cross milestone uses Host; no-branch families use Remote'
    Assert-True (-not (($milestonePlan.HostArguments -join ' ').Contains('InitialRouteD6'))) 'Manual milestone keeps secure route RNG'
}
$DeploymentSlice = $true
Assert-Throws { Get-NetworkPlayLaunchPlan } 'DeploymentSlice' 'Milestone and deployment fixture cannot be accidentally combined'
$DeploymentSlice = $false
. $launcherPath -UnrealEditorPath $testEnginePath

foreach ($mode in @('Goal','NoGoal','FinalGoal','FinalNoGoal')) {
    . $launcherPath -UnrealEditorPath $testEnginePath -CrossTerminalMilestone $mode
    $terminalPlan = Get-NetworkPlayLaunchPlan
    Assert-True ($terminalPlan.HostArguments -contains ('-FMCodexNetworkCrossTerminalMilestone=' + $mode)) ('Host-only canonical terminal fixture: ' + $mode)
    Assert-True (@($terminalPlan.ClientArguments | Where-Object { $_ -like '*FMCodexNetwork*' }).Count -eq 0) ('Client has no terminal fixture capability: ' + $mode)
}
$InitialRouteMilestone = 'Cross'
Assert-Throws { Get-NetworkPlayLaunchPlan } 'CrossTerminalMilestone' 'Terminal and route milestones cannot mix'
$InitialRouteMilestone = $null
$DeploymentSlice = $true
Assert-Throws { Get-NetworkPlayLaunchPlan } 'CrossTerminalMilestone' 'Terminal and deployment fixtures cannot mix'
. $launcherPath -UnrealEditorPath $testEnginePath
Assert-True (@((Get-NetworkPlayLaunchPlan).HostArguments | Where-Object { $_ -like '*CrossTerminalMilestone*' }).Count -eq 0) 'Normal launch retains its original production defaults'

foreach ($mode in @('Goal','NoGoal','FinalGoal','FinalNoGoal')) {
    . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingCrossMilestone $mode
    $uiPlan = Get-NetworkPlayLaunchPlan
    Assert-True ($uiPlan.HostArguments -contains ('-FMCodexNetworkPlayerFacingCrossMilestone=' + $mode)) 'Player-facing fixture is Host-only'
    Assert-True (($uiPlan.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($uiPlan.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Both owners use shared player-facing UI'
    Assert-True (-not (($uiPlan.ClientArguments -join ' ').Contains('Milestone='))) 'Remote UI flag grants no fixture authority'
    Assert-True (-not ($uiPlan.HostArguments -contains '-FMCodexNetworkDiagnostics')) 'Player-facing diagnostics hidden by default'
}
$NetworkDiagnostics = $true
Assert-True ((Get-NetworkPlayLaunchPlan).ClientArguments -contains '-FMCodexNetworkDiagnostics') 'Diagnostics remain explicitly available'
$CrossTerminalMilestone = 'Goal'
Assert-Throws { Get-NetworkPlayLaunchPlan } 'PlayerFacingCrossMilestone' 'Player-facing and diagnostic fixtures cannot mix'
. $launcherPath -UnrealEditorPath $testEnginePath
$normal = Get-NetworkPlayLaunchPlan
Assert-True (($normal.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($normal.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Default launch uses both shared player screens'
Assert-True (($normal.HostArguments -contains '-ResX=1600') -and ($normal.ClientArguments -contains '-ResY=900')) 'Default production viewport fits the shared screen'
Assert-True (@($normal.HostArguments + $normal.ClientArguments | Where-Object { $_ -match 'Milestone|DeploymentSlice|TestBFirst|NetworkDiagnostics|HandoffLatencyAudit' }).Count -eq 0) 'Default launch has no fixture, diagnostics or audit override'
$NetworkDiagnostics = $true
$normalDiagnostics = Get-NetworkPlayLaunchPlan
Assert-True (($normalDiagnostics.HostArguments -contains '-FMCodexNetworkDiagnostics') -and ($normalDiagnostics.ClientArguments -contains '-FMCodexNetworkDiagnostics')) 'Normal launch supports explicit diagnostics on both owners'
$NetworkDiagnostics = $false

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
$HandoffLatencyAudit = $false
$auditOff = Get-NetworkPlayLaunchPlan -EditorPath $testEnginePath
Assert-True (-not ($auditOff.HostArguments -contains '-HandoffLatencyAudit')) 'Audit disabled by default on Host'
Assert-True (-not ($auditOff.ClientArguments -contains '-HandoffLatencyAudit')) 'Audit disabled by default on Client'
$HandoffLatencyAudit = $true
$auditOn = Get-NetworkPlayLaunchPlan -EditorPath $testEnginePath
Assert-True ($auditOn.HostArguments -contains '-HandoffLatencyAudit') 'Audit opt in reaches Host'
Assert-True ($auditOn.ClientArguments -contains '-HandoffLatencyAudit') 'Audit opt in reaches Client'
$HandoffLatencyAudit = $false
foreach ($family in @('PassControl', 'ThroughBallFeet')) {
    foreach ($mode in @('Goal', 'NoGoal', 'FinalGoal', 'FinalNoGoal')) {
        if ($family -eq 'PassControl') { . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingPassControlMilestone $mode }
        else { . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingThroughBallFeetMilestone $mode }
        $ordinary = Get-NetworkPlayLaunchPlan
        Assert-True ($ordinary.HostArguments -contains ('-FMCodexNetworkPlayerFacing' + $family + 'Milestone=' + $mode)) 'Ordinary fixture reaches only authoritative Host'
        Assert-True (@($ordinary.ClientArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*RouteD6*' }).Count -eq 0) 'Remote cannot configure route or fixture'
        Assert-True (($ordinary.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($ordinary.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Both ordinary viewers reuse shared screen'
        Assert-True (($ordinary.HostArguments -contains '-ResX=1600') -and ($ordinary.ClientArguments -contains '-ResY=900')) 'Ordinary player-facing default dimensions'
    }
}
. $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingPassControlMilestone Goal -PassControlRouteD6 5
Assert-True ((Get-NetworkPlayLaunchPlan).HostArguments -contains '-FMCodexNetworkPassControlRouteD6=5') 'Explicit third PassControl route uses server RNG seam'
$PlayerFacingThroughBallFeetMilestone = 'Goal'
Assert-Throws { Get-NetworkPlayLaunchPlan } '请单独使用' 'Ordinary fixtures cannot mix'
. $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingThroughBallFeetMilestone Goal
$InitialRouteMilestone = 'ThroughBall'
Assert-Throws { Get-NetworkPlayLaunchPlan } '请单独使用' 'Feet milestone cannot mix with uncontrolled route fixture'
. $launcherPath -UnrealEditorPath $testEnginePath
Assert-True (@((Get-NetworkPlayLaunchPlan).HostArguments | Where-Object { $_ -like '*Milestone*' }).Count -eq 0) 'Ordinary extensions retain secure normal launch defaults'
Write-Host ("FMCODEX_NETWORK_LAUNCHER_TESTS=PASS ({0} assertions)" -f $script:NetworkPlayAssertions)

foreach ($path in @('BehindOneOnOne','BehindOutOfPlay','AntiOffside','AntiOneOnOne')) {
    foreach ($side in @('A','B')) {
        . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingThroughBallMilestone $path -ThroughBallActor $side -ThroughBallNoGoal -ThroughBallFinal
        $conditional = Get-NetworkPlayLaunchPlan
        Assert-True ($conditional.HostArguments -contains ('-FMCodexNetworkPlayerFacingThroughBallMilestone=' + $path)) 'Conditional fixture is explicit on Host'
        Assert-True ($conditional.HostArguments -contains ('-FMCodexNetworkThroughBallActor=' + $side)) 'Requested attack direction uses canonical setup'
        Assert-True ($conditional.HostArguments -contains '-FMCodexNetworkThroughBallNoGoal') 'OneOnOne outcome pins only provider seam'
        Assert-True ($conditional.HostArguments -contains '-FMCodexNetworkThroughBallFinal') 'Final fixture reuses canonical short opening'
        Assert-True (@($conditional.ClientArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*ThroughBallActor*' -or $_ -like '*ThroughBallNoGoal*' -or $_ -like '*ThroughBallFinal*' }).Count -eq 0) 'Client receives no fixture authority'
    }
}
Write-Host 'ThroughBall conditional launch-plan checks PASS.'

foreach ($family in @('LongShot', 'CutInside')) {
    foreach ($mode in @('DirectGoal', 'DirectMiss', 'ImmediateMiss', 'DeadCornerGoal', 'DeadCornerMiss')) {
        foreach ($side in @('A', 'B')) {
            $shotParameters = @{ UnrealEditorPath=$testEnginePath; ShotActor=$side; ShotFinal=$true }
            $shotParameters['PlayerFacing' + $family + 'Milestone'] = $mode
            . $launcherPath @shotParameters
            $shot = Get-NetworkPlayLaunchPlan
            Assert-True ($shot.HostArguments -contains ('-FMCodexNetworkPlayerFacing' + $family + 'Milestone=' + $mode)) 'Exact shot fixture reaches Host'
            Assert-True ($shot.HostArguments -contains ('-FMCodexNetworkShotActor=' + $side)) 'Both shot directions supported'
            Assert-True ($shot.HostArguments -contains '-FMCodexNetworkShotFinal') 'Final shot uses existing lifecycle'
            Assert-True (@($shot.ClientArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*ShotActor*' -or $_ -like '*ShotFinal*' }).Count -eq 0) 'Remote has no shot provider controls'
            Assert-True (($shot.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($shot.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Shared screen on both shot viewers'
            Assert-True (($shot.HostArguments -contains '-ResX=1600') -and ($shot.ClientArguments -contains '-ResY=900')) 'Shot player screen fits fixed pitch'
            Assert-True (-not ($shot.HostArguments -contains '-FMCodexNetworkDiagnostics')) 'No obscuring diagnostic panel'
        }
    }
}
$PlayerFacingLongShotMilestone = 'DeadCornerGoal'
Assert-Throws { Get-NetworkPlayLaunchPlan } '请单独使用' 'Two shot fixtures cannot mix'
. $launcherPath -UnrealEditorPath $testEnginePath
Assert-True (@((Get-NetworkPlayLaunchPlan).HostArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*ShotActor*' }).Count -eq 0) 'Normal launch keeps production RNG'
Write-Host ("FMCODEX_NETWORK_LAUNCHER_FINAL_TESTS=PASS ({0} assertions)" -f $script:NetworkPlayAssertions)

foreach ($kind in @('Runner', 'Helper', 'Skill', 'Marker')) {
    foreach ($side in @('A', 'B')) {
        . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingDeclineMilestone $kind -DeclineActor $side -DeclineFinal
        $decline = Get-NetworkPlayLaunchPlan
        Assert-True ($decline.HostArguments -contains ('-FMCodexNetworkPlayerFacingDeclineMilestone=' + $kind)) 'Exact optional wait on Host'
        Assert-True ($decline.HostArguments -contains ('-FMCodexNetworkDeclineActor=' + $side)) 'Both declining actors supported'
        Assert-True ($decline.HostArguments -contains '-FMCodexNetworkDeclineFinal') 'Final decline uses canonical prelude'
        Assert-True (@($decline.ClientArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*DeclineActor*' -or $_ -like '*DeclineFinal*' }).Count -eq 0) 'Remote receives no decline fixture authority'
        Assert-True (($decline.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($decline.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Declines reuse shared screen'
        Assert-True (-not ($decline.HostArguments -contains '-FMCodexNetworkDiagnostics')) 'Decline CTA unobscured by diagnostics'
    }
}
$PlayerFacingLongShotMilestone = 'DirectGoal'
Assert-Throws { Get-NetworkPlayLaunchPlan } '请单独使用' 'Decline and tactic fixtures cannot mix'
. $launcherPath -UnrealEditorPath $testEnginePath
Assert-True (@((Get-NetworkPlayLaunchPlan).HostArguments | Where-Object { $_ -like '*Decline*' -or $_ -like '*Milestone*' }).Count -eq 0) 'Decline fixture remains opt in'
Write-Host ("FMCODEX_OPTIONAL_DECLINE_LAUNCHER_TESTS=PASS ({0} assertions)" -f $script:NetworkPlayAssertions)

foreach ($mode in @('DirectGoal','DirectMiss','AngledGoal','AngledMiss')) {
    foreach ($side in @('A','B')) {
        . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingNearFreeKickMilestone $mode -SetPieceActor $side
        $near = Get-NetworkPlayLaunchPlan
        Assert-True ($near.HostArguments -contains ('-FMCodexNetworkPlayerFacingNearFreeKickMilestone=' + $mode)) 'Near fixture reaches Host only'
        Assert-True ($near.HostArguments -contains ('-FMCodexNetworkSetPieceActor=' + $side)) 'Near supports both actors'
        Assert-True (@($near.ClientArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*SetPieceActor*' }).Count -eq 0) 'Near Remote receives no deterministic controls'
        Assert-True (($near.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($near.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Near uses both shared screens'
        Assert-True (($near.HostArguments -contains '-ResX=1600') -and ($near.ClientArguments -contains '-ResY=900')) 'Near uses production viewport size'
    }
}
$PlayerFacingSetPieceMilestone = 'Near'
Assert-Throws { Get-NetworkPlayLaunchPlan } '请单独使用' 'Near fixture cannot mix with selection fixture'
. $launcherPath -UnrealEditorPath $testEnginePath
Assert-True (@((Get-NetworkPlayLaunchPlan).HostArguments | Where-Object { $_ -like '*Milestone*' }).Count -eq 0) 'Near fixture keeps secure defaults'
Write-Host ("FMCODEX_NEAR_LAUNCHER_TESTS=PASS ({0} assertions)" -f $script:NetworkPlayAssertions)

foreach ($mode in @('DirectGoal','DirectMiss','DirectEarlyNoGoal','PowerGoal','PowerMiss')) {
    foreach ($side in @('A','B')) {
        . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingLongFreeKickMilestone $mode -SetPieceActor $side
        $long = Get-NetworkPlayLaunchPlan
        Assert-True ($long.HostArguments -contains ('-FMCodexNetworkPlayerFacingLongFreeKickMilestone=' + $mode)) 'Long fixture reaches Host only'
        Assert-True ($long.HostArguments -contains ('-FMCodexNetworkSetPieceActor=' + $side)) 'Long supports both actors'
        Assert-True (@($long.ClientArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*SetPieceActor*' }).Count -eq 0) 'Long Remote receives no deterministic controls'
        Assert-True (($long.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($long.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Long uses both shared screens'
        Assert-True (($long.HostArguments -contains '-ResX=1600') -and ($long.ClientArguments -contains '-ResY=900')) 'Long uses production viewport size'
    }
}
$PlayerFacingSetPieceMilestone = 'Long'
Assert-Throws { Get-NetworkPlayLaunchPlan } '请单独使用' 'Long fixture cannot mix with selection fixture'
. $launcherPath -UnrealEditorPath $testEnginePath
Assert-True (@((Get-NetworkPlayLaunchPlan).HostArguments | Where-Object { $_ -like '*Milestone*' }).Count -eq 0) 'Long fixture keeps secure defaults'
Write-Host ("FMCODEX_LONG_LAUNCHER_TESTS=PASS ({0} assertions)" -f $script:NetworkPlayAssertions)

# Stage 7.24: host-only Penalty provider controls, both owning sides.
foreach ($mode in @('DirectGoal','DirectMiss','PanenkaGoal','PanenkaMiss')) {
    foreach ($side in @('A','B')) {
        . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingPenaltyMilestone $mode -SetPieceActor $side
        $penalty = Get-NetworkPlayLaunchPlan
        Assert-True ($penalty.HostArguments -contains ('-FMCodexNetworkPlayerFacingPenaltyMilestone=' + $mode)) 'Penalty provider fixture reaches Host only'
        Assert-True ($penalty.HostArguments -contains ('-FMCodexNetworkSetPieceActor=' + $side)) 'Penalty supports both actors'
        Assert-True (@($penalty.ClientArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*SetPieceActor*' }).Count -eq 0) 'Penalty Remote receives no provider controls'
        Assert-True (($penalty.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($penalty.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Penalty uses both shared screens'
    }
}
$PlayerFacingLongFreeKickMilestone = 'DirectGoal'
Assert-Throws { Get-NetworkPlayLaunchPlan } '请单独使用' 'Penalty cannot mix with Long fixture'
. $launcherPath -UnrealEditorPath $testEnginePath
Assert-True (@((Get-NetworkPlayLaunchPlan).HostArguments | Where-Object { $_ -like '*Milestone*' }).Count -eq 0) 'Penalty retains secure defaults'
Write-Host ("FMCODEX_PENALTY_LAUNCHER_TESTS=PASS ({0} assertions)" -f $script:NetworkPlayAssertions)

foreach ($mode in @('OpposedGoal','AttackZero','DefenseZero')) {
    . $launcherPath -UnrealEditorPath $testEnginePath -PlayerFacingCornerMilestone $mode -SetPieceActor B
    $corner = Get-NetworkPlayLaunchPlan
    Assert-True ($corner.HostArguments -contains ('-FMCodexNetworkPlayerFacingCornerMilestone=' + $mode)) 'Corner provider fixture reaches Host'
    Assert-True ($corner.HostArguments -contains '-FMCodexNetworkSetPieceActor=B') 'Corner identity is independent of Host'
    Assert-True (@($corner.ClientArguments | Where-Object { $_ -like '*Milestone*' -or $_ -like '*SetPieceActor*' }).Count -eq 0) 'Corner Remote receives no private provider controls'
    Assert-True (($corner.HostArguments -contains '-FMCodexNetworkPlayerFacingUI') -and ($corner.ClientArguments -contains '-FMCodexNetworkPlayerFacingUI')) 'Corner shares both production screens'
}
$PlayerFacingPenaltyMilestone = 'DirectGoal'
Assert-Throws { Get-NetworkPlayLaunchPlan } '请单独使用' 'Corner cannot mix with Penalty'
. $launcherPath -UnrealEditorPath $testEnginePath
Assert-True (@((Get-NetworkPlayLaunchPlan).HostArguments | Where-Object { $_ -like '*Milestone*' }).Count -eq 0) 'Corner provider remains opt in'
Write-Host ("FMCODEX_CORNER_LAUNCHER_TESTS=PASS ({0} assertions)" -f $script:NetworkPlayAssertions)
