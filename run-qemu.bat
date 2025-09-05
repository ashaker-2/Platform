@echo off
echo Starting ESP32 QEMU Simulation...
echo Press Ctrl+A then X to exit QEMU

set QEMU_PATH=%IDF_TOOLS_PATH%\tools\qemu-esp32\*\bin
set PROJECT_BIN=build\EspBranch.bin

if exist "%PROJECT_BIN%" (
    echo Found binary: %PROJECT_BIN%
    "%QEMU_PATH%\qemu-system-xtensa" -nographic -machine esp32 ^
        -drive file=%PROJECT_BIN%,if=mtd,format=raw ^
        -serial stdio
) else (
    echo Binary not found! Please build your project first.
    echo Expected: %PROJECT_BIN%
)

pause