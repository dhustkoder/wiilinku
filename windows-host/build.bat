@echo off
set BUILD_TYPE=%1
set RUN_AFTER_BUILD="%2"=="run"

set INCLUDE_DIRS=/Isrc^
                 /I"C:\devkitPro\projects\wiiupcx\windows-host\externals\ViGEmClient\include"^
                 /I"C:\devkitPro\projects\wiiupcx\windows-host\externals\x264-master"

set INCLUDE_LIBS=/LIBPATH:"C:\devkitPro\projects\wiiupcx\windows-host\externals\ViGEmClient\lib\release\x64"
set SRC=src\\*.c src\\*.cpp
set LIBS=user32.lib gdi32.lib winmm.lib ws2_32.lib^
        setupapi.lib ViGEmClient.lib mfreadwrite.lib^
        mfplat.lib mfuuid.lib ole32.lib

set CC=cl
set CFLAGS=/D_CRT_SECURE_NO_WARNINGS /wd4028 /wd4214 /wd4047 /wd4210 /W4  %INCLUDE_DIRS%
set LDFLAGS=%INCLUDE_LIBS% %LIBS% /SUBSYSTEM:WINDOWS

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
%CC% %CFLAGS% %SRC% /Febuild\wiiupcx.exe /link %LDFLAGS%
@echo off

set ERROR=%errorLevel%

:clean
del *.obj *.pdb

if  %ERROR%==0 ( if %RUN_AFTER_BUILD% .\build\wiiupcx.exe )

