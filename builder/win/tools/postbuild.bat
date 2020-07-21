@echo off
set MYDIR=%~p0
set PROJECTNAME=%~1
set PROJECTDIR=%~2
set INTDIR=%~3
set OUTDIR=%~4
@set DESTDIR=%~5

set QMDIR="%MYDIR%..\..\..\native"
echo PROJECTNAME=%PROJECTNAME% > tmp.txt
echo PROJECTDIR=%PROJECTDIR% >> tmp.txt

if exist "%PROJECTDIR%\deploy" (
"%MYDIR%\grep.exe" -q DigitizerGeneric tmp.txt
if not ERRORLEVEL 1 (
mkdir %OUTDIR%\plugins\
xcopy /S /D /Y "%PROJECTDIR%\deploy"\*.* "%OUTDIR%\plugins"
)else (
xcopy /S /D /Y "%PROJECTDIR%\deploy"\*.* "%OUTDIR%"
)
if not exist "%OUTDIR%\languages" (
mkdir %OUTDIR%\languages
xcopy /S /D /Y "%QMDIR%"\qm\*.* "%OUTDIR%\languages"
xcopy /S /D /Y "%QMDIR%"\qt_qm\*.* "%OUTDIR%\languages"
)
)

"%MYDIR%\grep.exe" -q PadPlotter tmp.txt
if not ERRORLEVEL 1 (
mkdir %DESTDIR%\plugins\
mkdir %DESTDIR%\plugins\plot\
xcopy /S /D /Y "%OUTDIR%"\*.* "%DESTDIR%\plugins\plot\"
)

del tmp.txt

:Done

exit /b 0
