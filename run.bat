@echo off
cd working
set PATH=%PATH%;..\vendor\vs
..\build\Debug\zombies.exe
cd ..
