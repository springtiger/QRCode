@echo off
set MYDIR=%~p0
set PROJECTNAME=%~1
set PROJECTDIR=%~2
set INTDIR=%~3

set CONFIGURATION=Debug

if not "%INTDIR:Release=%"=="%INTDIR%" (
	set CONFIGURATION=Release
)

echo %MYDIR%
echo %PROJECTNAME%
echo %PROJECTDIR%
echo %INTDIR%

::::::::::::::::::::::::::::::::::::::::::::::
:: build images resource
if not exist "%PROJECTDIR%\resource" md "%PROJECTDIR%\resource"
if exist "%PROJECTDIR%\images" call "%MYDIR%\create_qrc.bat" "%PROJECTDIR%\images" "%PROJECTDIR%\resource\qrc_images.cpp" "%PROJECTNAME%"
if exist "%PROJECTDIR%\qml" call "%MYDIR%\create_qrc.bat" "%PROJECTDIR%\qml" "%PROJECTDIR%\resource\qrc_qml.cpp" "%PROJECTNAME%"
if exist "%PROJECTDIR%\widgets" call "%MYDIR%\create_qrc.bat" "%PROJECTDIR%\widgets" "%PROJECTDIR%\resource\qrc_widgets.cpp" "%PROJECTNAME%"
::::::::::::::::::::::::::::::::::::::::::::::

::::::::::::::::::::::::::::::::::::::::::::::
:: build meta objects and dialogs
if not exist "%INTDIR%\moc" md "%INTDIR%\moc"
if exist %PROJECTDIR%\src (
	call "%MYDIR%\moc_files.bat" %PROJECTDIR%\src "%INTDIR%\moc"
)

if not exist "%INTDIR%\uic" md "%INTDIR%\uic" 
if exist "%PROJECTDIR%\dialogs" call "%MYDIR%\uic_files.bat" "%PROJECTDIR%\dialogs" "%INTDIR%\uic"
::::::::::::::::::::::::::::::::::::::::::::::

exit /b 0
