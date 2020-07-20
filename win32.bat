@echo off
set BUILD_TYPE=%1
set RUN_AFTER_BUILD="%2"=="run"

set BUILD_DIR="win32_build"

set INCLUDE_DIRS=/Isrc /Isrc\win32^
                 /I"externals\ViGEmClient\include"

set INCLUDE_LIBS=/LIBPATH:"externals\ViGEmClient\lib\release\x64"
set SRC=src\\win32\*.c
set LIBS=user32.lib gdi32.lib winmm.lib ws2_32.lib^
        setupapi.lib ViGEmClient.lib mfreadwrite.lib^
        mfplat.lib mfuuid.lib ole32.lib

set CC=cl
set CDEFS=/D_WINSOCKAPI_ /D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS /DWIIUPCX_HOST
set CFLAGS=%CDEFS% /wd4028 /wd4214 /wd4047 /wd4210 /W4  %INCLUDE_DIRS%
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
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

@echo on
%CC% %CFLAGS% %SRC% /Fe%BUILD_DIR%\wiiupcx.exe /link %LDFLAGS%
@echo off

set ERROR=%errorLevel%

:clean
del *.obj *.pdb

if  %ERROR%==0 ( if %RUN_AFTER_BUILD% .\build\wiiupcx.exe )

