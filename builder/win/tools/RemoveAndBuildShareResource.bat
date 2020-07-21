@echo off
setlocal EnableDelayedExpansion
cd "%~dp0"
set tools_path=%cd%
cd ../../../src
set src_path=%cd%

call python %tools_path%\..\..\create_qrc.py

cd %src_path%

set item=images qml widgets
for /d %%m in (*) do (
	cd %src_path%\%%m
	for /d %%n in (*) do (
		cd %src_path%\%%m\%%n
		rem find files which contains images or qml or widgets
		for %%i in (%item%) do (
			if exist "%%i" (
				if not exist "resource" (
					mkdir "resource"
				)
				if exist "resource\qrc_%%i.cpp" (
					del "resource\qrc_%%i.cpp"
				)
				call "%tools_path%\create_qrc.bat" "%src_path%\%%m\%%n\%%i" "%src_path%\%%m\%%n\resource\qrc_%%i.cpp" "%%n"
			)
		)
		cd ..
    )
	cd ..
)


pause