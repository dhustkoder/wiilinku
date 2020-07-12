@echo off
set BUILD_TYPE=%1
set RUN_AFTER_BUILD="%2"=="run"

set INCLUDE_DIRS=/Isrc
set SRC=src\\*.c
set LIBS=user32.lib gdi32.lib winmm.lib ws2_32.lib


set CC=cl
set CFLAGS=/D_CRT_SECURE_NO_WARNINGS /wd4028 /wd4214 /wd4047 /wd4210 /W4  %INCLUDE_DIRS%
set LDFLAGS=/SUBSYSTEM:CONSOLE %LIBS%

set CFLAGS_DEBUG=/Od /Zi /DEBUG:FULL /DDEBUG
set LDFLAGS_DEBUG=

set CFLAGS_RELEASE=/Ox /DEBUG:NONE /DNDEBUG
set LDFLAGS_RELEASE=

if "%BUILD_TYPE%"=="release" goto build_release else goto build_debug


:build_debug
set CFLAGS=%CFLAGS% %CFLAGS_DEBUG%
set LDFLAGS=%LDFLAGS% %LDFLAGS_DEBUG%
goto compile

:build_release
set CFLAGS=%CFLAGS% %CFLAGS_RELEASE%
set LDFLAGS=%LDFLAGS% %LDFLAGS_RELEASE%
goto compile

:compile
if not exist "build" mkdir build

@echo on
%CC% %CFLAGS% %SRC% %LIBS% /Febuild\wiiupcx.exe /link %LDFLAGS%
@echo off

set ERROR=%errorLevel%

:clean
del *.obj *.pdb

if  %ERROR%==0 ( if %RUN_AFTER_BUILD% .\build\wiiupcx.exe )

