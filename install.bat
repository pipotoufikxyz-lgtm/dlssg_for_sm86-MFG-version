@echo off
setlocal EnableExtensions

rem Installs the source-built Vulkan integration beside a caller-provided game
rem executable or directory. This does not replace version.dll or dinput8.dll.
set "INTERACTIVE=0"
set "ARG=%~1"

if "%ARG%"=="" (
    set "INTERACTIVE=1"
    echo Enter the game directory or rendering executable path.
    set /p "ARG=Game path: "
)

if "%ARG%"=="" (
    echo ERROR: No game path was entered.
    goto :finish_error
)

for %%P in ("%ARG%") do set "INPUT=%%~fP"
if "%INPUT%"=="" (
    echo ERROR: Could not resolve the supplied path.
    goto :finish_error
)

set "TARGET=%INPUT%"
set "ROOT=%~dp0"
set "SOURCE=%ROOT%vulkan"

if exist "%TARGET%\*" (
    set "TARGET_KIND=directory"
) else if exist "%TARGET%" (
    for %%P in ("%INPUT%") do set "TARGET=%%~dpP"
    set "TARGET_KIND=executable"
) else (
    echo ERROR: Target path does not exist:
    echo        %INPUT%
    goto :finish_error
)

if not exist "%TARGET%\." (
    echo ERROR: Could not resolve a target directory:
    echo        %TARGET%
    goto :finish_error
)

if exist "%SOURCE%\dlssg_vulkan_layer.dll" if not exist "%SOURCE%\dlssg_vulkan_layer.json" (
    echo ERROR: Found the Vulkan layer DLL but its manifest is missing:
    echo        %SOURCE%\dlssg_vulkan_layer.json
    goto :finish_error
)

for %%F in (
    "dlssg_vulkan_ngx.dll"
    "dlssg_vulkan_proxy.dll"
    "dlssg_vulkan_route.dll"
) do (
    if not exist "%SOURCE%\%%~F" (
        echo ERROR: Missing package artifact: %SOURCE%\%%~F
        goto :finish_error
    )

)

if exist "%SOURCE%\dlssg_vulkan_layer.dll" (
    copy /Y "%SOURCE%\dlssg_vulkan_layer.dll" "%TARGET%\dlssg_vulkan_layer.dll" >nul
    copy /Y "%SOURCE%\dlssg_vulkan_layer.json" "%TARGET%\dlssg_vulkan_layer.json" >nul
    reg add "HKCU\Software\Khronos\Vulkan\ImplicitLayers" /v "%TARGET%\dlssg_vulkan_layer.json" /t REG_DWORD /d 0 /f >nul
    if errorlevel 1 (
        echo ERROR: Could not register the Vulkan implicit-layer manifest.
        echo        The DLLs were copied, but the layer is not enabled.
        goto :finish_error
    )
    echo Installed Vulkan loader-layer DLL and manifest.
    echo Registered the layer for the current Windows user.
)

set "BACKUP=%TARGET%\dlssg-vulkan-backup-%RANDOM%"
mkdir "%BACKUP%" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not create backup directory. Try running this script
    echo from an Administrator command prompt if the game is under
    echo Program Files or another protected directory:
    echo        %BACKUP%
    goto :finish_error
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
            goto :finish_error
        )
    )
    copy /Y "%SOURCE%\%%~F" "%TARGET%\%%~F" >nul
    if errorlevel 1 (
        echo ERROR: Could not install %%~F. Close the game and check that
        echo        the target directory is writable:
        echo        %TARGET%
        goto :finish_error
    )
)

echo Installed Vulkan integration files into:
echo   %TARGET%
echo.
echo Backup:
echo   %BACKUP%
echo.
if /I "%TARGET_KIND%"=="executable" echo Target executable:
if /I "%TARGET_KIND%"=="executable" echo   %INPUT%
if /I "%TARGET_KIND%"=="executable" echo.
echo These DLLs are integration components. The existing proxy does not
echo automatically hook an application or replace version.dll/dinput8.dll.
echo A Vulkan loader (vulkan-1.dll) and a host that calls this ABI are still
echo required; this installer does not copy system or proprietary DLLs.
echo To remove the current-user layer registration, run:
echo   reg delete "HKCU\Software\Khronos\Vulkan\ImplicitLayers" /v "%TARGET%\dlssg_vulkan_layer.json" /f
goto :finish_success

:finish_error
if "%INTERACTIVE%"=="1" (
    echo.
    pause
)
exit /b 1

:finish_success
if "%INTERACTIVE%"=="1" (
    echo.
    pause
)
exit /b 0
