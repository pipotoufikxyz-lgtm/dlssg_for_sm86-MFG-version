@echo off
setlocal EnableExtensions

rem Installs the source-built Vulkan integration beside a caller-provided game
rem executable or directory. This does not replace version.dll or dinput8.dll.

if "%~1"=="" (
    echo Usage: %~nx0 "C:\Path\To\Game\bin"
    echo    or: %~nx0 "C:\Path\To\Game\bin\game.exe"
    echo.
    echo Run this script from the extracted release package.
    exit /b 2
)

set "TARGET=%~f1"
set "ROOT=%~dp0"
set "SOURCE=%ROOT%vulkan"

if exist "%TARGET%\NUL" (
    set "TARGET_KIND=directory"
) else if exist "%TARGET%" (
    set "TARGET=%~dp1"
    set "TARGET_KIND=executable"
) else (
    echo ERROR: Target path does not exist:
    echo        %~f1
    exit /b 3
)

if not exist "%TARGET%\." (
    echo ERROR: Could not resolve a target directory:
    echo        %TARGET%
    exit /b 3
)

for %%F in (
    "dlssg_vulkan_ngx.dll"
    "dlssg_vulkan_proxy.dll"
    "dlssg_vulkan_route.dll"
) do (
    if not exist "%SOURCE%\%%~F" (
        echo ERROR: Missing package artifact: %SOURCE%\%%~F
        exit /b 4
    )
)

set "BACKUP=%TARGET%\dlssg-vulkan-backup-%RANDOM%"
mkdir "%BACKUP%" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not create backup directory. Try running this script
    echo from an Administrator command prompt if the game is under
    echo Program Files or another protected directory:
    echo        %BACKUP%
    exit /b 5
)

for %%F in (
    "dlssg_vulkan_ngx.dll"
    "dlssg_vulkan_proxy.dll"
    "dlssg_vulkan_route.dll"
) do (
    if exist "%TARGET%\%%~F" (
        copy /Y "%TARGET%\%%~F" "%BACKUP%\%%~F" >nul
        if errorlevel 1 (
            echo ERROR: Could not back up %%~F
            exit /b 6
        )
    )
    copy /Y "%SOURCE%\%%~F" "%TARGET%\%%~F" >nul
    if errorlevel 1 (
        echo ERROR: Could not install %%~F. Close the game and check that
        echo        the target directory is writable:
        echo        %TARGET%
        exit /b 7
    )
)

echo Installed Vulkan integration files into:
echo   %TARGET%
echo.
echo Backup:
echo   %BACKUP%
echo.
if /I "%TARGET_KIND%"=="executable" echo Target executable:
if /I "%TARGET_KIND%"=="executable" echo   %~f1
if /I "%TARGET_KIND%"=="executable" echo.
echo These DLLs are integration components. The existing proxy does not
echo automatically hook an application or replace version.dll/dinput8.dll.
echo A Vulkan loader (vulkan-1.dll) and a host that calls this ABI are still
echo required; this installer does not copy system or proprietary DLLs.
exit /b 0
