@echo off
set BUILD_TYPE=%1
set RUN_AFTER_BUILD="%2"=="run"
if not %RUN_AFTER_BUILD% set RUN_AFTER_BUILD="%1"=="run"

set CC=cl


REM CHECK TARGET ARCH / WLU_VERSION_STR

pushd %tmp%

set TMP_FILE_PREFIX=test_%RANDOM%

echo #include ^<windows.h^>  > %TMP_FILE_PREFIX%.c
echo #ifndef _WIN64          >> %TMP_FILE_PREFIX%.c
echo #error "ARCH IS X86"    >> %TMP_FILE_PREFIX%.c
echo #endif                  >> %TMP_FILE_PREFIX%.c
echo int main() {}           >> %TMP_FILE_PREFIX%.c

%CC% /c %TMP_FILE_PREFIX%.c > nul 2>&1
if %errorLevel%==0 (set ARCH=x64) else (set ARCH=x86)

popd


git describe --tags --abbrev=0 > %tmp%\%TMP_FILE_PREFIX%.txt
if %errorLevel%==0 (set /P GITTAG=<%tmp%\%TMP_FILE_PREFIX%.txt) else (set GITTAG="")

git rev-list -n 1 --abbrev-commit %GITTAG% > %tmp%\%TMP_FILE_PREFIX%.txt
if %errorLevel%==0 (set /P GITTAG_HASH=<%tmp%\%TMP_FILE_PREFIX%.txt) else (set GITTAG_HASH="")

git rev-parse --short HEAD > %tmp%\%TMP_FILE_PREFIX%.txt
if %errorLevel%==0 (set /P GITHASH=<%tmp%\%TMP_FILE_PREFIX%.txt) else (set GITHASH="")

set WLU_VERSION_STR=%GITTAG%
if NOT %GITTAG_HASH%==%GITHASH% (set WLU_VERSION_STR=%WLU_VERSION_STR%-%GITHASH%)

git diff --quiet
if NOT %errorLevel%==0 (set WLU_VERSION_STR=%WLU_VERSION_STR%-dirty)

if %GITTAG%=="" (set WLU_VERSION_STR="unknown-version")

del %tmp%\%TMP_FILE_PREFIX%.*

if NOT "%BUILD_TYPE%"=="release" (set WLU_VERSION_STR=%WLU_VERSION_STR%-debug)

set WLU_VERSION_STR=\"%WLU_VERSION_STR%\"

REM CFLAGS SETUP
set CDEFS=/D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS /DWLU_HOST^
          /DWLU_VERSION_STR=%WLU_VERSION_STR%

set CDEFS_DEBUG=%CDEFS% /DDEBUG /DWLU_DEBUG 
set CDEFS_RELEASE=%CDEFS% /DNDEBUG /DWLU_RELEASE
set INCLUDE_DIRS=/Isrc /Isrc\windows /I"externals\ViGEmClient\include"
set CFLAGS=/W4 /wd4204 /wd4201
set CFLAGS_RELEASE=%CFLAGS% /Ox /DEBUG:NONE %CDEFS_RELEASE% %INCLUDE_DIRS% 
set CFLAGS_DEBUG=%CFLAGS% /Od /Zi /DEBUG:FULL %CDEFS_DEBUG% %INCLUDE_DIRS%  


REM LDFLAGS SETUP
set INCLUDE_LIBS_DEBUG=/LIBPATH:"externals\ViGEmClient\lib\debug\%ARCH%"
set INCLUDE_LIBS_RELEASE=/LIBPATH:"externals\ViGEmClient\lib\release\%ARCH%"
set LIBS=user32.lib gdi32.lib winmm.lib ws2_32.lib setupapi.lib ViGEmClient.lib^
         ole32.lib Iphlpapi.lib

set LIBS_DEBUG=%LIBS% msvcrtd.lib vcruntimed.lib
set LIBS_RELEASE=%LIBS% msvcrt.lib vcruntime.lib
set LDFLAGS=/SUBSYSTEM:WINDOWS /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCMTD
set LDFLAGS_DEBUG=%LDFLAGS% %INCLUDE_LIBS_DEBUG% %LIBS_DEBUG%
set LDFLAGS_RELEASE=%LDFLAGS% /LTCG %INCLUDE_LIBS_RELEASE% %LIBS_RELEASE%


REM SOURCE SETUP
set SRC=src\\windows\*.c src\\*.c


if "%BUILD_TYPE%"=="release" goto build_release else goto build_debug

:build_debug
	set CFLAGS=%CFLAGS_DEBUG%
	set LDFLAGS=%LDFLAGS_DEBUG%
	set BUILD_DIR="windows_%ARCH%_debug_build"
goto compile

:build_release
	set CFLAGS=%CFLAGS_RELEASE%
	set LDFLAGS=%LDFLAGS_RELEASE%
	set BUILD_DIR="windows_%ARCH%_release_build"
goto compile

:compile
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

@echo on
%CC% %CFLAGS% %SRC% /Fe%BUILD_DIR%\wiilinku.exe /link %LDFLAGS%
@echo off

set ERROR=%errorLevel%

:clean
del *.obj *.pdb

if  %ERROR%==0 ( if %RUN_AFTER_BUILD% .\%BUILD_DIR%\wiilinku.exe )

