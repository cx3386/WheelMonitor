@echo off
set tpath="..\tools\"
set dst="..\install\"
echo ----�������⴮�ڰ�װ��----
copy %tpath%ZLVircom\ZLVircom4.96_x64.msi %dst% 
copy %tpath%ZLVircom\ZLVircom_config %dst%
echo ----������������װ��----
copy %tpath%K-Lite\K-Lite_Codec_Pack_1455_Basic.exe %dst%
echo ----�����������װ��----
for /F %%i in ('dir Release\*.msi /b') do (set fn=%%i)
echo %fn%
copy Release\%fn% %dst%
echo ----����setup.exe----
%tpath%vbs2exe\vbs2exe.exe -vbs oneclick.vbs -save setup.exe -x64 -invisible -overwrite -upx
mv setup.exe ..\install\
REM pause