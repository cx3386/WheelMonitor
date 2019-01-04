@echo off
cd %~dp0
set tpath=..\tools\
REM for /F %%i in ('dir Release\*.msi /b') do (
for %%i in (Release\*.msi) do (
    set dst=..\install\%%~ni
    set fn=%%~nxi
)
echo ---创建安装文件夹---
if exist %dst% rmdir /s/q %dst%
mkdir %dst%
echo ----拷贝vcredist----
copy %tpath%vcredist\vc_redist.x64.exe %dst%
echo ----拷贝虚拟串口安装包----
copy %tpath%ZLVircom\ZLVircom4.96_x64.msi %dst% 
copy %tpath%ZLVircom\ZLVircom_config %dst%
echo ----拷贝解码器安装包----
copy %tpath%K-Lite\K-Lite_Codec_Pack_1455_Basic.exe %dst%
echo ----拷贝本软件安装包----
echo %fn%
copy Release\%fn% %dst%
echo ----拷贝软件说明----
copy 安装说明.txt %dst%
echo ----制作setup.exe----
call %tpath%vbs2exe\vbs2exe.exe -vbs oneclick.vbs -save setup.exe -x64 -invisible -overwrite -upx
move setup.exe %dst%
pause