@echo off
REM ========================================================================
REM Dungeon Reborn - Emscripten SDK Automated Setup Helper
REM ========================================================================

echo ========================================================
echo   DUNGEON REBORN - EMSCRIPTEN SDK SETUP
echo ========================================================

REM Check if emsdk directory already exists
if exist "emsdk\" (
    echo [Info] Found existing emsdk directory.
    cd emsdk
) else (
    echo [Info] Cloning official Emscripten SDK repository...
    git clone https://github.com/emscripten-core/emsdk.git
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] Git clone failed. Please check your internet connection.
        pause
        exit /b 1
    )
    cd emsdk
)

echo.
echo [Info] Installing latest Emscripten compiler toolchain...
call emsdk.bat install latest
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] emsdk install failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo [Info] Activating latest Emscripten toolchain...
call emsdk.bat activate latest
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] emsdk activate failed!
    cd ..
    pause
    exit /b 1
)

call emsdk_env.bat
cd ..

echo.
echo ========================================================
echo   EMSDK SETUP COMPLETE!
echo   You can now build for web using: build_web.bat
echo ========================================================

