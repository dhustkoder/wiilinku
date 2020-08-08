@echo off
set BUILD_TYPE=%1
set RUN_AFTER_BUILD="%2"=="run"
if not %RUN_AFTER_BUILD% set RUN_AFTER_BUILD="%1"=="run"


set CC=cl

REM CFLAGS SETUP
set CDEFS=/D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS /DWIILINKU_HOST
set CDEFS_DEBUG=%CDEFS% /DDEBUG /DWIILINKU_DEBUG
set CDEFS_RELEASE=%CDEFS% /DNDEBUG
set INCLUDE_DIRS=/Isrc /Isrc\win32 /I"externals\ViGEmClient\include"
set CFLAGS=/W4 /wd4204 /wd4201
set CFLAGS_RELEASE=%CFLAGS% /Ox /DEBUG:NONE %CDEFS_RELEASE% %INCLUDE_DIRS% 
set CFLAGS_DEBUG=%CFLAGS% /Od /Zi /DEBUG:FULL %CDEFS_DEBUG% %INCLUDE_DIRS%  


REM LDFLAGS SETUP
set INCLUDE_LIBS_DEBUG=/LIBPATH:"externals\ViGEmClient\lib\debug\x64"
set INCLUDE_LIBS_RELEASE=/LIBPATH:"externals\ViGEmClient\lib\release\x64"
set LIBS=user32.lib gdi32.lib winmm.lib ws2_32.lib setupapi.lib ViGEmClient.lib^
         ole32.lib

set LIBS_DEBUG=%LIBS% msvcrtd.lib vcruntimed.lib
set LIBS_RELEASE=%LIBS% msvcrt.lib vcruntime.lib
set LDFLAGS=/SUBSYSTEM:WINDOWS /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCMTD
set LDFLAGS_DEBUG=%LDFLAGS% %INCLUDE_LIBS_DEBUG% %LIBS_DEBUG%
set LDFLAGS_RELEASE=%LDFLAGS% %INCLUDE_LIBS_RELEASE% %LIBS_RELEASE%


REM SOURCE SETUP
set SRC=src\\win32\*.c src\\*.c


if "%BUILD_TYPE%"=="release" goto build_release else goto build_debug

:build_debug
	set CFLAGS=%CFLAGS_DEBUG%
	set LDFLAGS=%LDFLAGS_DEBUG%
	set BUILD_DIR="win32_debug_build"
goto compile

:build_release
set CFLAGS=%CFLAGS_RELEASE%
set LDFLAGS=%LDFLAGS_RELEASE%
set BUILD_DIR="win32_release_build"
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

