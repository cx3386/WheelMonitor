@echo off
set exe_path=..\x64\Release\
set exe_fn=WheelMonitor.exe
set dep_path=..\dependencies\
set qt_path=%dep_path%qt\
mkdir %qt_path% %dep_path%config\Log %dep_path%config\Capture
copy %exe_path%%exe_fn% %qt_path%
cd %qt_path%
call windeployqt %exe_fn%
del %exe_fn%