@echo off
set MYDIR=%~p0
set SRCDIR=%~1
set DESTDIR=%~2

set QT_FLAGS=-DUNICODE -DWIN32 -DQT_CORE_LIB -DQT_GUI_LIB -DQT_THREAD_SUPPORT
set QT_INCLUDES=-I"%QTDIR%\include\QtCore" -I"%QTDIR%\include\QtGui" -I"%QTDIR%\include\QtXml" -I"%QTDIR%\include" -I"%MYDIR%..\Common\Include"
set QT_MOC=%QTDIR%/bin/moc

for /r "%SRCDIR%" %%F in (*.h) do (
	"%MYDIR%\grep.exe" -q Q_OBJECT "%%~F"
	if not ERRORLEVEL 1 (
		"%MYDIR%\test.exe" "%DESTDIR%\moc_%%~nF.cpp" -nt "%%~F"
		if ERRORLEVEL 1 (
			echo "MOCing "%QT_MOC%" %QT_FLAGS% %QT_INCLUDES% "%%~F" -o "%DESTDIR%\moc_%%~nF.cpp" "
			"%QT_MOC%" %QT_FLAGS% %QT_INCLUDES% "%%~F" -o "%DESTDIR%\moc_%%~nF.cpp"
		)
	)
)

exit /b 0
