@echo off
setlocal
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0LaunchNetworkPlayDev.ps1" %*
set "LaunchExitCode=%ERRORLEVEL%"
echo.
echo Press any key to close this launcher console. Game windows stay open.
pause >nul
exit /b %LaunchExitCode%
