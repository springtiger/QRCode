@echo off
set MYDIR=%~p0
set SRCDIR=%~1
set DESTDIR=%~2

set QT_UIC=%QTDIR%/bin/uic

for /r "%SRCDIR%" %%F in (*.ui) do (
	"%MYDIR%\test.exe" "%DESTDIR%\ui_%%~nF.h" -nt "%%~F"
	if ERRORLEVEL 1 (
		echo UICing %%~nxF
		"%QT_UIC%" -o "%DESTDIR%\ui_%%~nF.h" "%%~F"
	)
)

exit /b 0
