@echo off
cd %~dp0
set tpath=..\tools\
REM for /F %%i in ('dir Release\*.msi /b') do (
for %%i in (Release\*.msi) do (
    set dst=..\install\%%~ni
    set fn=%%~nxi
)
echo ---������װ�ļ���---
if exist %dst% rmdir /s/q %dst%
mkdir %dst%
echo ----����vcredist----
copy %tpath%vcredist\vc_redist.x64.exe %dst%
echo ----�������⴮�ڰ�װ��----
copy %tpath%ZLVircom\ZLVircom4.96_x64.msi %dst% 
copy %tpath%ZLVircom\ZLVircom_config %dst%
echo ----������������װ��----
copy %tpath%K-Lite\K-Lite_Codec_Pack_1455_Basic.exe %dst%
echo ----�����������װ��----
echo %fn%
copy Release\%fn% %dst%
echo ----�������˵��----
copy ��װ˵��.txt %dst%
echo ----����setup.exe----
call %tpath%vbs2exe\vbs2exe.exe -vbs oneclick.vbs -save setup.exe -x64 -invisible -overwrite -upx
move setup.exe %dst%
pause