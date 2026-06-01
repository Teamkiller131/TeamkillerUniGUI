@echo off
setlocal EnableDelayedExpansion
rem ============================================================================
rem  cmake-msvc.cmd — run CMake inside a correctly initialized MSVC environment.
rem
rem  Why this wrapper exists:
rem    * vcvars64.bat puts the *current* MSVC toolset (cl/link/lib) at the front
rem      of PATH. Without it, a stale MSVC version left on PATH can be picked up
rem      and CMake caches a compiler path that later disappears on an update.
rem    * It locates Visual Studio dynamically via vswhere, so it keeps working
rem      across editions (Community/Professional/Enterprise/BuildTools) and
rem      version bumps — no hard-coded "18\Community" path.
rem
rem  Usage (identical to calling cmake directly):
rem    cmake-msvc.cmd --preset windows-msvc-debug
rem    cmake-msvc.cmd --build  build/windows-msvc-debug
rem    cmake-msvc.cmd --build  --preset windows-msvc-release
rem ============================================================================

rem --- 1. Locate Visual Studio via vswhere ------------------------------------
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

set "VSINSTALL="
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set "VSINSTALL=%%i"
)

rem Fallback: allow an explicit override via VS_INSTALL_DIR env var.
if not defined VSINSTALL if defined VS_INSTALL_DIR set "VSINSTALL=%VS_INSTALL_DIR%"

if not defined VSINSTALL (
    echo [cmake-msvc] ERROR: Could not locate a Visual Studio installation with the
    echo              C++ desktop workload ^(VC.Tools.x86.x64^).
    echo.
    echo   Fixes:
    echo     * Install "Desktop development with C++" via the Visual Studio Installer.
    echo     * Or set VS_INSTALL_DIR to your VS install root, e.g.:
    echo         set "VS_INSTALL_DIR=C:\Program Files\Microsoft Visual Studio\18\Community"
    exit /b 1
)

set "VCVARS=%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
    echo [cmake-msvc] ERROR: Found Visual Studio at:
    echo                "%VSINSTALL%"
    echo              but vcvars64.bat is missing at:
    echo                "%VCVARS%"
    echo              The C++ build tools may be only partially installed.
    exit /b 1
)

rem --- 2. Initialize the MSVC x64 environment ---------------------------------
call "%VCVARS%" >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [cmake-msvc] ERROR: vcvars64.bat failed to initialize the MSVC environment.
    exit /b 1
)

rem Preserve a user-provided VCPKG_ROOT if they exported VCPKG_ROOT_SAVED before
rem invoking the wrapper (vcvars may otherwise point it at the VS-bundled vcpkg).
if defined VCPKG_ROOT_SAVED set "VCPKG_ROOT=%VCPKG_ROOT_SAVED%"

rem --- 3. Sanity-check that the core tools are now resolvable ------------------
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [cmake-msvc] ERROR: 'cl.exe' not found after vcvars init. The MSVC toolset
    echo              appears to be missing. Re-run the Visual Studio Installer and
    echo              ensure "MSVC v143+ x64/x86 build tools" is checked.
    exit /b 1
)
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [cmake-msvc] ERROR: 'cmake.exe' not found on PATH. Install CMake 3.31+ or add
    echo              it to PATH ^(e.g. via the Visual Studio Installer component^).
    exit /b 1
)

rem --- 4. Run CMake with all forwarded arguments ------------------------------
cmake %*
exit /b %ERRORLEVEL%
