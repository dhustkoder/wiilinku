@echo off
set BUILD_TYPE=%1
set RUN_AFTER_BUILD="%2"=="run"

set BUILD_DIR="win32_build"

set INCLUDE_DIRS=/Isrc /Isrc\win32 /I"externals\ViGEmClient\include"

set INCLUDE_LIBS_DEBUG=/LIBPATH:"externals\ViGEmClient\lib\debug\x64"
set INCLUDE_LIBS_RELEASE=/LIBPATH:"externals\ViGEmClient\lib\release\x64"
set SRC=src\\win32\*.c src\\*.c

set LIBS=user32.lib gdi32.lib winmm.lib ws2_32.lib^
        setupapi.lib ViGEmClient.lib mfreadwrite.lib^
        mfplat.lib mfuuid.lib ole32.lib

set LIBS_DEBUG=msvcrtd.lib vcruntimed.lib

set CC=cl
set CDEFS=/D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS /DWIIUPCX_HOST
set CFLAGS_DEFAULT=%CDEFS% /wd4028 /wd4214 /wd4047 /wd4210 /wd4204 /W4  %INCLUDE_DIRS%

set CFLAGS_DEBUG=/Od /Zi /DEBUG:FULL /DDEBUG
set LDFLAGS_DEBUG=%INCLUDE_LIBS_DEBUG% %LIBS% %LIBS_DEBUG% /SUBSYSTEM:WINDOWS

set CFLAGS_RELEASE=/Ox /DEBUG:NONE /DNDEBUG
set LDFLAGS_RELEASE=%INCLUDE_LIBS_RELEASE% %LIBS% /SUBSYSTEM:WINDOWS

if "%BUILD_TYPE%"=="release" goto build_release else goto build_debug


:build_debug
set CFLAGS=%CFLAGS_DEFAULT% %CFLAGS_DEBUG%
set LDFLAGS=%LDFLAGS_DEBUG%
goto compile

:build_release
set CFLAGS=%CFLAGS_DEFAULT% %CFLAGS_RELEASE%
set LDFLAGS=%LDFLAGS_RELEASE%
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

