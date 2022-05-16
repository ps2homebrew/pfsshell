@echo off
cd /d %~dp0"
for /f %%a in ('dir /b /A:D') do (
echo BUILD "%%a":
cd %%a
md build
cd build
del /Q *.*
cmake .. "-GMinGW Makefiles"
cmake --build .
cd ..
cd ..
)
pause