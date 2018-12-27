@echo off
set tpath="..\tools\"
set dst="..\install\"
echo ----拷贝虚拟串口安装包----
copy %tpath%ZLVircom\ZLVircom4.96_x64.msi %dst% 
copy %tpath%ZLVircom\ZLVircom_config %dst%
echo ----拷贝解码器安装包----
copy %tpath%K-Lite\K-Lite_Codec_Pack_1455_Basic.exe %dst%
echo ----拷贝本软件安装包----
for /F %%i in ('dir Release\*.msi /b') do (set fn=%%i)
echo %fn%
copy Release\%fn% %dst%
echo ----制作setup.exe----
%tpath%vbs2exe\vbs2exe.exe -vbs oneclick.vbs -save setup.exe -x64 -invisible -overwrite -upx
mv setup.exe ..\install\
REM pause