@echo off
set tpath="..\tools\"
set dst="..\install\"
echo ----拷贝虚拟串口安装包----
copy %tpath%ZLVircom\ZLVircom4.96_x64.msi %dst%
echo ----拷贝解码器安装包----
copy %tpath%K-Lite\K-Lite_Codec_Pack_1455_Basic.exe %dst%
echo ----拷贝本软件安装包----
copy Release %dst%
REM pause