@echo on
REM cd %~dp0
REM echo 
set exe_path=..\x64\Release\
set exe_fn=WheelMonitor.exe
set qt_path=..\dependencies\qt\
if exist %qt_path% (
    rmdir /s/q %qt_path%
    REM del /Q %qt_path%
    mkdir %qt_path%
) else (
    mkdir %qt_path%
)
echo %qt_path%
copy %exe_path%%exe_fn% %qt_path%
cd %qt_path%
call windeployqt %exe_fn%
del %exe_fn%
pause