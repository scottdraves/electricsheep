@echo off
REM This builds all the libraries of the folder for 1 uname

call tecmake %1 %2 %3 %4 %5 %6 %7 %8

if "%1"==""         goto luaexe
if "%1"=="vc8"      goto luaexe
if "%1"=="vc8_64"   goto luaexe_64
if "%1"=="cygw15"   goto luaexe_cygw15
if "%1"=="dll8_64"  goto luadll8_64
if "%1"=="dll8"     goto luadll8
if "%1"=="all"      goto luaexe
goto end

:luaexe
call tecmake vc8 "MF=lua" %2 %3 %4 %5 %6 %7
call tecmake vc8 "MF=wlua" %2 %3 %4 %5 %6 %7
call tecmake vc8 "MF=luac" %2 %3 %4 %5 %6 %7
call tecmake vc8 "MF=bin2c" %2 %3 %4 %5 %6 %7
if "%1"=="all"  goto luaexe_64
goto end

:luaexe_64
call tecmake vc8_64 "MF=lua" %2 %3 %4 %5 %6 %7
call tecmake vc8_64 "MF=wlua" %2 %3 %4 %5 %6 %7
call tecmake vc8_64 "MF=luac" %2 %3 %4 %5 %6 %7
call tecmake vc8_64 "MF=bin2c" %2 %3 %4 %5 %6 %7
if "%1"=="all"  goto luaexe_cygw15
goto end

:luaexe_cygw15
call tecmake cygw15 "MF=lua" %2 %3 %4 %5 %6 %7
call tecmake cygw15 "MF=wlua" %2 %3 %4 %5 %6 %7
call tecmake cygw15 "MF=luac" %2 %3 %4 %5 %6 %7
call tecmake cygw15 "MF=bin2c" %2 %3 %4 %5 %6 %7
if "%1"=="all"  goto luadll8_64
goto end

:luadll8_64
copy /Y ..\lib\dll8_64\*.dll ..\bin\Win64
if "%1"=="all"  goto luadll8
goto end

:luadll8
copy /Y ..\lib\dll8\*.dll ..\bin\Win32
if "%1"=="all"  goto luadll
goto end

:luadllproxy
call tecmake dll "MF=dllproxy" %2 %3 %4 %5 %6 %7
call tecmake dll7 "MF=dllproxy" %2 %3 %4 %5 %6 %7
call tecmake dll8 "MF=dllproxy" %2 %3 %4 %5 %6 %7
call tecmake dll8_64 "MF=dllproxy" %2 %3 %4 %5 %6 %7
call tecmake dll9 "MF=dllproxy" %2 %3 %4 %5 %6 %7
goto end

:end
