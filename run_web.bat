@echo off
REM ========================================================================
REM Dungeon Reborn - Web Preview Launcher
REM Starts a local HTTP server and opens the game in your default browser.
REM ========================================================================

python "%~dp0serve_web.py"
if %ERRORLEVEL% NEQ 0 (
    pause
)

