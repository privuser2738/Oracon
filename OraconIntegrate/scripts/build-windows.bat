@echo off
REM Build script for Windows

setlocal EnableDelayedExpansion

echo =====================================
echo   Building Oracon Integrate for Windows
echo =====================================
echo.

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_ROOT%\build-windows
set INSTALL_DIR=%PROJECT_ROOT%\dist\windows

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo Build configuration:
echo   Build type: %BUILD_TYPE%
echo   Build directory: %BUILD_DIR%
echo   Install directory: %INSTALL_DIR%
echo.

REM Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake is not installed or not in PATH
    exit /b 1
)
echo [OK] CMake found

REM Check for Visual Studio
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: Visual Studio compiler not found
    echo Please run this script from Visual Studio Developer Command Prompt
    exit /b 1
)
echo [OK] Visual Studio compiler found

REM Check for Qt6 (optional)
where qmake6 >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [OK] Qt6 found - GUI will be built
) else (
    echo [WARN] Qt6 not found - GUI will not be built
)

echo.

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM Configure
echo Configuring...
cmake "%PROJECT_ROOT%" ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DBUILD_TESTS=ON ^
    -DBUILD_EXAMPLES=ON

if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake configuration failed
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config %BUILD_TYPE% -j

if %ERRORLEVEL% NEQ 0 (
    echo Error: Build failed
    exit /b 1
)

REM Install
echo.
echo Installing to %INSTALL_DIR%...
cmake --install . --config %BUILD_TYPE%

if %ERRORLEVEL% NEQ 0 (
    echo Error: Installation failed
    exit /b 1
)

REM Create ZIP package
echo.
echo Creating distribution package...
cd "%PROJECT_ROOT%\dist"
set ZIP_NAME=oracon-integrate-windows-x64.zip
powershell Compress-Archive -Path windows\* -DestinationPath %ZIP_NAME% -Force

echo.
echo ========================================
echo   Build completed successfully!
echo ========================================
echo.
echo Binaries location: %INSTALL_DIR%\bin
echo Package: %PROJECT_ROOT%\dist\%ZIP_NAME%
echo.
echo To run:
echo   %INSTALL_DIR%\bin\oracon-integrate.exe --help
echo.

endlocal
