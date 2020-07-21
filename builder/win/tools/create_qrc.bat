@echo off
set MYDIR=%~p0
set RESDIR=%~1
set RESDIRPATH=%~dp1
set RESDIRNAME=%~nx1
set DEST=%~2
set DESTDIR=%~dp2
set DESTNAME=%~nx2
set PRJNAME=%~3

setlocal disabledelayedexpansion

set QRCFILE=%RESDIRPATH%\%PRJNAME%.qrc

echo Updating %DESTNAME%

"%QTDIR%\bin\rcc.exe" -name "%RESDIRNAME%" "%QRCFILE%" -o "%DEST%"

exit /b 0
