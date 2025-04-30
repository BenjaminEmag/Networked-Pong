@echo off
start cmd /k "cd /d out/build/x64-Debug && network_pong.exe server"
timeout /t 1 >nul
start cmd /k "cd /d out/build/x64-Debug && network_pong.exe client"