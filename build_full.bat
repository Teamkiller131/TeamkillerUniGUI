@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d D:\TeamkillerUniGUI\build\windows-msvc-debug
cmake --build . -j1
