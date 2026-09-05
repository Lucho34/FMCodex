# DEV only. Starts real UE game windows; never writes project/Engine settings.
[CmdletBinding()]
param(
    [string]$UnrealEditorPath = 'E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe',
    [ValidateRange(1024, 65535)][int]$Port = 7777,
    [ValidateRange(640, 3840)][int]$ResX = 900,
    [ValidateRange(480, 2160)][int]$ResY = 700,
    [ValidateRange(5, 120)][int]$ReadyTimeoutSeconds = 60,
    [switch]$ValidateOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-NetworkPlayLaunchPlan {
    param(
        [string]$ScriptDirectory = $PSScriptRoot,
        [string]$EditorPath = $UnrealEditorPath,
        [int]$ListenPort = $Port,
        [int]$Width = $ResX,
        [int]$Height = $ResY
    )
    $projectRoot = [IO.Path]::GetFullPath((Join-Path $ScriptDirectory '..\..'))
    $projectPath = Join-Path $projectRoot 'FMCodex.uproject'
    if (-not (Test-Path -LiteralPath $projectPath -PathType Leaf)) {
        throw "找不到项目文件 / Project not found: $projectPath"
    }
    if (-not (Test-Path -LiteralPath $EditorPath -PathType Leaf)) {
        throw "找不到 UnrealEditor.exe: $EditorPath。请通过 -UnrealEditorPath 指定引擎路径。"
    }
    $editorFullPath = (Resolve-Path -LiteralPath $EditorPath).ProviderPath
    if ([IO.Path]::GetFileName($editorFullPath) -ine 'UnrealEditor.exe') {
        throw '必须使用可显示游戏窗口的 UnrealEditor.exe，不能使用 UnrealEditor-Cmd.exe。'
    }
    $runName = (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '-' + [guid]::NewGuid().ToString('N').Substring(0, 8)
    $logDirectory = Join-Path $projectRoot ('Saved\Logs\NetworkPlayDev\' + $runName)
    $hostLog = Join-Path $logDirectory 'Host.log'
    $clientLog = Join-Path $logDirectory 'Client.log'
    $hostUrl = '/Engine/Maps/Templates/OpenWorld?listen?game=/Script/FMCodex.FMCodexNetworkMatchGameMode'
    $clientUrl = '127.0.0.1:' + $ListenPort
    # Start-Process joins ArgumentList into one native command line: quote paths
    # explicitly so spaces survive. Windows filenames cannot contain double quotes.
    $commonArguments = @(
        ('"{0}"' -f $projectPath), '-game', '-windowed', '-nosplash',
        '-NoLiveCoding', '-NoAutoSave', '-NoSaveConfig',
        # Prevent UE 5.3 Live Coding auto-start; these remain visible game windows.
        '-unattended', ('-ResX={0}' -f $Width), ('-ResY={0}' -f $Height)
    )
    [pscustomobject]@{
        ProjectRoot = $projectRoot
        ProjectPath = $projectPath
        UnrealEditorPath = $editorFullPath
        Port = $ListenPort
        HostUrl = $hostUrl
        ClientUrl = $clientUrl
        LogDirectory = $logDirectory
        HostLog = $hostLog
        ClientLog = $clientLog
        HostArguments = @($commonArguments[0], ('"{0}"' -f $hostUrl)) +
            @($commonArguments | Select-Object -Skip 1) +
            @('-WinX=20', '-WinY=40', ('-port={0}' -f $ListenPort), ('-abslog="{0}"' -f $hostLog))
        ClientArguments = @($commonArguments[0], $clientUrl) +
            @($commonArguments | Select-Object -Skip 1) +
            @(('-WinX={0}' -f ($Width + 40)), '-WinY=40', ('-abslog="{0}"' -f $clientLog))
    }
}

function Assert-NetworkPlayPortAvailable {
    param([int]$ListenPort)
    $network = [Net.NetworkInformation.IPGlobalProperties]::GetIPGlobalProperties()
    $occupied = @($network.GetActiveUdpListeners()) + @($network.GetActiveTcpListeners()) |
        Where-Object { $_.Port -eq $ListenPort }
    if ($occupied) {
        throw "Port $ListenPort is already in use / 端口已占用。请关闭旧 NetworkPlay 窗口，或使用 -Port 指定其他端口；本工具不会终止已有进程。"
    }
}

function Test-NetworkPlayHostLog {
    param([string]$LogText, [int]$ListenPort)
    # UE IpNetDriver listens on UDP, not TCP. Require this run's successful
    # Network GameMode + listen + host admission before the remote may join.
    return $LogText.Contains("Game class is 'FMCodexNetworkMatchGameMode'") -and
        [regex]::IsMatch($LogText, ('IpNetDriver listening on port {0}(?!\d)' -f $ListenPort)) -and
        $LogText.Contains('Admitted participant as Side A (same path for host/remote).')
}

function Read-NetworkPlayHostLog {
    param([string]$LogPath)
    # UE retains a writable log handle. The reader must permit that writer.
    $stream = [IO.File]::Open($LogPath, [IO.FileMode]::Open, [IO.FileAccess]::Read,
        ([IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete))
    try {
        $reader = New-Object IO.StreamReader($stream)
        try { return $reader.ReadToEnd() } finally { $reader.Dispose() }
    } finally { $stream.Dispose() }
}

function Wait-NetworkPlayHost {
    param($HostProcess, [string]$LogPath, [int]$ListenPort, [int]$TimeoutSeconds)
    $watch = [Diagnostics.Stopwatch]::StartNew()
    while ($watch.Elapsed.TotalSeconds -lt $TimeoutSeconds) {
        $HostProcess.Refresh()
        if ($HostProcess.HasExited) {
            throw "Host 已提前退出。Host PID=$($HostProcess.Id); Log=$LogPath"
        }
        if (Test-Path -LiteralPath $LogPath -PathType Leaf) {
            try { $logText = Read-NetworkPlayHostLog -LogPath $LogPath }
            catch [IO.IOException] { $logText = '' } # Retry transient creation/sharing within the same deadline.
            if (Test-NetworkPlayHostLog -LogText $logText -ListenPort $ListenPort) {
                $udpListeners = [Net.NetworkInformation.IPGlobalProperties]::GetIPGlobalProperties().GetActiveUdpListeners()
                if (@($udpListeners | Where-Object { $_.Port -eq $ListenPort }).Count -gt 0) {
                    return
                }
            }
        }
        Start-Sleep -Milliseconds 250
    }
    throw "等待 Listen Server 超时（$TimeoutSeconds 秒），未启动 Client。Host PID=$($HostProcess.Id); Port=$ListenPort; Log=$LogPath。请检查日志并关闭本次 Host 窗口后重试。"
}

# Dot-sourcing is used only by the focused tests; it never starts UE.
if ($MyInvocation.InvocationName -eq '.') { return }

$hostProcess = $null
try {
    $plan = Get-NetworkPlayLaunchPlan
    if ($ValidateOnly) {
        $plan
        return
    }
    Write-Host 'FMCodex DEV NetworkPlay'
    Write-Host ('Project: ' + $plan.ProjectPath)
    Write-Host ('UnrealEditor: ' + $plan.UnrealEditorPath)
    Write-Host ('Port: ' + $plan.Port)
    Assert-NetworkPlayPortAvailable -ListenPort $plan.Port
    New-Item -ItemType Directory -Path $plan.LogDirectory -Force | Out-Null
    $plan | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $plan.LogDirectory 'Launch.json') -Encoding UTF8
    # Visible windows are the explicit purpose of this launcher.
    $hostProcess = Start-Process -FilePath $plan.UnrealEditorPath -ArgumentList $plan.HostArguments -WorkingDirectory $plan.ProjectRoot -WindowStyle Normal -PassThru
    Write-Host ('Host PID: ' + $hostProcess.Id)
    Write-Host ('Host log: ' + $plan.HostLog)
    Write-Host '等待 Listen Server / Waiting for Listen Server...'
    Wait-NetworkPlayHost -HostProcess $hostProcess -LogPath $plan.HostLog -ListenPort $plan.Port -TimeoutSeconds $ReadyTimeoutSeconds
    Write-Host 'Host ready / 主机已就绪'
    $clientProcess = Start-Process -FilePath $plan.UnrealEditorPath -ArgumentList $plan.ClientArguments -WorkingDirectory $plan.ProjectRoot -WindowStyle Normal -PassThru
    Write-Host ('Client PID: ' + $clientProcess.Id)
    Write-Host ('Client log: ' + $plan.ClientLog)
    [pscustomobject]@{ HostPID=$hostProcess.Id; ClientPID=$clientProcess.Id; Port=$plan.Port; StartedAt=(Get-Date).ToString('o') } |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $plan.LogDirectory 'Processes.json') -Encoding UTF8
    Write-Host ''
    Write-Host '预期：左侧 Host = Side A / 玩家 A / 阿森纳；右侧 Client = Side B / 玩家 B / 曼彻斯特城。'
    Write-Host '双方应为 MatchReady、0-0，比赛实例 ID 相同。不要点击“开始本地对战”。'
    Write-Host '测试结束请先关闭 Client，再关闭 Host；无需保存地图或恢复设置。'
} catch {
    Write-Host ('启动失败 / Launch failed: ' + $_.Exception.Message) -ForegroundColor Red
    if ($null -ne $hostProcess) {
        Write-Host ('本工具不会自动终止进程。请按需关闭本次 Host 窗口，PID=' + $hostProcess.Id)
    }
    exit 1
}
