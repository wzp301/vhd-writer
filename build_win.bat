@echo off
echo build project for windows...

if not exist .\build_win64 mkdir .\build_win64
cd .\build_win64
cmake -A x64 %~dp0
cd ..