@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if %ERRORLEVEL% NEQ 0 exit /b 1
if defined VCPKG_ROOT_SAVED set "VCPKG_ROOT=%VCPKG_ROOT_SAVED%"
cmake %*
exit /b %ERRORLEVEL%
