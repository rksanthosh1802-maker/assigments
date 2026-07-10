@echo off
REM =============================================================================
REM script_build_and_run.bat - Configure, build, and run the Desktop target.
REM =============================================================================

IF NOT EXIST build_desktop MKDIR build_desktop

cmake -S . -B build_desktop -A x64
IF %ERRORLEVEL% NEQ 0 (
    echo CMake configure FAILED.
    pause
    exit /b 1
)

cmake --build build_desktop --config Debug
IF %ERRORLEVEL% NEQ 0 (
    echo Build FAILED.
    pause
    exit /b 1
)

echo.
echo Build succeeded. Launching...
build_desktop\Debug\AndroidProgrammingQuiz.exe
pause
