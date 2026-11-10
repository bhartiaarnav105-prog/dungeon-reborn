@echo off
REM ========================================================================
REM Dungeon Reborn - WebAssembly Build Script (Emscripten)
REM ========================================================================

echo ========================================================
echo   DUNGEON REBORN - COMPILING TO WEBASSEMBLY (WASM)
echo ========================================================

REM 1. Activate Emscripten compiler environment if emsdk is present
if exist "%~dp0emsdk\emsdk_env.bat" (
    call "%~dp0emsdk\emsdk_env.bat" >nul 2>&1
)

where emcmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Emscripten (emcmake / emcc) was not found in PATH!
    echo.
    echo Please run setup_emsdk.bat first to install the Emscripten SDK.
    pause
    exit /b 1
)

REM 2. Configure build directory
if not exist "build-web" mkdir build-web
cd build-web

echo [1/2] Configuring CMake with Emscripten toolchain...
call emcmake cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo [CMake Error] Configuration failed. Trying with MinGW Makefiles...
    call emcmake cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    if %ERRORLEVEL% NEQ 0 (
        echo [FATAL ERROR] Emscripten CMake configuration failed.
        cd ..
        pause
        exit /b 1
    )
)

echo.
echo [2/2] Building DungeonReborn WebAssembly bundle...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [FATAL ERROR] WebAssembly compilation failed!
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================================
echo   WEBASSEMBLY BUILD SUCCEEDED!
echo   Output generated in build_web/:
echo     - DungeonReborn.html
echo     - DungeonReborn.js
echo     - DungeonReborn.wasm
echo     - DungeonReborn.data (bundled assets)
echo ========================================================
echo.
echo To launch and test in your browser, run:
echo   run_web.bat
echo ========================================================

