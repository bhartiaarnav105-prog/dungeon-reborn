@echo off
REM Launch DungeonReborn.exe with build\ as the working directory
REM so that relative asset paths like "assets/images/..." resolve correctly.
cd /d "%~dp0build"
echo [Launcher] Working directory: %CD%
echo [Launcher] Starting DungeonReborn.exe ...
start "" ".\DungeonReborn.exe"
