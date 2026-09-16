@echo off
setlocal EnableExtensions

rem Installs the source-built Vulkan integration beside a caller-provided game
rem executable. This does not replace version.dll or dinput8.dll.

if "%~1"=="" (
    echo Usage: %~nx0 "C:\Path\To\Game\bin"
    echo.
    echo The target must be the directory containing the game's rendering EXE.
    exit /b 2
)

set "TARGET=%~f1"
set "ROOT=%~dp0"
set "SOURCE=%ROOT%vulkan"

if not exist "%TARGET%\." (
    echo ERROR: Target directory does not exist:
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
    echo ERROR: Could not create backup directory:
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
        echo ERROR: Could not install %%~F
        exit /b 7
    )
)

echo Installed Vulkan integration files into:
echo   %TARGET%
echo.
echo Backup:
echo   %BACKUP%
echo.
echo These DLLs are integration components. The existing proxy does not
echo automatically hook an application or replace version.dll/dinput8.dll.
exit /b 0
